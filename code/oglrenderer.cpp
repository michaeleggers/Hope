#include <string.h>

#include <windows.h>
#include <wingdi.h>
#include <dwmapi.h>

#include <stdio.h>
#include <stdlib.h>

#include <gl/gl.h>
#include <gl/glu.h>

#include "openglext/glext.h"
#include "openglext/wglext.h"
#include "Mathx.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define global_var static;

global_var PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
global_var PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

global_var PFNGLGENBUFFERSPROC              glGenBuffers;
global_var PFNGLBINDBUFFERPROC              glBindBuffer;
global_var PFNGLBUFFERDATAPROC              glBufferData;
global_var PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
global_var PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
global_var PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
global_var PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
global_var PFNGLCREATESHADERPROC            glCreateShader;
global_var PFNGLSHADERSOURCEPROC            glShaderSource;
global_var PFNGLCOMPILESHADERPROC           glCompileShader;
global_var PFNGLCREATESHADERPROC            glCreateShaderProc;
global_var PFNGLCREATEPROGRAMPROC           glCreateProgram;
global_var PFNGLATTACHSHADERPROC            glAttachShader;
global_var PFNGLLINKPROGRAMPROC             glLinkProgram;
global_var PFNGLUSEPROGRAMPROC              glUseProgram;
global_var PFNGLBINDATTRIBLOCATIONPROC      glBindAttribLocation;
global_var PFNGLACTIVETEXTUREPROC           glActiveTexture;
global_var PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
global_var PFNGLUNIFORM1IPROC               glUniform1i;
global_var PFNGLGETPROGRAMIVPROC            glGetProgramiv;
global_var PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
global_var PFNGLDETACHSHADERPROC            glDetachShader;
global_var PFNGLISPROGRAMPROC               glIsProgram;
global_var PFNGLGETSHADERIVPROC             glGetShaderiv;
global_var PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;

global_var HGLRC global_oglRenderContext;
global_var HWND  global_windowHandle;
global_var HDC   global_deviceContext;
global_var bool running = true;

// TODO(Michael): id for ortho matrix uniform global for now.
global_var GLuint ortho_loc;

char* load_text(char const * filename)
{
    FILE* f = fopen(filename, "r");
    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char* buffer =  (char *)VirtualAlloc(0,
                                         size,
                                         MEM_RESERVE | MEM_COMMIT,
                                         PAGE_READWRITE);
    fread(buffer, sizeof(char), size, f);
    
    return buffer;
}

struct meg_strbuf
{
    char * buffer;
    int pos;
};

meg_strbuf meg_strbuf_create()
{
    meg_strbuf strbuf;
    strbuf.pos = 0;
    strbuf.buffer = (char *)VirtualAlloc(0,
                                         256,
                                         MEM_RESERVE | MEM_COMMIT,
                                         PAGE_READWRITE);
    return strbuf;
}

// TODO(Michael): check if buffer overflow
int meg_strbuf_write(meg_strbuf* strbuf, char const * s)
{
    int s_length = strlen(s);
    strcpy(strbuf->buffer + strbuf->pos, s);
    strbuf->pos += s_length;
    
    return strbuf->pos - s_length;
}

LRESULT CALLBACK WindowProcCallback(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (uMsg)
    {
        case WM_SIZE:
        {
            RECT rect;
            GetClientRect(windowHandle, &rect);
            glViewport(0, 0, rect.right , rect.bottom);
            
            // recompute orthographic projection matrix
            float aspectRatio = (float)rect.right / (float)rect.bottom;
            float orthoMatrix[16] = { };
            ortho(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f, orthoMatrix);
            glUniformMatrix4fv(ortho_loc, 1, GL_FALSE, orthoMatrix);
        }
        break;
        
        case WM_CLOSE:
        {
            running = false;
            PostQuitMessage(0);
        }
        break;
        
        default:
        {
            result = DefWindowProc(windowHandle, uMsg, wParam, lParam);
        }
    }
    return result;
}

int initGL(HWND* windowHandle, WNDCLASS* windowClass)
{
    PIXELFORMATDESCRIPTOR pixelFormatDescriptor =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    global_deviceContext = GetDC(*windowHandle);
    // try to find best matching pixel format.
    int pixelFormatNr = ChoosePixelFormat(global_deviceContext, &pixelFormatDescriptor);
    if (!pixelFormatNr) // was null
    {
        OutputDebugStringA("could not get pxl format nr\n");
        return 1;
    }
    
    DescribePixelFormat(global_deviceContext, 
                        pixelFormatNr,
                        sizeof(pixelFormatDescriptor), 
                        &pixelFormatDescriptor);
    
    if (!SetPixelFormat(global_deviceContext, pixelFormatNr, &pixelFormatDescriptor))
    {
        return 1;
    }
    
    global_oglRenderContext = wglCreateContext(global_deviceContext);
    if (!global_oglRenderContext)
    {
        return 1;
    }
    
    // load extensions
    if (wglMakeCurrent(global_deviceContext, global_oglRenderContext))
    {
        OutputDebugStringA("ogl rendering context made current\n");
        // TODO(Michael): query supported extensions
        
        // LOAD OGL EXTENSIONS HERE (according to ogl docu, msdn says sth different)
        // TODO(Michael): do some macros like Casey?
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
            wglGetProcAddress("wglChoosePixelFormatARB");
        if (!wglChoosePixelFormatARB) return 0;
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
            wglGetProcAddress("wglCreateContextAttribsARB");
        if (!wglCreateContextAttribsARB) return 0;
        glGenBuffers = (PFNGLGENBUFFERSPROC)
            wglGetProcAddress("glGenBuffers");
        glBindBuffer = (PFNGLBINDBUFFERPROC)
            wglGetProcAddress("glBindBuffer");
        glBufferData = (PFNGLBUFFERDATAPROC)
            wglGetProcAddress("glBufferData");
        glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)
            wglGetProcAddress("glGenVertexArrays");
        glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)
            wglGetProcAddress("glBindVertexArray");
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)
            wglGetProcAddress("glEnableVertexAttribArray");
        glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)
            wglGetProcAddress("glVertexAttribPointer");
        glCreateShader = (PFNGLCREATESHADERPROC)
            wglGetProcAddress("glCreateShader");
        glShaderSource = (PFNGLSHADERSOURCEPROC)
            wglGetProcAddress("glShaderSource");
        glCompileShader = (PFNGLCOMPILESHADERPROC)
            wglGetProcAddress("glCompileShader");
        glCreateShaderProc = (PFNGLCREATESHADERPROC)
            wglGetProcAddress("glCreateShaderProc");
        glCreateProgram = (PFNGLCREATEPROGRAMPROC)
            wglGetProcAddress("glCreateProgram");
        glAttachShader = (PFNGLATTACHSHADERPROC)
            wglGetProcAddress("glAttachShader");
        glLinkProgram = (PFNGLLINKPROGRAMPROC)
            wglGetProcAddress("glLinkProgram");
        glUseProgram = (PFNGLUSEPROGRAMPROC)
            wglGetProcAddress("glUseProgram");
        glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)
            wglGetProcAddress("glBindAttribLocation");
        glActiveTexture = (PFNGLACTIVETEXTUREPROC)
            wglGetProcAddress("glActiveTexture");
        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)
            wglGetProcAddress("glGetUniformLocation");
        glUniform1i = (PFNGLUNIFORM1IPROC)
            wglGetProcAddress("glUniform1i");
        glGetProgramiv = (PFNGLGETPROGRAMIVPROC)
            wglGetProcAddress("glGetProgramiv");
        glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)
            wglGetProcAddress("glGetProgramInfoLog");
        glDetachShader = (PFNGLDETACHSHADERPROC)
            wglGetProcAddress("glDetachShader");
        glIsProgram = (PFNGLISPROGRAMPROC)
            wglGetProcAddress("glIsProgram");
        glGetShaderiv= (PFNGLGETSHADERIVPROC)
            wglGetProcAddress("glGetShaderiv");
        glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)
            wglGetProcAddress("glUniformMatrix4fv");
    }
    
    // create extended DC/RC 
#if 1
    // destroy dummy RC, DC, Window
    if (!wglMakeCurrent(0, 0))
    {
        return 1;
    }
    
    wglDeleteContext(global_oglRenderContext);
    ReleaseDC(*windowHandle, global_deviceContext);
    DestroyWindow(*windowHandle);
    
    // create "real" opengl context
    *windowHandle = CreateWindow(
        windowClass->lpszClassName,
        "Hello OpenGL2",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        0,
        0,
        1280,
        720,
        0,
        0,
        windowClass->hInstance,
        0
        );
    
    if (windowHandle == NULL)
    {
        return 1;
    }
    global_deviceContext = GetDC(*windowHandle);
    
    const int pixelAttribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB, 24,
        WGL_ALPHA_BITS_ARB, 8,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
        WGL_SAMPLES_ARB, 4,
        0
    };
    
    int pixelFormat;
    UINT numFormats;
    // get the pixel format id by checking the DC and the attributes
    int ok = wglChoosePixelFormatARB(global_deviceContext,
                                     pixelAttribs,
                                     0,
                                     1, // max pixel formats we want
                                     &pixelFormat,   // return: pixel format numbers
                                     &numFormats); // return: how many were created
    if (!ok)
    {
        OutputDebugStringA("Failed to create ARB pixel format\n");
        return 1;
    }
    
    
    // fill the PFD struct by telling what the pixel format is set
    PIXELFORMATDESCRIPTOR PFD;
    DescribePixelFormat(global_deviceContext, pixelFormat, sizeof(PFD), &PFD);
    // set the DC's pixel format
    if (SetPixelFormat(global_deviceContext, pixelFormat, &PFD) == 0)
    {
        return 1;
    }
    
    int contextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 2,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB,// WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    
    // create gl render context
    global_oglRenderContext = wglCreateContextAttribsARB(global_deviceContext, 0, contextAttribs);
    
    if (global_oglRenderContext == 0)
    {
        return 1;
    }
    
    if (!wglMakeCurrent(global_deviceContext, global_oglRenderContext))
    {
        return 1;
    }
#endif
    
    return 0;
}

void l_drawTriangle()
{
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);   glVertex2f(0.0f,   1.0f);
    glColor3f(0.0f, 1.0f, 0.0f);   glVertex2f(0.87f,  -0.5f);
    glColor3f(0.0f, 0.0f, 1.0f);   glVertex2f(-0.87f, -0.5f);
    glEnd();
}

void printGlErrMsg()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) 
    {
        //printf("OGL Err");
        GLubyte const * errString;
        errString = gluErrorString(err);
        printf("%s\n", errString);
    }
}

struct Shader
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint shaderProgram;
};

void check_shader_error(GLuint shader)
{
    GLint success = 0;
    GLint logSize = 0;
    GLchar buffer[255];
    
    if (glIsProgram(shader))
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
    }
    else
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    }
    
    if (success == GL_FALSE)
    {
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logSize);
        glGetProgramInfoLog(shader, 255, &logSize, &buffer[0]);
        printf("Failed to Link Shader Program: %s\n", buffer);
    }
}

// TODO(michael): create shader from config 
Shader create_shader(char const * vs_file, char const * fs_file)
{
    Shader result = {};
    
    // load shader text from files
    char * vertCode = load_text(vs_file);
    char * fragCode = load_text(fs_file);
    
    // compile shader program
    result.vertexShader = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource(result.vertexShader, 1, &vertCode, NULL);
    glCompileShader(result.vertexShader);
    check_shader_error(result.vertexShader);
    
    result.fragmentShader = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource(result.fragmentShader, 1, &fragCode, NULL);
    glCompileShader(result.fragmentShader);
    check_shader_error(result.fragmentShader);
    
    // actual compilation
    result.shaderProgram = glCreateProgram();
    glAttachShader(result.shaderProgram, result.vertexShader);
    glAttachShader(result.shaderProgram, result.fragmentShader);
    
    // tell the shader what attribute belongs to which in variable name (OGL3.2 compatibility)
    // has to be done BEFORE linking!
    glBindAttribLocation(result.shaderProgram, 0, "vertex_pos");
    glBindAttribLocation(result.shaderProgram, 1, "texture_pos");
    
    glLinkProgram(result.shaderProgram);
    check_shader_error(result.shaderProgram);
    // link success?
    
    glDetachShader(result.shaderProgram, result.vertexShader);
    glDetachShader(result.shaderProgram, result.fragmentShader);
    
    return result;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    WNDCLASS windowClass = { };
    
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc   = WindowProcCallback;
    windowClass.hInstance     = hInstance;
    windowClass.lpszClassName = "meg_oglcontext";
    
    RegisterClass(&windowClass);
    
    global_windowHandle = CreateWindow(
        windowClass.lpszClassName,
        "Hello OpenGL",
        WS_OVERLAPPEDWINDOW, // style,so there are buttons like close,minimize,etc
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        hInstance,
        0
        );
    
    if (global_windowHandle == NULL)
    {
        return 0;
    }
    
    // init console
    AllocConsole();
    FILE* pCin;
    FILE* pCout;
    FILE* pCerr;
    freopen_s(&pCin, "conin$", "r", stdin);
    freopen_s(&pCout, "conout$", "w", stdout);
    freopen_s(&pCerr, "conout$", "w", stderr);
    
    // init GL
    initGL(&global_windowHandle, &windowClass);
    
    ShowWindow(global_windowHandle, nCmdShow);
    
    GLubyte const * glVersion = glGetString(GL_VERSION);
    GLubyte const * glRenderer = glGetString(GL_RENDERER);
    meg_strbuf strbuf = meg_strbuf_create();
    meg_strbuf_write(&strbuf, "GL-VERSION: ");
    meg_strbuf_write(&strbuf, (char*)glVersion);
    meg_strbuf_write(&strbuf, " ::: RENDERER: ");
    meg_strbuf_write(&strbuf, (char*)glRenderer);
    SetWindowText(global_windowHandle, LPCSTR(strbuf.buffer));
    DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
    
    
    // some test OGL data
    GLfloat points[] = {
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f
    };
    GLfloat texturePos[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f
    };
    
    
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof (points), points, GL_STATIC_DRAW);
    
    // first param is index
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    // enable, affects only the previously bound VBOs!
    glEnableVertexAttribArray(0);
    
    GLuint vbo2 = 0;
    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texturePos), texturePos, GL_STATIC_DRAW);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    
    //glEnableVertexAttribArray (0);
    //glEnableVertexAttribArray (1);
    
    // set viewport dimensions
    RECT rect;
    GetClientRect(global_windowHandle, &rect);
    glViewport(0, 0, rect.right, rect.bottom);
    
    // backface/frontface culling (creates less shaders if enabled)
    glEnable (GL_CULL_FACE); // cull face
    glCullFace (GL_BACK); // cull back face
    glFrontFace (GL_CW); // GL_CCW for counter clock-wise
    
    Shader shader = create_shader(
        "..\\code\\anton_vert_hello_triangle.vert",
        "..\\code\\anton_frag_hello_triangle.frag");
    
    // STBI image loading
    int x, y, n;
    unsigned char * image_data = stbi_load("..\\assets\\uv_checkerboard.jpg", &x, &y, &n, 4);
    
    // ogl texture creation
    GLuint tex = 0;
    glGenTextures (1, &tex);
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, tex);
    glTexImage2D (
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        x,
        y,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image_data
        );
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glUseProgram(shader.shaderProgram); // TODO(Michael): necessary? -> OMG! YES!!!
    
    // in ogl 4 uniform 0 will do. this is necessary for ogl 3.2
    int tex_loc = glGetUniformLocation(shader.shaderProgram, "tex");
    glUniform1i(tex_loc, 0); // use active texture 0
    
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // create ortho matrix
    float aspectRatio = (float)rect.right / (float)rect.bottom;
    float orthoMatrix[16] = { };
    ortho(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f, orthoMatrix);
    ortho_loc = glGetUniformLocation(shader.shaderProgram, "ortho");
    printGlErrMsg();
    glUniformMatrix4fv(ortho_loc, 1, GL_FALSE, orthoMatrix);
    printGlErrMsg();
    
    while (running)
    {
        // Run the message loop.
        MSG msg = { };
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // render with OpenGL
        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader.shaderProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        SwapBuffers(global_deviceContext);
    }
    
    fclose(pCin);
    fclose(pCout);
    fclose(pCerr);
    FreeConsole();
    
    wglMakeCurrent(0, 0);
    wglDeleteContext(global_oglRenderContext);
    ReleaseDC(global_windowHandle, global_deviceContext);
    
    return 0;
}

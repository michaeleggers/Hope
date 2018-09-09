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

global_var PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB;
global_var PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
global_var PFNWGLSWAPINTERVALEXTPROC         wglSwapIntervalEXT;
global_var PFNGLGENBUFFERSPROC               glGenBuffers;
global_var PFNGLBINDBUFFERPROC               glBindBuffer;
global_var PFNGLBUFFERDATAPROC               glBufferData;
global_var PFNGLGENVERTEXARRAYSPROC          glGenVertexArrays;
global_var PFNGLBINDVERTEXARRAYPROC          glBindVertexArray;
global_var PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray;
global_var PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer;
global_var PFNGLCREATESHADERPROC             glCreateShader;
global_var PFNGLSHADERSOURCEPROC             glShaderSource;
global_var PFNGLCOMPILESHADERPROC            glCompileShader;
global_var PFNGLCREATESHADERPROC             glCreateShaderProc;
global_var PFNGLCREATEPROGRAMPROC            glCreateProgram;
global_var PFNGLATTACHSHADERPROC             glAttachShader;
global_var PFNGLLINKPROGRAMPROC              glLinkProgram;
global_var PFNGLUSEPROGRAMPROC               glUseProgram;
global_var PFNGLBINDATTRIBLOCATIONPROC       glBindAttribLocation;
global_var PFNGLACTIVETEXTUREPROC            glActiveTexture;
global_var PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation;
global_var PFNGLUNIFORM1IPROC                glUniform1i;
global_var PFNGLGETPROGRAMIVPROC             glGetProgramiv;
global_var PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog;
global_var PFNGLDETACHSHADERPROC             glDetachShader;
global_var PFNGLISPROGRAMPROC                glIsProgram;
global_var PFNGLGETSHADERIVPROC              glGetShaderiv;
global_var PFNGLUNIFORMMATRIX4FVPROC         glUniformMatrix4fv;
global_var PFNGLUNIFORM2FPROC                glUniform2f;
global_var PFNGLUNIFORM4FPROC                glUniform4f;


#include "helper.cpp"
#include "ogl_render.cpp"
#include "game.h"
#include "game.cpp"

global_var HGLRC global_oglRenderContext;
global_var HWND  global_windowHandle;
global_var HDC   global_deviceContext;
global_var bool running = true;
global_var LARGE_INTEGER performanceFrequency;

Rect get_window_dimensions()
{
    RECT rect;
    GetClientRect(global_windowHandle, &rect);
    Rect result;
    result.width  = rect.right;
    result.height = rect.bottom;
    
    return result;
}

void update_messages()
{
    MSG msg = { };
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK WindowProcCallback(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (uMsg)
    {
        // TODO(Michael): get uniform id from game layer
        case WM_SIZE:
        {
            RECT rect;
            GetClientRect(windowHandle, &rect);
            for (int i = 0;
                 i < MAX_SHADERS;
                 ++i)
            {
                set_ortho(rect.right, rect.bottom, &shaders[i]);
            }
            glViewport(0, 0, rect.right, rect.bottom);
            
            //game_update_and_render(100000000.0f);
            //SwapBuffers(global_deviceContext);
            
            /*
        // recompute orthographic projection matrix
        float aspectRatio = (float)rect.right / (float)rect.bottom;
        float orthoMatrix[16] = { };
        ortho(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f, orthoMatrix);
        glUniformMatrix4fv(ortho_loc, 1, GL_FALSE, orthoMatrix);
*/
        }
        break;
        
        case WM_SIZING:
        {
            printf("WM_SIZING message received!\n");
            game_update_and_render(0.0f);
            SwapBuffers(global_deviceContext);
        }
        
        case WM_MOVE:
        {
            printf("WM_MOVE message received!\n");
            //game_update_and_render(100000000.0f);
            SwapBuffers(global_deviceContext);
        }
        break;
        
        // TODO(Michael): What's the deal with WM_PAINT?
        case WM_PAINT:
        {
            PAINTSTRUCT pstr;
            HDC deviceContext = BeginPaint(windowHandle, &pstr);
            //game_update_and_render(100000000.0f);
            SwapBuffers(deviceContext); // same handle as global DC?
            EndPaint(windowHandle, &pstr);
            result = DefWindowProc(windowHandle, WM_ERASEBKGND, wParam, lParam);
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
        // TODO(Michael): check if swap control is actually possible
        // see: https://www.khronos.org/opengl/wiki/Swap_Interval
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
            wglGetProcAddress("wglSwapIntervalEXT");
        
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
        glUniform2f = (PFNGLUNIFORM2FPROC)
            wglGetProcAddress("glUniform2f");
        glUniform4f = (PFNGLUNIFORM4FPROC)
            wglGetProcAddress("glUniform4f");
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
        1000,
        1000,
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    WNDCLASS windowClass = { };
    
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc   = WindowProcCallback;
    windowClass.hInstance     = hInstance;
    windowClass.lpszClassName = "meg_oglcontext";
    windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    
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
    
    // show some GL info in window title-bar
    GLubyte const * glVersion = glGetString(GL_VERSION);
    GLubyte const * glRenderer = glGetString(GL_RENDERER);
    meg_strbuf strbuf = meg_strbuf_create();
    meg_strbuf_write(&strbuf, "GL-VERSION: ");
    meg_strbuf_write(&strbuf, (char*)glVersion);
    meg_strbuf_write(&strbuf, " ::: RENDERER: ");
    meg_strbuf_write(&strbuf, (char*)glRenderer);
    SetWindowText(global_windowHandle, LPCSTR(strbuf.buffer));
    DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
    
    //glEnableVertexAttribArray (0);
    //glEnableVertexAttribArray (1);
    
    // set viewport dimensions
    RECT rect;
    GetClientRect(global_windowHandle, &rect);
    glViewport(0, 0, rect.right, rect.bottom);
    
    game_init();
    
    // set up timing stuff
    QueryPerformanceFrequency(&performanceFrequency);
    LARGE_INTEGER startingTime, endingTime, elapsedTime;
    QueryPerformanceCounter(&startingTime);
    QueryPerformanceCounter(&endingTime);
    elapsedTime.QuadPart = 0;
    float fps = 0.0f;
    
    
    
    while (running)
    {
        
        // if test
#if _WIN32
        printf("ON WIN32 PLATFORM!\n");
#endif
        // Run the message loop.
        MSG msg = { };
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        game_update_and_render((float)elapsedTime.QuadPart); 
        SwapBuffers(global_deviceContext);
        
        QueryPerformanceCounter(&endingTime);
        elapsedTime.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
        fps = (float)performanceFrequency.QuadPart / (float)elapsedTime.QuadPart;
        elapsedTime.QuadPart *= 1000000; // microseconds
        elapsedTime.QuadPart /= performanceFrequency.QuadPart;
        
        printf("%f\n", fps);
        printf("elapsed time: %f\n", (float)elapsedTime.QuadPart / 1000.0f);
        QueryPerformanceCounter(&startingTime);
        //Sleep(500); // HACK(Michael): artificial time
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

#include "win32_opengl.h"
#include "opengl_render.cpp"
#include "common_os.h"

int win32_initGL(HWND* windowHandle, WNDCLASS* windowClass)
{
    gRenderState.windowHandle = windowHandle;
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
    gRenderState.deviceContext = GetDC(*windowHandle);
    // try to find best matching pixel format.
    int pixelFormatNr = ChoosePixelFormat(gRenderState.deviceContext, &pixelFormatDescriptor);
    if (!pixelFormatNr) // was null
    {
        OutputDebugStringA("could not get pxl format nr\n");
        return 1;
    }
    
    DescribePixelFormat(gRenderState.deviceContext, 
                        pixelFormatNr,
                        sizeof(pixelFormatDescriptor), 
                        &pixelFormatDescriptor);
    
    if (!SetPixelFormat(gRenderState.deviceContext, pixelFormatNr, &pixelFormatDescriptor))
    {
        return 1;
    }
    
    gRenderState.renderContext = wglCreateContext(gRenderState.deviceContext);
    if (!gRenderState.renderContext)
    {
        return 1;
    }
    
    // load extensions
    if (wglMakeCurrent(gRenderState.deviceContext, gRenderState.renderContext))
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
        glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)
            wglGetProcAddress("glDeleteVertexArrays");
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
        glUniform3f = (PFNGLUNIFORM3FPROC)
            wglGetProcAddress("glUniform3f");
        glUniform4f = (PFNGLUNIFORM4FPROC)
            wglGetProcAddress("glUniform4f");
        glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)
            wglGetProcAddress("glDrawElementsBaseVertex");
    }
    
    // create extended DC/RC 
#if 1
    // destroy dummy RC, DC, Window
    if (!wglMakeCurrent(0, 0))
    {
        return 1;
    }
    
    wglDeleteContext(gRenderState.renderContext);
    ReleaseDC(*windowHandle, gRenderState.deviceContext);
    DestroyWindow(*windowHandle);
    
    // create "real" opengl context
    *windowHandle = CreateWindow(
        windowClass->lpszClassName,
        "Hello OpenGL2",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        0,
        0,
        1024,
        576,
        0,
        0,
        windowClass->hInstance,
        0
        );
    
    if (windowHandle == NULL)
    {
        return 1;
    }
    gRenderState.deviceContext = GetDC(*windowHandle);
    
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
    int ok = wglChoosePixelFormatARB(gRenderState.deviceContext,
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
    DescribePixelFormat(gRenderState.deviceContext, pixelFormat, sizeof(PFD), &PFD);
    // set the DC's pixel format
    if (SetPixelFormat(gRenderState.deviceContext, pixelFormat, &PFD) == 0)
    {
        return 1;
    }
    
    int contextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 4,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB,// WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    
    // create gl render context
    gRenderState.renderContext = wglCreateContextAttribsARB(gRenderState.deviceContext, 0, contextAttribs);
    
    if (gRenderState.renderContext == 0)
    {
        return 1;
    }
    
    if (!wglMakeCurrent(gRenderState.deviceContext, gRenderState.renderContext))
    {
        printf("wglMakeCurrent failed!\n\n"); // TODO(Michael): better error message
        return 1;
    }
#endif
    OutputDebugStringA("created extended ogl context\n");
    
    RECT windowDimension;
    GetClientRect(
        *windowHandle,
        &windowDimension
        );
    glViewport(windowDimension.left, windowDimension.top, 
               windowDimension.right, windowDimension.bottom);
    
    
    // global buffer handles for new rendering API
    glGenBuffers(1, &gvtxHandle);
    glGenBuffers(1, &gidxHandle);
    
    // init shaders
    initShaders();
    
    // upload orthographic projection uniform
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[SPRITE], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[SPRITE_SHEET], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[STANDARD_MESH], "projectionMat");
    
    // backface/frontface culling (creates less shaders if enabled)
    glEnable (GL_CULL_FACE); // cull face
    glCullFace (GL_BACK); // cull back face
    //glFrontFace (GL_CW); // GL_CCW is default
    
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // enable vsync
    wglSwapIntervalEXT(1);
    
    // multisampling
    glEnable(GL_MULTISAMPLE);
    
    OutputDebugStringA("end init ogl\n");
    return 0;
}

#include "ogl_render.h"


global_var RenderState gRenderState;
global_var Shader gShaders[MAX_SHADERS];
global_var Sprite gSprites[100];

// load all rooms (or later on scenes) onto GPU (for now just one room)
// TODO(Michael): get data from actual room data, lol
// TODO(Michael): load data from asset file or some other resource handling stuff
void glLoadRooms(Room* room)
{
    
    char * shaderAttribs[] = {
        "vertex_pos",
        "texture_pos",
    };
    gShaders[SPRITE] = create_shader("..\\code\\sprite.vert", "..\\code\\sprite.frag",
                                     shaderAttribs,
                                     sizeof(shaderAttribs) / sizeof(*shaderAttribs));
    gSprites[0] = create_sprite("..\\assets\\azores.png", &gShaders[SPRITE]);
    
    glUseProgram(gSprites[0].shader.shaderProgram);
    set_ortho(1000, 1000, &gSprites[0].shader);
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

// legacy immediate rendering
// NOTE(Michael): only works with compatibility attrib bit set
void l_drawTriangle()
{
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);   glVertex2f(0.0f,   1.0f);
    glColor3f(0.0f, 1.0f, 0.0f);   glVertex2f(0.87f,  -0.5f);
    glColor3f(0.0f, 0.0f, 1.0f);   glVertex2f(-0.87f, -0.5f);
    glEnd();
}

Shader create_shader(char const * vs_file,
                     char const * fs_file,
                     char ** attribLocations,
                     int numAttribs)
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
    for (int i = 0;
         i < numAttribs;
         ++i)
    {
        // TODO(Michael): what's the deal with the index???
        glBindAttribLocation(result.shaderProgram, i, *attribLocations);
        attribLocations++;
    }
    
    glLinkProgram(result.shaderProgram);
    check_shader_error(result.shaderProgram);
    
    glDetachShader(result.shaderProgram, result.vertexShader);
    glDetachShader(result.shaderProgram, result.fragmentShader);
    
    return result;
}

Texture create_texture(char const * texture_file)
{
    // STBI image loading
    int x, y, n;
    unsigned char * image_data = stbi_load(texture_file, &x, &y, &n, 4);
    
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(
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
    // TODO(Michael): pull this out later, or is this per texture?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    return Texture { tex, x, y };
}

Texture create_texture_from_background(Background* bg)
{
    unsigned char * image_data = bg->image;
    int x = bg->x;
    int y = bg->y;
    
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(
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
    // TODO(Michael): pull this out later, or is this per texture?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    return Texture { tex, x, y };
}

Quad create_quad()
{
    Quad mesh;
    GLfloat points[] = {
        -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f
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
    
    mesh.vao = vao;
    // TODO(Michael): use pointers to a global mesh table
    memcpy(mesh.vertexPoints, points, 18);
    memcpy(mesh.textureUVs, texturePos, 12);
    
    return mesh;
}

void set_ortho(int width, int height, Shader * shader)
{
    int targetHeight = ((float)width * 9.0f) / 16.0f;
    float squeeze = (float)targetHeight / (float)height;
    float orthoMatrix[16] = { };
    float aspectRatio = (float)width / (float)height;
    ortho(-10.0f * aspectRatio, 10.0f * aspectRatio,
          -10.0f, 10.0f,
          -1.0f, 1.0f,
          orthoMatrix
          );
#if 0 // TODO(Michael): compute matrices for scaling and set glViewport for independent res.
    if (height >= width)
    {
    }
#endif
    
    glUseProgram(shader->shaderProgram);
    GLuint ortho_loc = glGetUniformLocation(shader->shaderProgram, "ortho");
    glUniformMatrix4fv(ortho_loc, 1, GL_FALSE, orthoMatrix);
}

void set_model(GLfloat modelMatrix[])
{
    GLint shaderID;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderID);
    GLuint model_loc = glGetUniformLocation(shaderID, "model");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, modelMatrix);
}

// TODO(Michael): use existing texture (if loaded) from data-base
// NOTE(Michael): maybe instead of passing a shader, we could just
// a predefined one.
Sprite create_sprite(char const * file, Shader * shader)
{
    Quad quad = create_quad();
    
    // NOTE(Michael): is it OK to use the same texture-slot per model?
    Texture texture = create_texture(file);
    
    // has to active BEFORE call to glGetUniformLocation!
    glUseProgram(shader->shaderProgram);
    
    // in ogl 4 uniform 0 will do. this is necessary for ogl 3.2
    int tex_loc = glGetUniformLocation(shader->shaderProgram, "tex");
    glUniform1i(tex_loc, 0); // use active texture (why is this necessary???)
    
    Sprite result;
    result.mesh = quad;
    result.shader = *shader;
    result.texture = texture;
    
    return result;
}

// TODO(Michael): do not genereate model matrix for each call
void draw_sprite(Sprite * sprite)
{
    glUseProgram(sprite->shader.shaderProgram);
    GLfloat modelMatrix[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    set_model(modelMatrix);
    glBindVertexArray(sprite->mesh.vao);
    glBindTexture(GL_TEXTURE_2D, sprite->texture.texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// TODO(Michael): get window dimensions (from current
// render context maybe?).
// TODO(Michael): do not generate matrix new all the time?
// rather create a transform in the sprite struct?
// TODO(Michael): also get dimensions of projection matrix.
// as at the moment it is set to the range -1 to 1 for both
// axis.
// TODO(Michael): THIS (ALLONE) IS BROKEN DUE TO DRAW_FRAME FUNCTION WHICH
// USES ANOTHER SHADER...
void draw_sprite(Sprite * sprite,
                 float x,
                 float y)
{
    glUseProgram(sprite->shader.shaderProgram);
    GLfloat modelMatrix[] = { // only translate by x,y atm
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x, -y, 0.0f, 1.0f, // in OpenGL y's negative is bottom of screen
    };
    set_model(modelMatrix);
    glBindVertexArray(sprite->mesh.vao);
    glBindTexture(GL_TEXTURE_2D, sprite->texture.texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

Spritesheet create_spritesheet(Texture * texture,
                               int width, int height, // framesize in pixel
                               int numFrames)
{
    Spritesheet spritesheet;
    Window window;
    int textureWidth = texture->width;
    int textureHeight = texture->height;
    window.width  = (1.0f / (float)textureWidth) * (float)width;
    window.height = (1.0f / (float)textureHeight) * (float)height;
    
    for (int i = 0;
         i < numFrames;
         ++i)
    {
        window.x = (1.0f / (float)textureWidth) * (float)((i * width) % textureWidth);
        window.y = (1.0f / (float)textureHeight) * (float)(((i * width) / textureWidth) * height);
        spritesheet.windows[i] = window;
    }
    
    return spritesheet;
}

// TODO(Michael): keep one model matrix rather than generating it all the time on the stack, duh!
void draw_frame(Sprite * sprite, Spritesheet * spritesheet, int frame,
                float x, float y,
                float scaleX, float scaleY)
{
    Window window = spritesheet->windows[frame];
    glUseProgram(sprite->shader.shaderProgram);
    GLfloat modelMatrix[] = {
        scaleX * window.width * 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, scaleY * window.height * 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x, -y, 0.0f, 1.0f, // in OpenGL y's negative is bottom of screen
    };
    set_model(modelMatrix);
    int window_loc = glGetUniformLocation(sprite->shader.shaderProgram, "window");
    glUniform4f(window_loc,
                // offsets
                window.x, window.y,
                //0.067f, 0.1f,
                window.width, window.height
                ); // use active texture
    glBindVertexArray(sprite->mesh.vao);
    glBindTexture(GL_TEXTURE_2D, sprite->texture.texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// for now
static char * shaderAttribs[] = {
    "vertex_pos",
    "texture_pos",
};

// determine from game logic what is to render and set it up here
void glRender()
{
    gl_renderFrame(gSprites, 1);
}

void gl_renderFrame(Sprite* sprites, int spriteCount) // later on render-groups, so I can also render moving sprites?
{
    if (spriteCount == 0) return;
    
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    GLfloat modelMatrix[] = { // only translate by x,y atm
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, // in OpenGL y's negative is bottom of screen
    };
    int i = 0;
    while (i < spriteCount)
    {
        Sprite sprite = sprites[i];
        
        // in ogl 4 uniform 0 will do. this is necessary for ogl 3.2
        glUseProgram(sprite.shader.shaderProgram); // has to active BEFORE call to glGetUniformLocation!
        set_model(modelMatrix);
        glBindVertexArray(sprite.mesh.vao);
        glBindTexture(GL_TEXTURE_2D, sprite.texture.texture_id);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        i++;
    }
    
    SwapBuffers(gRenderState.deviceContext);
}

/*
GLfloat texturePos[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f
};
*/

int win32_initGL(HWND* windowHandle, WNDCLASS* windowClass)
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
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 2,
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
        return 1;
    }
#endif
    
    glViewport(0, 0, 1000, 1000); // TODO(Michael): variable size!
    
    // backface/frontface culling (creates less shaders if enabled)
    glEnable (GL_CULL_FACE); // cull face
    glCullFace (GL_BACK); // cull back face
    glFrontFace (GL_CW); // GL_CCW for counter clock-wise
    
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // enable vsync
    wglSwapIntervalEXT(1);
    
    // multisampling
    glEnable(GL_MULTISAMPLE);
    
    
    return 0;
}


// init the struct
refexport_t GetRefAPI()
{
    refexport_t re;
    re.init = win32_initGL;
    re.loadRooms = glLoadRooms;
    re.render = glRender;
    
    return re;
}

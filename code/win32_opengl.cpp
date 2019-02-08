#include "win32_opengl.h"
#include "opengl_renderer.cpp"
#include "common_os.h"

// TODO(Michael): separate sprite size = the size of the sprite on screen
// from texture size = size of the texture data on GPU

global_var RenderState gRenderState;
global_var Shader gShaders[MAX_SHADERS];

global_var GPUSprite gSpritesKnown[MAX_SPRITES]; 
global_var int gUnknownSpriteIndex;
global_var Texture gTexturesKnown[MAX_TEXTURES];
global_var int gUnknownTextureIndex;
global_var Texture gFallbackTexture;
global_var GPUMeshData gMeshList[MAX_MESHES];
global_var int gUnknownMeshIndex;

// TODO(Michael): we might want to NOT have platform stuff in here (but also I might be wrong).
global_var PlatformAPI* gPlatformAPI;

Window gl_createWindow(int textureWidth, int textureHeight,
                       int xOffset, int yOffset,
                       int width, int height)
{
    
    Window window;
    window.width  = (1.0f / (float)textureWidth) * (float)width;
    window.height = (1.0f / (float)textureHeight) * (float)height;
    if (xOffset > 0)
        window.x = (1.0f / (float)textureWidth) * (float) (xOffset % textureWidth);
    else
        window.x = 0;
    if (yOffset > 0)
        window.y = (1.0f / (float)textureHeight) * (float) ((width / textureWidth) * height + yOffset);
    else
        window.y = 0;
    window.intWidth = width;
    window.intHeight = height;
    
    return window;
}

Sprite glRegisterSprite(
char * filename, 
unsigned char * imageData,
int textureWidth, int textureHeight,
int xOffset, int yOffset,
int width, int height)
{
    Sprite sprite;
    sprite.currentFrame = 0;
    sprite.frameCount = 1;
    GPUSprite gpuSpriteData;
    
    gpuSpriteData.freeWindowIndex = 0;
    gpuSpriteData.shader = &gShaders[SPRITE_SHEET];
    gpuSpriteData.mesh = create_quad();
    gpuSpriteData.width = width;
    gpuSpriteData.height = height;
    Texture * texture = createTexture(filename, imageData, textureWidth, textureHeight);
    gpuSpriteData.texture = texture;
    
    // TODO(Michael): this is shader specific, why is this here?!
    // has to active BEFORE call to glGetUniformLocation!
    glUseProgram(gShaders[SPRITE_SHEET].shaderProgram);
    // in ogl 4 uniform 0 will do. this is necessary for ogl 3.2
    int tex_loc = glGetUniformLocation(gShaders[SPRITE_SHEET].shaderProgram, "tex");
    glUniform1i(tex_loc, 0); // use active texture (why is this necessary???)
    
    Window window = gl_createWindow(textureWidth, textureHeight,
                                    xOffset, yOffset,
                                    width, height);
    
    gpuSpriteData.windows[0] = window;
    gpuSpriteData.freeWindowIndex++;
    
    if (gUnknownSpriteIndex == MAX_SPRITES)
    {
        gSpritesKnown[gUnknownSpriteIndex-1] = gpuSpriteData;
        sprite.spriteHandle = (void *)&gSpritesKnown[gUnknownSpriteIndex-1];
    }
    else
    {
        gSpritesKnown[gUnknownSpriteIndex] = gpuSpriteData;
        sprite.spriteHandle = (void *)&gSpritesKnown[gUnknownSpriteIndex];
        gUnknownSpriteIndex++;
    }
    
    return sprite;
}

void gl_addSpriteFrame(Sprite * sprite, int xOffset, int yOffset, int width, int height)
{
    GPUSprite * gpuSprite = (GPUSprite *)sprite->spriteHandle;
    if (gpuSprite->freeWindowIndex == MAX_SPRITESHEET_WINDOWS) return;
    
    int textureWidth = gpuSprite->texture->width;
    int textureHeight = gpuSprite->texture->height;
    gpuSprite->windows[gpuSprite->freeWindowIndex] = gl_createWindow(textureWidth, textureHeight,
                                                                     xOffset, yOffset,
                                                                     width, height);
    gpuSprite->freeWindowIndex++;
    sprite->frameCount++;
}

// TODO(Michael): how to unregister meshes, like, how do we free
// data on VRAM??
// NOTE(Michael): registerMesh just pushes vertex data onto the GPU,
// it does _not_ check if the same mesh has been loaded before. This has
// to be done by a higher level system.
void * gl_RegisterMesh(Vertex * vertices, int count)
{
    void* handle;
    GPUMeshData gpuMeshData;
    
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, count*sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
    
    // 0 1 2 | 3 4 5 | 6  7
    // v v v | n n n | uv uv
    // positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    // enable, affects only the previously bound VBOs!
    glEnableVertexAttribArray(0);
    //normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(v3));
    // enable, affects only the previously bound VBOs!
    glEnableVertexAttribArray(1);
    // UVs
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(v3)*2));
    // enable, affects only the previously bound VBOs!
    glEnableVertexAttribArray(2);
    
    glUseProgram(gShaders[STANDARD_MESH].shaderProgram);
    // in ogl 4 uniform 0 will do. this is necessary for ogl 3.2
    /*
    int vertexPos  = glGetUniformLocation(gShaders[STANDARD_MESH].shaderProgram, "vertex_pos");
    glUniform1i(vertexPos, 0);
    int normalPos  = glGetUniformLocation(gShaders[STANDARD_MESH].shaderProgram, "normals");
    glUniform1i(normalPos, 1);
    int uvPos      = glGetUniformLocation(gShaders[STANDARD_MESH].shaderProgram, "UVs");
    glUniform1i(uvPos, 2);
    */
    
    gpuMeshData.vao = vao;
    gpuMeshData.vertexCount = count;
    // TODO(Michael): better managing for this?!
    if (gUnknownMeshIndex == MAX_MESHES)
    {
        gMeshList[gUnknownMeshIndex-1] = gpuMeshData;
        handle = (void *)&gMeshList[gUnknownMeshIndex-1];
    }
    else
    {
        gMeshList[gUnknownMeshIndex] = gpuMeshData;
        handle = (void *)&gMeshList[gUnknownMeshIndex];
        gUnknownMeshIndex++;
    }
    
    return handle;
}

void initShaders()
{
    char * shaderAttribs[] = {
        "vertex_pos",
        "texture_pos",
    };
    char * shaderAttribsMesh[] = {
        "vertex_pos",
        "normals",
        "UVs"
    };
    gShaders[SPRITE] = create_shader("..\\code\\sprite_v.glsl", "..\\code\\sprite_f.glsl",
                                     shaderAttribs,
                                     sizeof(shaderAttribs) / sizeof(*shaderAttribs));
    gShaders[SPRITE_SHEET] = create_shader("..\\code\\sprite_v.glsl", "..\\code\\sprite_sheet_f.glsl",
                                           shaderAttribs,
                                           sizeof(shaderAttribs) / sizeof(*shaderAttribs));
    gShaders[STANDARD_MESH] = create_shader("..\\code\\standard_mesh_v.glsl", "..\\code\\standard_mesh_f.glsl",
                                            shaderAttribsMesh,
                                            sizeof(shaderAttribsMesh) / sizeof(*shaderAttribsMesh));
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
    char * vertCode = gPlatformAPI->readTextFile(vs_file);
    char * fragCode = gPlatformAPI->readTextFile(fs_file);
    
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

// NOTE(Michael): just a test! remove this stuff later!
#define gCheckImageHeight 64
#define gCheckImageWidth 64
global_var GLubyte gCheckImage[gCheckImageWidth * gCheckImageHeight * 4];
void createFallbackTexture(Texture * texture)
{
    int i, j, c;
    
    for (i = 0; i < gCheckImageHeight; i++) {
        for (j = 0; j < gCheckImageWidth; j++) {
            c = ( (((i&0x8)==0)^((j&0x8))==0) ) * 255;
            int index = 4*(i * gCheckImageWidth + j);
            gCheckImage[index] = (GLubyte) c;
            gCheckImage[index+1] = (GLubyte) c;
            gCheckImage[index+2] = (GLubyte) c;
            gCheckImage[index+3] = (GLubyte) 255;
        }
    }
    
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        gCheckImageWidth,
        gCheckImageHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        gCheckImage
        );
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, tex);
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, gCheckImage);
    //free(pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    // gFallbackTexture = Texture { tex, gCheckImageWidth, gCheckImageHeight };
    texture->texture_id = tex;
    texture->width = gCheckImageWidth;
    texture->height = gCheckImageHeight;
}

Texture * createTexture(char * filename, unsigned char * imageData, int width, int height)
{
    Texture * texture = gTexturesKnown;
    
    // search currently loaded textures
    for (int i = 0;
         i < gUnknownTextureIndex;
         i++)
    {
        if (!strcmp(texture->name, filename))
            return texture;
        texture++;
        
    }
    
    // find free slot for new texture
    for (int i = 0;
         i < gUnknownTextureIndex;
         i++)
    {
        if (!texture->name[0])
            break; // free slot found
        texture++;
    }
    if (gUnknownTextureIndex == MAX_TEXTURES) // texture slots full, overwriting last sprite!
        printf("createTexture error: gUnknownTextureIndex == MAX_TEXTURES\n");
    else
        gUnknownTextureIndex++;
    
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    if (imageData)
    {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            imageData
            );
    }
    else
    {
        createFallbackTexture(texture);
        return texture;
    }
    
    // TODO(Michael): pull this out later, or is this per texture?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    texture->texture_id = tex;
    texture->width = width;
    texture->height = height;
    strcpy(texture->name, filename);
    return texture;
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
    memcpy(mesh.vertexPoints, points, 18*sizeof(GLfloat));
    memcpy(mesh.textureUVs, texturePos, 12*sizeof(GLfloat));
    
    return mesh;
}

void set_ortho(int width, int height, Shader * shader, char * location)
{
    int targetHeight = ((float)width * 9.0f) / 16.0f;
    float squeeze = (float)targetHeight / (float)height;
    float orthoMatrix[16] = { };
    float aspectRatio = (float)width / (float)height;
    ortho(-100.0f * aspectRatio, 100.0f * aspectRatio,
          -100.0f, 100.0f,
          -1.0f, 1.0f,
          orthoMatrix
          );
#if 0 // TODO(Michael): compute matrices for scaling and set glViewport for independent res.
    if (height >= width)
    {
    }
#endif
    
    //GLuint ortho_loc = glGetUniformLocation(shader->shaderProgram, "ortho");
    //glUniformMatrix4fv(ortho_loc, 1, GL_FALSE, orthoMatrix);
    setUniformMat4fv(shader, location, orthoMatrix);
}

void set_model(GLfloat modelMatrix[], Shader * shader, char * location)
{
    setUniformMat4fv(shader, location, modelMatrix);
}

void setUniformMat4fv(Shader * shader, char * location, GLfloat mat4data[])
{
    glUseProgram(shader->shaderProgram);
    GLuint uniformLocation = glGetUniformLocation(shader->shaderProgram, location);
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, mat4data);
}

Spritesheet create_spritesheet(Texture * texture,
                               int xOffset, int yOffset,
                               int width, int height, // framesize in pixel
                               int numFrames)
{
    Spritesheet spritesheet;
    Window window;
    int textureWidth = texture->width;
    int textureHeight = texture->height;
    window.width  = (1.0f / (float)textureWidth) * (float)width;
    window.height = (1.0f / (float)textureHeight) * (float)height;
    if (xOffset > 0)
        window.x = (1.0f / (float)textureWidth) * (float) (xOffset % textureWidth);
    if (yOffset > 0)
        window.y = (1.0f / (float)textureHeight) * (float) ((width / textureWidth) * height + yOffset);
    spritesheet.windows[0] = window;
    
    for (int i = 1;
         i < numFrames;
         ++i)
    {
        window.x = (1.0f / (float)textureWidth) * (float)((i * width) % textureWidth);
        window.y = (1.0f / (float)textureHeight) * (float)(((i * width) / textureWidth) * height);
        spritesheet.windows[i] = window;
    }
    
    return spritesheet;
}

// for now
static char * shaderAttribs[] = {
    "vertex_pos",
    "texture_pos",
};

void glSetViewport(int xLeft, int yBottom, int width, int height)
{
    glViewport((GLint)xLeft, (GLint)yBottom, (GLsizei)width, (GLsizei)height);
}

void glSetProjection(Projection_t projType)
{
    switch (projType)
    {
        case ORTHO:
        {
            RECT rect;
            GetClientRect(*gRenderState.windowHandle, &rect);
            set_ortho(rect.right, rect. bottom, &gShaders[SPRITE], "ortho");
        }
        break;
        
        default: // TODO(Michael): implement perspective projection
        {
            RECT rect;
            GetClientRect(*gRenderState.windowHandle, &rect);
            set_ortho(rect.right, rect. bottom, &gShaders[SPRITE], "ortho");
        }
        break;
    }
}

// stuff we have to update when the platform calls notify.
// eg. update some uniforms in the shader so projection matrix is being adjusted.
void gl_notify()
{
    RECT windowDimension;
    GetClientRect(
        *gRenderState.windowHandle,
        &windowDimension
        );
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[SPRITE], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[SPRITE_SHEET], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[STANDARD_MESH], "projectionMat");
}

mat4 updateModelMat(Entity * entity)
{
    mat4 translationMatrix = hope_translate(
        entity->transform.xPos,
        entity->transform.yPos,
        entity->transform.zPos);
    mat4 scaleMatrix = hope_scale(
        entity->transform.xScale,
        entity->transform.yScale,
        entity->transform.zScale);
    mat4 rotationMatrix = hope_rotate_around_z(entity->transform.angle);
    
    mat4 modelMatrix = mat4xmat4(translationMatrix, rotationMatrix);
    modelMatrix = mat4xmat4(modelMatrix, scaleMatrix);
    
    return modelMatrix;
}

void gl_renderFrame(Refdef * refdef)
{
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    RECT windowDimension;
    GetClientRect(
        *gRenderState.windowHandle,
        &windowDimension
        );
    
    // render sprite entities
    Entity * spriteEntity = refdef->spriteEntities;
    int numSpriteEntities = refdef->numSpriteEntities;
    glUseProgram(gShaders[SPRITE_SHEET].shaderProgram);
    for (int i = 0;
         i < numSpriteEntities;
         i++)
    {
        
        GPUSprite * sprite = (GPUSprite *)(spriteEntity->sprite.spriteHandle);
        int frame = spriteEntity->sprite.currentFrame;
        int intWidth = sprite->windows[frame].intWidth;
        int intHeight = sprite->windows[frame].intHeight;
        mat4 modelMatrix = updateModelMat(spriteEntity);
        float ratio = (float)intWidth / (float)intHeight;
        modelMatrix.c[0] *= ratio;
        set_model(modelMatrix.c, &gShaders[SPRITE_SHEET], "model");
        gl_renderFrame(sprite, frame);
        spriteEntity++;
    }
    
    // render mesh entities
    Entity * meshEntity = refdef->meshEntities;
    int numMeshEntities = refdef->numMeshEntities;
    glUseProgram(gShaders[STANDARD_MESH].shaderProgram);
    for (int i = 0;
         i < numMeshEntities;
         ++i)
    {
        GPUMeshData * meshData = (GPUMeshData *)(meshEntity->mesh.meshHandle);
        mat4 modelMatrix = updateModelMat(meshEntity);
        set_model(modelMatrix.c, &gShaders[STANDARD_MESH], "modelMat");
        gl_renderMesh(meshData);
        meshEntity++;
    }
    
    // render player entity
    Entity* playerEntity = refdef->playerEntity;
    glUseProgram(gShaders[SPRITE_SHEET].shaderProgram);
    GPUSprite * sprite = (GPUSprite *)(playerEntity->sprite.spriteHandle);
    int frame = playerEntity->sprite.currentFrame;
    int intWidth = sprite->windows[frame].intWidth;
    int intHeight = sprite->windows[frame].intHeight;
    mat4 modelMatrix = updateModelMat(playerEntity);
    float ratio = (float)intWidth / (float)intHeight;
    modelMatrix.c[0] *= ratio;
    set_model(modelMatrix.c, &gShaders[SPRITE_SHEET], "model");
    gl_renderFrame(sprite, frame);
    
    SwapBuffers(gRenderState.deviceContext);
    glFinish();
}

void gl_renderMesh(GPUMeshData* meshData)
{
    glBindVertexArray(meshData->vao);
    glDrawArrays(GL_TRIANGLES, 0, meshData->vertexCount);
}

void gl_renderFrame(GPUSprite * sprite, int frame) // later on render-groups, so I can also render moving sprites?
{
    Window window = sprite->windows[frame];
    // in ogl 4 uniform 0 will do. this is necessary for ogl 3.2
    int window_loc = glGetUniformLocation(gShaders[SPRITE_SHEET].shaderProgram, "window");
    glUniform4f(window_loc,
                // offsets
                window.x, window.y,
                //0.067f, 0.1f,
                window.width, window.height
                ); // use active texture
    glBindVertexArray(sprite->mesh.vao);
    glBindTexture(GL_TEXTURE_2D, sprite->texture->texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

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
    
    RECT windowDimension;
    GetClientRect(
        *windowHandle,
        &windowDimension
        );
    glViewport(windowDimension.left, windowDimension.top, 
               windowDimension.right, windowDimension.bottom);
    
    // init shaders
    initShaders();
    
    // upload orthographic projection uniform
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[SPRITE], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[SPRITE_SHEET], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[STANDARD_MESH], "projectionMat");
    
    // backface/frontface culling (creates less shaders if enabled)
    glEnable (GL_CULL_FACE); // cull face
    glCullFace (GL_BACK); // cull back face
    glFrontFace (GL_CW); // GL_CCW for counter clock-wise
    
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // enable vsync
    wglSwapIntervalEXT(0);
    
    // multisampling
    glEnable(GL_MULTISAMPLE);
    
    
    return 0;
}


// init the struct
refexport_t GetRefAPI(PlatformAPI* platform_api)
{
    gPlatformAPI = platform_api;
    refexport_t re;
    re.init = win32_initGL;
    //re.loadRooms = glLoadRooms;
    re.setViewport = glSetViewport;
    //re.render = glRender;
    re.setProjection = glSetProjection;
    re.registerSprite = glRegisterSprite;
    re.registerMesh = gl_RegisterMesh;
    re.renderFrame = gl_renderFrame;
    re.notify = gl_notify;
    re.addSpriteFrame = gl_addSpriteFrame;
    re.addTwoNumbers = commonAddTwoNumbers;
    return re;
}


// TODO(Michael): separate sprite size = the size of the sprite on screen
// from texture size = size of the texture data on GPU

#include "opengl_render.h"

#define MAX_FRAMEBUFFERS 3

global_var RenderState gRenderState;
global_var Shader gShaders[MAX_SHADERS];
global_var FrameBuffer gFrameBuffers[MAX_FRAMEBUFFERS];
global_var int gFrameBufferCount;
global_var GPUSprite gSpritesKnown[MAX_SPRITES]; 
global_var int gUnknownSpriteIndex;
global_var Texture gTexturesKnown[MAX_TEXTURES];
global_var int gUnknownTextureIndex;
global_var Texture gFallbackTexture;
global_var GPUMeshData gMeshList[MAX_MESHES];
global_var int gUnknownMeshIndex;
global_var GLuint gvtxHandle;
global_var GLuint gidxHandle;
global_var GLint gTintLocation;
global_var GLint gTextureLocation;
global_var GLuint vaoHandle;

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
    
    glUseProgram(gShaders[STANDARD_MESH].program);
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
        "normal",
        "texture_pos",
    };
    char * shaderAttribsMesh[] = {
        "vertex_pos",
        "normals",
        "UVs"
    };
    char * shaderAttribsLine[] = {
        "vertex_pos",
    };
    char * shaderAttribsCustomFramebuffer[] = {
        "texture_pos"
    };
    char * shaderAttribsTTF[] = {
        "vertex_pos",
        "texture_pos"
    };
    gShaders[SHADER_FRAMEBUFFER] = create_shader("..\\code\\framebuffer_v.glsl", "..\\code\\framebuffer_f.glsl",
                                                 shaderAttribsCustomFramebuffer,
                                                 sizeof(shaderAttribsCustomFramebuffer) / sizeof(*shaderAttribsCustomFramebuffer));
    gShaders[SPRITE] = create_shader("..\\code\\sprite_v.glsl", "..\\code\\sprite_f.glsl",
                                     shaderAttribs,
                                     sizeof(shaderAttribs) / sizeof(*shaderAttribs));
    gShaders[SPRITE_SHEET] = create_shader("..\\code\\sprite_v.glsl", "..\\code\\sprite_sheet_f.glsl",
                                           shaderAttribs,
                                           sizeof(shaderAttribs) / sizeof(*shaderAttribs));
    gShaders[TTF] = create_shader("..\\code\\ttf_vert.glsl", "..\\code\\ttf_frag.glsl",
                                  shaderAttribsTTF,
                                  sizeof(shaderAttribsTTF) / sizeof(*shaderAttribsTTF));
    gShaders[LINE] = create_shader("..\\code\\line_vert.glsl", "..\\code\\line_frag.glsl",
                                   shaderAttribsLine,
                                   sizeof(shaderAttribsLine) / sizeof(*shaderAttribsLine));
    gShaders[FILLED_RECT] = create_shader("..\\code\\line_vert.glsl", "..\\code\\line_frag.glsl",
                                          shaderAttribsLine,
                                          sizeof(shaderAttribsLine) / sizeof(*shaderAttribsLine));
    glUseProgram(gShaders[SPRITE_SHEET].program);
    gTintLocation = glGetUniformLocation(gShaders[SPRITE_SHEET].program, "tint");
    gTextureLocation = glGetUniformLocation(gShaders[SPRITE_SHEET].program, "tex");
    //glUniform1i(gTextureLocation, 0); // use active texture (why is this necessary???)
    glUseProgram(0);
    
    gShaders[STANDARD_MESH] = create_shader("..\\code\\standard_mesh_v.glsl", "..\\code\\standard_mesh_f.glsl",
                                            shaderAttribsMesh,
                                            sizeof(shaderAttribsMesh) / sizeof(*shaderAttribsMesh));
    glGenVertexArrays(1, &vaoHandle);
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
    //printf("%s\n\n", vertCode);
    char * fragCode = gPlatformAPI->readTextFile(fs_file);
    //printf("%s\n\n\n\n", fragCode);
    
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
    result.program = glCreateProgram();
    glAttachShader(result.program, result.vertexShader);
    glAttachShader(result.program, result.fragmentShader);
    
    // tell the shader what attribute belongs to which in variable name (OGL3.2 compatibility)
    // has to be done BEFORE linking!
    for (int i = 0;
         i < numAttribs;
         ++i)
    {
        // TODO(Michael): what's the deal with the index???
        glBindAttribLocation(result.program, i, *attribLocations);
        attribLocations++;
    }
    
    glLinkProgram(result.program);
    check_shader_error(result.program);
    
    glDetachShader(result.program, result.vertexShader);
    glDetachShader(result.program, result.fragmentShader);
    
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

Texture * createTextureFromBitmap(unsigned char * bmp, int width, int height)
{
    Texture * texture = &gTexturesKnown[gUnknownTextureIndex];
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        bmp
        );
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glGenerateMipmap(GL_TEXTURE_2D);
    
#if 0    
    // TODO(Michael): pull this out later, or is this per texture?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
#endif
    texture->texture_id = tex;
    texture->width = width;
    texture->height = height;
    strcpy(texture->name, "I don't know about this");
    gUnknownTextureIndex++;
    return texture;
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
    hope_create_ortho_matrix(
        -10.0f*aspectRatio, 10.0f*aspectRatio,
        -10, 10.0f,
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
    glUseProgram(shader->program);
    GLuint uniformLocation = glGetUniformLocation(shader->program, location);
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

void glSetViewport(int xLeft, int yBottom, int width, int height)
{
    glViewport((GLint)xLeft, (GLint)yBottom, (GLsizei)width, (GLsizei)height);
}

#if 0
void glSetProjection(Projection_t projType)
{
    switch (projType)
    {
        case ORTHO:
        {
            RECT rect;
            GetClientRect(*gRenderState.windowHandle, &rect);
            set_ortho(rect.right, rect.bottom, &gShaders[SPRITE], "ortho");
            set_ortho(rect.right, rect.bottom, &gShaders[LINE], "ortho");
            set_ortho(rect.right, rect.bottom, &gShaders[FILLED_RECT], "ortho");
            set_ortho(rect.right, rect.bottom, &gShaders[SPRITE_SHEET], "ortho");
            set_ortho(rect.right, rect.bottom, &gShaders[TTF], "ortho");
            set_ortho(rect.right, rect.bottom, &gShaders[SHADER_FRAMEBUFFER], "ortho");
            set_ortho(rect.right, rect.bottom, &gShaders[STANDARD_MESH], "projectionMat");
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
#endif

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
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[SHADER_FRAMEBUFFER], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[LINE], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[FILLED_RECT], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[SPRITE_SHEET], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[TTF], "ortho");
    set_ortho(windowDimension.right, windowDimension.bottom, &gShaders[STANDARD_MESH], "projectionMat");
}

#if 0
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
    
    mat4 modelMatrix = mat4x4(translationMatrix, rotationMatrix);
    modelMatrix = mat4x4(modelMatrix, scaleMatrix);
    
    return modelMatrix;
}
#endif

void gl_renderMesh(GPUMeshData* meshData)
{
    glBindVertexArray(meshData->vao);
    glDrawArrays(GL_TRIANGLES, 0, meshData->vertexCount);
}

void gl_endFrame(DrawList* drawList)
{
    glClearColor(.2, .2f, .2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //GLuint vaoHandle = 0;
    //glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);
    
    glBindBuffer(GL_ARRAY_BUFFER, gvtxHandle);
    glBufferData(GL_ARRAY_BUFFER, drawList->vtxCount*sizeof(Vertex),
                 (GLvoid *)drawList->vtxBuffer, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gidxHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, drawList->idxCount*sizeof(uint16_t),
                 (GLvoid *)drawList->idxBuffer, GL_STREAM_DRAW);
    RenderCommand * renderCommands = drawList->renderCmds;
    RenderCommand * renderCmd = renderCommands;
    
    for (int i = 0;
         i < drawList->freeIndex;
         ++i)
    {
        RenderCommandType renderType = renderCmd->type;
        switch (renderType)
        {
            case RENDER_CMD_TEXTURED_RECT:
            {
                v3 tint = renderCmd->tint;
                uint32_t alphaColor = renderCmd->alphaColor;
                GLint alphaColorLocation = glGetUniformLocation(gShaders[SPRITE_SHEET].program, "alphaColor");
                glUseProgram(gShaders[SPRITE_SHEET].program);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, renderCmd->textureID);
                glUniform1i(gTextureLocation, 0);
                glUniform3f(gTintLocation, tint.x, tint.y, tint.z);
                glUniform1i(alphaColorLocation, alphaColor);
                setUniformMat4fv(&gShaders[SPRITE_SHEET], "ortho", renderCmd->projectionMatrix.c);
                
                // 0 1 2 | 3 4 5 | 6  7
                // v v v | n n n | uv uv
                // positions
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
                glEnableVertexAttribArray(0);
                //normals
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(v3));
                glEnableVertexAttribArray(1);
                // UVs
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(v3)*2));
                glEnableVertexAttribArray(2);
                
                glDrawElementsBaseVertex(GL_TRIANGLES, 6*renderCmd->quadCount, 
                                         GL_UNSIGNED_SHORT, (GLvoid *)(renderCmd->idxBufferOffset*sizeof(uint16_t)),
                                         renderCmd->vtxBufferOffset);
                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);
                glUseProgram(0);
            }
            break;
            
            case RENDER_CMD_TTF:
            {
                v3 tint = renderCmd->tint;
                
                glUseProgram(gShaders[TTF].program);
                setUniformMat4fv(&gShaders[TTF], "ortho", renderCmd->projectionMatrix.c);
                glBindTexture(GL_TEXTURE_2D, renderCmd->textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
                
                glActiveTexture(GL_TEXTURE0);
                GLint tintLocation = glGetUniformLocation(gShaders[TTF].program, "tint");
                GLint textureLocation = glGetUniformLocation(gShaders[TTF].program, "tex");
                glUniform1i(textureLocation, 0);
                glUniform3f(tintLocation, tint.x, tint.y, tint.z);
                
                // 0 1 2 | 3 4 5 | 6  7
                // v v v | n n n | uv uv
                // positions
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
                glEnableVertexAttribArray(0);
                // UVs
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(v3)*2));
                glEnableVertexAttribArray(1);
                
                glDrawElementsBaseVertex(GL_TRIANGLES, 6*renderCmd->quadCount, 
                                         GL_UNSIGNED_SHORT, (GLvoid *)(renderCmd->idxBufferOffset*sizeof(uint16_t)),
                                         renderCmd->vtxBufferOffset);
                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glUseProgram(0);
            }
            break;
            
            case RENDER_CMD_FILLED_RECT:
            {
                v3 tint = renderCmd->tint;
                // cannot use gTintLocation and LINE shader, because nVidia driver somehow "chaches" (?)
                // and then won't actually update properly. Worked on intel integrated GPU, though...
                glUseProgram(gShaders[FILLED_RECT].program);
                GLuint tintLocation = glGetUniformLocation(gShaders[FILLED_RECT].program, "tint");
                glUniform3f(tintLocation, tint.x, tint.y, tint.z);
                setUniformMat4fv(&gShaders[FILLED_RECT], "ortho", renderCmd->projectionMatrix.c);
                // 0 1 2 | 3 4 5 | 6  7
                // v v v | n n n | uv uv
                // positions
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
                glEnableVertexAttribArray(0);
                
                glDrawElementsBaseVertex(GL_TRIANGLES, 6*renderCmd->quadCount,
                                         GL_UNSIGNED_SHORT, (GLvoid *)(renderCmd->idxBufferOffset*sizeof(uint16_t)),
                                         renderCmd->vtxBufferOffset);
                glDisableVertexAttribArray(0);
                glUseProgram(0);
            }
            break;
            
            case RENDER_CMD_TEXT:
            {
                v3 tint = renderCmd->tint;
                glUseProgram(gShaders[SPRITE_SHEET].program);
                
                //glBindBuffer(GL_ARRAY_BUFFER, gvtxHandle);
                //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gidxHandle);
                //glBindTexture(GL_TEXTURE_2D, 0);
                //int tex_loc = glGetUniformLocation(gShaders[SPRITE_SHEET].shaderProgram, "tex");
                //glUniform1i(tex_loc, 0); // use active texture (why is this necessary???)
                
                glUniform3f(gTintLocation, tint.x, tint.y, tint.z);
                // 0 1 2 | 3 4 5 | 6  7
                // v v v | n n n | uv uv
                // positions
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
                glEnableVertexAttribArray(0);
                //normals
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(v3));
                glEnableVertexAttribArray(1);
                // UVs
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(v3)*2));
                glEnableVertexAttribArray(2);
                
                glBindTexture(GL_TEXTURE_2D, renderCmd->textureID);
                glDrawElementsBaseVertex(GL_TRIANGLES, 6*renderCmd->quadCount, 
                                         GL_UNSIGNED_SHORT, (GLvoid *)(renderCmd->idxBufferOffset*sizeof(uint16_t)),
                                         renderCmd->vtxBufferOffset);
                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);
                glUseProgram(0);
            }
            break;
            
            case RENDER_CMD_LINE:
            {
                v3 tint = renderCmd->tint;
                glUseProgram(gShaders[LINE].program);
                //set_ortho(600, 400, &gShaders[LINE], "ortho");
                GLint tintLocation = glGetUniformLocation(gShaders[LINE].program, "tint");
                glLineWidth(renderCmd->thickness);
                glUniform3f(tintLocation, tint.x, tint.y, tint.z);
                // positions
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
                glEnableVertexAttribArray(0);
                
                glDrawElementsBaseVertex(GL_LINES, 2*renderCmd->lineCount, 
                                         GL_UNSIGNED_SHORT, (GLvoid *)(renderCmd->idxBufferOffset*sizeof(uint16_t)),
                                         renderCmd->vtxBufferOffset);
                glDisableVertexAttribArray(0);
                glUseProgram(0);
            }
            break;
            
            case RENDER_CMD_SET_FRAMEBUFFER:
            {
                gl_bindFramebuffer(renderCmd->framebufferHandle);
            }
            break;
            
            case RENDER_CMD_SET_DEFAULT_FRAMEBUFFER:
            {
                gl_defaultFramebuffer(renderCmd->framebufferHandle);
            }
            break;
        }
        ++renderCmd;
    }
    drawList->vtxCount = 0;
    drawList->idxCount = 0;
    drawList->freeIndex = 0;
    drawList->prevRenderCmd = 0;
    drawList->quadCount = 0;
    drawList->lineCount = 0;
    //glDeleteVertexArrays(1, &vaoHandle);
    SwapBuffers(gRenderState.deviceContext);
}

int gl_createFramebuffer(int width, int height)
{
    if (gFrameBufferCount < MAX_FRAMEBUFFERS) {
        FrameBuffer framebuffer = {};
        framebuffer.width = width;
        framebuffer.height = height;
        glGenFramebuffers(1, &framebuffer.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
        
        glGenTextures(1, &framebuffer.colorTexture);
        glBindTexture(GL_TEXTURE_2D, framebuffer.colorTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glGenTextures(1, &framebuffer.depthTexture);
        glBindTexture(GL_TEXTURE_2D, framebuffer.depthTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);
        
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebuffer.colorTexture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebuffer.depthTexture, 0);
        
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        gFrameBuffers[gFrameBufferCount] = framebuffer;
        gFrameBufferCount++;
        return gFrameBufferCount-1;
    }
    return -1;
}

void gl_bindFramebuffer(int handle)
{
    FrameBuffer framebuffer = gFrameBuffers[handle];
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
    glViewport(0, 0, framebuffer.width, framebuffer.height);
    
    // this is dumb. in C99 the following code would be valid:
    // glClearBufferfv(GL_COLOR, 0, &(0,1,0,1));
    float white[] = { 1.f, 1.f, 1.f, 0.f };
    float one[] = { 1.f, 1.f, 1.f, 1.f };
    glClearBufferfv(GL_COLOR, 0, white);
    glClearBufferfv(GL_DEPTH, 0, one);
}

void gl_defaultFramebuffer(int handle)
{
    FrameBuffer framebuffer = gFrameBuffers[handle];
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Rect windowSize = gPlatformAPI->getWindowDimensions();
    glViewport(0, 0, windowSize.width, windowSize.height);
    glBindTexture(GL_TEXTURE_2D, framebuffer.colorTexture);
    glUseProgram(gShaders[SHADER_FRAMEBUFFER].program);
    GLint texture_location = glGetUniformLocation(gShaders[SHADER_FRAMEBUFFER].program, "texture");
    glUniform1i(texture_location, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// init the struct
refexport_t GetRefAPI(PlatformAPI* platform_api)
{
    gPlatformAPI = platform_api;
    refexport_t re;
    re.init = win32_initGL;
    re.setViewport = glSetViewport;
    //re.setProjection = glSetProjection;
    re.registerMesh = gl_RegisterMesh;
    re.notify = gl_notify;
    re.createTexture = createTexture;
    re.endFrame = gl_endFrame;
    re.createTextureFromBitmap = createTextureFromBitmap;
    re.createFramebuffer = gl_createFramebuffer;
    re.bindFramebuffer = gl_bindFramebuffer;
    re.defaultFramebuffer = gl_defaultFramebuffer;
    return re;
}



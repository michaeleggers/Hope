#include "ogl_render.h"


struct Window
{
    float x, y;
    float width, height;
};

struct Rect
{
    int width, height;
};

struct Shader
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint shaderProgram;
};

struct Texture
{
    GLuint texture_id;
    int width, height;
};

struct Quad
{
    GLuint vao;
    GLfloat vertexPoints[18];
    GLfloat textureUVs[12];
};

struct Sprite
{
    Shader shader;
    Texture texture;
    Quad mesh;
};

struct Spritesheet
{
    Window windows[256]; // max 256 frames
};

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

Texture create_texture(char const * texture_file, GLuint textureslot)
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
    // TODO(Michael): pull out later -> this is global texture state!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    return Texture { tex, x, y };
}

Quad create_quad()
{
    Quad mesh;
    // some test OGL data
    GLfloat points[] = {
        -5.0f, 5.0f, 0.0f,
        5.0f, 5.0f, 0.0f,
        5.0f, -5.0f, 0.0f,
        5.0f, -5.0f, 0.0f,
        -5.0f, -5.0f, 0.0f,
        -5.0f, 5.0f, 0.0f
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

void set_ortho(int width, int height)
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
    
    GLint shaderID;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderID);
    // TODO(Michael): glUseProgram should be called. Maybe pass shader to function?
    GLuint ortho_loc = glGetUniformLocation(shaderID, "ortho");
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
    Texture texture = create_texture(file, GL_TEXTURE0);
    
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

void draw_frame(Sprite * sprite, Spritesheet * spritesheet, int frame)
{
    Window window = spritesheet->windows[frame];
    
    int thing_loc = glGetUniformLocation(sprite->shader.shaderProgram, "thing");
    glUniform4f(thing_loc,
                //0.067f, 0.1f,
                window.width, window.height,
                // offsets
                window.x, window.y
                ); // use active texture
    
    draw_sprite(sprite, 0, 0);
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
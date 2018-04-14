#include "render.h"


struct Shader
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint shaderProgram;
};

struct Texture
{
    GLuint texture_id;
};

// TODO(Michael): put this elsewhere
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

Texture create_texture(char const * texture_file)
{
    // STBI image loading
    int x, y, n;
    unsigned char * image_data = stbi_load(texture_file, &x, &y, &n, 4);
    
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    return Texture { tex };
}
#ifndef RENDER_H
#define RENDER_H


#include <stdio.h>

#include <windows.h>

#include <gl/gl.h>
#include <gl/glu.h>

#include "openglext/glext.h"
#include "openglext/wglext.h"

#include "helper.h"
#include "Mathx.h"

#include "ref.h"
#include "scene.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define global_var static;

#define MAX_SPRITESHEET_WINDOWS 256

enum ShaderType
{
    SPRITE,
    SPRITE_SHEET,
    STANDARD_MESH,
    MAX_SHADERS
};

struct Window
{
    float x, y;
    float width, height;
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
    char name[64];
    int width, height;
};

struct Quad
{
    GLuint vao;
    GLfloat vertexPoints[18];
    GLfloat textureUVs[12];
};

struct GPUMeshData
{
    GLuint vao;
    int vertexCount;
};

struct Spritesheet
{
    Window windows[MAX_SPRITESHEET_WINDOWS]; // max 256 frames
    int freeSlot;
};

struct Sprite
{
    Shader shader;
    Texture * texture;
    Quad mesh;
    int width, height; // width, height of sprite. init to image width, height
    int x, y, z; // pos (where to draw, not necessarily logical pos of entity)
    char name[64];
    Window windows[MAX_SPRITESHEET_WINDOWS];
    int freeWindowIndex;
};

struct RenderState
{
    HWND* windowHandle;
    HDC deviceContext;
    HGLRC renderContext;
};

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

Rect get_window_dimensions();
void printGlErrMsg();
void check_shader_error(GLuint shader);
void l_drawTriangle();
Shader create_shader(char const * vs_file, char const * fs_file,
                     char ** attribLocations,
                     int numAttribs);
Texture * createTexture(unsigned char * imageData, int width, int height);
Texture create_texture(char const * texture_file);
Texture create_texture(Background * bg);
Quad create_quad();
void set_ortho(int width, int height, Shader * shader, char * location);
void set_model(GLfloat modelMatrix[], Shader * shader, char * location);

Spritesheet create_spritesheet(Texture * texture,
                               int xOffset, int yOffset,
                               int width, int height,
                               int numFrames);

Sprite create_sprite(char * spriteID, char * filename, unsigned char * imageData, 
                     int textureWidth, int textureHeight,
                     int xOffset, int yOffset,
                     int width, int height,
                     Shader * shader);

void draw_frame(Sprite * sprite, Spritesheet * spritesheet, int frame,
                float x, float y, float scaleX, float scaleY);

void gl_renderFrame(Sprite* sprites, int spriteCount);
void gl_renderMesh(GPUMeshData* meshData);
void createFallbackTexture(Texture * texture);
void createWindow(int xOffset, int yOffset, int width, int height);
void setUniformMat4fv(Shader * shader, char * location, GLfloat mat4data[]);

// exported stuff
int win32_initGL(HWND* windowHandle, WNDCLASS* windowClass);
void glLoadRooms(Room* room);
void glRender(Room * room);
void glSetViewport(int xLeft, int yBottom, int width, int height);
void glSetProjection(Projection_t projType);
Sprite * glRegisterSprite(char * spriteID, char * filename, unsigned char * imageData,
                          int textureWidth, int textureHeight,
                          int xOffset, int yOffset,
                          int width, int height);
Mesh gl_RegisterMesh(float * vertices, int count);
void gl_renderFrame(Refdef * refdef);

// exported functions
extern "C"
{
    refexport_t GetRefAPI();
}

#endif
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
#include "game.h"

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

enum ShaderType;
struct Window;
struct Shader;
struct Texture;
struct Quad;
struct Sprite;
struct Spritesheet;

Rect get_window_dimensions();
void printGlErrMsg();
void check_shader_error(GLuint shader);
void l_drawTriangle();
Shader create_shader(char const * vs_file, char const * fs_file);
Texture create_texture(char const * texture_file);
Quad create_quad();
void set_ortho(int width, int height, Shader * shader);

Spritesheet create_spritesheet(Texture * texture,
                               int width, int height,
                               int numFrames);

void draw_frame(Sprite * sprite, Spritesheet * spritesheet, int frame,
                float x, float y, float scaleX, float scaleY);

void gl_renderFrame(Room* room);

// exported functions
extern "C"
{
    refexport_t GetRefAPI();
}

#endif
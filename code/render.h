#ifndef RENDER_H
#define RENDER_H

struct Shader;
struct Texture;

char* load_text(char const * filename);
void printGlErrMsg();
void check_shader_error(GLuint shader);
void l_drawTriangle();
Shader create_shader(char const * vs_file, char const * fs_file);
Texture create_texture(char const * texture_file);

#endif
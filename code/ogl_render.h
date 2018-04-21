#ifndef RENDER_H
#define RENDER_H

struct Window;
struct Rect;
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
void set_ortho(int width, int height);
Spritesheet create_spritesheet(Texture * texture,
                               int width, int height,
                               int numFrames);
void draw_frame(Sprite * sprite, Spritesheet * spritesheet, int frame);

#endif
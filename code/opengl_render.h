
#ifndef OPENGL_RENDER_H
#define OPENGL_RENDER_H

struct FrameBuffer
{
    GLuint fbo;
    GLuint colorTexture;
    GLuint depthTexture;
    int width, height;
};


int gl_createFramebuffer(int width, int height);
void gl_bindFramebuffer(int handle);
void gl_defaultFramebuffer();

#endif
#ifndef COMMON_RENDER_H
#define COMMON_RENDER_H


enum Projection_t
{
    ORTHO,
    PERSPECTIVE
};


float gModelMatrix[] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

#endif


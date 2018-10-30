#ifndef COMMON_RENDER_H
#define COMMON_RENDER_H


//
// serves as 'glue' between renderer and game
//

enum Projection_t
{
    ORTHO,
    PERSPECTIVE
};

struct EntityResource
{
    void * modelData;
};

#endif


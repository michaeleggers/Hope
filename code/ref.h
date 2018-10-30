#ifndef REF_H
#define REF_H

// interface for renderer. To be implemented in DLL (OpenGL, DirectX, ...)
// for the moment not platform 

#include "scene.h"
#include "common_render.h"

struct refexport_t
{
    int (*init)(HWND* windowHandle, WNDCLASS* windowClass);
    void (*loadRooms)(Room* room);
    void (*render)(Room* room);
    void (*setViewport)(int xLeft, int yBottom, int width, int height);
    void (*setProjection)(Projection_t projType);
    void * (*registerSprite)(char const * filename);
};

typedef refexport_t (*GetRefAPI_t)();

#endif
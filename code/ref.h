#ifndef REF_H
#define REF_H

// interface for renderer. To be implemented in DLL (OpenGL, DirectX, ...)
// for the moment not platform 

#include "scene.h"

struct refexport_t
{
    int (*init)(HWND* windowHandle, WNDCLASS* windowClass);
    void (*loadRooms)(Room* room);
    void (*render)();
};

typedef refexport_t (*GetRefAPI_t)();

#endif
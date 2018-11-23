#ifndef REF_H
#define REF_H

// interface for renderer. To be implemented in DLL (OpenGL, DirectX, ...)
// for the moment not platform independent

#include "common_render.h"


#define MAX_SPRITES 512
#define MAX_TEXTURES 256

struct Sprite;

struct Entity
{
    Sprite * sprite;
};

struct Refdef
{
    int numEntities;
    Entity * entities;
};

struct refexport_t
{
    int (*init)(HWND* windowHandle, WNDCLASS* windowClass);
    //void (*loadRooms)(Room* room);
    //void (*render)(Room* room);
    void (*setViewport)(int xLeft, int yBottom, int width, int height);
    void (*setProjection)(Projection_t projType);
    Sprite * (*registerSprite)(char * filename, unsigned char * imageData, 
                               int textureWidth, int textureHeight,
                               int xOffset, int yOffset,
                               int width, int height);
    void (*renderFrame)(Refdef * refdef);
};

typedef refexport_t (*GetRefAPI_t)();

#endif
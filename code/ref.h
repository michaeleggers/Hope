#ifndef REF_H
#define REF_H

// interface for renderer. To be implemented in DLL (OpenGL, DirectX, ...)
// for the moment not platform independent

#include "common_render.h"


#define MAX_SPRITES 512
#define MAX_TEXTURES 256
#define MAX_MESHES 1024

struct Sprite;

enum EntityType
{
    SPRITE_E,
    MESH_E
};


struct Mesh
{
    void * meshHandle;
};

struct Entity
{
    EntityType entityType;
    float xPos, yPos, zPos;
    // TODO(Michael): figure out what the difference between union name at beginning vs end is!
    union
    {
        Sprite * sprite;
        Mesh mesh;
    } EntityDescriptor;
};

struct Refdef
{
    int numSpriteEntities;
    Entity * spriteEntities;
    int numMeshEntities;
    Entity * meshEntities;
};

struct refexport_t
{
    int (*init)(HWND* windowHandle, WNDCLASS* windowClass);
    //void (*loadRooms)(Room* room);
    //void (*render)(Room* room);
    void (*setViewport)(int xLeft, int yBottom, int width, int height);
    void (*setProjection)(Projection_t projType);
    Sprite * (*registerSprite)(char * spriteID, char * filename, unsigned char * imageData, 
                               int textureWidth, int textureHeight,
                               int xOffset, int yOffset,
                               int width, int height);
    Mesh (*registerMesh)(float * vertices, int count);
    void (*renderFrame)(Refdef * refdef);
    void (*notify)(void);
};

typedef refexport_t (*GetRefAPI_t)();

#endif
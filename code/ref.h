#ifndef REF_H
#define REF_H

// interface for renderer. To be implemented in DLL (OpenGL, DirectX, ...)
// for the moment not platform independent

#include "common_render.h"


#define MAX_SPRITES 512
#define MAX_TEXTURES 256
#define MAX_MESHES 1024


enum EntityType
{
    SPRITE_E,
    MESH_E
};

struct Transform
{
    float modelMat[16];
    float xPos, yPos, zPos; // translate
    float xScale, yScale, zScale;
    float xRot, yRot, zRot;
};

struct Sprite
{
    void * spriteHandle;
};

struct Mesh
{
    void * meshHandle;
};

struct Entity
{
    EntityType entityType;
    Transform transform;
    // TODO(Michael): figure out what the difference between union name at beginning vs end is!
    union
    {
        Sprite sprite;
        Mesh mesh;
    };
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
    Sprite (*registerSprite)(char * spriteID, char * filename, unsigned char * imageData,
                             int textureWidth, int textureHeight,
                             int xOffset, int yOffset,
                             int width, int height);
    Mesh (*registerMesh)(float * vertices, int count);
    void (*renderFrame)(Refdef * refdef);
    void (*notify)(void);
};

typedef refexport_t (*GetRefAPI_t)();

#endif
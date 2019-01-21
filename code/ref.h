#ifndef REF_H
#define REF_H

// interface for renderer. To be implemented in DLL (OpenGL, DirectX, ...)
// for the moment not platform independent

#include "platform.h"
#include "common_render.h"


#define MAX_SPRITES 512
#define MAX_TEXTURES 256
#define MAX_MESHES 1024


enum EntityType
{
    SPRITE_E,
    MESH_E,
    PLAYER_E
};

struct Transform
{
    float modelMat[16];
    float xPos, yPos, zPos; // translate
    float xScale, yScale, zScale;
    float xRot, yRot, zRot;
    float angle; // in degrees
};

struct Sprite
{
    void * spriteHandle;
    int currentFrame;
    int frameCount;
};

struct v3
{
    float x, y, z;
};

struct v2
{
    float x, y;
};

v3 v3add(v3 lhs, v3 rhs)
{
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z
    };
};

float v3length(v3 v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

float v2length(v2 v)
{
    return sqrt(v.x*v.x + v.y*v.y);
}

v3 normalize(v3 in)
{
    float length = v3length(in);
    return {
        in.x / length,
        in.y / length,
        in.z / length
    };
}

struct Vertex
{
    v3 position;
    v3 normal;
    v2 UVs;
};

struct Mesh
{
    void * meshHandle;
    int vertexCount;
    Vertex VVVNNNST[1024];
};

struct Entity
{
    EntityType entityType;
    Transform transform;
    union
    {
        Sprite sprite;
        Mesh mesh;
    };
    v3 velocity;
    v2 speed;
};

struct Refdef
{
    int numSpriteEntities;
    Entity * spriteEntities;
    int numMeshEntities;
    Entity * meshEntities;
    Entity * playerEntity;
};

struct refexport_t
{
    int (*init)(HWND* windowHandle, WNDCLASS* windowClass);
    //void (*loadRooms)(Room* room);
    //void (*render)(Room* room);
    void (*setViewport)(int xLeft, int yBottom, int width, int height);
    void (*setProjection)(Projection_t projType);
    Sprite (*registerSprite)(char * filename, unsigned char * imageData,
                             int textureWidth, int textureHeight,
                             int xOffset, int yOffset,
                             int width, int height);
    void* (*registerMesh)(Vertex * vertices, int count);
    void (*renderFrame)(Refdef * refdef);
    void (*notify)(void);
    void (*addSpriteFrame)(Sprite * sprite, int xOffset, int yOffset, int width, int height);
    int (*addTwoNumbers)(int a, int b); // dummy
};

typedef refexport_t (*GetRefAPI_t)(PlatformAPI* platform_api);

#endif
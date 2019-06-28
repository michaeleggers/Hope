#ifndef REF_H
#define REF_H

// interface for renderer. To be implemented in DLL (OpenGL, DirectX, ...)
// for the moment not platform independent

#include "hope_math.h"
#include "hope_math.c"
#include "platform.h"
#include "common_render.h"


#define MAX_SPRITES             512
#define MAX_TEXTURES            256
#define MAX_MESHES              1024
#define MAX_SPRITESHEET_WINDOWS 256

struct Vertex
{
    v3 position;
    v3 normal;
    v2 UVs;
};

// New Rendering API
// ---------------------------------------
enum RenderCommandType
{
    RENDER_CMD_NONE,
    RENDER_CMD_QUAD,
    RENDER_CMD_TEXT,
    RENDER_CMD_LINE
};

// Every draw call issues a RenderCommand.
// Each RenderCommand essentially is one draw-call.
struct RenderCommand
{
    RenderCommandType type;
    uint32_t          textureID;
    // Vertex           *vtxBufferPos;
    // uint16_t         *idxBufferPos;
    uint32_t          idxBufferOffset;
    uint16_t          vtxBufferOffset;
    
    // render cmd specific
    union
    {
        struct
        {
            uint32_t quadCount;
            v3 tint;
        };
        
        struct
        {
            uint32_t lineCount;
            v3 tint;
            float thickness;
        };
    };
};

// gets passed to the rendering API (eg OpenGL),
// which then goes through the renderCmds and issues draw calls.
struct DrawList
{
    Vertex           *vtxBuffer;
    uint32_t          vtxCount;
    uint16_t         *idxBuffer;
    uint32_t          idxCount;
    uint32_t          highestIndex;
    RenderCommand     renderCmds[256];
    uint32_t          freeIndex;
    RenderCommand    *prevRenderCmd;
    
    // these are just crutches for offsetting into the idxBuffer
    // if the previous render command was the same.
    // These do _not_ tell you how many quads/lines in total were being pushed!!!
    uint32_t          quadCount;
    uint32_t          lineCount;
};

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

struct Texture
{
    uint32_t texture_id;
    char name[64];
    uint32_t width, height;
};

struct Sprite
{
    void * spriteHandle;
    int currentFrame;
    int frameCount;
};

struct Window
{
    float x, y;
    float width, height;
    int intWidth, intHeight;
};

struct SpriteSheet
{
    Texture * texture;
    int currentFrame;
    int frameCount;
    int width, height; // width, height of sprite. init to image width, height
    char name[64];
    Window windows[MAX_SPRITESHEET_WINDOWS];
    int freeWindowIndex;
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
    void (*renderText)(char * text, int xPos, int yPos, float xScale, float yScale, Sprite * sprite);
    Texture* (*createTexture)(char * filename, unsigned char * imageData, int width, int height);
    void (*endFrame)(DrawList* drawList);
};

typedef refexport_t (*GetRefAPI_t)(PlatformAPI* platform_api);

#endif
#ifndef REF_H
#define REF_H

// interface for renderer. To be implemented in DLL (OpenGL, DirectX, ...)
// for the moment not platform independent
#include <stdint.h>

#include "hope_math.h"
#include "hope_math.c"
#include "platform.h"
#include "stb_truetype.h"

#define MAX_SPRITES              512
#define MAX_TEXTURES             256
#define MAX_MESHES               1024
#define MAX_SPRITESHEET_WINDOWS  256
#define MAX_FRAMEBUFFERS         3
#define MAX_SPRITESHEETS         32

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
    RENDER_CMD_TEXTURED_RECT,
    RENDER_CMD_TTF,
    RENDER_CMD_TEXT,
    RENDER_CMD_LINE,
    RENDER_CMD_FILLED_RECT,
    RENDER_CMD_SET_FRAMEBUFFER,
    RENDER_CMD_SET_DEFAULT_FRAMEBUFFER,
    RENDER_CMD_DRAW_FROM_FRAMEBUFFER
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
    mat4 projectionMatrix;
    
    // render cmd specific
    union
    {
        struct
        {
            uint32_t quadCount;
            uint32_t alphaColor;
            v3 tint;
        };
        
        struct
        {
            uint32_t lineCount;
            v3 tint;
            float thickness;
        };
        
        struct
        {
            int framebufferHandle;
            float pos_x;
            float pos_y;
            float scale_x;
            float scale_y;
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

enum SpriteSequenceType
{
    WALK_FRONT,
    WALK_BACK,
    WALK_SIDE_LEFT,
    WALK_SIDE_RIGHT,
    FIGHT_READY,
    FIGHT_WALK_RIGHT,
    FIGHT_WALK_LEFT,
    PUNCH_HIGH,
    PUNCH_MID,
    PUNCH_LOW,
    HIT_HIGH,
    HIT_MID,
    KO,
    DEAD,
    MAX_SEQUENCE_TYPES
};

struct SpriteSequence
{
    SpriteSequenceType sequenceType;
    char name[64];
    int start, end;
    bool flipHorizontal;
    bool flipVertical;
};

struct SpriteSheet
{
    Texture * texture;
    int frameCount;
    int width, height; // width, height of sprite. init to image width, height
    char name[64];
    Window windows[MAX_SPRITESHEET_WINDOWS];
    int freeWindowIndex;
    SpriteSequence sequences[MAX_SEQUENCE_TYPES];
};

struct Mesh
{
    void * meshHandle;
    int vertexCount;
    Vertex VVVNNNST[1024];
};

enum Projection_t
{
    ORTHO,
    PERSPECTIVE
};

struct refexport_t
{
    int (*init)(HWND* windowHandle, WNDCLASS* windowClass);
    void (*setViewport)(int xLeft, int yBottom, int width, int height);
    //void (*setProjection)(Projection_t projType);
    void* (*registerMesh)(Vertex * vertices, int count);
    void (*notify)(void);
    Texture* (*createTexture)(char * filename, unsigned char * imageData, int width, int height);
    Texture * (*createTextureFromBitmap)(unsigned char * bmp, int width, int height);
    void (*start_frame)(void);
    void (*endFrame)(DrawList* drawList);
    int (*createFramebuffer)(int width, int height);
    void (*bindFramebuffer)(int handle);
    void (*defaultFramebuffer)();
    void (*set_ortho_matrix)(float orthoMatrix[]);
    int (*get_framebuffer_width)(int fbHandle);
    int (*get_framebuffer_height)(int fbHandle);
    void (*bind_framebuffer)(int framebuffer_handle);
    void (*bind_default_framebuffer)(void);
    void (*render_from_framebuffer)(int framebuffer_handle, DrawList * draw_list);
    void (*flush)(DrawList * drawList);
};

// creating function pointer type:
// https://de.wikipedia.org/wiki/Typedef
typedef refexport_t (*GetRefAPI_t)(PlatformAPI* platform_api);

#endif
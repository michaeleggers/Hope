#ifndef GAME_H
#define GAME_H

#include "hope_math.h"
#include "platform.h"
#include "ref.h"
#include "stb_truetype.h"
#include "input.h"

struct Quad
{
    float x0, y0, s0, t0;
    float x1, y1, s1, t1;
};

enum EntityFacingDirection
{
    FACING_RIGHT,
    FACING_LEFT
};

enum EntityState
{
    ENTITY_STATE_WALK_FRONT,
    ENTITY_STATE_WALK_BACK,
    ENTITY_STATE_WALK_SIDE_LEFT,
    ENTITY_STATE_WALK_SIDE_RIGHT,
    ENTITY_STATE_FIGHT_READY,
    ENTITY_STATE_FIGHT_WALK_RIGHT,
    ENTITY_STATE_FIGHT_WALK_LEFT,
    ENTITY_STATE_PUNCH_HIGH,
    ENTITY_STATE_PUNCH_MID,
    ENTITY_STATE_PUNCH_LOW,
    ENTITY_STATE_HIT_HIGH,
    ENTITY_STATE_HIT_MID,
    ENTITY_STATE_KO,
    ENTITY_STATE_DEAD,
    MAX_ENTITY_STATES
};

enum EntityType
{
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_FATGUY
};

struct Entity
{
    int spritesheet;
    float xPos, yPos;
    float cooldown;
    float cooldownInit;
    float frameTime;
    int hitpoints;
    EntityFacingDirection facingDirection;
    EntityState state;
    SpriteSequenceType currentSequence;
    int currentFrame;
    v2 direction;
    float velocity;
};

// ---------------------------------------

/* local to game.cpp */
Sprite * loadSprite(refexport_t* re,
                    char * textureID,
                    char * texturename,
                    int textureWidth, int textureHeight,
                    int xOffset, int yOffset,
                    int spriteWidth, int spriteHeight);
Mesh loadMeshFromOBJ(char * objfile);
int extractIndex(char * input, int length, int * outLength);

/* services from platform layer */
void update_messages();
bool fileExists(char const * file);

/* services to platform layer */
void game_init(PlatformAPI* platform_api, InputDevice* input_device, refexport_t* re);
void game_update_and_render(float dt, InputDevice* inputDevice, refexport_t* re);

#endif GAME_H

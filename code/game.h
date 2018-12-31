#ifndef GAME_H
#define GAME_H

#include "ref.h"
#include "scene.h"

enum Keycode
{
    ARROW_LEFT,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    MAX_KEYCODES = 255
};

struct Controller
{
    int dpadUp;
    int dpadDown;
    int dpadLeft;
    int dpadRight;
    
    int dpadA;
    int dpadB;
    int dpadX;
    int dpadY;
    
    int connected;
};

struct Keyboard
{
    int keycodes[256];
    int prevKeycodes[256];
};

/* local to game.cpp */
Background loadBackground(char * file);
void addEntity(Entity * entity);
Sprite * loadSprite(refexport_t* re,
                    char * textureID,
                    char * texturename,
                    int textureWidth, int textureHeight,
                    int xOffset, int yOffset,
                    int spriteWidth, int spriteHeight);

/* services from platform layer */
void update_messages();
bool fileExists(char const * file);

/* services to platform layer */
void game_init(refexport_t* re);
void game_update_and_render(float dt, Controller* controller, Keyboard* keyboard, refexport_t* re);

#endif GAME_H

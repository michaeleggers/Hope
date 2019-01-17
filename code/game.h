#ifndef GAME_H
#define GAME_H

#include "platform.h"
#include "ref.h"
#include "scene.h"

enum Keycode
{
    ARROW_LEFT,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    LETTER_A,
    MAX_KEYCODES = 255
};

enum ControllerKeycode
{
    DPAD_LEFT,
    DPAD_RIGHT,
    DPAD_UP,
    DPAD_DOWN,
    DPAD_A,
    DPAD_B,
    DPAD_X,
    DPAD_Y,
    NONE
};

struct GenericKeycode
{
    union
    {
        Keycode keyboardKeycode;
        ControllerKeycode controllerKeycode;
    };
};

enum DeviceType
{
    KEYBOARD,
    CONTROLLER
};

struct Controller
{
    int keycodes[256];
    int prevKeycodes[256];
    int connected;
};

struct Keyboard
{
    int keycodes[256];
    int prevKeycodes[256];
};

struct InputDevice
{
    DeviceType deviceType;
    Keyboard * keyboard;
    Controller * controller;
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
Mesh loadMeshFromOBJ(char * objfile);
int extractIndex(char * input, int length, int * outLength);

/* services from platform layer */
void update_messages();
bool fileExists(char const * file);

/* services to platform layer */
void game_init(PlatformAPI* platform_api, refexport_t* re);
void game_update_and_render(float dt, InputDevice* inputDevice, refexport_t* re);

#endif GAME_H

#ifndef GAME_H
#define GAME_H

#include "hope_math.h"
#include "platform.h"
#include "ref.h"
#include "scene.h"
#include "stb_truetype.h"

// Input
// ---------------------------------------
enum Keycode
{
    ARROW_LEFT,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    LETTER_A,
    ESCAPE,
    KEYBOARD_NONE,
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
    DPAD_NONE
};

enum MouseKeycode
{
    LBUTTON_DOWN,
    LBUTTON_UP,
    RBUTTON_DOWN,
    MBUTTON_DOWN,
    WHEEL_UP,
    WHEEL_DOWN
};

enum GameInput
{
    FACE_LEFT,
    FACE_RIGHT,
    PUNCH
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
    CONTROLLER,
    MOUSE
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

struct Mouse
{
    // NOTE(Michael): not sure why we would need that many keycodes...
    int keycodes[256];
    int prevKeycodes[256];
    short x, y;
};

struct InputDevice
{
    DeviceType deviceType;
    Keyboard * keyboard;
    Controller * controller;
    Mouse * mouse;
};

struct Quad
{
    float x0, y0, s0, t0;
    float x1, y1, s1, t1;
};

// ---------------------------------------

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
Keycode toKeyboardKeycode(GameInput gameInput);

/* services from platform layer */
void update_messages();
bool fileExists(char const * file);

/* services to platform layer */
void game_init(PlatformAPI* platform_api, InputDevice* input_device, refexport_t* re);
void game_update_and_render(float dt, InputDevice* inputDevice, refexport_t* re);

#endif GAME_H

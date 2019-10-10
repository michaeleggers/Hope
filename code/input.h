
#ifndef INPUT_H
#define INPUT_H

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
    Controller * controller1;
    Controller * controller2;
    Mouse * mouse;
};

void init_input(InputDevice * device);
ControllerKeycode toControllerKeycode(GameInput gameInput);
Keycode toKeyboardKeycode(GameInput gameInput);
bool keyPressed(GameInput gameInput);
bool keyPressed(Controller * controller, ControllerKeycode controllerKeycode);
bool keyDown(GameInput gameInput);
bool keyDown(Keycode keycode);
bool keyDown(Controller * controller, ControllerKeycode keycode);
bool keyUp(GameInput gameInput);
bool leftMouseButtonDown();
bool leftMouseButtonPressed();
int get_mouse_x();
int get_mouse_y();

#endif
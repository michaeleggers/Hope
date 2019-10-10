
#include "input.h"

static InputDevice * gInputDevice;

void init_input(InputDevice * device)
{
    gInputDevice = device;
}

ControllerKeycode toControllerKeycode(GameInput gameInput)
{
    switch (gameInput)
    {
        case FACE_LEFT  : return DPAD_LEFT; break;
        case FACE_RIGHT : return DPAD_RIGHT; break;
        case PUNCH      : return DPAD_A; break;
        default         : return DPAD_NONE; break;
    }
}

Keycode toKeyboardKeycode(GameInput gameInput)
{
    switch (gameInput)
    {
        case FACE_LEFT  : return ARROW_LEFT; break;
        case FACE_RIGHT : return ARROW_RIGHT; break;
        case PUNCH      : return ARROW_UP; break;
        default         : return KEYBOARD_NONE;
    }
}

#if 0
bool keyPressed(GameInput gameInput)
{
    switch (gInputDevice->gInputDeviceType)
    {
        case KEYBOARD:
        {
            Keycode keycode = toKeyboardKeycode(gameInput);
            Keyboard* keyboard = gInputDevice->keyboard;
            if (keyboard->keycodes[keycode] && !keyboard->prevKeycodes[keycode])
            {
                keyboard->prevKeycodes[keycode] = 1;
                return true;
            }
            else if (!keyboard->keycodes[keycode] && keyboard->prevKeycodes[keycode])
            {
                keyboard->prevKeycodes[keycode] = 0;
                return false;
            }
            return false;
        }
        break;
        
        case CONTROLLER:
        {
            ControllerKeycode controllerKeycode = toControllerKeycode(gameInput);
            if (controllerKeycode == DPAD_NONE) return false;
            Controller* controller = gInputDevice->controller;
            if (controller->keycodes[controllerKeycode] && !controller->prevKeycodes[controllerKeycode])
            {
                controller->prevKeycodes[controllerKeycode] = 1;
                return true;
            }
            else if (!controller->keycodes[controllerKeycode] && controller->prevKeycodes[controllerKeycode])
            {
                controller->prevKeycodes[controllerKeycode] = 0;
                return false;
            }
            return false;
        }
        break;
        
        default:
        return false;
    }
}
#endif

bool keyPressed(Controller * controller, ControllerKeycode controllerKeycode)
{
    if (controllerKeycode == DPAD_NONE) return false;
    if (controller->keycodes[controllerKeycode] && !controller->prevKeycodes[controllerKeycode])
    {
        controller->prevKeycodes[controllerKeycode] = 1;
        return true;
    }
    else if (!controller->keycodes[controllerKeycode] && controller->prevKeycodes[controllerKeycode])
    {
        controller->prevKeycodes[controllerKeycode] = 0;
        return false;
    }
    return false;
}

#if 0
bool keyDown(GameInput gameInput)
{
    switch (gInputDevice->gInputDeviceType)
    {
        case KEYBOARD:
        {
            Keycode keycode = toKeyboardKeycode(gameInput);
            Keyboard* keyboard = gInputDevice->keyboard;
            if (keyboard->keycodes[keycode])
                return true;
            return false;
        }
        break;
        
        case CONTROLLER:
        {
            ControllerKeycode controllerKeycode = toControllerKeycode(gameInput);
            if (controllerKeycode == DPAD_NONE) return false;
            Controller* controller = gInputDevice->controller;
            if (controller->keycodes[controllerKeycode])
                return true;
            return false;
        }
        break;
        
        default: return false;
    }
}
#endif

bool keyDown(Keycode keycode)
{
    switch (gInputDevice->deviceType)
    {
        case KEYBOARD:
        {
            Keyboard* keyboard = gInputDevice->keyboard;
            if (keyboard->keycodes[keycode])
                return true;
            return false;
        }
        break;
        
        default: return false;
    }
}

bool keyDown(Controller * controller, ControllerKeycode keycode)
{
    if (controller->keycodes[keycode])
        return true;
    return false;
}

#if 0
bool keyUp(GameInput gameInput)
{
    switch (gInputDevice->gInputDeviceType)
    {
        case KEYBOARD:
        {
            Keycode keycode = toKeyboardKeycode(gameInput);
            Keyboard* keyboard = gInputDevice->keyboard;
            if (!keyboard->keycodes[keycode] && keyboard->prevKeycodes[keycode])
            {
                keyboard->prevKeycodes[keycode] = 0;
                return true;
            }
            else if (keyboard->keycodes[keycode] && !keyboard->prevKeycodes[keycode])
            {
                keyboard->prevKeycodes[keycode] = 1;
                return false;
            }
            return false;
        }
        break;
        
        case CONTROLLER:
        {
            ControllerKeycode controllerKeycode = toControllerKeycode(gameInput);
            if (controllerKeycode == DPAD_NONE) return false;
            Controller* controller = gInputDevice->controller;
            if (!controller->keycodes[controllerKeycode] && controller->prevKeycodes[controllerKeycode])
            {
                controller->prevKeycodes[controllerKeycode] = 0;
                return true;
            }
            else if (controller->keycodes[controllerKeycode] && !controller->prevKeycodes[controllerKeycode])
            {
                controller->prevKeycodes[controllerKeycode] = 1;
                return false;
            }
            return false;
        }
        break;
        
        default:
        return false;
    }
}
#endif

bool leftMouseButtonDown()
{
    Mouse * mouse = gInputDevice->mouse;
    if (mouse->keycodes[LBUTTON_DOWN])
        return true;
    return false;
}

int get_mouse_x()
{
    return gInputDevice->mouse->x;
}

int get_mouse_y()
{
    return gInputDevice->mouse->y;
}

bool leftMouseButtonPressed()
{
    Mouse * mouse = gInputDevice->mouse;
#if 1    
    if (mouse->keycodes[LBUTTON_DOWN] &&
        !mouse->prevKeycodes[LBUTTON_DOWN])
    {
        mouse->prevKeycodes[LBUTTON_DOWN] = 1;
        return false;
    }
    else if (!mouse->keycodes[LBUTTON_DOWN] &&
             mouse->prevKeycodes[LBUTTON_DOWN])
    {
        mouse->prevKeycodes[LBUTTON_DOWN] = 0;
        return true;
    }
    return false;
#endif
}



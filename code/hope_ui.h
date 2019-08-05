#ifndef HOPE_UI_H
#define HOPE_UI_H

#define GUID (__LINE__)
#define HOPE_UI_MAX_BUTTONS    256


#define GET_WINDOW_WIDTH(name) int name(void)
typedef GET_WINDOW_WIDTH(ui_get_window_width);
#define GET_WINDOW_HEIGHT(name) int name(void)
typedef GET_WINDOW_HEIGHT(ui_get_window_height);

#define GET_MOUSE_X(name) int name(void)
typedef GET_MOUSE_X(ui_get_mouse_x);
#define GET_MOUSE_Y(name) int name(void)
typedef GET_MOUSE_Y(ui_get_mouse_y);

#define LEFT_MOUSE_BUTTON_DOWN(name) bool name(void)
typedef LEFT_MOUSE_BUTTON_DOWN(ui_left_mouse_button_down);
#define LEFT_MOUSE_BUTTON_PRESSED(name) bool name(void);
typedef LEFT_MOUSE_BUTTON_PRESSED(ui_left_mouse_button_pressed);
struct HopeUIBinding
{
    ui_get_window_width  * getWindowWidth;
    ui_get_window_height * getWindowHeight;
    ui_get_mouse_x * getMouseX;
    ui_get_mouse_y * getMouseY;
    ui_left_mouse_button_down * leftMouseButtonDown;
    ui_left_mouse_button_pressed * leftMouseButtonPressed;
};

struct HopeUIColor
{
    float r, g, b;
};

struct HopeUIRect
{
    int x0, y0;
    int x1, y1;
};

struct HopeUIButton
{
    HopeUIRect rect;
    HopeUIColor color;
    char text[32];
};

struct HopeUIWindow
{
    HopeUIRect rect;
};

struct HopeUIDrawList
{
    HopeUIWindow windows[32];
    HopeUIButton buttons[32]; // later: more complex structures of windows, buttons, etc.
    int buttonCount = 0;
    int windowCount = 0;
    float fontSize;
};

struct HopeUIID
{
    int intID;
};

struct HopeUIContext
{
    HopeUIBinding * binding;
    HopeUIID hotID;
    HopeUIID prevHotID;
    HopeUIID activeID;
    HopeUIID prevActiveID;
    HopeUIWindow * activeWindow;
    HopeUIID windowID;
    bool mouseWasDown = false;
    bool mouseDown = false;
    bool clickedInRegion = false;
    int mouseX;
    int mouseY;
    int oldMouseX;
    int oldMouseY;
};

void hope_ui_init(HopeUIBinding binding);
HopeUIDrawList * hope_ui_get_drawlist();
void hope_ui_begin();
void hope_ui_end();
void hope_ui_dummy_button(int guid, HopeUIRect rect);
bool hope_ui_button(int guid, char const * name, Rect rect);
bool hope_ui_hit_region(int x, int y, HopeUIRect rect);
void hope_ui_render();

#endif


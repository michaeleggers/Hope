#include "hope_ui.h"

static HopeUIBinding * gBinding;
static HopeUIRect gRects[32];
static int gRectCounter;
static HopeUIWindow gWindow;

void hope_ui_init(HopeUIBinding * binding)
{
    gBinding = binding;
}

void hope_ui_begin()
{
    gWindow.rects = &gRects[gRectCounter++];
}

void hope_ui_end()
{
    
}

bool hope_ui_button(char const * name, HopeUIRect rect)
{
    gWindow.rectCount++;
    int mouseX = gBinding->getMouseX();
    int mouseY = gBinding->getMouseY();
    bool inRegion = hope_ui_hit_region(mouseX, mouseY, rect);
    bool leftMBPressed = gBinding->leftMouseButtonPressed();
    if (inRegion && leftMBPressed)
        return true;
    return false;
}

bool hope_ui_hit_region(int x, int y, HopeUIRect rect)
{
    if (
        (x >= rect.x0 && x <= rect.x1) &&
        (y >= rect.y0 && y <= rect.y1)
        )
    {
        gRects[gRectCounter++] = rect;
        return true;
    }
    return false;
}

void hope_ui_render()
{
    gRectCounter = 0;
}


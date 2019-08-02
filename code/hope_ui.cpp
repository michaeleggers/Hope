#include "hope_ui.h"

static HopeUIWindow gWindow;
static HopeUIContext gContext;
static HopeUIDrawList gHopeUIDrawList;

void hope_ui_init(HopeUIBinding * binding)
{
    gContext.binding = binding;
}

void hope_ui_begin()
{
}

void hope_ui_end()
{
}

HopeUIDrawList * hope_ui_get_drawlist()
{
    return &gHopeUIDrawList;
}

bool hope_ui_button(char const * name, HopeUIRect rect)
{
    bool result = false;
    HopeUIColor color = {0.f,0.f,.7f};
    int mouseX = gContext.binding->getMouseX();
    int mouseY = gContext.binding->getMouseY();
    bool inRegion = hope_ui_hit_region(mouseX, mouseY, rect);
    if (inRegion)
    {
        // NOTE(Michael): inRegion check *before* lftMBPressed,
        // because the input update function does update state and
        // it will have the wrong state for subsequent calls to this function.
        color = {0.f,0.f,1.f};
        bool leftMBDown = gContext.binding->leftMouseButtonDown();
        if (leftMBDown)
            color = {0.5f,0.f,.1f};
        bool leftMBPressed = gContext.binding->leftMouseButtonPressed();
        if (leftMBPressed)
        {
            result = true;
        }
    }
    gHopeUIDrawList.buttons[gHopeUIDrawList.buttonCount++] = { rect, color };
    return result;
}

bool hope_ui_hit_region(int x, int y, HopeUIRect rect)
{
    if ((x >= rect.x0 && x <= rect.x1) &&
        (y >= rect.y0 && y <= rect.y1))
        return true;
    return false;
}

void hope_ui_render()
{
    for (int i = 0; i < gHopeUIDrawList.buttonCount; ++i)
    {
    }
}

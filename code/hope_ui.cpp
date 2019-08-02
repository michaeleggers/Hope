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

bool hope_ui_button(char const * name, HopeUIRect rect)
{
    gContext.rects[gContext.rectCount++] = rect;
    int mouseX = gContext.binding->getMouseX();
    int mouseY = gContext.binding->getMouseY();
    bool inRegion = hope_ui_hit_region(mouseX, mouseY, rect);
    bool leftMBPressed = gContext.binding->leftMouseButtonPressed();
    if (inRegion && leftMBPressed)
        return true;
    return false;
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
    for (int i = 0; i < gContext.rectCount; ++i)
    {
        gHopeUIDrawList.rects[i] = gContext.rects[i];
    }
    gContext.rectCount = 0;
}


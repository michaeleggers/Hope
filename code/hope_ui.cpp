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
    gContext.mouseWasDown = gContext.mouseDown;
    gContext.mouseDown = gContext.binding->leftMouseButtonDown();
    gContext.prevActiveID.intID = gContext.activeID.intID;
    gContext.prevHotID.intID = gContext.prevHotID.intID;
    gContext.mouseX = gContext.binding->getMouseX();
    gContext.mouseY = gContext.binding->getMouseY();
}

void hope_ui_end()
{
}

HopeUIDrawList * hope_ui_get_drawlist()
{
    return &gHopeUIDrawList;
}

bool hope_ui_button(int guid, char const * name, HopeUIRect rect)
{
    bool result = false;
    bool inRegion = hope_ui_hit_region(gContext.mouseX, gContext.mouseY, rect);
    
    if (gContext.activeID.intID == guid)
    {
        if (gContext.mouseWasDown && !gContext.mouseDown)
        {
            if (inRegion)
            {
                if (gContext.hotID.intID == guid)
                {
                    result = true;
                }
            }
            gContext.activeID.intID = -1;
        }
    }
    else if (gContext.hotID.intID == guid)
    {
        if (inRegion)
        {
            if (gContext.mouseDown)
            {
                gContext.activeID.intID = guid;
            }
        }
        else
        {
            gContext.hotID.intID = -1;
        }
    }
    if (inRegion && !gContext.mouseDown)
    {
        gContext.hotID.intID = guid;
    }
    
    // define buttons appearance
    // TODO(Michael): this really should be in ui_render_impl. and just
    // *define* the appearance via render commands here!
    HopeUIColor color = {0.f,0.f,.7f};
    if (gContext.activeID.intID == guid && inRegion)
        color = {0.0f,1.f,1.f};
    else if (gContext.hotID.intID == guid)
        color = {0.f,0.f,1.f};
    
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


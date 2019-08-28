#include "hope_ui.h"

static HopeUIWindow gWindow;
static HopeUIContext gContext;
static HopeUIDrawList gHopeUIDrawList;

void hope_ui_init(HopeUIBinding * binding)
{
    gContext.binding = binding;
    gContext.activeID.intID = -1;
    gContext.hotID.intID = -1;
    gContext.activeWindow = &gHopeUIDrawList.windows[gHopeUIDrawList.windowCount];
}

void hope_ui_begin(int guid, HopeUILayout layout)
{
    gContext.mouseWasDown = gContext.mouseDown;
    gContext.mouseDown = gContext.binding->leftMouseButtonDown();
    gContext.oldMouseX = gContext.mouseX;
    gContext.oldMouseY = gContext.mouseY;
    gContext.mouseX = gContext.binding->getMouseX();
    gContext.mouseY = gContext.binding->getMouseY();
    gContext.windowID.intID = guid;
    gContext.hotID.intID = -1;
    gContext.layout = layout;
    gContext.windowMoveable = false;
    HopeUIWindow * window = &gHopeUIDrawList.windows[gHopeUIDrawList.windowCount++];
    gContext.currentWindow = window;
    gContext.currentWindow->buttonCount = 0;
    //hope_ui_dummy_button(GUID, window->rect);
    
    bool inRegion = hope_ui_hit_region(gContext.mouseX,
                                       gContext.mouseY,
                                       window->rect);
    if (inRegion)
    {
        gContext.windowMoveable = true;
    }
}

void hope_ui_end()
{
    bool inRegion = hope_ui_hit_region(gContext.mouseX,
                                       gContext.mouseY,
                                       gContext.currentWindow->rect);
    if (inRegion)
    {
        gContext.hotID.intID = gContext.windowID.intID;
        if ( (gContext.activeID.intID == -1) && gContext.mouseDown)
        {
            gContext.activeID.intID = gContext.windowID.intID;
        }
    }
    if (gContext.windowMoveable && gContext.activeID.intID == gContext.windowID.intID)
    {
        float mouseDX = gContext.oldMouseX - gContext.mouseX;
        float mouseDY = gContext.oldMouseY - gContext.mouseY;
        gContext.currentWindow->rect.x0 -= mouseDX;
        gContext.currentWindow->rect.x1 -= mouseDX;
        gContext.currentWindow->rect.y0 -= mouseDY;
        gContext.currentWindow->rect.y1 -= mouseDY;
    }
}

HopeUIDrawList * hope_ui_get_drawlist()
{
    return &gHopeUIDrawList;
}

void hope_ui_dummy_button(int guid, HopeUIRect rect)
{
    HopeUIRect buttonRect = { 
        rect.x0, 
        rect.y0,
        rect.x1,
        rect.y1};
    bool inRegion = hope_ui_hit_region(gContext.mouseX, gContext.mouseY, buttonRect);
    if (!inRegion && !gContext.mouseDown)
    {
        gContext.hotID.intID = guid;
    }
    if (gContext.activeID.intID == guid)
    {
        if (gContext.mouseWasDown && !gContext.mouseDown)
        {
            gContext.activeID.intID = -1;
        }
    }
    else if (gContext.hotID.intID == guid)
    {
        if (!inRegion)
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
}

bool hope_ui_button(int guid, char const * name)
{
    HopeUIRect rect = {};
    HopeUIWindow * window = gContext.currentWindow;
    HopeUIRect windowRect = window->rect;
    switch (gContext.layout)
    {
        case HOPE_UI_LAYOUT_COLUMNS:
        {
            rect = {
                10,
                window->yLayoutOffset + 10,
                (windowRect.x1 - windowRect.x0) - 10,
                window->yLayoutOffset + 60
            };
            window->yLayoutOffset += 60.f;
        }
        break;
    }
    
    return hope_ui_button(guid, name, rect);
}

bool hope_ui_button(int guid, char const * name, HopeUIRect rect)
{
    bool result = false;
    HopeUIWindow * window = gContext.currentWindow;
    HopeUIRect windowRect = window->rect;
    HopeUIRect buttonHitRect = { 
        rect.x0 + windowRect.x0, 
        rect.y0 + windowRect.y0,
        rect.x1 + windowRect.x0,
        rect.y1 + windowRect.y0};
    HopeUIColor color = {0.f,0.f,.7f};
    bool inRegion = hope_ui_hit_region(gContext.mouseX, gContext.mouseY, buttonHitRect);
    if (inRegion)
    {
        gContext.windowMoveable = false;
        gContext.hotID.intID = guid;
        if ( (gContext.activeID.intID == -1) && gContext.mouseDown)
        {
            gContext.activeID.intID = guid;
        }
    }
    if (gContext.hotID.intID == guid)
    {
        if (gContext.activeID.intID == guid)
        {
            color = {0.0f,1.f,1.f};
        }
        else
        {
            if (!gContext.mouseDown)
            {
                color = {0.f,0.f,1.f};
            }
        }
    }
    else
    {
        color = {0.f,0.f,.7f};
    }
    
    if (!gContext.mouseDown &&
        gContext.hotID.intID == guid &&
        gContext.activeID.intID == guid)
    {
        result = true;
    }
    else
    {
        result = false;
    }
    
    HopeUIButton * button = &gHopeUIDrawList.buttons[gHopeUIDrawList.buttonCount++];
    button->rect = rect;
    button->color = color;
    strcpy(button->text, name);
    gContext.currentWindow->buttons[gContext.currentWindow->buttonCount++] = button;
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
    
    if (!gContext.mouseDown)
    {
        gContext.activeID.intID = -1;
    }
    else
    {
        if (gContext.activeID.intID == -1)
        {
            gContext.activeID.intID = -2;
        }
    }
    
    for (int i = 0; i < gHopeUIDrawList.buttonCount; ++i)
    {
    }
}

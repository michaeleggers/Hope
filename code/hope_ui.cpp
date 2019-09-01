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

void hope_ui_start()
{
    gContext.mouseWasDown = gContext.mouseDown;
    gContext.mouseDown = gContext.binding->leftMouseButtonDown();
    gContext.oldMouseX = gContext.mouseX;
    gContext.oldMouseY = gContext.mouseY;
    gContext.mouseX = gContext.binding->getMouseX();
    gContext.mouseY = gContext.binding->getMouseY();
}

void hope_ui_begin(int guid, HopeUILayout layout)
{
    gContext.windowID.intID = guid;
    gContext.hotID.intID = -1;
    gContext.layout = layout;
    gContext.windowMoveable = false;
    HopeUIWindow * window = &gHopeUIDrawList.windows[gHopeUIDrawList.windowCount++];
    gContext.currentWindow = window;
    gContext.currentWindow->buttonCount = 0;
    
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
    gContext.windowID.intID = -1;
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

// NOTE(Michael): only works when user calls hope_ui_begin() before and
// hope_ui_end() after this!
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
    HopeUIRect buttonHitRect = {};
    
    HopeUIButton * button = &gHopeUIDrawList.buttons[gHopeUIDrawList.buttonCount++];
    button->rect = rect;
    strcpy(button->text, name);
    if (gContext.windowID.intID >= 0)
    {
        buttonHitRect = {
            rect.x0 + windowRect.x0, 
            rect.y0 + windowRect.y0,
            rect.x1 + windowRect.x0,
            rect.y1 + windowRect.y0};
        gContext.currentWindow->buttons[gContext.currentWindow->buttonCount++] = button;
    }
    else
    {
        gHopeUIDrawList.freeButtons[gHopeUIDrawList.freeButtonCount++] = button;
        buttonHitRect = rect;
    }
    
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
    
    button->color = color;
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
#if 0
    HopeUIWindow * window = gHopeUIDrawList.windows;
    for (int i=0; i<gHopeUIDrawList.windowCount; ++i)
    {
        if (window->renderPrio == 0)
        {
            if (gHopeUIDrawList.windowCount > 1)
            {
                HopeUIWindow lastWindow = gHopeUIDrawList.windows[gHopeUIDrawList.windowCount - 1];
                gHopeUIDrawList.windows[gHopeUIDrawList.windowCount - 1] = *window;
                gHopeUIDrawList.windows[i] = lastWindow;
                break;
            }
        }
    }
#endif
    
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

/*
int cmpWindows(const void * windowA, const void * windowB)
{
    HopeUIWindow * windowA_ = (HopeUIWindow*)windowA;
    HopeUIWindow * windowB_ = (HopeUIWindow*)windowB;
    return windowA_->renderPrio > windowB_->renderPrio;
}
*/
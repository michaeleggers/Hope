#include "hope_ui.h"

static HopeUIWindow gWindow;
static HopeUIContext gContext;
static HopeUIDrawList gHopeUIDrawList;

void hope_ui_init(HopeUIBinding * binding)
{
    gContext.binding = binding;
    HopeUIWindow * window = &gHopeUIDrawList.windows[gHopeUIDrawList.windowCount++];
    window->rect = {0,0,400,600};
    gContext.activeWindow = window;
    gContext.activeID.intID = -1;
    gContext.hotID.intID = -1;
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
    gContext.layout = layout;
    
    // NOTE(Michael): this dummy_button call is necessary so the
    // window cannot be dragged when the user moves from outside
    // of the window's boundaries into it (with mous button down).
    // Using the call, the window thinks the mouse is over another
    // widget and hotID is -1.
    hope_ui_dummy_button(GUID, gContext.activeWindow->rect);
    bool inRegion = hope_ui_hit_region(gContext.mouseX,
                                       gContext.mouseY,
                                       gContext.activeWindow->rect);
    if (gContext.hotID.intID == -1)
    {
        if (inRegion && gContext.mouseDown)
        {
            float mouseDX = gContext.oldMouseX - gContext.mouseX;
            float mouseDY = gContext.oldMouseY - gContext.mouseY;
            HopeUIWindow * window = gContext.activeWindow;
            window->rect.x0 -= mouseDX;
            window->rect.x1 -= mouseDX;
            window->rect.y0 -= mouseDY;
            window->rect.y1 -= mouseDY;
        }
    }
}

void hope_ui_end()
{
    gContext.yLayoutOffset = 0;
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
    HopeUIWindow * window = gContext.activeWindow;
    HopeUIRect windowRect = window->rect;
    switch (gContext.layout)
    {
        case HOPE_UI_LAYOUT_COLUMNS:
        {
            rect = {
                10,
                gContext.yLayoutOffset + 10,
                (windowRect.x1 - windowRect.x0) - 10,
                gContext.yLayoutOffset + 60
            };
            gContext.yLayoutOffset += 60.f;
        }
        break;
    }
    
    return hope_ui_button(guid, name, rect);
}

bool hope_ui_button(int guid, char const * name, HopeUIRect rect)
{
    bool result = false;
    HopeUIWindow * window = gContext.activeWindow;
    HopeUIRect windowRect = window->rect;
    HopeUIRect buttonRect = { 
        rect.x0 += windowRect.x0, 
        rect.y0 += windowRect.y0,
        rect.x1 += windowRect.x0,
        rect.y1 += windowRect.y0};
    bool inRegion = hope_ui_hit_region(gContext.mouseX, gContext.mouseY, buttonRect);
    if (inRegion && !gContext.mouseDown)
    {
        gContext.hotID.intID = guid;
    }
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
    
    // define buttons appearance
    // TODO(Michael): this really should be in ui_render_impl. and just
    // *define* the appearance via render commands here!
    HopeUIColor color = {0.f,0.f,.7f};
    if (gContext.activeID.intID == guid && inRegion)
        color = {0.0f,1.f,1.f};
    else if (gContext.hotID.intID == guid)
        color = {0.f,0.f,1.f};
    
    HopeUIButton * button = &gHopeUIDrawList.buttons[gHopeUIDrawList.buttonCount++];
    button->rect = buttonRect;
    button->color = color;
    strcpy(button->text, name);
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


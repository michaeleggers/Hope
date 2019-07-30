#include "hope_ui.h"

static HopeUIBinding * gBinding;

void hope_ui_init(HopeUIBinding * binding)
{
    gBinding = binding;
}

void hope_ui_begin()
{
}

void hope_ui_end()
{
}

bool hope_ui_button(char const * name, HopeUIRect rect)
{
    bool result = false;
    int mouseX = gBinding->getMouseX();
    int mouseY = gBinding->getMouseY();
    return hope_ui_hit_region(mouseX, mouseY, rect);
}

bool hope_ui_hit_region(int x, int y, HopeUIRect rect)
{
    if (
        (x >= rect.x0 && x <= rect.x1) &&
        (y >= rect.y0 && y <= rect.y1)
        )
    {
        return true;
    }
    return false;
}

void hope_ui_render()
{
}


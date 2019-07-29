#include "hope_ui.h"

static HopeUIBinding gBinding;

void hope_ui_init(HopeUIBinding binding)
{
    gBinding.mouseX = &binding.mouseX;
    gBinding.mouseY = &
}

void hope_ui_begin()
{
}

void hope_ui_end()
{
}

bool hope_ui_button(char const * name, Rect rect)
{
    bool result = false;
    int mouseX = context.mouseX;
    int mouseY = context.mouseY;
    return hope_ui_hit_region(x, y, rect);
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


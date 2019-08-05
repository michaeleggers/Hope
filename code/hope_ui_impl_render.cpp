#include "hope_ui.h"
#include "hope_draw.h"
#include "ref.h"

void hopeUIImpLAddToDrawList(HopeUIDrawList * uiDrawList)
{
    HopeUIWindow * window = uiDrawList->windows;
    HopeUIRect windowRect = window->rect;
    float x0 = windowRect.x0;
    float y0 = windowRect.y0;
    float width = windowRect.x1 - x0;
    float height = windowRect.y1 - y0;
    pushFilledRect(x0, y0, width, height, {0,.7f,0});
    HopeUIButton * button = uiDrawList->buttons;
    for (int i=0; i<uiDrawList->buttonCount; ++i)
    {
        HopeUIRect rect = button->rect;
        HopeUIColor color = button->color;
        float x0 = rect.x0;
        float y0 = rect.y0;
        float width = rect.x1 - x0;
        float height = rect.y1 - y0;
        pushFilledRect(x0, y0, width, height, {color.r, color.g, color.b} );
        pushTTFTextInBoundaries(button->text, rect.x0, rect.y0, {x0, y0, width, height}, {1.f,1.f,1.f});
        button++;
    }
    uiDrawList->buttonCount = 0;
    uiDrawList->windowCount = 0;
}


#include "hope_ui.h"
#include "hope_draw.h"
#include "ref.h"

void hopeUIImpLAddToDrawList(HopeUIDrawList * uiDrawList)
{
    HopeUIButton * buttons = uiDrawList->buttons;
    for (int i=0; i<uiDrawList->buttonCount; ++i)
    {
        HopeUIRect rect = buttons->rect;
        HopeUIColor color = buttons->color;
        float x0 = rect.x0;
        float y0 = rect.y0;
        float width = rect.x1 - x0;
        float height = rect.y1 - y0;
        pushFilledRect(x0, y0, width, height, {color.r, color.g, color.b} );
        buttons++;
    }
    uiDrawList->buttonCount = 0;
}


#include "hope_ui.h"
#include "hope_draw.h"
#include "ref.h"

void hopeUIImpLAddToDrawList(HopeUIDrawList * uiDrawList)
{
    HopeUIRect * currRect = uiDrawList->rects;
    for (int i=0; i<uiDrawList->rectCount; ++i)
    {
        float x0 = currRect->x0;
        float y0 = currRect->y0;
        float width = currRect->x1 - x0;
        float height = currRect->y1 - y0;
        pushFilledRect(x0, y0, width, height, {1.f, 0.f, 0.f});
        currRect++;
    }
    uiDrawList->rectCount = 0;
}


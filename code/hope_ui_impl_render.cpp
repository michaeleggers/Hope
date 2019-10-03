#include <stdlib.h>

#include "hope_ui.h"
#include "hope_draw.h"
#include "ref.h"

void hopeUIImpLAddToDrawList(HopeUIDrawList * uiDrawList)
{
    HopeUIWindow * window = uiDrawList->windows;
    for (int i=0; i<uiDrawList->windowCount; ++i)
    {
        HopeUIRect windowRect = window->rect;
        float x0 = windowRect.x0;
        float y0 = windowRect.y0;
        float width = windowRect.x1 - x0;
        float height = windowRect.y1 - y0;
        pushFilledRect(x0, y0, width, height, {0,.7f,0});
        window->yLayoutOffset = 0;
        window->xLayoutOffset = 0;
        for (int w=0; w<window->buttonCount; ++w)
        {
            HopeUIButton * button = window->buttons[w];
            button->rect = { 
                button->rect.x0 + windowRect.x0, 
                button->rect.y0 + windowRect.y0,
                button->rect.x1 + windowRect.x0,
                button->rect.y1 + windowRect.y0};
            HopeUIColor color = button->color;
            float x0 = button->rect.x0;
            float y0 = button->rect.y0;
            float width = button->rect.x1 - x0;
            float height = button->rect.y1 - y0;
            pushFilledRect(x0, y0, width, height, {color.r, color.g, color.b} );
            pushTTFTextInBoundaries(button->text, button->rect.x0, button->rect.y0, {x0, y0, width, height}, {1.f,1.f,1.f});
        }
        window++;
    }
    
    for (int i=0; i<uiDrawList->freeButtonCount; ++i)
    {
        HopeUIButton * freeButton = uiDrawList->freeButtons[i];
        HopeUIRect rect = freeButton->rect;
        HopeUIColor color = freeButton->color;
        float x0 = rect.x0;
        float y0 = rect.y0;
        float width = rect.x1 - x0;
        float height = rect.y1 - y0;
        pushFilledRect(x0, y0, width, height, {color.r, color.g, color.b} );
        pushTTFTextInBoundaries(freeButton->text, rect.x0, rect.y0, {x0, y0, width, height}, {1.f,1.f,1.f});
    }
    
    HopeUIProgressBar * progressBar = uiDrawList->progressBars;
    for (int i=0; i<uiDrawList->progressBarCount; ++i) {
        HopeUIRect maxProgressRect = progressBar->maxProgressRect;
        HopeUIRect progressRect = progressBar->progressRect;
        // max progress
        float x0 = maxProgressRect.x0;
        float y0 = maxProgressRect.y0;
        float width = maxProgressRect.x1 - x0;
        float height = maxProgressRect.y1 - y0;
        HopeUIColor color = progressBar->maxProgressColor;
        pushFilledRect(x0, y0, width, height, {color.r, color.g, color.b} );
        // current progress
        x0 = progressRect.x0;
        y0 = progressRect.y0;
        width = progressRect.x1 - x0;
        height = progressRect.y1 - y0;
        color = progressBar->progressColor;
        pushFilledRect(x0, y0, width, height, {color.r, color.g, color.b} );
        progressBar++;
    }
    
    uiDrawList->buttonCount      = 0;
    uiDrawList->windowCount      = 0;
    uiDrawList->progressBarCount = 0;
    uiDrawList->freeButtonCount  = 0;
}

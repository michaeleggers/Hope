#ifndef HOPE_UI_H
#define HOPE_UI_H

#define HOPE_UI_MAX_BUTTONS    256

HopeUIBinding
{
    short *mouseX, *mouseY;
    int   *windowWidth, *windowHeight;
};

HopeUIID
{
    char const * strid = "";
};

HopeUIContext
{
    int windowWidth, windowHeight;
    int mouseX, mouseY;
};

struct HopeUIRect
{
    int x0, y0;
    int x1, y1;
};




#endif


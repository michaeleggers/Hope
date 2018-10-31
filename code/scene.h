#ifndef SCENE_H
#define SCENE_H

#include "ref.h"

struct Background
{
    char * imageFile;
    int imgWidth, imgHeight, channels;
    int x, y;
    Sprite * refSprite;
};

struct Object
{
    char * imageFile;
    int imgWidth, imgHeight, channels;
    int x, y;
};

struct Room
{
    Background background;
    Object object;
    // other stuff...
};


#endif
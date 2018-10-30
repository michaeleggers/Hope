#ifndef SCENE_H
#define SCENE_H

#include "common_render.h"

struct Background
{
    char * imageFile;
    int imgWidth, imgHeight, channels;
    int x, y;
    EntityResource entityResource;
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
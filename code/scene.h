#ifndef SCENE_H
#define SCENE_H

struct Background
{
    char * imageFile;
    int imgWidth, imgHeight, channels;
    int x, y;
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
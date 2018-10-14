#ifndef SCENE_H
#define SCENE_H

struct Background
{
    unsigned char * image;
    int x, y, n;
};

struct Room
{
    Background background;
    // other stuff...
};


#endif
#ifndef SCENE_H
#define SCENE_H

struct Background
{
    char * imageFile;
    int x, y, n;
};

struct Room
{
    Background background;
    // other stuff...
};


#endif
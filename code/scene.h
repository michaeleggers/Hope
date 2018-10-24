#ifndef SCENE_H
#define SCENE_H

struct Background
{
    char * imageFile;
    int imgWidth, imgHeight, channels;
};

struct Object
{
    char * imageFile;
    int imgWidth, imgHeight, channels;
    int xPos, yPos;
};

struct Room
{
    Background background;
    Object object;
    // other stuff...
};


#endif
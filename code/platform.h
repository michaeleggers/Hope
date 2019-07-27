#ifndef PLATFORM_H
#define PLATFORM_H

struct Rect
{
    int width, height; // width, height in pixels
};

// stuff the platform layer has to implement

#define PLATFORM_READ_TEXT_FILE(name) char* name(char const * file)
typedef PLATFORM_READ_TEXT_FILE(platform_read_text_file);
// => typedef char* platform_read_text_file(char const * file)

#define PLATFORM_GET_WINDOW_DIMENSIONS(name) Rect name(void)
typedef PLATFORM_GET_WINDOW_DIMENSIONS(platform_get_window_dimensions);

struct PlatformAPI
{
    platform_read_text_file* readTextFile;
    //char * (*readTextFile)(char const * file);
    
    platform_get_window_dimensions* getWindowDimensions;
};

#endif
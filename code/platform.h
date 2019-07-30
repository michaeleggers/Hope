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

#define PLATFORM_GET_WINDOW_WIDTH(name) int name(void)
typedef PLATFORM_GET_WINDOW_WIDTH(platform_get_window_width);
#define PLATFORM_GET_WINDOW_HEIGHT(name) int name(void)
typedef PLATFORM_GET_WINDOW_HEIGHT(platform_get_window_height);

struct PlatformAPI
{
    platform_read_text_file* readTextFile;
    //char * (*readTextFile)(char const * file);
    
    platform_get_window_dimensions* getWindowDimensions;
    
    // NOTE(Michael): not initialized atm! Not sure if we ever need those?
    platform_get_window_width* getWindowWidth;
    platform_get_window_height* getWindowHeight;
};

#endif
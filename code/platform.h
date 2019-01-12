#ifndef PLATFORM_H
#define PLATFORM_H

// stuff the platform layer has to implement

#define PLATFORM_READ_TEXT_FILE(name) char* name(char const * file)
typedef PLATFORM_READ_TEXT_FILE(platform_read_text_file);

struct PlatformAPI
{
    platform_read_text_file* readTextFile;
};

#endif
// TODO(Michael): there is platform specific code in here!
#include "helper.h"

#include <stdio.h>
#include <windows.h>

meg_strbuf meg_strbuf_create()
{
    meg_strbuf strbuf;
    strbuf.pos = 0;
    strbuf.buffer = (char *)VirtualAlloc(0,
                                         256,
                                         MEM_RESERVE | MEM_COMMIT,
                                         PAGE_READWRITE);
    return strbuf;
}

// TODO(Michael): check if buffer overflow
int meg_strbuf_write(meg_strbuf* strbuf, char const * s)
{
    int s_length = strlen(s);
    strcpy(strbuf->buffer + strbuf->pos, s);
    strbuf->pos += s_length;
    
    return strbuf->pos - s_length;
}

// TODO(Michael): check if fopen was successful.
// TODO(Michael): we actually should be able to use
// win32 api directly from the platform layer?!
char* load_text(char const * filename)
{
    FILE* f = fopen(filename, "r");
    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char* buffer =  (char *)VirtualAlloc(0,
                                         size,
                                         MEM_RESERVE | MEM_COMMIT,
                                         PAGE_READWRITE);
    fread(buffer, sizeof *buffer, size, f);
    fclose(f);
    
    return buffer;
}

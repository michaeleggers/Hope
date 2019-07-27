#ifndef HELPER_H
#define HELPER_H


struct meg_strbuf
{
    char * buffer;
    int pos;
};

#if 0
struct Rect
{
    int width, height;
};
#endif

meg_strbuf meg_strbuf_create();
int meg_strbuf_write(meg_strbuf* strbuf, char const * s);
char* load_text(char const * filename);


#endif
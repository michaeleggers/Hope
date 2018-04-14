#ifndef HELPER_H
#define HELPER_H

struct meg_strbuf;
meg_strbuf meg_strbuf_create();
int meg_strbuf_write(meg_strbuf* strbuf, char const * s);

#endif
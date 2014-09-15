#ifndef CODESET_PROCESSAPPDATA_H_H
#define CODESET_PROCESSAPPDATA_H_H

#include "sockets_buffer.h"

struct processappdata;
struct processappdata * processappdata_create(struct sockets_buffer* sbuf,int fd_sigfmt);
int processappdata_join(struct processappdata* pad);
int processappdata_destroy(struct processappdata* pad);

#endif

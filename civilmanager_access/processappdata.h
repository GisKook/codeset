#ifndef CODESET_PROCESSAPPDATA_H_H
#define CODESET_PROCESSAPPDATA_H_H

#include "sockets_buffer.h"
#include "loginmanager.h"

struct processappdata;
struct sockets_buffer;
struct processappdata * processappdata_create(struct sockets_buffer* sbuf, struct loginmanager * loginmanager, int fd_sigfmt);
int processappdata_join(struct processappdata* pad);
int processappdata_destroy(struct processappdata* pad);
void processappdata_delete(struct processappdata * pad, int fd);
struct sockets_buffer * processappadata_getsocketbuffer(struct processappdata * pad);

#endif

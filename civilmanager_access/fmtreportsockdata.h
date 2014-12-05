#ifndef CODESET_FMTREPORTSOCKDATA_H_H
#define CODESET_FMTREPORTSOCKDATA_H_H

#include "kfifo.h"
#include "list.h"

struct sockets_buffer;

struct fmtreportsockdata{
	struct parseprotocol_request * message;
	struct list_head list;
};

int fmtreportsockdata_add( struct sockets_buffer * sockbuffer, int fd); 
int fmtreportsockdata_clear(struct fmtreportsockdata* msg );

#endif

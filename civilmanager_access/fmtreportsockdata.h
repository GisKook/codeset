#ifndef CODESET_FMTREPORTSOCKDATA_H_H
#define CODESET_FMTREPORTSOCKDATA_H_H

#include "kfifo.h"
#include "list.h"

struct fmtreportsockdata{
	struct parseprotocol_request * message;
	struct list_head list;
};

int fmtreportsockdata_add(struct list_head * head, struct kfifo* fifo, int fd); 
int fmtreportsockdata_clear(struct fmtreportsockdata* msg );

#endif

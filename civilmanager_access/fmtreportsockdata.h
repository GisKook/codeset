#ifndef CODESET_FMTREPORTSOCKDATA_H_H
#define CODESET_FMTREPORTSOCKDATA_H_H

#include "kfifo.h"
#include "list.h"

struct fmtreportsockdata;

int fmtreportsockdata_add(struct list_head * head, struct kfifo* fifo); 
int fmtreportsockdata_clear(struct fmtreportsockdata* msg );

#endif

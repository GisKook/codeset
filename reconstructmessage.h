#ifndef CODESET_RECONSTRUCTMESSAGE_H_H
#define CODESET_RECONSTRUCTMESSAGE_H_H

#include "kfifo.h"
#include "list.h"

struct reconstructmessage;

int reconstructmessage_add(struct list_head * head, struct kfifo* fifo); 
int reconstructmessage_clear(struct reconstructmessage* msg );

#endif

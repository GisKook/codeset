#ifndef CIVILMANGER_ACCESS_FDLIST_H_H
#define CIVILMANGER_ACCESS_FDLIST_H_H

struct fdfifo
{
	unsigned int * entry;
	unsigned int capacity;
	unsigned int in;
	unsigned int out;
} ;

void fdfifo_init(struct fdfifo * fifo, int capcity);

void fdfifo_destroy(struct fdfifo * fifo);

void fdfifo_put(struct fdfifo * fifo, int fd);

unsigned int fdfifo_get(struct fdfifo * fifo);

unsigned int fdfifo_len(struct fdfifo * fifo);

void fdfifo_getall(struct fdfifo * fifo, int * buf);

#endif

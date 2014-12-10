#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fdfifo.h"

void fdfifo_init(struct fdfifo * fifo, int capacity){
	fifo->entry = (unsigned int *)malloc(capacity);
	if(fifo->entry == NULL){
		fprintf(stderr, "fdfifo create error %s %s %d\n", __FUNCTION__, __FILE__, __LINE__);
		return;
	}
	fifo->capacity = capacity;
	fifo->in = fifo->out = 0;
}

void fdfifo_destroy(struct fdfifo * fifo){ 
	free(fifo->entry);
	fifo->entry = NULL;
}

unsigned int fdfifo_len(struct fdfifo * fifo){
	unsigned int len = fifo->in - fifo->out;
	if(len < 0){
		len = len + fifo->capacity;
	}

	return len;
}

void fdfifo_put(struct fdfifo * fifo, int fd){ 
	unsigned int i;
	if(fifo->in > fifo->out){
		for( i = fifo->out; i < fifo->in; ++i){ 
			if(fifo->entry[i] == fd){
				return;
			}
		}
	}else if(fifo->in < fifo->out){ 
		for( i = 0; i<fifo->in; ++i){
			if(fifo->entry[i] == fd){
				return;
			}
		}

		for(i = fifo->out; i<fifo->capacity; ++i){
			if(fifo->entry[i] == fd){
				return;
			}
		}
	}

	fifo->entry[fifo->in++] = fd;
	if(fifo->in >= fifo->capacity){
		fifo->in = 0;
	}
}

unsigned int fdfifo_get(struct fdfifo * fifo){
	unsigned int fd = fifo->entry[fifo->out++];
	if(fifo->out >= fifo->capacity){
		fifo->out = 0;
	}

	return fd;
}

void fdfifo_getall(struct fdfifo * fifo, int * buf){
	if(fifo->in > fifo->out){
		memcpy(buf, &fifo->entry[fifo->out], (fifo->in - fifo->out)*sizeof(int));
	}

	if(fifo->in <= fifo->out){ 
		memcpy(buf, &fifo->entry[fifo->out], (fifo->capacity - fifo->out)*sizeof(int));
		memcpy(buf + (fifo->capacity-fifo->out), &fifo->entry[0], (fifo->in + 1)*sizeof(int));
	}
	fifo->in = fifo->out = 0;
}

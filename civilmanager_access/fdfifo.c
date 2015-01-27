#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fdfifo.h"

void fdfifo_init(struct fdfifo * fifo, int capacity){
	fifo->entry = (int *)malloc(capacity*sizeof(int));
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
	unsigned int start = fifo->in % fifo->capacity;
	unsigned int end = fifo->out % fifo->capacity;
	if(fifo->in > fifo->out){
		for( i = start; i < end; ++i){ 
			if(fifo->entry[i] == fd){
				return;
			}
		}
	}else if(fifo->in < fifo->out){ 
		for( i = 0; i<start; ++i){
			if(fifo->entry[i] == fd){
				return;
			}
		}

		for(i = end; i<fifo->capacity; ++i){
			if(fifo->entry[i] == fd){
				return;
			}
		}
	}

	fifo->entry[fifo->in%fifo->capacity] = fd;
	fifo->in++;
	if(fifo->in - fifo->out > fifo->capacity){
		fifo->out++;
	}
}

unsigned int fdfifo_get(struct fdfifo * fifo){
	unsigned int fd = fifo->entry[fifo->out%fifo->capacity];
	fifo->out++;

	return fd;
}

void fdfifo_getall(struct fdfifo * fifo, int * buf){
	int out = fifo->out % fifo->capacity;
	int in = fifo->in % fifo->capacity;
	if(in > out){
		memcpy(buf, &fifo->entry[out], (fifo->in-fifo->out)*sizeof(int));
	}

	if(in <= out){ 
		memcpy(buf, &fifo->entry[out], (fifo->capacity - out)*sizeof(int));
		memcpy(buf + (fifo->capacity-out), &fifo->entry[0], in*sizeof(int));
	}
	fifo->in = fifo->out = 0;
}

//void main(){
//	struct fdfifo * fdfifo = (struct fdfifo *)malloc(sizeof(struct fdfifo));
//	fdfifo_init(fdfifo, 1024);
//	unsigned int i = 0;
//	
//	for(i = 0; i < 512; ++i){
//		fdfifo_put(fdfifo, i);
//	}
//	assert(fdfifo_len(fdfifo) == 512);
//
//	for(i = 0; i < 512; ++i){
//		fdfifo_put(fdfifo, i+512);
//	}
//	for(i = 0; i < 512; ++i){
//		fdfifo_put(fdfifo, i+1024);
//	}
//	assert(fdfifo_len(fdfifo) == 1024);
////	for(i = 0; i < 1024; ++i){
////		fprintf(stdout, "%d\n", fdfifo->entry[i]);
////	}
//	int len = fdfifo_len(fdfifo);
////	for(i = 0; i < len; ++i){
////		fprintf(stdout, "%d %d\n ", fdfifo_get(fdfifo), fdfifo_len(fdfifo));
////	}
//	int * fds = (int *)malloc(len*sizeof(int));
//	fdfifo_getall(fdfifo, fds);
//	for(i = 0; i < len; ++i){
//		fprintf(stdout, "%d %d\n", fds[i], len);
//	}
//
//	fdfifo_destroy(fdfifo);
//	free(fdfifo);
//}

#include <stdio.h>
#include <stdlib.h> 
#include <assert.h>
#include <string.h>
#include "kfifo.h"
#include "cndef.h"
#include "toolkit.h"

#define MAXFIFOLEN 4096

struct write_buffer {
	struct write_buffer * next;
	int sz;
	void *buffer;
};

struct fd_buffer{
	int fd;
	char ip[16];
	unsigned int id;
	struct kfifo * fifo;	
	struct write_buffer * buffer;
	struct fd_buffer * next;
};

struct sockets_buffer{
	int slotcount;
	struct fd_buffer** slot;
};

struct sockets_buffer* sockets_buffer_create(unsigned int slotcount){
	struct sockets_buffer *sockets_buf = (struct sockets_buffer*)malloc(sizeof(struct sockets_buffer));
	if(unlikely(sockets_buf == NULL)){
		fprintf(stderr, "malloc error. %s %s %d\n", __FILE__,__FUNCTION__, __LINE__);

		return NULL;
	}
	memset(sockets_buf, 0, sizeof(struct sockets_buffer));
	sockets_buf->slotcount = slotcount;

	sockets_buf->slot = (struct fd_buffer**)malloc(sizeof(struct fd_buffer*)*slotcount);
	if(unlikely(sockets_buf->slot == NULL)){
		fprintf(stderr, "malloc error. %s %s %d\n", __FILE__,__FUNCTION__, __LINE__);

		return NULL;
	}
	memset(sockets_buf->slot, 0, sizeof(struct fd_buffer*)*slotcount);

	return sockets_buf;
}

int sockets_buffer_add(struct sockets_buffer* sockets_buf, int fd,char *ip, unsigned char *buf, int len){
	assert(buf != NULL);
	assert(len > 0);
	assert(sockets_buf != NULL);
	if(unlikely((buf == NULL)) || (len <= 0) || (sockets_buf == NULL)){
		fprintf(stderr, "add error. %s %s %d\n",__FILE__,__FUNCTION__,__LINE__);

		return -1;
	}
	int index = fd % sockets_buf->slotcount;
	if(unlikely(sockets_buf->slot[index] == NULL)){
		sockets_buf->slot[index] = (struct fd_buffer*)malloc(sizeof(struct fd_buffer));
		if(unlikely(sockets_buf->slot[index] == NULL)){
			fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

			return -1;
		}
		memset(sockets_buf->slot[index], 0 , sizeof(struct fd_buffer));
		sockets_buf->slot[index]->fd = fd;
		memcpy(sockets_buf->slot[index]->ip, ip, strlen(ip));
		if( unlikely((sockets_buf->slot[index]->fifo = kfifo_init(MAXFIFOLEN)) == NULL)){
			fprintf(stderr, "fifo init error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

			return -1;
		}
		kfifo_put(sockets_buf->slot[index]->fifo, buf, len);
	}else{ 
		struct fd_buffer* p = sockets_buf->slot[index];
		struct fd_buffer* prev = p;
		for( ;p != NULL; prev=p, p=p->next){
			if(p->fd == fd){
				kfifo_put(sockets_buf->slot[index]->fifo, buf,len);
				break;
			}
		}
		if(p == NULL){ 
			struct fd_buffer* element = (struct fd_buffer*)malloc(sizeof(struct fd_buffer));
			memset(element, 0, sizeof(struct fd_buffer));
			element->fd = fd;
			memcpy(sockets_buf->slot[index]->ip, ip, strlen(ip));
			if(unlikely((element->fifo = kfifo_init(MAXFIFOLEN)) == NULL)){ 
				fprintf(stderr, "fifo init error.%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);

				return -1; 
			}
			kfifo_put(element->fifo, buf, len);
			prev->next = element;
		}
	}
	return 0;
}

int sockets_buffer_del(struct sockets_buffer* buf, int fd){ 
	assert(buf != NULL);
	if(unlikely(buf == NULL)){
		return -1;
	}
	int index = fd % MAXFIFOLEN;
	struct fd_buffer* p = buf->slot[index];
	struct fd_buffer* prev = NULL;
	struct fd_buffer* next = NULL;
	assert(p != NULL);
	for(;p!=NULL;prev = p,p=p->next, next = p->next){
		if(p->fd == fd){
			kfifo_free(p->fifo);
			p->fifo = NULL;
			break;
		}
	}
	prev->next = next;

	return 0;
}

int sockets_buffer_destroy(struct sockets_buffer* buf){
	assert(buf != NULL);
	if(unlikely(buf == NULL)){
		fprintf(stderr, "buf is NULL, %s %s %d\n", __FILE__,__FUNCTION__,__LINE__);

		return -1;
	}
	if(likely(buf->slot != NULL)){
		free(buf->slot);
		buf->slot = NULL;
	}

	free(buf);
	buf = NULL; 

	return 0; 
}

int sockets_buffer_print(struct sockets_buffer* buf){
	assert(buf != NULL);
	struct fd_buffer** fdbuf = buf->slot;
	int i;
	struct fd_buffer* p;

	for(i = 0;i < buf->slotcount; ++i){ 
		if(fdbuf[i] != NULL){ 
			p = fdbuf[i];
			for(;p!=NULL;p=p->next){
				fprintf(stdout, "received message from %s  :", p->ip);
				debug_printbytes(p->fifo->buffer+p->fifo->out, kfifo_len(p->fifo));
			}
		}
	}
}

struct kfifo* sockets_buffer_getfifo(struct sockets_buffer * sbuf, int fd){
	assert(sbuf != NULL);
	if(unlikely( sbuf == NULL )){
		fprintf(stderr, "arguments error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	int slotindex = fd % sbuf->slotcount;
	struct fd_buffer* pos;
	for(pos = sbuf->slot[slotindex]; pos != NULL; pos = sbuf->slot[slotindex]->next){ 
		if( sbuf->slot[slotindex]->fd  == fd ){
			return sbuf->slot[slotindex]->fifo;
		}
	}

	return NULL;
}

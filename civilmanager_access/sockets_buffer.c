#include <stdio.h>
#include <stdlib.h> 
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include "kfifo.h"
#include "list.h"
#include "cndef.h"
#include "toolkit.h"
#include "fdfifo.h"
#include "mqueue.h"
#include "encodeprotocol.h"
#include "fmtreportsockdata.h"

#define MAXFIFOLEN 4096
#define MAXWRITEQUEUELEN 4096

struct fd_buffer{
	int fd;
	char ip[16];
	unsigned int id; 
	struct kfifo * fifo;	
	struct list_head highprilist;
	struct list_head normalprilist;
	struct mqueue * writebuffer;
	struct fd_buffer * next;
	struct fd_buffer * prev;
};

struct sockets_buffer{
	int slotcount; 

	pthread_cond_t tasklistready;
	pthread_mutex_t tasklistlock;
	struct fdfifo * tasklist;

	pthread_cond_t normaltasklistready;
	pthread_mutex_t normaltasklistlock;
	struct fdfifo * normaltasklist;

	pthread_cond_t taskdownstreamready;
	pthread_mutex_t taskdownstreamlock;
	struct fdfifo * taskdownstream;

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

	pthread_cond_init(&sockets_buf->tasklistready, NULL);
	pthread_mutex_init(&sockets_buf->tasklistlock, NULL);
	sockets_buf->tasklist =(struct fdfifo*)malloc(sizeof(struct fdfifo));
	fdfifo_init(sockets_buf->tasklist, 1024);

	pthread_cond_init(&sockets_buf->normaltasklistready, NULL);
	pthread_mutex_init(&sockets_buf->normaltasklistlock, NULL);
	sockets_buf->normaltasklist =(struct fdfifo*)malloc(sizeof(struct fdfifo));
	fdfifo_init(sockets_buf->normaltasklist, 1024);

	pthread_cond_init(&sockets_buf->taskdownstreamready, NULL);
	pthread_mutex_init(&sockets_buf->taskdownstreamlock, NULL);
	sockets_buf->taskdownstream = (struct fdfifo *)malloc(sizeof(struct fdfifo));
	fdfifo_init(sockets_buf->taskdownstream, 1024);


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
		INIT_LIST_HEAD(&sockets_buf->slot[index]->highprilist);
		INIT_LIST_HEAD(&sockets_buf->slot[index]->normalprilist);
		memcpy(sockets_buf->slot[index]->ip, ip, strlen(ip));
		if( (sockets_buf->slot[index]->fifo = kfifo_init(MAXFIFOLEN)) == NULL){
			fprintf(stderr, "fifo init error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

			return -1;
		}
		kfifo_put(sockets_buf->slot[index]->fifo, buf, len);
		sockets_buf->slot[index]->writebuffer = mqueue_create(MAXWRITEQUEUELEN, sizeof(struct encodeprotocol_respond));

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
			INIT_LIST_HEAD(&element->highprilist);
			INIT_LIST_HEAD(&element->normalprilist);
			memcpy(sockets_buf->slot[index]->ip, ip, strlen(ip));
			if(unlikely((element->fifo = kfifo_init(MAXFIFOLEN)) == NULL)){ 
				fprintf(stderr, "fifo init error.%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);

				return -1; 
			}
			kfifo_put(element->fifo, buf, len);
			prev->next = element;
			element->prev = prev;
			element->writebuffer = mqueue_create(MAXWRITEQUEUELEN, sizeof(struct encodeprotocol_respond));
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
	struct fd_buffer* next = NULL;

	while(p!=NULL){
		next = p->next;
		if(p->fd == fd){
			if(p->fifo != NULL) 
			{
				kfifo_free(p->fifo);
				p->fifo = NULL; 
			}

			mqueue_destroy(p->writebuffer);
			p->writebuffer = NULL;

			break;
		}
		p = p->next;
	}

	if(p != NULL && p->prev != NULL){
		p->prev->next = next;
	}else if(p != NULL && p->prev == NULL){ // the first node 
		buf->slot[fd] = p->next;
	}

	if(p != NULL){
		struct list_head * pos, *n;
		list_for_each_safe(pos, n, &p->normalprilist){
			struct fmtreportsockdata *fmtreportsockdata = list_entry(pos, struct fmtreportsockdata, list);
			fmtreportsockdata_clear(fmtreportsockdata);
			list_del(pos);
		}
		list_for_each_safe(pos, n, &p->highprilist){
			struct fmtreportsockdata *fmtreportsockdata = list_entry(pos, struct fmtreportsockdata, list);
			fmtreportsockdata_clear(fmtreportsockdata);
			list_del(pos);
		}
		free(p);
	}

	return 0;
}

int sockets_buffer_clear(struct sockets_buffer * buf, int fd){
	assert(buf != NULL);
	if(unlikely(buf == NULL)){
		return -1;
	}
	int index = fd % MAXFIFOLEN;
	struct fd_buffer* p = buf->slot[index];

	for(;p!=NULL;p=p->next){
		if(p->fd == fd){
			if(p->fifo != NULL){
				kfifo_reset(p->fifo);
			}
			p->fd = 0;
			break;
		}
	}

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

	pthread_mutex_destroy(&buf->tasklistlock);
	pthread_cond_destroy(&buf->tasklistready);
	fdfifo_destroy(buf->tasklist);


	pthread_mutex_destroy(&buf->taskdownstreamlock);
	pthread_cond_destroy(&buf->taskdownstreamready);
	fdfifo_destroy(buf->taskdownstream);

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
				toolkit_printbytes(p->fifo->buffer+p->fifo->out, kfifo_len(p->fifo));
			}
		}
	}

	return 0;
}

struct fd_buffer* sockets_buffer_getfdbuffer(struct sockets_buffer* sbuf, int fd){
	assert(sbuf != NULL);
	if(unlikely( sbuf == NULL )){
		fprintf(stderr, "arguments error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	int slotindex = fd % sbuf->slotcount;
	struct fd_buffer* pos;
	for(pos = sbuf->slot[slotindex]; pos != NULL; pos = sbuf->slot[slotindex]->next){ 
		if( sbuf->slot[slotindex]->fd  == fd ){
			return sbuf->slot[slotindex];
		}
	}

	return NULL;
}

struct kfifo* sockets_buffer_getrawdata(struct sockets_buffer * sbuf, int fd){
	struct fd_buffer * fdbuf = sockets_buffer_getfdbuffer(sbuf, fd);
	if(fdbuf != NULL){
		return fdbuf->fifo;
	}

	return NULL;
}
struct list_head* sockets_buffer_gethighlist(struct sockets_buffer * sbuf, int fd){
	struct fd_buffer * fdbuf = sockets_buffer_getfdbuffer(sbuf, fd);
	if(fdbuf != NULL){
		return &fdbuf->highprilist;
	}

	return NULL;
}
struct list_head* sockets_buffer_getnormallist(struct sockets_buffer * sbuf, int fd){
	struct fd_buffer * fdbuf = sockets_buffer_getfdbuffer(sbuf, fd);
	if(fdbuf != NULL){
		return &fdbuf->normalprilist;
	}

	return NULL;
}

void sockets_buffer_signal(struct sockets_buffer * sbuf, int fd){
	pthread_mutex_lock(&sbuf->tasklistlock);
	fdfifo_put(sbuf->tasklist, fd);
	pthread_mutex_unlock(&sbuf->tasklistlock);
	pthread_cond_signal(&sbuf->tasklistready);
}

int * sockets_buffer_getsignalfdfifo(struct sockets_buffer * sbuf){
	int * activefds = NULL;
	int fdfifolen = 0;
	for(;;){
		pthread_mutex_lock(&sbuf->tasklistlock);
		while(fdfifo_len(sbuf->tasklist) == 0){
			pthread_cond_wait(&sbuf->tasklistready, &sbuf->tasklistlock);
		} 
		fdfifolen = fdfifo_len(sbuf->tasklist);
		activefds = (int*)malloc((fdfifolen+1)*sizeof(int)); 
		if(activefds == NULL){
			fprintf(stderr, "malloc %d bytes error. %s %s %d\n", fdfifolen+1, __FILE__, __FUNCTION__, __LINE__);

			return NULL;
		}
		*activefds = fdfifolen;
		fdfifo_getall(sbuf->tasklist, activefds+1);
		pthread_mutex_unlock(&sbuf->tasklistlock);

		return activefds;
	}
}

void sockets_buffer_normaltasksignal(struct sockets_buffer * sbuf, int fd){
	pthread_mutex_lock(&sbuf->normaltasklistlock);
	fdfifo_put(sbuf->normaltasklist, fd);
	pthread_mutex_unlock(&sbuf->normaltasklistlock);
	pthread_cond_signal(&sbuf->normaltasklistready);
}

int * sockets_buffer_getnormaltasklist(struct sockets_buffer * sbuf){
	int * activefds = NULL;
	int fdfifolen = 0;
	for(;;){
		pthread_mutex_lock(&sbuf->normaltasklistlock);
		while(fdfifo_len(sbuf->normaltasklist) == 0){
			pthread_cond_wait(&sbuf->normaltasklistready, &sbuf->normaltasklistlock);
		} 
		fdfifolen = fdfifo_len(sbuf->normaltasklist);
		activefds = (int*)malloc((fdfifolen+1)*sizeof(int)); 
		if(activefds == NULL){
			fprintf(stderr, "malloc %d bytes error. %s %s %d\n", fdfifolen+1, __FILE__, __FUNCTION__, __LINE__);

			return NULL;
		}
		*activefds = fdfifolen;
		fdfifo_getall(sbuf->normaltasklist, activefds+1);
		pthread_mutex_unlock(&sbuf->normaltasklistlock);

		return activefds;
	}
}

int sockets_buffer_write(struct sockets_buffer * sbuf, int fd, struct encodeprotocol_respond * epr){
	int index = fd % sbuf->slotcount;
	struct fd_buffer * fdbuffer  = sbuf->slot[index];
	while(fdbuffer != NULL){
		if(fdbuffer->fd != fd){
			fdbuffer = fdbuffer->next;
		}else{ 
			struct encodeprotocol_respond * encodeprotocol_respond = NULL;
			if((encodeprotocol_respond = (struct encodeprotocol_respond *)mqueue_writer_parpare(fdbuffer->writebuffer)) != NULL){
				encodeprotocol_copy(encodeprotocol_respond, epr);
				mqueue_writer_commit(fdbuffer->writebuffer, encodeprotocol_respond);
				pthread_mutex_lock(&sbuf->taskdownstreamlock);
				fdfifo_put(sbuf->taskdownstream, fd);
				pthread_mutex_unlock(&sbuf->taskdownstreamlock);
				pthread_cond_signal(&sbuf->taskdownstreamready);
			}else{
				fprintf(stderr, "the queue is full. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			}
			break;
		}
	}

	return 0;
}

struct mqueue * sockets_buffer_getwritequeue(struct sockets_buffer * sbuf, int fd){
	struct fd_buffer * fdbuffer = sockets_buffer_getfdbuffer(sbuf, fd);

	return fdbuffer->writebuffer;
}

int * sockets_buffer_getdownstreamsignal(struct sockets_buffer * sbuf){
	int * activefds = NULL;
	int fdfifolen = 0;
	for(;;){
		pthread_mutex_lock(&sbuf->taskdownstreamlock);
		while(fdfifo_len(sbuf->taskdownstream) == 0){
			pthread_cond_wait(&sbuf->taskdownstreamready, &sbuf->taskdownstreamlock);
		} 
		fdfifolen = fdfifo_len(sbuf->taskdownstream);
		activefds = (int*)malloc((fdfifolen+1)*sizeof(int)); 
		if(activefds == NULL){
			fprintf(stderr, "malloc %d bytes error. %s %s %d\n", fdfifolen+1, __FILE__, __FUNCTION__, __LINE__);

			return NULL;
		}
		*activefds = fdfifolen;
		fdfifo_getall(sbuf->taskdownstream, activefds+1);
		pthread_mutex_unlock(&sbuf->taskdownstreamlock);

		return activefds;
	}
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include "fmtreportsockdata.h"
#include "list.h"
#include "sockets_buffer.h"
#include "toolkit.h" 
#include "cndef.h"


#define MAX_FIFO_LEN 4096
struct processappdata{
	pthread_t threadid_fmt;
	int fd_sigfmt;
	struct list_head head;
	struct sockets_buffer* sbuf;
};

void * operationdb(void * param){ 
//	struct list_head * head = ((struct datalist*)param)->head;
//	int fd = ((struct datalist*)param)->fd;
//	struct reconstructmessage* entry;
//	struct list_head *pos, *n;
//	for(;;){
//		read(fd, buf, 1); 
//		list_for_each_safe(pos, n, head){
//			entry = container_of(pos, struct reconstructmessage, list);
//			if(entry->processed == 0 && entry->message->messagetype == REQ_LOGIN){ 
//
//			}
//			
//		}
//
//	}

	pthread_exit(0);
}

void * forwardmsg(void * param){ 
//	struct list_head * head = ((struct datalist*)param)->head;
//	int fd = ((struct datalist*)param)->fd;
//	struct reconstructmessage* entry;
//	struct list_head *pos, *n;
//	for(;;){
//		read(fd, buf, 1); 
//		list_for_each_safe(pos, n, head){
//			entry = container_of(pos, struct reconstructmessage, list);
//			if(entry->processed == 0){ 
//
//			}
//			
//		}
//
//	}
	pthread_exit(0);
}

void * fmtmsg(void * p){
	struct processappdata * pad = (struct processappdata *)p;
	pthread_t tid_fmt = pad->fd_sigfmt;
	char * buf = (char*)malloc(MAX_FIFO_LEN);
	memset(buf, 0, MAX_FIFO_LEN);
	int len;

	pthread_t tid_db;
	if(0 != pthread_create(&tid_db, NULL, operationdb, NULL)){
		free(pad);
		pad = NULL;
		fprintf(stderr, "thread create error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	fprintf(stdout, "thread %d process database related create successfully.\n", tid_db);
		
	pthread_t tid_upward;
	if(0 != pthread_create(&tid_upward, NULL, forwardmsg, NULL)){
		fprintf(stderr,"thread create error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	fprintf(stdout, "thread %d forward message create successfully.\n", tid_db);

	int socketfd;
	char* tmp = buf;
	int leftlength = 0;
	for(;;){ 
		len = read(pad->fd_sigfmt, buf+leftlength, MAX_FIFO_LEN);
		while(toolkit_strsep(buf, '*') != NULL){ 
			socketfd=atoi(buf); 
			fmtreportsockdata_add(&pad->head, sockets_buffer_getfifo(pad->sbuf,socketfd));
		}
		leftlength = strlen(buf);
		memmove(tmp, buf, leftlength);
		memset(tmp+leftlength, 0 , len - leftlength); 
		buf = tmp; 
	}

}

struct processappdata * processappdata_create(struct sockets_buffer * sbuf, int fd_sigfmt){
	struct processappdata * pad = (struct processappdata*)malloc(sizeof(struct processappdata));
	memset(pad, 0, sizeof(struct processappdata));
	assert(pad != NULL);
	if(unlikely(pad == NULL)){
		fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	} 
	INIT_LIST_HEAD(&pad->head); 
	pad->fd_sigfmt = fd_sigfmt;
	pad->sbuf = sbuf;
	if(0 != pthread_create(&pad->threadid_fmt, NULL, fmtmsg, pad)){
		free(pad);
		pad = NULL;
		fprintf(stderr,"thread create error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	fprintf(stdout, "thread %d process application data create successfully.\n", pad->threadid_fmt);

	return pad;
}

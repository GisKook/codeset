#include <pthread.h>
#include <assert.h>
#include "reconstructmessage.h"
#include "list.h"

struct processappdata{
	pthread_t threadid_db;
	pthread_t threadid_upward;
	int fd_sigdb;
	int fd_sigupward;
};

struct datalist{
	list_head* head;
	int fd;
};

void * operationdb(void * param){ 
	struct list_head * head = ((struct datalist*)param)->head;
	int fd = ((struct datalist*)param)->fd;
	struct reconstructmessage* entry;
	struct list_head *pos, *n;
	for(;;){
		read(fd, buf, 1); 
		list_for_each_safe(pos, n, head){
			entry = container_of(pos, struct reconstructmessage, list);
			if(entry->processed == 0 && entry->message->messagetype == REQ_LOGIN){ 

			}
			
		}

	}

	pthread_exit(0);
}

void * forwardmsg(void * param){ 

	struct list_head * head = ((struct datalist*)param)->head;
	int fd = ((struct datalist*)param)->fd;
	struct reconstructmessage* entry;
	struct list_head *pos, *n;
	for(;;){
		read(fd, buf, 1); 
		list_for_each_safe(pos, n, head){
			entry = container_of(pos, struct reconstructmessage, list);
			if(entry->processed == 0){ 
				if(entry->message->type == REQ_

			}
			
		}

	}
	pthread_exit(0);
}

struct processappdata * processappdata_create(int fd_sigdb, int fd_sigupward, struct list_head* head){ 
	struct processappdata * pad = (struct processappdata*)malloc(sizeof(struct processappdata));
	memset(pad, 0, sizeof(struct processappdata));
	assert(pad != NULL);
	if(unlikely(pad == NULL)){
		fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	} 
	pad->fd_sigdb = fd_sigdb;
	pad->fd_sigupward = fd_sigupward;
	struct datalist opdbdatalist = {head, fd_sigdb};
	struct datalist upwardmsgdatalist = {head, fd_sigupward};

	if(0 != pthread_create(&pad->threadid_db, NULL, operationdb, &opdbdatalist)){
		free(pad);
		pad = NULL;
		fprintf("thread create error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
		
	if(0 != pthread_create(&pad->threadid_upward, NULL, forwardmsg, &upwardmsgdatalist)){
		fprintf("thread create error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}


	return pad;
}

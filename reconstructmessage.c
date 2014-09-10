#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parseprotocol.h"
#include "kfifo.h"
#include "list.h"
#include "cndef.h"
#include "reconstructmessage.h"


struct reconstructmessage{
	struct parseprotocol_request * message;
	struct list_head list;
	unsigned char processed;
};

#define TMPSIZE 1024
unsigned char tmp[TMPSIZE];
unsigned int tmplen = 0;
int reconstructmessage_add(struct list_head * head, struct kfifo* fifo){ 
	memset(tmp+tmplen, 0, TMPSIZE - tmplen);
	unsigned int len = kfifo_get(fifo, tmp+tmplen, TMPSIZE - tmplen); 
	unsigned int parselen = len+tmplen;
	struct reconstructmessage* rcmsg;
	while( toolkit_cmdsep( tmp, parselen, '$') != NULL){
		rcmsg = malloc(sizeof(struct reconstructmessage));
		if(unlikely( rcmsg == NULL )){
			fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		}
		rcmsg->message = malloc(sizeof(struct parseprotocol_request));
		if(unlikely( rcmsg->message == NULL )){
			fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		}

		if( likely(0 == parseprotocol_parserequest( rcmsg->message, tmp, parselen))){ 
			list_add(&rcmsg->list, head);
		}
		rcmsg = NULL;
	}

	return 0;
}

int reconstructmessage_clear(struct reconstructmessage* msg ){ 
	assert(msg != NULL);
	if( unlikely(msg == NULL) ){
		return -1;
	}
	assert(msg->message != NULL);
	if( unlikely(msg->message == NULL )){
		return -1;
	}
	parseprotocol_clear(msg->message);
	msg->message = NULL;
	list_del(&msg->list);
	free(msg);
	msg = NULL;

	return 0;
}

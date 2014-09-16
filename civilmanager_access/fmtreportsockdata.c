#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parseprotocol.h"
#include "kfifo.h"
#include "list.h"
#include "cndef.h"
#include "fmtreportsockdata.h"
#include "toolkit.h"

#define TMPSIZE 1024
unsigned char tmp[TMPSIZE];
unsigned int tmplen = 0;
int fmtreportsockdata_add(struct list_head * head, struct kfifo* fifo, int fd){ 
	if(unlikely(fifo == NULL)){
		assert(0);
		fprintf(stderr, "arguments error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(tmp+tmplen, 0, TMPSIZE - tmplen);
	unsigned int len = kfifo_get(fifo, tmp+tmplen, TMPSIZE - tmplen); 
	unsigned int parselen = len+tmplen;
	struct fmtreportsockdata* rcmsg;
	while( toolkit_cmdsep( tmp, parselen, '$') != NULL){
		rcmsg = (struct fmtreportsockdata*)malloc(sizeof(struct fmtreportsockdata));
		if(unlikely( rcmsg == NULL )){
			fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		}
		rcmsg->message = (struct parseprotocol_request*)malloc(sizeof(struct parseprotocol_request));
		rcmsg->message.fd = fd;
		if(unlikely( rcmsg->message == NULL )){
			fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		}

		if( likely(0 == parseprotocol_parserequest( rcmsg->message, tmp, parselen))){ 
			list_add_tail(&rcmsg->list, head);
		}
		rcmsg = NULL;
	}

	return 0;
}

int fmtreportsockdata_clear(struct fmtreportsockdata* msg ){ 
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

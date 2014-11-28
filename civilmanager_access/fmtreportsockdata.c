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
#include "sockets_buffer.h"

#define TMPSIZE 1024
unsigned char tmp[TMPSIZE];
unsigned int tmplen = 0;

int fmtreportsockdata_add(struct sockets_buffer * sbuf, int fd){
	struct kfifo* fifo = sockets_buffer_getfifo(sbuf, fd);
	struct list_head* highpri_head = sockets_buffer_gethighlist(sbuf, fd);
	struct list_head* normalpri_head = sockets_buffer_getnormallist(sbuf, fd);

	memset(tmp+tmplen, 0, TMPSIZE - tmplen);
	unsigned int len = kfifo_get(fifo, tmp+tmplen, TMPSIZE - tmplen); 
	unsigned int parselen = len+tmplen;
	struct fmtreportsockdata* rcmsg;
	int retcode = 0;
	while( toolkit_cmdsep( tmp, parselen, '$') != NULL){
		rcmsg = (struct fmtreportsockdata*)malloc(sizeof(struct fmtreportsockdata));
		if(unlikely( rcmsg == NULL )){
			fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		}
		rcmsg->message = (struct parseprotocol_request*)malloc(sizeof(struct parseprotocol_request));
		if(unlikely( rcmsg->message == NULL )){
			fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		}

		retcode = parseprotocol_parserequest( rcmsg->message, tmp, parselen);
		if(retcode == REQ_LOGIN){
			list_add_tail(rcmsg->list, highpri_head);
		}else(retcode == REQ_LOGOFF || retcode == REQ_HEARTBEAT || retcode == REQ_REQ){
			list_add_tail(rcmsg->list, normalpri_head);
		}

		rcmsg = NULL;
	}

	sockets_buffer_signal(sbuf, fd);

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
	free(msg);
	msg = NULL;

	return 0;
}

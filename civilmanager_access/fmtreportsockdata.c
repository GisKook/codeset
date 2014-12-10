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

unsigned char *buffer = NULL;
int bufferlen = 0; 

int fmtreportsockdata_add(struct sockets_buffer * sbuf, int fd){
	struct kfifo* fifo = sockets_buffer_getrawdata(sbuf, fd);
	struct list_head* highpri_head = sockets_buffer_gethighlist(sbuf, fd);
	struct list_head* normalpri_head = sockets_buffer_getnormallist(sbuf, fd);

	int fifolen = kfifo_len(fifo);
	if(bufferlen < fifolen){
		buffer = (unsigned char *)malloc(sizeof(char)*fifolen);
		bufferlen = fifolen;
		if(buffer == NULL){
			fprintf(stderr, "malloc %d bytes erro. %s %s %d\n", fifolen, __FILE__, __FUNCTION__, __LINE__);

			return 0;
		}
	}
	memset(buffer, 0, fifolen); 

	unsigned int len = kfifo_get(fifo, buffer, fifolen);
	unsigned char * cmd = NULL;
	unsigned int cmdlen = 0;
	struct fmtreportsockdata* rcmsg = NULL;
	int retcode = 0;
	
	while( len != 0){
		cmd = toolkit_cmdsep( &buffer, &len,&cmdlen, '$');
		rcmsg = (struct fmtreportsockdata*)malloc(sizeof(struct fmtreportsockdata));
		memset(rcmsg, 0, sizeof(struct fmtreportsockdata));
		if(unlikely( rcmsg == NULL )){
			fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		}
		rcmsg->message = (struct parseprotocol_request*)malloc(sizeof(struct parseprotocol_request));
		if(unlikely( rcmsg->message == NULL )){
			fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		}

		retcode = parseprotocol_parserequest( rcmsg->message, cmd, cmdlen);
		fprintf(stdout, " 1 ");
		if(retcode == REQ_LOGIN){
			list_add_tail(&(rcmsg->list), highpri_head);
			sockets_buffer_signal(sbuf, fd);
		}else if(retcode == REQ_LOGOFF || retcode == REQ_HEARTBEAT || retcode == REQ_REQ){
			list_add_tail(&rcmsg->list, normalpri_head);
			sockets_buffer_normaltasksignal(sbuf, fd);
		}else{
			free(rcmsg->message);
			rcmsg->message = NULL;
			free(rcmsg);
			rcmsg = NULL;
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
	free(msg);
	msg = NULL;

	return 0;
}

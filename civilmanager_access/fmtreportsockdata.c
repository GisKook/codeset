#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "parseprotocol.h"
#include "kfifo.h"
#include "list.h"
#include "cndef.h"
#include "fmtreportsockdata.h"
#include "toolkit.h"
#include "sockets_buffer.h"
#include "cardmanager.h"

int fmtreportsockdata_add(struct sockets_buffer * sbuf, int fd, struct cardmanager * cardmanager){
	struct kfifo* fifo = sockets_buffer_getrawdata(sbuf, fd);
	assert(fifo != NULL);
	struct list_head* highpri_head = sockets_buffer_gethighlist(sbuf, fd);
	assert(highpri_head != NULL);
	struct list_head* normalpri_head = sockets_buffer_getnormallist(sbuf, fd);
	assert(normalpri_head != NULL);

	unsigned char *buffer = NULL;
	unsigned char * initbuffer = NULL;

	int fifolen = kfifo_len(fifo);
	buffer = (unsigned char *)malloc(sizeof(unsigned char)*fifolen);
	initbuffer = buffer;
	memset(buffer, 0, fifolen); 

	unsigned int len = kfifo_get(fifo, buffer, fifolen);
	unsigned char * cmd = NULL;
	unsigned int cmdlen = 0;
	struct fmtreportsockdata* rcmsg = NULL;
	int retcode = 0;
	while(len != 0){
		cmd = parseprotocol_pickcmd(&buffer, &len, &cmdlen);
		if(cmd != NULL){
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
			if(retcode == REQ_LOGIN){
				list_add_tail(&(rcmsg->list), highpri_head);
				sockets_buffer_signal(sbuf, fd);
			}else if(retcode == REQ_LOGOFF || retcode == REQ_HEARTBEAT || retcode == REQ_REQ){
				if( cardmanager_search(cardmanager, fd) != NULL){
					list_add_tail(&rcmsg->list, normalpri_head);
					sockets_buffer_normaltasksignal(sbuf, fd);
				}else{
					struct sockaddr addr;
					socklen_t addrlen;
					char ipstr[256] = {0};
					int port = 0;
					if(0 == getpeername(fd, &addr, &addrlen)){
						struct sockaddr_in *s = (struct sockaddr_in *)&addr;
						if (s->sin_family == AF_INET) {
							port = ntohs(s->sin_port);
							inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
						}
					}
					fprintf(stdout, "unlogin connection find %s %d\n", ipstr, port);
				}
			}else if(retcode == -2){ 

			}else{
				free(rcmsg->message);
				rcmsg->message = NULL;
				free(rcmsg);
				rcmsg = NULL;
			}

			rcmsg = NULL;
		}
	}

	free(initbuffer);

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

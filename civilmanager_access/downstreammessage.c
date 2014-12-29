//将经过程序处理后的数据或者反馈发送给client.
#include "sockets_buffer.h"
#include "encodeprotocol.h"
#include "mqueue.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

struct downstreammessage{
	struct sockets_buffer * sockbuffer;
	pthread_t tid;
};

void * downstream(void *param){ 
	struct sockets_buffer * sockbuffer = (struct sockets_buffer *)param;
	int * activefds = NULL;
	struct mqueue * mqueue = NULL;
	int len = 0;
	int i,j;
	int count = 0;
	struct reader_result res;
	struct encodeprotocol_respond * epr = NULL;
	unsigned char * content = NULL;
	int contentlen = 0;
	int writelen = 0;
	int fd;
	for(;;){ 
		activefds = sockets_buffer_getdownstreamsignal(sockbuffer);
		len = *activefds; 
		for(i = 0; i < len; ++i){
			fd = activefds[i+1];
			mqueue = sockets_buffer_getwritequeue(sockbuffer, fd);
			if((count = mqueue_reader_parpare(mqueue, &res)) > 0){ 
				for(j = 0; j < count; ++j){ 
					epr = (struct encodeprotocol_respond *)mqueue_reader_next(&res);
					if(epr != NULL){
						switch(epr->messagetype){
							case RES_LOGIN: 
								content= encodeprotocol_respondlogin(epr->message.respondlogin, contentlen);
								break;
							case RES_HEARTBEAT:
								content = encodeprotocol_respondheartbeat(epr->message.respondheartbeat, contentlen);
								break;
							case RES_POSITION:
								content = encodeprotocol_positioninfo(epr->message.postioninfo, contentlen);
								break;
							case RES_COMMUNICATION:
								content = encodeprotocol_communicationinfo(epr->message.communicationinfo, contentlen);
								break; 
							case RES_COMMUNICATIONRECEIPT:
								content = encodeprotocol_communicationreceipt(epr->message.communicationreceipt, contentlen);
								break;
							case RES_SENDFEEDBACK:
								content = encodeprotocol_sendfeedback(epr->message.sendfeedback, contentlen);
								break;
						}
						int ret;
						writelen = 0;
						while(writelen < contentlen){ 
							ret = write(fd, content, contentlen);
							if(ret < 0 && errno != EAGAIN){
								close(fd);
								break;
							}
							writelen += ret;
						}
						free(content);
						content = NULL;
					}
				}
				mqueue_reader_commit(mqueue, &res);
			}
		}
	}

	return NULL;
}

struct downstreammessage * downstreammessage_create(struct sockets_buffer * sbuffer){ 
	struct downstreammessage * downstreammessage = (struct downstreammessage *)malloc(sizeof(struct downstreammessage));
	downstreammessage->sockbuffer = sbuffer; 
	pthread_t tid;
	pthread_create(&tid, NULL, downstream, sbuffer);
	downstreammessage->tid = tid;

	return downstreammessage;
}

void downstreammessage_destroy(struct sockets_buffer * sbuffer){
	free(sbuffer);
}

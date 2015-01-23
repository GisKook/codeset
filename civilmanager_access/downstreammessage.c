//将经过程序处理后的数据或者反馈发送给client.
#include "sockets_buffer.h"
#include "encodeprotocol.h"
#include "processappdata.h"
#include "mqueue.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

struct downstreammessage{
	struct processappdata * processappdata;
	pthread_t tid;
};

void * downstream(void *param){ 
	struct processappdata * pad = (struct processappdata *)param;
	 
	struct sockets_buffer * sockbuffer = processappadata_getsocketbuffer(pad);
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
								sockets_buffer_del(sockbuffer, fd);
								processappdata_delete(pad, fd);

								close(fd);
								fprintf(stdout, "close connect %d\n", fd);
								break;
							}
							writelen += ret;
						}
						free(content);
						content = NULL;
						encodeprotocol_clear(epr);
					}
				}
				mqueue_reader_commit(mqueue, &res);
			}
		}
		free(activefds);
	}

	return NULL;
}

struct downstreammessage * downstreammessage_create(struct processappdata * pad){ 
	struct downstreammessage * downstreammessage = (struct downstreammessage *)malloc(sizeof(struct downstreammessage));
	downstreammessage->processappdata= pad; 
	pthread_t tid;
	if(0 != pthread_create(&tid, NULL, downstream, pad)){
		fprintf(stderr, "create downstream message thread error.\n");
		free(downstreammessage);

		return NULL;
	}
	fprintf(stdout, "thread 0x%lx downstreammessage create successfully.\n", tid); 

	downstreammessage->tid = tid;

	return downstreammessage;
}

void downstreammessage_destroy(struct downstreammessage * downstreammessage){
	free(downstreammessage);
}

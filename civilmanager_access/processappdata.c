#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include <zmq.h>
#include "fmtreportsockdata.h"
#include "parseprotocol.h"
#include "list.h"
#include "sockets_buffer.h"
#include "toolkit.h" 
#include "cndef.h"
#include "zmq_buffer.h"
#include "loginmanager.h"
#include "loginenterprisemanager.h"
#include "list.h"
#include "encodeprotocol.h"
#include "downstreammessage.h"

#define MAX_FIFO_LEN 4096
struct processappdata{
	pthread_t threadid_fmt;
	pthread_t threadid_upward;
	pthread_t threadid_downward;
	int fd_sigfmt;
	struct sockets_buffer* sbuf;
	struct loginmanager * loginmanager;
	struct downstreammessage * dsm;
	struct loginenterprisemanager * loginenterprisemanager;
};

void * processlogin(void * param){
	struct processappdata * pad = (struct processappdata*)param; 
	struct loginmanager * loginmanager = pad->loginmanager;
	struct loginenterprisemanager * loginenterprisemanager = pad->loginenterprisemanager;
	struct login * logindatadb;
	int * fds;
	int fdscount = 0;
	struct list_head *loginlist;
	int i;
	struct fmtreportsockdata * logindata; 
	struct parseprotocol_request * request;
	struct list_head * pos, * n;
	char * login;
	char * password;
	int loginresult = -1; // 0 成功 1 密码错误 2 没有此用户 3 该用户已经登录
	struct encodeprotocol_respond epr;
	struct respondlogin respondlogin;
	memset(&respondlogin, 0, sizeof(struct respondlogin));
	int len = 0;
	for(;;){
		fds = sockets_buffer_getsignalfdfifo(pad->sbuf);

		fdscount = fds[0]; 
		for(i = 0; i < fdscount; ++i){ 
			loginlist = sockets_buffer_gethighlist(pad->sbuf, fds[i+1]); 
			list_for_each_safe(pos,n,loginlist){ 
				logindata = list_entry(pos, struct fmtreportsockdata, list);
				assert(logindata != NULL);

				request = logindata->message;
				if(request->messagetype == REQ_LOGIN){ 
					login = request->message.login->account;
					password = request->message.login->password; 
					logindatadb = loginmanager_search(loginmanager, login);

					memset(respondlogin.account, 0, 12);
					memcpy(respondlogin.account, login, MIN(12, strlen(login)));
					if(logindatadb == NULL){
						respondlogin.loginresult = 2;
					}else{ 
						if(1 == loginenterprisemanager_search(loginenterprisemanager, logindatadb->enterpriseid, login)){
							respondlogin.loginresult = 3;
						}

						if(strlen(password) == strlen(logindatadb->password) && strcmp(password, logindatadb->password)){ 
							respondlogin.loginresult = 0;
							loginenterprisemanager_insert(loginenterprisemanager, logindatadb->enterpriseid, logindatadb->login, fds[i+1]);
						}else{
							respondlogin.loginresult = 1;
						}
					}
					epr.messagetype = RES_LOGIN;
					epr.message.respondlogin= &respondlogin;
					sockets_buffer_write(pad->sbuf, fds[i+1], &epr);
					if(logindata != NULL){
						list_del(&logindata->list);
						free(logindata);
						logindata = NULL;
					}
				} else{
					assert(0);
				}
			}
		}
	}

	return NULL;
}

void * processmessage(void * param){ 
	void* ctx = zmq_ctx_new();
	assert(ctx != NULL);
	void* socket = zmq_socket(ctx, ZMQ_PUSH); 
	assert(socket != NULL);
	int rc = zmq_bind(socket, "tcp://*:8888");
	assert(rc == 0);

	struct processappdata * pad = (struct processappdata*)param; 
	struct loginenterprisemanager * loginenterprisemanager = pad->loginenterprisemanager;
	int * fds;
	int fdscount = 0;
	struct list_head *tasklist;
	int i;
	struct fmtreportsockdata * fmtrepdata = NULL; 
	struct parseprotocol_request * request;
	struct list_head * pos, * n;
	struct encodeprotocol_respond epr;
	struct respondheartbeat respondheartbeat;
	memset(&respondheartbeat, 0, sizeof(struct respondheartbeat));
	struct positioninfo positioninfo;
	memset(&positioninfo, 0, sizeof(struct positioninfo));
	struct communicationinfo communicationinfo;
	memset(&communicationinfo, 0, sizeof(struct communicationinfo));
	struct communicationreceipt communicationreceipt;
	memset(&communicationreceipt, 0, sizeof(struct communicationreceipt));
	struct sendfeedback sendfeedback;
	memset(&sendfeedback, 0, sizeof(struct sendfeedback));
	int len = 0;
	for(;;){
		fds = sockets_buffer_getnormaltasklist(pad->sbuf);
		fdscount = fds[0];
		for( i = 0; i < fdscount; ++i){
			tasklist = sockets_buffer_getnormallist(pad->sbuf, fds[i+1]);
			list_for_each_safe(pos, n, tasklist){
				fmtrepdata = list_entry(pos, struct fmtreportsockdata, list);
				assert(fmtrepdata != NULL); 
				request = fmtrepdata->message;
				switch(request->messagetype){
					case REQ_HEARTBEAT:
						epr.messagetype = RES_HEARTBEAT;
						memset(&respondheartbeat, 0, sizeof(struct respondheartbeat));
						memcpy(respondheartbeat.account, request->message.heartbeat->account, MIN(12, strlen(request->message.heartbeat->account)));
						epr.message.respondheartbeat = &respondheartbeat;
						sockets_buffer_write(pad->sbuf, fds[i+1], &epr);
						break;
					case REQ_LOGOFF:
						loginenterprisemanager_delete(loginenterprisemanager, request->message.logoff->account, fds[i+1]);
						close(fds[i+1]);
						break;
					case REQ_REQ:
						break;
					default:
						break;
				}
				if(fmtrepdata != NULL){
					list_del(pos);
					free(fmtrepdata);
					fmtrepdata = NULL;
				}
			}
		}

	}
	zmq_msg_t msg;
	rc = zmq_msg_init(&msg);
	zmq_close(socket);
	zmq_ctx_destroy(ctx);

	pthread_exit(0);
}

//int proexit(struct processappdata* pad, pthread_t tid_db, pthread_t tid_upward){
//	char tipsbuf[64] = {0};
//	sprintf(tipsbuf, "thread 0x%lx prepare to exit.\n", pthread_self()); 
//	fprintf(stdout, tipsbuf);
//	pthread_join(tid_db, NULL);
//	pthread_join(tid_upward, NULL);
//
//	return 0;
//}

void * formatmessage(void * p){
	struct processappdata * pad = (struct processappdata *)p;
	pthread_t tid_fmt = pad->fd_sigfmt;
	char * buf = (char *)malloc(MAX_FIFO_LEN);
	char * primerbuffer = buf;
	char * tok = NULL;

	int len;
	int socketfd;
	for(;;){ 
		memset(primerbuffer, 0, MAX_FIFO_LEN);
		len = read(pad->fd_sigfmt, buf, MAX_FIFO_LEN);
		while((tok = toolkit_strsep(&buf, '*')) != NULL){ 
			socketfd=atoi(tok); 
			if(unlikely(socketfd == 1)){ // magic number 1 means exit.
				//			exit(pad, tid_db, tid_upward);
				free(buf);
				return NULL;
			}
			fmtreportsockdata_add(pad->sbuf, socketfd);
		}
	}
}

struct processappdata * processappdata_create(struct sockets_buffer * sbuf, struct loginmanager * loginmanager, int fd_sigfmt){
	
	struct loginenterprisemanager * loginenterprisemanager = loginenterprisemanager_create();
	struct processappdata * pad = (struct processappdata*)malloc(sizeof(struct processappdata));
	memset(pad, 0, sizeof(struct processappdata));
	assert(pad != NULL);
	if(unlikely(pad == NULL)){
		fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	} 
	pad->fd_sigfmt = fd_sigfmt;
	pad->sbuf = sbuf;
	pad->loginmanager = loginmanager;
	pad->loginenterprisemanager = loginenterprisemanager;

	if(0 != pthread_create(&pad->threadid_upward, NULL, processmessage, pad)){
		free(pad);
		pad = NULL;
		fprintf(stderr, "thread upword error. %s %s %d\n",__FILE__, __FUNCTION__, __LINE__);

		return NULL;
	}
	fprintf(stdout, "thread 0x%lx upword application data create successfully.\n", pad->threadid_upward);

	if(0 != pthread_create(&pad->threadid_downward, NULL, processlogin, pad)){
		free(pad);
		pad = NULL;
		fprintf(stderr, "thread processlogin error. %s %s %d\n",__FILE__, __FUNCTION__, __LINE__);

		return NULL;
	}
	fprintf(stdout, "thread 0x%lx processlogin create successfully.\n", pad->threadid_downward);

	if(0 != pthread_create(&pad->threadid_fmt, NULL, formatmessage, pad)){
		free(pad);
		pad = NULL;
		fprintf(stderr,"thread create error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	fprintf(stdout, "thread 0x%lx format application data create successfully.\n", pad->threadid_fmt);


	struct downstreammessage * dsm = downstreammessage_create(sbuf);
	pad->dsm = dsm;

	return pad;
}

int processappdata_join(struct processappdata* pad){
	return pthread_join(pad->threadid_fmt, NULL);
}

int processappdata_destroy(struct processappdata* pad){
	assert(pad != NULL);
	if(likely(pad != NULL)){ 
		free(pad);
		pad = NULL;
	}

	return 0;
}

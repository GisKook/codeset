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
};

void * processlogin(void * param){
	struct processappdata * pad = (struct processappdata*)param; 
	struct loginmanager * loginmanager = pad->loginmanager;
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
	struct loginenterprisemanager * loginenterprisemanager = loginenterprisemanager_create();
	struct encodeprotocol_respond epr;
	struct respondlogin respondlogin = {{0}, -1};
	int len = 0;
	for(;;){
		fds = sockets_buffer_getsignalfdfifo(pad->sbuf);
		fdscount = fds[0]; 
		for(i = 0; i < fdscount; ++i){ 
			loginlist = sockets_buffer_gethighlist(pad->sbuf, fds[i+1]); 
			list_for_each_safe(pos,n,loginlist){ 
				logindata = list_entry(pos, struct fmtreportsockdata, list);
				request = logindata->message;
				if(request->messagetype == REQ_LOGIN){ 
					login = request->message.login->account;
					password = request->message.login->password; 
					logindata = NULL;
					logindatadb = loginmanager_search(loginmanager, login);

					memset(respondlogin.account, 0, 12);
					memcpy(respondlogin.account, login, MIN(12, strlen(login)));
					if(logindata == NULL){
						respondlogin.loginresult = 2;
						continue;
					}else{ 
						if(1 == loginenterprisemanager_search(loginenterprisemanager, logindatadb->enterpriseid, login)){
							respondlogin.loginresult = 3;
							continue;
						}

						if(strlen(password) == strlen(logindatadb->password) && strcmp(password, logindatadb->password)){ 
							respondlogin.loginresult = 0;
							loginenterprisemanager_insert(loginenterprisemanager, logindatadb->enterpriseid, logindatadb->login, fds[i+1]);
							continue;
						}else{
							respondlogin.loginresult = 1;
							continue;
						}
					}
					epr.messagetype = RES_LOGIN;
					epr.message.respondlogin= &respondlogin;
					sockets_buffer_write(pad->sbuf, fds[i+1], &epr);
				}
			}
		}
	}

	return NULL;
}

void * processmessage(void * param){ 
//	void* ctx = zmq_ctx_new();
//	assert(ctx != NULL);
//	void* socket = zmq_socket(ctx, ZMQ_PUSH); 
//	assert(socket != NULL);
//	int rc = zmq_bind(socket, "tcp://*:8888");
//	assert(rc == 0);
//
//	struct list_head * head = ((struct processappdata*)param)->head;
//	int fd = ((struct processappdata*)param)->fd_sigprocess;
//	struct fmtreportsockdata* entry;
//	struct list_head *pos, *n;
//	for(;;){
//		read(fd, buf, 1); 
//		list_for_each_safe(pos, n, head){
//			entry = container_of(pos, struct fmtreportsockdata, list); 
//			switch( entry->messagetype ){
//				case REQ_LOGIN:
//					break;
//				case REQ_LOGOFF:
//					break;
//				case REQ_HEARTBEAT:
//					break;
//				case REQ_REQ:
//					break;
//defalut:
//					assert(0);
//			}
//		}
//	}
//	zmq_msg_t msg;
//	rc = zmq_msg_init(&msg);
//	zmq_close(socket);
//	zmq_ctx_destroy(ctx);

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
	char * buf = (char*)malloc(MAX_FIFO_LEN);
	memset(buf, 0, MAX_FIFO_LEN);

	int len;
	int socketfd;
	char* tmp = buf;
	int leftlength = 0;
	for(;;){ 
		len = read(pad->fd_sigfmt, buf+leftlength, MAX_FIFO_LEN);
		while(toolkit_strsep2(buf, '*') != NULL){ 
			socketfd=atoi(buf); 
			if(unlikely(socketfd == 1)){ // magic number 1 means exit.
	//			exit(pad, tid_db, tid_upward);
				free(buf);
				buf = NULL;
				return NULL;
			}
			fmtreportsockdata_add(pad->sbuf, socketfd);
		}
		leftlength = strlen(buf);
		memmove(tmp, buf, leftlength);
		memset(tmp+leftlength, 0 , len - leftlength); 
		buf = tmp; 
	}
}

struct processappdata * processappdata_create(struct sockets_buffer * sbuf, struct loginmanager * loginmanager, int fd_sigfmt){
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

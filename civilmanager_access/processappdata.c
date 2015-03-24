#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include <zmq.h>
#include <sys/select.h>
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
#include "cardmanager.h"
#include "dbcardinfo.h"
#include "connectionmanager.h"
#include "cnconfig.h"

#define MAX_FIFO_LEN 4096
#define MAX_ZMQ_FIFO_LEN 40960
#define LOGINSUCCESS 0
#define LOGINPASSWORDERROR 1
#define LOGINILLEGALUSER 2
#define LOGINREPEAT 3

struct processappdata{
	pthread_t threadid_fmt;
	pthread_t threadid_upward;
	pthread_t threadid_downward;
	pthread_t threadid_checkheart;
	int fd_sigfmt;
	struct sockets_buffer* sbuf;
	struct loginmanager * loginmanager;
	struct downstreammessage * dsm;
	struct loginenterprisemanager * loginenterprisemanager;
	struct cardmanager * cardmanager;
	struct zmq_buffer * zmq_buffer;
	struct connectionmanager * connectionmanager;
};

void processappdata_delete(struct processappdata * pad, int fd); 

void * processcheckheart(void * param){
	struct processappdata * processappdata = (struct processappdata *)param;
	const char * sztimeout = cnconfig_getvalue(TIMEOUT);
	int timeout = atoi(sztimeout);
	struct timeval timeval; 
	struct connectionmanager * connectionmanager = processappdata->connectionmanager;
	memset(&timeval, 0, sizeof(struct timeval));
	int *fds;
	int fdcounts;
	int i;

	for(;;){
		timeval.tv_sec = timeout;
		select(0, NULL, NULL, NULL, &timeval);
		fds = connectionmanager_gettimeout(connectionmanager, timeout, &fdcounts);
		for(i = 0; i < fdcounts; ++i){
			processappdata_delete(processappdata, fds[i]);
			sockets_buffer_del(processappdata->sbuf, fds[i]);
			fprintf(stdout, "close connection timeout. %d \n", fds[i]);
			
			close(fds[i]);
		}
		connectionmanager_resettimeout(connectionmanager);
	}

	return NULL;
}

void * processlogin(void * param){
	struct processappdata * pad = (struct processappdata*)param; 
	struct loginmanager * loginmanager = pad->loginmanager;
	struct loginenterprisemanager * loginenterprisemanager = pad->loginenterprisemanager;
	struct connectionmanager * connectionmanager = pad->connectionmanager;
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
	struct encodeprotocol_respond epr;
	struct respondlogin respondlogin;
	memset(&respondlogin, 0, sizeof(struct respondlogin));
	for(;;){
		fds = sockets_buffer_getsignalfdfifo(pad->sbuf);

		fdscount = fds[0]; 
		for(i = 0; i < fdscount; ++i){
			loginlist = sockets_buffer_gethighlist(pad->sbuf, fds[i+1]); 
			pos = NULL; n = NULL;
			list_for_each_safe(pos,n,loginlist){ 
				logindata = NULL;
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
						respondlogin.loginresult = LOGINILLEGALUSER;
					}else{ 
						if(1 == loginenterprisemanager_search(loginenterprisemanager, logindatadb->enterpriseid, login)){
							respondlogin.loginresult = LOGINREPEAT;
						}else if(strlen(password) == strlen(logindatadb->password) && strcmp(password, logindatadb->password) == 0){ 
							respondlogin.loginresult = LOGINSUCCESS;
							loginenterprisemanager_insert(loginenterprisemanager, logindatadb->enterpriseid, logindatadb->login, fds[i+1]);

							struct connection * connection = connection_create(fds[i+1], logindatadb->login, logindatadb->loginname, logindatadb->enterpriseid);
							connectionmanager_insert(connectionmanager, connection);
						}else{
							respondlogin.loginresult = LOGINPASSWORDERROR;
						}
					}
					epr.messagetype = RES_LOGIN;
					epr.message.respondlogin= &respondlogin;
					sockets_buffer_write(pad->sbuf, fds[i+1], &epr);
				} 
				if(logindata != NULL){
					list_del(&logindata->list);
					fmtreportsockdata_clear(logindata);
				}
			}
		}
		free(fds);
	}

	return NULL;
}

void * processmessage(void * param){
	struct processappdata * pad = (struct processappdata*)param; 

	struct zmq_buffer * _zmq_buffer = zmq_buffer_create(pad->sbuf, pad->cardmanager, pad->loginenterprisemanager, MAX_ZMQ_FIFO_LEN); 
	assert(_zmq_buffer); 
	pad->zmq_buffer = _zmq_buffer;


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
	struct zmq_buffer * zmq_buffer = pad->zmq_buffer;
	struct connectionmanager * connectionmanager = pad->connectionmanager;
	for(;;){
		fds = sockets_buffer_getnormaltasklist(pad->sbuf);
		fdscount = fds[0];
		for( i = 0; i < fdscount; ++i){
			tasklist = sockets_buffer_getnormallist(pad->sbuf, fds[i+1]);
			if(tasklist == NULL){ 
				continue;
			}
			pos = NULL; n= NULL; 
			list_for_each_safe(pos, n, tasklist){
				fmtrepdata = NULL;
				fmtrepdata = list_entry(pos, struct fmtreportsockdata, list);
				assert(fmtrepdata != NULL); 
				request = fmtrepdata->message;
				switch(request->messagetype){
					case REQ_HEARTBEAT:
						connectionmanager_updateheartcheck(connectionmanager, fds[i+1]);
						epr.messagetype = RES_HEARTBEAT;
						memset(&respondheartbeat, 0, sizeof(struct respondheartbeat));
						memcpy(respondheartbeat.account, request->message.heartbeat->account, MIN(12, strlen(request->message.heartbeat->account)));
						epr.message.respondheartbeat = &respondheartbeat;
						sockets_buffer_write(pad->sbuf, fds[i+1], &epr);
						break;
					case REQ_LOGOFF:
						{
							loginenterprisemanager_delete(loginenterprisemanager, request->message.logoff->account, fds[i+1]);
							struct connection * connection = connectionmanager_delete(connectionmanager, fds[i+1]); 
							connection_destroy(connection);
							free(connection);

							close(fds[i+1]);
						}
						break;
					case REQ_REQ: 
						{ 
							struct connection * connection = connectionmanager_search(connectionmanager, fds[i+1]);
							if(connection != NULL){
								char * enterpriseid = connection_getenterpriseid(connection);
								zmq_buffer_upstream_add(zmq_buffer, fmtrepdata, 1, enterpriseid, fds[i+1],request->message.request->requestid);
							}
						}
						break;
					default:
						break;
				}
				if(fmtrepdata != NULL){
					list_del(pos);
					fmtreportsockdata_clear(fmtrepdata);
				}
			}
		}
		free(fds);

	}

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
	char * buf = (char *)malloc(MAX_FIFO_LEN);
	char * primerbuffer = buf;
	char * tok = NULL;


	int socketfd;
	for(;;){ 
		memset(buf, 0, MAX_FIFO_LEN);
		read(pad->fd_sigfmt, buf, MAX_FIFO_LEN);
		while((tok = toolkit_strsep(&buf, '*')) != NULL){ 
			socketfd=atoi(tok); 
			if(unlikely(socketfd == 1)){ // magic number 1 means exit.
				//			exit(pad, tid_db, tid_upward);
				free(primerbuffer);
				return NULL;
			}
			fmtreportsockdata_add(pad->sbuf, socketfd, pad->connectionmanager);
		}
		buf = primerbuffer;
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
	struct cardmanager * cardmanager = cardmanager_create();
	assert(cardmanager != NULL);
	struct dbcardinfo * dbcardinfo = dbcardinfo_start(cardmanager);
	assert(dbcardinfo != NULL); 
	pad->cardmanager = cardmanager;

	struct loginenterprisemanager * loginenterprisemanager = loginenterprisemanager_create();

	struct connectionmanager * connectionmanager = connectionmanager_create();
	pad->connectionmanager = connectionmanager;

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

	if(0 != pthread_create(&pad->threadid_checkheart, NULL, processcheckheart, pad)){
		free(pad);
		pad = NULL;
		fprintf(stderr,"thread create error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	fprintf(stdout, "thread 0x%lx heartcheck create successfully.\n", pad->threadid_checkheart);

	struct downstreammessage * dsm = downstreammessage_create(pad);
	pad->dsm = dsm;

	return pad;
}

int processappdata_join(struct processappdata* pad){
	return pthread_join(pad->threadid_fmt, NULL);
}

void processappdata_delete(struct processappdata * pad, int fd){ 
	struct connection * connection = connectionmanager_delete(pad->connectionmanager, fd);
	if(connection != NULL){
		loginenterprisemanager_delete(pad->loginenterprisemanager, connection_getenterpriseid(connection), fd);
		free(connection);
	}
	//assert(connection != NULL);
}

struct sockets_buffer * processappadata_getsocketbuffer(struct processappdata * pad){
	return pad->sbuf;
}

int processappdata_destroy(struct processappdata* pad){
	assert(pad != NULL);
	if(likely(pad != NULL)){ 
		free(pad);
		pad = NULL;
	}

	return 0;
}

void processappdata_printconnections(struct processappdata * pad){
	connectionmanager_print(pad->connectionmanager);
}

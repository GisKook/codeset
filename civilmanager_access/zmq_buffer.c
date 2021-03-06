#include <zmq.h>
#include <stdio.h>
#include <pthread.h>
#include <string>
#include <time.h>
#include "list.h"
#include "rbtree.h"
#include "pb/beidoumessage.pb.h"
#include "pb/smsTx.pb.h"
#include "pb/bcTx.pb.h"
#include "encodeprotocol.h"
#include "encodeprotocolupstream.h"
#include "sockets_buffer.h"
#include "fmtreportsockdata.h"
#include "loginenterprisemanager.h"
#include "cardmanager.h"
#include "cndef.h"
#include "parseprotocol.h" 
#include "cnconfig.h"
#include "toolkit.h"

using namespace std;
#define MAXFIFOLEN 1024
#define UPSTREAM 0
#define DOWNSTREAM 1
#define MESSAGETYPE_BEIDOU 0
#define MESSAGETYPE_MSM 1
#define INPORCPROTO "inproc://upstream"

int zmq_buffer_push(struct zmq_buffer *, unsigned char * buf, int len);
int zmq_buffer_pushex(struct zmq_buffer * zb, unsigned char * buf, int len);
int zmq_buffer_downstream_add(struct zmq_buffer * zmq_buffer, Beidoumessage * beidoumessage);
void zmq_buffer_downstream_push(struct zmq_buffer * zmq_buffer, char * enterpriseid, struct encodeprotocol_respond * encodeprotocol_respond);
unsigned char * zmq_buffer_generateauthentication(unsigned int sendindex, char * enterpriseid, unsigned int messagetype, unsigned int * len);
int zmq_buffer_upstream_add(struct zmq_buffer * zmq_buffer, struct fmtreportsockdata * fmtreportsockdata,  unsigned int messagetype, char * enterpriseid, int fd, unsigned int usersendindex);
struct zmq_buffer_authentication * zmq_buffer_get(struct zmq_buffer * zmq_buffer, unsigned int authenticationid); 
void zmq_buffer_charge(struct zmq_buffer * zmq_buffer, char * enterpriseid, unsigned char messagetype, unsigned long long recvid );
void _zmq_buffer_passmesssage(void * pairsocket, void * sendsocket);
int zmq_buffer_pushex(void * socket, unsigned char * buf, int len);

// 这里设置一个hash表，这个hash表可能被覆盖。也就是发送了验证消息后一直收不到回执，这应该被作为一个错误回馈给用户。 这里的sendindex是内部使用的，和用户传过来的sendindex是不一样的。
struct zmq_buffer_authentication{
	unsigned long long recvid; // 
	unsigned int internalsendindex;
	unsigned char * authenticationbuf;
	unsigned int authenticationlen;
	unsigned char * messagebuf;
	unsigned int messagelen;
	struct encodeprotocol_respond * encodeprotocol_respond;
	char enterpriseid[MAXENTERPRISEIDLEN+1];
	int fd;
	unsigned int usersendindex; // 用户提交上来的sendindex
	unsigned char messagetype; 
	unsigned char stream; 
};

struct zmq_buffer{
	void * zmq_ctx;
	void * pairsocket; 
	void * sendsocket;
	volatile unsigned int internalsendindex; 
	int slotcount;
	struct zmq_buffer_authentication ** slot;
	struct sockets_buffer * sockets_buffer;
	struct cardmanager * cardmanager;
	struct loginenterprisemanager * loginenterprisemanager;
};

static inline void zmq_buffer_clear(struct zmq_buffer * zmq_buffer, unsigned int authenticationid){
	if(zmq_buffer != NULL){
		zmq_buffer->slot[authenticationid % zmq_buffer->slotcount] = NULL;
	}
}

#if 0
void * recv_downstream(void* p){
	void * ctx = zmq_ctx_new();
	assert(ctx);
	void * socket = zmq_socket(ctx, ZMQ_SUB);
	assert(socket);
	int rc = zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
	assert(rc == 0);
	const char * zmqrecvaddr = cnconfig_getvalue(ZMQRECVADDR);
	rc = zmq_connect(socket, zmqrecvaddr);
	assert(rc == 0);

	GOOGLE_PROTOBUF_VERIFY_VERSION;

	struct zmq_buffer * zb = (struct zmq_buffer*)p;
	string str;
	Beidoumessage * bdmsg = NULL;
	BsfkMsg * authentication = NULL;
	FsfkMsg * sendfeedbackmessage = NULL;

	struct parseprotocoldownstream * parseprotocoldownstream = NULL;
	int messagetype = downstream_unknown;
	for(;;){
		zmq_msg_t msg;
		int rc = zmq_msg_init(&msg);
		assert(rc == 0); 
		rc = zmq_msg_recv(&msg, socket, 0);
		assert(rc != -1); 
		if(rc > 0){
			parseprotocoldownstream = parseprotocoldownstream_parse((char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
			messagetype = parseprotocoldownstream_getmessagetype(parseprotocoldownstream);

			switch(messagetype){
				case downstream_beidoumessage:
					bdmsg = parseprotocoldownstream->message.beidoumessage; 
					zmq_buffer_downstream_add(zb, bdmsg); 
					break;
				case downstream_authentication:
					{
						authentication = parseprotocoldownstream->message.authentication;
						struct zmq_buffer_authentication * zba = zmq_buffer_get(zb, authentication->nauthenticationid());
						if(authentication != NULL && authentication->nres() == 0 && zba != NULL){
							if(zba->internalsendindex == authentication->nauthenticationid()){
								if(zba->stream == UPSTREAM){  // 收到反馈才算是成功
									zmq_buffer_push(zb, zba->messagebuf, zba->messagelen); 
									//									free(zba->authenticationbuf);
									//									zba->authenticationbuf = NULL;
									//									free(zba->messagebuf);
									//									zba->messagebuf = NULL;
									//									free(zba);
									//									zba = NULL;
								}else if(zba->stream == DOWNSTREAM){ 
									int * fds = NULL;
									int fdscount = 0;
									fds = loginenterprisemanager_getfds(zb->loginenterprisemanager, zba->enterpriseid, &fdscount);
									if(fds != NULL){
										int i = 0;
										for(;i<fdscount;++i){
											sockets_buffer_write(zb->sockets_buffer,fds[i], zba->encodeprotocol_respond);
										}
									}
									free(zba->authenticationbuf);
									zba->authenticationbuf = NULL;
									encodeprotocol_clear(zba->encodeprotocol_respond);
									zba->encodeprotocol_respond = NULL;
									free(zba);
									zba = NULL;
									zmq_buffer_clear(zb, authentication->nauthenticationid());
								}else{
									fprintf(stderr, "recved message has no sense.  %s %s %d", __FILE__, __FUNCTION__, __LINE__);
								}
							}else{
								fprintf(stderr, "recved message has no sense.  %s %s %d", __FILE__, __FUNCTION__, __LINE__);
							}
						}else if(authentication != NULL && authentication->nres() != 0 && zba != NULL){ // 没有发送
							//0:ok,1:state error,2:no money,3:teach date,4:no register,5 no category
							struct encodeprotocol_respond epr;
							struct sendfeedback sendfeedback;
							epr.messagetype = RES_SENDFEEDBACK;
							epr.message.sendfeedback = &sendfeedback;
							sendfeedback.sendindex = zba->usersendindex;
							sendfeedback.feedback = FEEDBACK_NOMONEY;
							sockets_buffer_write(zb->sockets_buffer, zba->fd, &epr);

							free(zba->authenticationbuf);
							zba->authenticationbuf = NULL;
							free(zba->messagebuf);
							zba->messagebuf = NULL;
							free(zba);
							zba = NULL;
							zmq_buffer_clear(zb, authentication->nauthenticationid()); 
						}else{
							fprintf(stdout, "recv a error auth message. %s %s %d zba: 0x%lx authentication: 0x%lx\n", __FILE__, __FUNCTION__, __LINE__, (unsigned long int)zba, (unsigned long int)authentication); 
							toolkit_printbytes((unsigned char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
						}
					}
					break;
				case downstream_sendfeedback:
					{
						sendfeedbackmessage = parseprotocoldownstream->message.sendfeedback;
						unsigned int sendindex = sendfeedbackmessage->nserialid();
						int result = sendfeedbackmessage->nres();
						struct zmq_buffer_authentication * zba = zmq_buffer_get(zb, sendindex);
						if(zba != NULL && sendindex == zba->internalsendindex){
							if(result == 0){ 
								zmq_buffer_charge(zb, zba->enterpriseid, zba->messagetype, zba->recvid);
							}
							struct encodeprotocol_respond epr;
							struct sendfeedback sendfeedback;
							epr.messagetype = RES_SENDFEEDBACK;
							epr.message.sendfeedback = &sendfeedback;
							sendfeedback.sendindex = sendindex;
							sendfeedback.feedback = result;
							sockets_buffer_write(zb->sockets_buffer,zba->fd, &epr); 

							free(zba->authenticationbuf);
							zba->authenticationbuf = NULL;
							free(zba->messagebuf);
							zba->messagebuf = NULL;
							free(zba);
							zmq_buffer_clear(zb, sendindex);
						}else{
							fprintf(stdout, "recv a error feedback. index %u %s %s %d\n", sendindex, __FILE__, __FUNCTION__, __LINE__);
							toolkit_printbytes((unsigned char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
						}
					}
					break;
				case downstream_unknown:
					fprintf(stderr, "recv a unknow message from 中转软件.%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
					toolkit_printbytes((unsigned char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
					break;
			}

			zmq_msg_close(&msg);
			parseprotocoldownstream_clear(parseprotocoldownstream);
		}
	}
	rc = zmq_close(socket);
	zmq_ctx_destroy(ctx);
	pthread_exit(0);
}
#endif

void * zmqhub( void * param ){
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	struct zmq_buffer * zb = (struct zmq_buffer*)param;

	void * subsocket = zmq_socket(zb->zmq_ctx, ZMQ_PULL);
	assert(subsocket);
//	int rc = zmq_setsockopt(subsocket, ZMQ_SUBSCRIBE, "", 0);
//	int rc = zmq_setsockopt(subsocket, ZMQ_SUBSCRIBE, "", 0);
	const char * zmqrecvaddr = cnconfig_getvalue(ZMQRECVADDR);
	int rc = zmq_connect(subsocket, zmqrecvaddr);
	assert(rc == 0);

	void * pairsocket = zmq_socket(zb->zmq_ctx, ZMQ_PAIR);
	zmq_connect(pairsocket, INPORCPROTO);

	void * sendsocket = zmq_socket(zb->zmq_ctx, ZMQ_PUSH); 
	assert(sendsocket != NULL);
	const char * zmqsendaddr = cnconfig_getvalue(ZMQBINDADDR);
	//rc = zmq_bind(sendsocket, zmqsendaddr);
	zmq_connect(sendsocket, zmqsendaddr);
	assert(rc == 0);
	zb->sendsocket = sendsocket;

	string str;
	Beidoumessage * bdmsg = NULL;
	BsfkMsg * authentication = NULL;
	FsfkMsg * sendfeedbackmessage = NULL;

	struct parseprotocoldownstream * parseprotocoldownstream = NULL;
	int messagetype = downstream_unknown;

	for(;;){
		zmq_pollitem_t items[] = {
			{ pairsocket, 0, ZMQ_POLLIN, 0 },
			{ subsocket, 0, ZMQ_POLLIN, 0 }
		};

		rc = zmq_poll(items, 2, -1);
		assert(rc != -1);
		if(rc == -1){ break; }
		if(items[0].revents & ZMQ_POLLIN){ 
			_zmq_buffer_passmesssage(pairsocket, sendsocket);
		}
		if(items[1].revents & ZMQ_POLLIN){
			zmq_msg_t msg;
			int rc = zmq_msg_init(&msg);
			assert(rc == 0); 
			rc = zmq_msg_recv(&msg, subsocket, 0);
			assert(rc != -1); 
			if(rc > 0){
				parseprotocoldownstream = parseprotocoldownstream_parse((char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
				messagetype = parseprotocoldownstream_getmessagetype(parseprotocoldownstream);

				switch(messagetype){
					case downstream_beidoumessage:
						bdmsg = parseprotocoldownstream->message.beidoumessage; 
						zmq_buffer_downstream_add(zb, bdmsg); 
						break;
					case downstream_authentication:
						{
							authentication = parseprotocoldownstream->message.authentication;
							struct zmq_buffer_authentication * zba = zmq_buffer_get(zb, authentication->nauthenticationid());
							if(authentication != NULL && authentication->nres() == 0 && zba != NULL){
								if(zba->internalsendindex == authentication->nauthenticationid()){
									if(zba->stream == UPSTREAM){  // 收到反馈才算是成功
										zmq_buffer_pushex(zb, zba->messagebuf, zba->messagelen);
										//									free(zba->authenticationbuf);
										//									zba->authenticationbuf = NULL;
										//									free(zba->messagebuf);
										//									zba->messagebuf = NULL;
										//									free(zba);
										//									zba = NULL;
									}else if(zba->stream == DOWNSTREAM){ 
										int * fds = NULL;
										int fdscount = 0;
										fds = loginenterprisemanager_getfds(zb->loginenterprisemanager, zba->enterpriseid, &fdscount);
										if(fds != NULL){
											int i = 0;
											for(;i<fdscount;++i){
												sockets_buffer_write(zb->sockets_buffer,fds[i], zba->encodeprotocol_respond);
											}
										}
										free(zba->authenticationbuf);
										zba->authenticationbuf = NULL;
										encodeprotocol_clear(zba->encodeprotocol_respond);
										zba->encodeprotocol_respond = NULL;
										free(zba);
										zba = NULL;
										zmq_buffer_clear(zb, authentication->nauthenticationid());
									}else{
										fprintf(stderr, "recved message has no sense.  %s %s %d", __FILE__, __FUNCTION__, __LINE__);
									}
								}else{
									fprintf(stderr, "recved message has no sense.  %s %s %d", __FILE__, __FUNCTION__, __LINE__);
								}
							}else if(authentication != NULL && authentication->nres() != 0 && zba != NULL){ // 没有发送
								//0:ok,1:state error,2:no money,3:teach date,4:no register,5 no category
								struct encodeprotocol_respond epr;
								struct sendfeedback sendfeedback;
								epr.messagetype = RES_SENDFEEDBACK;
								epr.message.sendfeedback = &sendfeedback;
								sendfeedback.sendindex = zba->usersendindex;
								sendfeedback.feedback = FEEDBACK_NOMONEY;
								sockets_buffer_write(zb->sockets_buffer, zba->fd, &epr);

								free(zba->authenticationbuf);
								zba->authenticationbuf = NULL;
								free(zba->messagebuf);
								zba->messagebuf = NULL;
								free(zba);
								zba = NULL;
								zmq_buffer_clear(zb, authentication->nauthenticationid()); 
							}else{
								fprintf(stdout, "recv a error auth message. %s %s %d zba: 0x%lx authentication: 0x%lx\n", __FILE__, __FUNCTION__, __LINE__, (unsigned long int)zba, (unsigned long int)authentication); 
								toolkit_printbytes((unsigned char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
							}
						}
						break;
					case downstream_sendfeedback:
						{
							sendfeedbackmessage = parseprotocoldownstream->message.sendfeedback;
							unsigned int sendindex = sendfeedbackmessage->nserialid();
							int result = sendfeedbackmessage->nres();
							struct zmq_buffer_authentication * zba = zmq_buffer_get(zb, sendindex);
							if(zba != NULL && sendindex == zba->internalsendindex){
								if(result == 0){ 
									zmq_buffer_charge(zb, zba->enterpriseid, zba->messagetype, zba->recvid);
								}
								struct encodeprotocol_respond epr;
								struct sendfeedback sendfeedback;
								epr.messagetype = RES_SENDFEEDBACK;
								epr.message.sendfeedback = &sendfeedback;
								sendfeedback.sendindex = sendindex;
								sendfeedback.feedback = result;
								sockets_buffer_write(zb->sockets_buffer,zba->fd, &epr); 

								free(zba->authenticationbuf);
								zba->authenticationbuf = NULL;
								free(zba->messagebuf);
								zba->messagebuf = NULL;
								free(zba);
								zmq_buffer_clear(zb, sendindex);
							}else{
								fprintf(stdout, "recv a error feedback. index %u %s %s %d\n", sendindex, __FILE__, __FUNCTION__, __LINE__);
								toolkit_printbytes((unsigned char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
							}
						}
						break;
					case downstream_unknown:
						fprintf(stderr, "recv a unknow message from 中转软件.%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
						toolkit_printbytes((unsigned char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
						break;
				}

				zmq_msg_close(&msg);
				parseprotocoldownstream_clear(parseprotocoldownstream);
			}

		}
	}

	rc = zmq_close(subsocket);
	rc = zmq_close(pairsocket);
	pthread_exit(0);
}

struct zmq_buffer * zmq_buffer_create(struct sockets_buffer * sockets_buffer, struct cardmanager * cardmanager,  struct loginenterprisemanager * loginenterprisemanager, int capacity){
	struct zmq_buffer * zmq_buffer = (struct zmq_buffer*)malloc(sizeof(struct zmq_buffer));
	memset(zmq_buffer, 0, sizeof(struct zmq_buffer));
	assert(zmq_buffer);
	zmq_buffer->zmq_ctx = zmq_ctx_new();
	assert(zmq_buffer->zmq_ctx);

	zmq_buffer->pairsocket = zmq_socket(zmq_buffer->zmq_ctx, ZMQ_PAIR);
	assert(zmq_buffer->pairsocket != NULL);
	int rc = zmq_bind(zmq_buffer->pairsocket, INPORCPROTO);
	if(rc != 0){
		fprintf(stderr, "errno is %d error message: %s\n", errno, strerror(errno));
		zmq_close(zmq_buffer->pairsocket);
		zmq_ctx_destroy(zmq_buffer->zmq_ctx);

		return NULL;
	}
	assert(rc == 0);

	zmq_buffer->slotcount = capacity;
	zmq_buffer->slot = (struct zmq_buffer_authentication **)malloc(sizeof(struct zmq_buffer_authentication *)*capacity);
	memset(zmq_buffer->slot, 0, sizeof(struct zmq_buffer_authentication *)*capacity);

	if(zmq_buffer->slot == NULL){
		zmq_close(zmq_buffer->pairsocket);
		free(zmq_buffer);
		fprintf(stderr, "malloc %li bytes error.%s %s %d\n", sizeof(struct zmq_buffer_authentication)*capacity, __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	pthread_t tid;
	zmq_buffer->sockets_buffer = sockets_buffer;
	zmq_buffer->cardmanager = cardmanager;
	zmq_buffer->loginenterprisemanager = loginenterprisemanager;
	if( 0 != pthread_create(&tid, NULL, zmqhub, (void*)zmq_buffer)){
		zmq_close(zmq_buffer->pairsocket);
		free(zmq_buffer->slot);
		zmq_buffer->slot = NULL;
		free(zmq_buffer);
		zmq_buffer = NULL; 
		fprintf(stderr, "create thread error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

		return NULL;
	}
	fprintf(stdout, "thread 0x%lx zmqpub create successfully, \n", tid);

	return zmq_buffer; 
}

int zmq_buffer_push(struct zmq_buffer * zb, unsigned char * buf, int len){
	zmq_msg_t msg;
	int rc = zmq_msg_init_size(&msg, len);
	assert(rc == 0);
	memcpy(zmq_msg_data(&msg), buf, len);
	rc = zmq_msg_send(&msg, zb->pairsocket, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

	return 0;
}

int zmq_buffer_pushex(struct zmq_buffer * zb, unsigned char * buf, int len){
	zmq_msg_t msg;
	int rc = zmq_msg_init_size(&msg, len);
	assert(rc == 0);
	memcpy(zmq_msg_data(&msg), buf, len);
	rc = zmq_msg_send(&msg, zb->sendsocket, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

	return 0;
}

unsigned char * zmq_buffer_generateauthentication(unsigned int sendindex, char * enterpriseid, unsigned int messagetype, unsigned int * len){
	BsTxMsg message;
	message.set_nrecvtype(BsTxMsg_recvtype_FWJQ);
	BsTxMsg_FwjqMsg *authenticationmessage = message.mutable_fwjqmsg();
	string authenticationstring;
	authenticationmessage->set_nauthenticationid(sendindex);
	authenticationmessage->set_sqtsentid(enterpriseid);
	authenticationmessage->set_ncategory(messagetype); // 1 for 北斗发送 2 for 短信发送
	bool rc = message.SerializeToString(&authenticationstring);

	assert(rc); 
	authenticationmessage->clear_sqtsentid();
	message.clear_fwjqmsg();
	message.release_fwjqmsg();
	delete authenticationmessage;
	message.Clear();
	struct encodeprotocolupstream * encodeprotocolupstream = encodeprotocolupstream_create(BUSINESSSOFTWARE, 1, SENDSOFTWARE, (unsigned char *)authenticationstring.c_str(), authenticationstring.length()); // 1 means charge software
	if(encodeprotocolupstream == NULL){
		*len = 0;
		return NULL;
	}
	unsigned int authenticationlen = encodeprotocolupstream_getmessagelen(encodeprotocolupstream); 
	unsigned char * authenticationbuffer = (unsigned char *)malloc(sizeof(unsigned char)*authenticationlen);
	memcpy(authenticationbuffer, encodeprotocolupstream_getmessage(encodeprotocolupstream), sizeof(unsigned char)*authenticationlen);
	*len = sizeof(unsigned char)*authenticationlen;
	encodeprotocolupstream_destroy(encodeprotocolupstream);

	return authenticationbuffer;
}

unsigned char * zmq_buffer_generatebeidoumessage(unsigned int internalsendindex, struct request * request, unsigned int * len){
	BdfsMsg beidousendmessage;
	beidousendmessage.set_nserialid(internalsendindex);
	beidousendmessage.set_nsourceaddress(request->sendaddr);
	beidousendmessage.set_ndestaddress(request->recvaddr);
	beidousendmessage.set_nmsgtype(request->encodingtype);
	beidousendmessage.set_ninfolen(request->messagelength);
	beidousendmessage.set_sinfobuff((const char *)request->message); 
	string beidoumessagestring;
	bool rc = beidousendmessage.SerializeToString(&beidoumessagestring);
	assert(rc);
	struct encodeprotocolupstream * epumessage = encodeprotocolupstream_create(BUSINESSSOFTWARE, 2, SENDSOFTWARE, (unsigned char *)beidoumessagestring.c_str(), beidoumessagestring.length()); // 2 means beidou send software
	unsigned int epumessagelen = encodeprotocolupstream_getmessagelen(epumessage);
	*len = epumessagelen;
	unsigned char * beidoumessage = (unsigned char *)malloc(epumessagelen);
	memcpy(beidoumessage, encodeprotocolupstream_getmessage(epumessage), epumessagelen);
	encodeprotocolupstream_destroy(epumessage);

	return beidoumessage; 
}

int zmq_buffer_upstream_add(struct zmq_buffer * zmq_buffer, struct fmtreportsockdata * fmtreportsockdata,  unsigned int messagetype, char * enterpriseid, int fd, unsigned int usersendindex){
	if( fmtreportsockdata->message->message.request == NULL){
		fprintf(stderr, "can't add to upstream list. the to be add is not correct. \n");

		return -1;
	}
	unsigned int internalsendindex = __sync_add_and_fetch(&zmq_buffer->internalsendindex, 1);
	unsigned int index = internalsendindex % zmq_buffer->slotcount;
	struct zmq_buffer_authentication * entry = zmq_buffer->slot[index]; 
	string authenticationstring;

	struct encodeprotocol_respond epr;
	struct sendfeedback sendfeedback;

	if(entry == NULL){
		entry = (struct zmq_buffer_authentication *)malloc(sizeof(struct zmq_buffer_authentication));
		memset(entry, 0, sizeof(struct zmq_buffer_authentication));
		entry->internalsendindex = internalsendindex;
		zmq_buffer->slot[index] = entry;
	}else{
		free(entry->authenticationbuf);
		entry->authenticationbuf = NULL;
		free(entry->messagebuf);
		entry->messagebuf = NULL;
		encodeprotocol_clear(entry->encodeprotocol_respond);
		entry->internalsendindex = internalsendindex;

		sendfeedback.sendindex = entry->usersendindex;
		sendfeedback.feedback = FEEDBACK_FULL; // 发送队列已满
		epr.messagetype = RES_SENDFEEDBACK;
		epr.message.sendfeedback = &sendfeedback;
		sockets_buffer_write(zmq_buffer->sockets_buffer, entry->fd, &epr);
		memset(entry, 0, sizeof(struct zmq_buffer_authentication));
	}
	unsigned int authenticationlen = 0;
	unsigned char * authenticationbuffer = zmq_buffer_generateauthentication(internalsendindex, enterpriseid, messagetype, &authenticationlen);
	entry->authenticationbuf = authenticationbuffer;
	entry->authenticationlen = authenticationlen;
	unsigned int beidoumessagelen = 0;
	unsigned char * beidoumessage = zmq_buffer_generatebeidoumessage(internalsendindex, fmtreportsockdata->message->message.request, &beidoumessagelen); 
	entry->messagebuf = beidoumessage;
	entry->messagelen = beidoumessagelen;

	memcpy(entry->enterpriseid, enterpriseid, MIN(strlen(enterpriseid), MAXENTERPRISEIDLEN));
	entry->fd = fd;
	entry->usersendindex = usersendindex; 
	entry->stream = UPSTREAM;

	zmq_buffer_push(zmq_buffer, entry->authenticationbuf, entry->authenticationlen);

	return 0;
}

struct encodeprotocol_respond * zmq_buffer_downstream_beidoumessage(struct zmq_buffer * zmq_buffer, Beidoumessage * bdmsg){
	struct encodeprotocol_respond * encodeprotocol_respond = NULL;
	struct positioninfo positioninfo;
	struct communicationinfo communicationinfo;
	struct communicationreceipt communicationreceipt;

	if(bdmsg->messagetype() == 0){ // positioninfo 
		positioninfo.id = bdmsg->mutable_positioninfo()->userid();
		positioninfo.accuracy = bdmsg->mutable_positioninfo()->accuracy();
		positioninfo.urgentposition = bdmsg->mutable_positioninfo()->emergencypostion();
		positioninfo.multivaluesolution = bdmsg->mutable_positioninfo()->multivaluesolution();
		positioninfo.positiontime.hour = bdmsg->mutable_positioninfo()->applytime_hour();
		positioninfo.positiontime.minutes = bdmsg->mutable_positioninfo()->applytime_minute();
		positioninfo.positiontime.seconds = bdmsg->mutable_positioninfo()->applytime_second();
		positioninfo.positiontime.tenms = bdmsg->mutable_positioninfo()->applytime_tenths();
		positioninfo.longitude = bdmsg->mutable_positioninfo()->longitude_degree()*1000000+bdmsg->mutable_positioninfo()->longitude_minute()*1000000/60 + bdmsg->mutable_positioninfo()->longitude_second()*1000000/3600 + bdmsg->mutable_positioninfo()->longitude_tenths()*1000000/36000;
		positioninfo.latitude= bdmsg->mutable_positioninfo()->latitude_degree()*1000000+bdmsg->mutable_positioninfo()->latitude_minute()*1000000/60 + bdmsg->mutable_positioninfo()->latitude_second()*1000000/3600 + bdmsg->mutable_positioninfo()->latitude_tenths()*1000000/36000;
		positioninfo.geodeticheight = bdmsg->mutable_positioninfo()->geodeticheight();
		encodeprotocol_respond = (struct encodeprotocol_respond *)malloc(sizeof(struct encodeprotocol_respond));
		encodeprotocol_respond->messagetype = RES_POSITION;
		encodeprotocol_respond->message.postioninfo = (struct positioninfo *)malloc(sizeof(struct positioninfo));
		memcpy(encodeprotocol_respond->message.postioninfo, &positioninfo, sizeof(struct positioninfo));
	}else if(bdmsg->messagetype() == 3){
		communicationinfo.sendaddr = bdmsg->mutable_commuincation()->sendaddr();
		communicationinfo.recvaddr = bdmsg->mutable_commuincation()->recvaddr();
		communicationinfo.sendtime.hour = bdmsg->mutable_commuincation()->sendtime_hour();
		communicationinfo.sendtime.minutes = bdmsg->mutable_commuincation()->sendtime_minute();
		communicationinfo.sendtime.seconds = bdmsg->mutable_commuincation()->sendtime_second();
		communicationinfo.encodingtype = bdmsg->mutable_commuincation()->messagecategory();
		communicationinfo.messagelength = bdmsg->mutable_commuincation()->messagelength();
		assert(communicationinfo.messagelength <= 210 && communicationinfo.messagelength > 0);
		memcpy(communicationinfo.message, bdmsg->mutable_commuincation()->mutable_messagebuffer(), communicationinfo.messagelength);
		encodeprotocol_respond = (struct encodeprotocol_respond *)malloc(sizeof(struct encodeprotocol_respond));
		encodeprotocol_respond->messagetype = RES_COMMUNICATION;
		encodeprotocol_respond->message.communicationinfo = (struct communicationinfo*)malloc(sizeof(struct communicationinfo));
		memcpy(encodeprotocol_respond->message.communicationinfo, &communicationinfo, sizeof(struct communicationinfo));
	}else if(bdmsg->messagetype() == 4){
		communicationreceipt.sendaddr = bdmsg->mutable_communicationreceipt()->sendaddr();
		communicationreceipt.recvaddr = bdmsg->mutable_communicationreceipt()->recvaddr();
		communicationreceipt.receipttime.hour = bdmsg->mutable_communicationreceipt()->receipttime_hour();
		communicationreceipt.receipttime.minutes = bdmsg->mutable_communicationreceipt()->receipttime_minute();
		communicationreceipt.receipttime.seconds = bdmsg->mutable_communicationreceipt()->receipttime_second();
		encodeprotocol_respond = (struct encodeprotocol_respond *)malloc(sizeof(struct encodeprotocol_respond));
		encodeprotocol_respond->messagetype = RES_COMMUNICATIONRECEIPT;
		encodeprotocol_respond->message.communicationreceipt = (struct communicationreceipt *)malloc(sizeof(struct communicationreceipt));
		memcpy(encodeprotocol_respond->message.communicationreceipt, &communicationreceipt, sizeof(struct communicationreceipt));
	}

	return encodeprotocol_respond;
}

void zmq_buffer_downstream_getcardid(Beidoumessage * beidoumessage, int * sendaddr, int * recvaddr){ 
	*sendaddr = *recvaddr = -1;
	if(beidoumessage->messagetype() == 0){ 
		*sendaddr = beidoumessage->mutable_positioninfo()->userid();
	}else if(beidoumessage->messagetype() == 3){
		*sendaddr = beidoumessage->mutable_commuincation()->sendaddr();
		*recvaddr = beidoumessage->mutable_commuincation()->recvaddr();
	}else if(beidoumessage->messagetype() == 4){
		*sendaddr = beidoumessage->mutable_communicationreceipt()->sendaddr();
		*recvaddr = beidoumessage->mutable_communicationreceipt()->recvaddr();
	}
}

void zmq_buffer_downstream_push(struct zmq_buffer * zmq_buffer, char * enterpriseid, struct encodeprotocol_respond * encodeprotocol_respond){
	if(loginenterprisemanager_check(zmq_buffer->loginenterprisemanager, enterpriseid) != 0){
		unsigned int internalsendindex = __sync_add_and_fetch(&(zmq_buffer->internalsendindex), 1);
		unsigned int index = internalsendindex % (zmq_buffer->slotcount);
		struct zmq_buffer_authentication * entry = zmq_buffer->slot[index]; 
		if(entry == NULL){
			entry = (struct zmq_buffer_authentication *)malloc(sizeof(struct zmq_buffer_authentication));
			memset(entry, 0, sizeof(struct zmq_buffer_authentication));
			zmq_buffer->slot[index] = entry;
		}else{
			fprintf(stdout, "the list is full now. the list's size is %d %s %s %d\n",zmq_buffer->slotcount, __FILE__, __FUNCTION__, __LINE__);
			struct sendfeedback sendfeedback;
			struct encodeprotocol_respond epr;
			sendfeedback.sendindex = entry->usersendindex;
			sendfeedback.feedback = FEEDBACK_FULL; // 发送队列已满
			epr.messagetype = RES_SENDFEEDBACK;
			epr.message.sendfeedback = &sendfeedback;
			sockets_buffer_write(zmq_buffer->sockets_buffer, entry->fd, &epr);
			free(entry->authenticationbuf);
			free(entry->messagebuf);
			memset(entry, 0, sizeof(struct zmq_buffer_authentication));
		}
		unsigned int authlen = 0;
		unsigned char * authbuffer = zmq_buffer_generateauthentication(internalsendindex, enterpriseid, 1, &authlen); 
		memset(entry, 0, sizeof(struct zmq_buffer_authentication));
		entry->internalsendindex = internalsendindex;
		entry->authenticationbuf = authbuffer;
		entry->authenticationlen = authlen;
		entry->encodeprotocol_respond = encodeprotocol_respond;
		memcpy(entry->enterpriseid, enterpriseid, MIN(MAXENTERPRISEIDLEN, strlen(enterpriseid)));

		entry->stream = DOWNSTREAM;
		zmq_buffer_pushex(zmq_buffer, entry->authenticationbuf, entry->authenticationlen);
	}
}

int zmq_buffer_downstream_add(struct zmq_buffer * zmq_buffer, Beidoumessage * beidoumessage){
	int sendaddr, recvaddr;
	char *enterpriseid1 = NULL; 
	char *enterpriseid2 = NULL;
	struct cardmanager * cardmanager = zmq_buffer->cardmanager;
	struct card * card = NULL;
	zmq_buffer_downstream_getcardid(beidoumessage, &sendaddr, &recvaddr);
	if(sendaddr != -1){
		card = cardmanager_search(cardmanager, sendaddr); 
		enterpriseid1 = card_getenterpriseid(card);
	}
	if(recvaddr != -1){
		card = cardmanager_search(cardmanager, recvaddr);
		enterpriseid2 = card_getenterpriseid(card); 
	}
	if(enterpriseid1 == NULL && enterpriseid2 == NULL){
		fprintf(stderr, "The card has no ower! the first addr %d second addr %d. %s %s %d\n",sendaddr, recvaddr, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	struct encodeprotocol_respond * encodeprotocol_respond1; 
	struct encodeprotocol_respond * encodeprotocol_respond2 = NULL;
	if(sendaddr != -1 && recvaddr != -1){
		if(strlen(enterpriseid1) == strlen(enterpriseid2) && strcmp(enterpriseid1, enterpriseid2) == 0){
			encodeprotocol_respond1 = zmq_buffer_downstream_beidoumessage(zmq_buffer, beidoumessage );
			zmq_buffer_downstream_push(zmq_buffer, enterpriseid1, encodeprotocol_respond1);
		}else{
			encodeprotocol_respond1 = zmq_buffer_downstream_beidoumessage(zmq_buffer, beidoumessage);
			encodeprotocol_respond2 = (struct encodeprotocol_respond *)malloc(sizeof(struct encodeprotocol_respond));
			encodeprotocol_copy(encodeprotocol_respond2, encodeprotocol_respond1);
			zmq_buffer_downstream_push(zmq_buffer, enterpriseid1, encodeprotocol_respond1);
			zmq_buffer_downstream_push(zmq_buffer, enterpriseid2, encodeprotocol_respond2);
		}
	}else if(sendaddr != -1 && recvaddr == -1){
		encodeprotocol_respond1 = zmq_buffer_downstream_beidoumessage(zmq_buffer, beidoumessage);
		zmq_buffer_downstream_push(zmq_buffer, enterpriseid1, encodeprotocol_respond1);
	}else{
		fprintf(stderr, "error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	}

	return 0;
}

struct zmq_buffer_authentication * zmq_buffer_get(struct zmq_buffer * zmq_buffer, unsigned int authenticationid){ 
	struct zmq_buffer_authentication * zba = zmq_buffer->slot[authenticationid % zmq_buffer->slotcount];
	if(zba != NULL && zba->internalsendindex == authenticationid){
		return zba; 
	}

	return NULL;
}

void zmq_buffer_charge(struct zmq_buffer * zmq_buffer, char * enterpriseid, unsigned char messagetype, unsigned long long recvid ){
	BsTxMsg message;
	message.set_nrecvtype(BsTxMsg_recvtype_KFQQ);
	BsTxMsg_KfqqMsg * chargemessage = message.mutable_kfqqmsg();
	chargemessage->set_sqtsentid(enterpriseid);
	chargemessage->set_ncategory(messagetype); // 1 for 北斗发送 2 for 短信发送
	chargemessage->set_nrecvid(recvid);
	string chargestring;
	bool rc = message.SerializeToString(&chargestring);
	chargemessage->clear_sqtsentid();
	assert(rc);
	message.clear_kfqqmsg();
	message.release_kfqqmsg();
	delete chargemessage;
	message.Clear();

	struct encodeprotocolupstream * encodeprotocolupstream = encodeprotocolupstream_create(BUSINESSSOFTWARE, 1, SENDSOFTWARE, (unsigned char *)chargestring.c_str(), chargestring.length()); // 1 means charge software
	if(encodeprotocolupstream == NULL){
		return;
	}
	zmq_buffer_pushex(zmq_buffer, encodeprotocolupstream_getmessage(encodeprotocolupstream), encodeprotocolupstream_getmessagelen(encodeprotocolupstream));

	encodeprotocolupstream_destroy(encodeprotocolupstream);
}

void _zmq_buffer_passmesssage(void * pairsocket, void * sendsocket){
	zmq_msg_t msg;
	int rc = zmq_msg_init(&msg);
	assert(rc == 0); 
	rc = zmq_msg_recv(&msg, pairsocket, 0);
	assert(rc != -1); 
	if(rc > 0){ 
		rc = zmq_msg_send(&msg, sendsocket, ZMQ_DONTWAIT);
	}

	zmq_msg_close(&msg); 
}

int zmq_buffer_destroy(struct zmq_buffer * zmq_buffer){
	int rc = zmq_ctx_destroy(zmq_buffer->zmq_ctx);
	assert(rc == 0);
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}

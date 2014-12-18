#include <zmq.h>
#include <stdio.h>
#include <pthread.h>
#include <string>
#include <time.h>
#include "list.h"
#include "pb/beidoumessage.pb.h"
#include "pb/smsTx.pb.h"
#include "pb/bcTx.pb.h"
#include "encodeprotocol.h"
#include "encodeprotocolupstream.h"

using namespace std;
#define MAXFIFOLEN 1024
// 这里设置一个hash表，这个hash表可能被覆盖。也就是发送了验证消息后一直收不到回执，这应该被作为一个错误回馈给用户。
struct zmq_buffer_authentication{
	unsigned char * buf;
	unsigned int len;
	unsigned int authenticationid;
	int fd;
};

struct zmq_buffer{
	void * zmq_ctx;
	void * recvsocket;
	void * sendsocket; 
	unsigned int minauthenticationid;
	unsigned int maxauthenticationid; 
	int slotcount;
	struct zmq_buffer_authentication ** slot;
};

void * recv_downstream(void* p){
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	struct zmq_buffer * zb = (struct zmq_buffer*)p;
	string str;
	Beidoumessage bdmsg;
	BsfkMsg authentication;
	FsfkMsg sendfeedback;
	struct positioninfo postioninfo;
	struct communicationinfo communicationinfo;
	struct communicationreceipt communicationreceipt;
	unsigned char * socket_buf;
	int socket_len;
	struct encodeprotocol_respond encodeprotocol_respond;

	struct parseprotocoldownstream * parseprotocoldownstream = NULL;
	enum downstreammessagetype messagetype = downstream_unknown;
	char * sendaddr = NULL;

	for(;;){
		zmq_msg_t msg;
		int rc = zmq_msg_init(&msg);
		assert(rc == 0); 
		rc = zmq_msg_recv(&msg, zb->recvsocket, 0);
		assert(rc != -1); 
		if(rc > 0){
			parseprotocoldownstream = parseprotocoldownstream_parse((char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
			messagetype = parseprotocoldownstream_getmessagetype(parseprotocoldownstream);
			sendaddr = parseprotocoldownstream_getsendaddr(parseprotocoldownstream);
			switch(messagetype){
				case downstream_beidoumessage:
					break;
				case downstream_authentication:
					break;
				case downstream_sendfeedback:
					break;
				case downstream_unknown:
					fprintf(stderr, "recv a unknow message from 中转软件.%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
					break;
			}

			memcpy(buffer->sendaddr, zmq_msg_data(&msg),20); 
			unsigned short messagelen = !ISBIGENDIAN:swap16(zmq_msg_data(&msg)+20):(zmq_msg_data(&msg)+20); 
			assert(messagelen == rc - 22);
			if(messagelen > 0 && messagelen < 1024){ // 1024 magic number message should not so big!
				str = string((char*)(zmq_msg_data(&msg)+22), messagelen);
				bdmsg.ParseFromString(str);
				if(bdmsg.messagetype() == 0){ // positioninfo
					positioninfo.id = bdmsg.mutable_positioninfo()->userid();
					positioninfo.accuracy = bdmsg.mutable_positioninfo()->accuracy();
					positioninfo.urgentposition = bdmsg.mutable_positioninfo()->emergencypostion();
					positioninfo.multivaluesolution = bdmsg.mutable_positioninfo()->multivaluesolution();
					positioninfo.positiontime.hour = bdmsg.mutable_positioninfo()->applaytime_hour();
					positioninfo.positiontime.minutes = bdmsg.mutable_positioninfo()->applaytime_minute();
					positioninfo.positiontime.seconds = bdmsg.mutable_positioninfo()->applaytime_second();
					positioninfo.positiontime.tenms = bdmsg.mutable_positioninfo()->applaytime_tenths();
					positioninfo.longitude = bdmsg.mutable_positioninfo()->longitude_degree()*1000000+bdmsg.mutable_positioninfo()->longitude_minute()*1000000/60 + bdmsg.mutable_positioninfo()->longitude_second()*1000000/3600 + bdmsg.mutable_positioninfo()->longitude_tenths()*1000000/36000;
					positioninfo.latitude= bdmsg.mutable_positioninfo()->latitude_degree()*1000000+bdmsg.mutable_positioninfo()->latitude_minute()1000000/60 + bdmsg.mutable_positioninfo()->latitude_second()*1000000/3600 + bdmsg.mutable_positioninfo()->latitude_tenths()*1000000/36000;
					positioninfo.geodeticheight = bdmsg.mutable_positioninfo()->geodeticheight();
					positioninfo.detlaelevation = bdmsg.mutable_positioninfo()->detlaelevation();

					socket_buf = encodeprotocol_positioninfo(&positioninfo, socket_len);
				}else if(bdmsg.messagetype() == 3){
					communicationinfo.sendaddr = bdmsg.mutable_commuincation()->sendaddr();
					communicationinfo.recvaddr = bdmsg.mutable_commuincation()->recvaddr();
					communicationinfo.sendtime.hour = bdmsg.mutable_commuincation()->sendtime_hour();
					communicationinfo.sendtime.minutes = bdmsg.mutable_commuincation()->sendtime_minute();
					communicationinfo.sendtime.seconds = bdmsg.mutable_commuincation()->sendtime_second();
					communicationinfo.encodingtype = bdmsg.mutable_commuincation()->messagecategory();
					communicationinfo.messagelength = bdmsg.mutable_commuincation()->messagelength();
					assert(communicationinfo.messagelength <= 210 && communicationinfo.messagelength > 0);
					memcpy(communicationinfo.message, bdmsg.mutable_commuincation()->mutable_messagebuffer(), communicationinfo.messagelength);
					socket_buf = encodeprotocol_communicationinfo(&communicationinfo, socket_len);
				}else if(bdmsg.messagetype() == 4){
					communicationreceipt.sendaddr = bdmsg.mutable_communicationreceipt()->sendaddr();
					communicationreceipt.recvaddr = bdmsg.mutable_communicationreceipt()->recvaddr();
					communicationreceipt.receipttime.hour = bdmsg.mutable_communicationreceipt()->receipttime_hour();
					communicationreceipt.receipttime.minutes = bdmsg.mutable_communicationreceipt()->receipttime_minute();
					communicationreceipt.receipttime.seconds = bdmsg.mutable_communicationreceipt()->receipttime_second();
					socket_buf = encodeprotocol_communicationreceipt(&communicationinfo, socket_len);
				}else{
					assert(0);
				}
				bdmsg.Clear();
				buffer->buf = socket_buf;
				buffer->buflen = socket_len;

			}
		}
		zmq_msg_close(&msg);
	}

	pthread_exit(0);
}

struct zmq_buffer* zmq_buffer_create(int capacity){
	struct zmq_buffer * zmq_buffer = (struct zmq_buffer*)malloc(sizeof(struct zmq_buffer));
	memset(zmq_buffer, 0, sizeof(struct zmq_buffer));
	assert(zmq_buffer);
	zmq_buffer->zmq_ctx = zmq_ctx_new();
	assert(zmq_buffer->zmp_ctx);
	zmq_buffer->recvsocket = zmq_socket(zmq_buffer->zmp_ctx, ZMQ_SUB);
	assert( zmq_buffer->recvsocket );
	int rc = zmq_setsockopt(zmq_buffer->recvsocket, ZMQ_SUBSCRIBE, "", 0);
	assert(rc == 0);
	rc = zmq_connect(zmq_buffer->recvsocket, "tcp://192.168.1.155:5555");
	assert(rc == 0);
	
	void * zmq_buffer->sendsocket = zmq_socket(ctx, ZMQ_PUSH); 
	assert(socket != NULL);
	int rc = zmq_bind(socket, "tcp://*:8888");
	assert(rc == 0);

	zmq_buffer->slotcount = capacity;
	zmq_buffer->slot = (struct zmq_buffer_authentication*)malloc(sizeof(struct zmq_buffer_authentication*)*capacity);
	memset(zmq_buffer->slot, 0, sizeof(struct zmq_buffer_authentication*)*capacity);

	if(zmq_buffer->slot == NULL){
		free(zmq_buffer);
		fprintf(stderr, "malloc %d bytes error.%s %s %d\n", sizeof(struct zmq_buffer_authentication)*capacity, __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	pthread_t tid;
	if( 0 != pthread_create(&tid, NULL, recv_downstream, (void*)zmq_buffer)){
		free(zmq_buffer->slot);
		zmq_buffer->slot = NULL;
		free(zmq_buffer);
		zmq_buffer = NULL; 
		fprintf(stderr, "create thread error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

		return NULL;
	}

	return zmq_buffer; 
}

int zmq_buffer_push(struct zmq_buffer * zb, unsigned char * buf, int len){ 
	zmq_msg_t msg;
	int rc = zmq_msg_init_size(&msg, len);
	assert(rc == 0);
	memset(zmq_msg_data(&msg), buf, len);
	rc = zmq_msg_send(&msg, zb->sendsocket, 0);
	assert(rc == len);
}

int zmq_buffer_add(struct zmq_buffer * zmq_buffer, unsigned char * buf, unsigned int len, int fd, unsigned int authentication){
	unsigned int index = authenticationid % 1024;
	struct zmq_buffer_authentication * tmp = zmq_buffer->slot[index]; 
	if(tmp == NULL){ 
		tmp = (struct zmq_buffer_authentication*)malloc(sizeof(struct zmq_buffer_authentication));
	}else{
		// 之前没有得到认证结果的消息被覆盖 应该给用户个说法吧
		free(tmp->buf);
	}

	tmp->buf = buf;
	tmp->len = len;
	tmp->fd = fd;
	tmp->authenticationid = authenticationid;

	return 0;
}

int zmq_buffer_destroy(struct zmq_buffer * zmq_buffer){ 
	int rc = zmq_close(zmq_buffer->recvsocket);
	assert(rc == 0);
	rc = zmq_ctx_destroy(zmq_buffer->zmq_ctx);
	assert(rc == 0);
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}

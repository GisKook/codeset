#include <zmq.h>
#include <stdio.h>
#include <pthread.h>
#include <string>
#include "list.h"
#include "beidoumessage.pb.h"
#include "encodeprotocol.c"

using namespace std;

void* recv_downstream(void* p){
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	struct zmq_buffer_head * zbh = (struct zmq_buffer_head*)p;
	string str;
	Beidoumessage bdmsg;
	struct positioninfo postioninfo;
	struct communicationinfo communicationinfo;
	struct communicationreceipt communicationreceipt;
	unsigned char * socket_buf;
	int socket_len;

	for(;;){
		zmq_msg_t msg;
		int rc = zmq_msg_init(&msg);
		assert(rc == 0); 
		rc = zmq_msg_recv(&msg, socket, 0);
		assert(rc != -1); 
		if(rc > 0){
			struct zmq_buffer * buffer = (struct zmq_buffer*)malloc(sizeof(struct zmq_buffer));
			if(unlikely(buffer == NULL)){
				fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				zmq_msg_close(&msg);
				return NULL;
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

				list_add_tail(&buffer->list, zbh->head);
			}
		}
		zmq_msg_close(&msg);
	}

	pthread_exit(0);
}

struct zmq_buffer_head* zmq_buffer_create(){
	struct zmq_buffer_head * zmq_buffer_head = (struct zmq_buffer_head*)malloc(sizeof(struct zmq_buffer_head));
	assert(zmq_buffer_head);
	zmq_buffer_head->zmq_ctx = zmq_ctx_new();
	assert(zmq_buffer_head->zmp_ctx);
	zmq_buffer_head->socket = zmq_socket(zmq_buffer_head->zmp_ctx, ZMQ_SUB);
	assert( zmq_buffer_head->socket );
	int rc = zmq_setsockopt(zmq_buffer_head->socket, ZMQ_SUBSCRIBE, "", 0);
	assert(rc == 0);
	rc = zmq_connect(zmq_buffer_head->socket, "tcp://192.168.1.155:5555");
	assert(rc == 0);

	INIT_LIST_HEAD(zmq_buffer_head->head);

	pthread_t tid;
	if( 0 != pthread_create(&tid, NULL, recv_downstream, (void*)zmq_buffer_head)){
		free(zmq_buffer_head);
		zmq_buffer_head = NULL; 
		fprintf(stderr, "create thread error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

		return NULL;
	}

	return zmq_buffer_head; 
}

int zmq_buffer_add(struct zmq_buffer_head * zmq_buffer_head, unsigned char * buf, int len, int fd){
	struct zmq_buffer * buffer = (struct zmq_buffer*)malloc(sizeof(struct zmq_buffer));
	if(unlikely(buffer == NULL)){ 
		fprintf(stderr, "malloc error. %s %s %d\n",__FILE__, __FUNCTION__, __LINE__);

		return -1;
	}
	memset(buffer, 0, sizeof(struct zmq_buffer));
	buffer->buf = buf;
	buffer->buflen = len;
	buffer->fd = fd;
	list_add_tail(&buffer->list, zmq_buffer_head->head);

	return 0;
}

int zmq_buffer_destroy(struct zmq_buffer_head * zmq_buffer_head){ 
	int rc = zmq_close(zmq_buffer_head->socket);
	assert(rc == 0);
	rc = zmq_ctx_destroy(zmq_buffer_head->zmq_ctx);
	assert(rc == 0);
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}

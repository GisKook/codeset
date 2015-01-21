#include <assert.h>
#include <zmq.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include "pb/bcTx.pb.h"
#include "pb/smsTx.pb.h"
#include "pb/beidoumessage.pb.h"

using std::string;

int main(){
	void* ctx = zmq_ctx_new();
	assert(ctx != NULL);
	void* socket = zmq_socket(ctx, ZMQ_PULL);
	assert(ctx != NULL);
	int rc = zmq_connect(socket, "tcp://192.168.1.155:18888");
	assert(rc == 0);
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	for(;;){
		zmq_msg_t msg;
		rc = zmq_msg_init(&msg);
		assert(rc == 0);
		
		rc = zmq_msg_recv(&msg, socket, 0);
		unsigned char * temp = (unsigned char *)zmq_msg_data(&msg);
		printf("%s\n", temp);
//		temp += 43;
//		fprintf(stdout, "message length %d\n", zmq_msg_size(&msg)-43);
//		assert(rc != -1);
//		BsTxMsg_FwjqMsg authmessage;
//		authmessage.ParseFromString((char *)temp);
//		fprintf(stdout, "authid %d, entid: %s category: %d\n", authmessage.nauthenticationid(), authmessage.sqtsentid().c_str(), authmessage.ncategory());

		zmq_msg_close(&msg);
	}

	return 0;

}

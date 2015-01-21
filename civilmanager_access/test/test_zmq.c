#include <assert.h>
#include <zmq.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "pb/bcTx.pb.h"
#include "pb/smsTx.pb.h"
#include "pb/beidoumessage.pb.h"

using std::string;

int main(){
	void* ctx = zmq_ctx_new();
	assert(ctx != NULL);
	void* socket = zmq_socket(ctx, ZMQ_PUSH);
	assert(ctx != NULL);
	int rc = zmq_bind(socket, "tcp://*:18888");
	assert(rc == 0);
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	for(;;){ 
		BsTxMsg_FwjqMsg authmessage;
		authmessage.set_nauthenticationid(123);
		authmessage.set_sqtsentid("cetcnav");
		authmessage.set_ncategory(1);
		string strauth;
		authmessage.SerializeToString(&strauth);
		authmessage.Clear();
		int nstrauthlen = strauth.length();

		zmq_msg_t msg;
		rc = zmq_msg_init_size(&msg, 43+nstrauthlen);
		
		fprintf(stdout ,"total len: %d\n", 43+nstrauthlen);
		assert(rc == 0);
		unsigned char * data = NULL;
		data = (unsigned char *)zmq_msg_data(&msg);
		memset(data, 0, 43+nstrauthlen);
		memcpy(data, "beidou", sizeof("beidou")-1);
		data += 20;
		*data++ = 1;
		memcpy(data, "charge", sizeof("charge")-1);
		data += 20; 
		*((unsigned short*)data) = nstrauthlen;
		data += 2;
		memcpy(data, strauth.c_str(), nstrauthlen);
		rc = zmq_msg_send(&msg, socket, 0);
		assert(rc != -1);
		zmq_msg_close(&msg);
		sleep(1);
	}

	return 0;
}

#include <zmq.h>
#include "list.h"

struct zmq_buffer{
	char sendaddr[20];
	unsigned short messagelen;
	unsigned char* message;
	struct list_head list; 
};

struct zmq_buffer_head{ 
	void* zmq_ctx;
	void* socket;
	struct list_head head;
}

struct zmq_buffer_head* zmq_buffer_create(){
	struct zmq_buffer_head * zmq_buffer_head = (struct zmq_buffer_head*)malloc(sizeof(struct zmq_buffer_head));
	assert(zmq_buffer_head);
	zmq_buffer_head->zmq_ctx = zmq_ctx_new();
	assert(zmq_buffer_head->zmp_ctx);
	zmq_buffer_head->socket = zmq_socket(zmq_buffer_head->zmp_ctx, ZMQ_SUB);
	assert( zmq_buffer_head->socket );
	INIT_LIST_HEAD(zmq_buffer_head->head );
	
	return zmq_buffer_head; 
}


int zmq_buffer_destroy(struct zmq_buffer_head * zmq_buffer_head){ 
	int rc = zmq_close(zmq_buffer_head->socket);
	assert(rc == 0);
	rc = zmq_ctx_destroy(zmq_buffer_head->zmq_ctx);
	assert(rc == 0);

	return 0;
}

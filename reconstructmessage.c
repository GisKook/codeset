#include "parseprotocol.h"
#include "kfifo.h"

// 可能出现的指令 $YHZC $YHZX $XTJC $BDFS
unsigned char* getmessagestart(unsigned char* buf, unsigned int len){
	unsigned char* p;
	for(p = buf; p!= buf + len; ++p){

	}
}

struct reconstructmessage{
	struct parseprotocol_request* message;
	struct reconstructmessage* next;
}

int reconstructmessage_add(struct reconstructmessage* message, struct kfifo* fifo){ 

	return 0;
}

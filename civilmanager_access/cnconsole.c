#include <stdio.h>
#include <string.h>
#include "cnconsole.h"
#include "sockets_buffer.h"

int console_print_cmds(){ 
	fprintf(stdout, "--------------------------------------------\n");
	fprintf(stdout, "recognized cmds:\n");
	fprintf(stdout, "    1.quit    \n");
	fprintf(stdout, "    2.prm      : print received message.\n");
	fprintf(stdout, "    3.unprm    : stop print received message.\n");
	fprintf(stdout, "    4.pcm      : print cached message. \n");
	fprintf(stdout, "    5.pcs      : print connection message. \n");
	fprintf(stdout, "--------------------------------------------\n");
	return 0;
}
int console_parsecmd(unsigned char* buf, struct sockets_buffer* socketbuf){ 
	if(memcmp(buf, "quit",4) == 0){
		return QUIT;
	}else if(memcmp(buf, "prm", 3) == 0){
		return PRM;
	}else if(memcmp(buf, "unprm", 5) == 0){
		return UNPRM;
	}else if(memcmp(buf, "pcm", 3) == 0){ 
		sockets_buffer_print(socketbuf);
		return PCM;
	}else if(memcmp(buf, "pcs", 3) == 0){ 
		return PCS;
	}else{
		console_print_cmds();
		return -1;
	}
}

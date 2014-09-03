#include <stdio.h>
#include <string.h>
#include "cnconsole.h"
#include "sockets_buffer.h"

int print_cmds(){ 
	fprintf(stdout, "--------------------------------------------\n");
	fprintf(stdout, "recognized cmds:\n");
	fprintf(stdout, "    1.quit    \n");
	fprintf(stdout, "    2.prm      : print received message.\n");
	fprintf(stdout, "    3.pcm      : print cached message. \n");
	fprintf(stdout, "--------------------------------------------\n");
	return 0;
}
int getcmd(unsigned char* buf){ 
	if(memcmp(buf, "quit",4) == 0){
		return QUIT;
	}else if(memcmp(buf, "prm", 3) == 0){
		return PRM;
	}else if(memcmp(buf, "pcm", 3) == 0){

		return PCM;
	}else{
		print_cmds();
		return -1;
	}
}

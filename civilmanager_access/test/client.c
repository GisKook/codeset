#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "endianness.h"

unsigned char calcchecksum(unsigned char * buf, unsigned int len){
	unsigned char result = buf[0];
	unsigned int i = 1;
	for(;i < len;++i){ 
		result ^= buf[i];
	}

	return result;
}

int main(){ 
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	memset((void*)&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("192.168.1.155");
	int port = 40000;
	addr.sin_port = htons(port);
	int rc = connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr));

	if(rc != 0){
		fprintf(stdout, "connect error. error message: %s\n", strerror(errno));
		return -1;
	}
	static unsigned int index = 0;
	unsigned char buffer[128] = {0};

	unsigned char * temp = buffer;
	unsigned short * ptotollen = NULL;
	char message[] = "$$$$$$$$$$$";
	unsigned int sendaddr = 0;
	unsigned int recvaddr = 0;
	unsigned char encodetype = 1;
	unsigned short messagelen = sizeof(message)-1;
	for(;;){ 
		index++; 
		memcpy(temp, "$BDFS", sizeof("$BDFS")-1); 
		temp += sizeof("$BDFS")-1;
		ptotollen = (unsigned short*)temp;
		temp += sizeof(unsigned short);
		*((unsigned int *)temp) = ISBIGENDIAN?index:swap32(index);
		temp += sizeof(unsigned int);
		*((unsigned int *)temp) = ISBIGENDIAN?sendaddr:swap32(sendaddr);
		temp += sizeof(unsigned int);
		*((unsigned int *)temp) = ISBIGENDIAN?recvaddr:swap32(recvaddr);
		temp += sizeof(unsigned int);
		*temp++ = encodetype;
		*((unsigned short *)temp) = ISBIGENDIAN?messagelen:swap16(messagelen);
		temp += sizeof(unsigned short); 
		memcpy(temp, message, messagelen);
		temp += messagelen;
		int len = temp-buffer+1;
		*ptotollen = ISBIGENDIAN?(len):swap16(len);
		*temp++ = calcchecksum(buffer, len-1); 

		unsigned int templen = len *2;
		unsigned char * sendbuffer = (unsigned char *)malloc(templen);
		memcpy(sendbuffer, buffer, len);
		memcpy(sendbuffer+len, buffer, len);
		rc = send(sock,sendbuffer, templen, 0);

		//rc = send(sock,buffer, len, 0);

		fprintf(stdout, "send %d \n", len);
		len = 0;
		memset(buffer, 0, len);
		temp = buffer;
		if(rc == -1){
			fprintf(stderr, "error message: %s\n", strerror(errno));
		}
		sleep(1);
	}
	return 0;
}

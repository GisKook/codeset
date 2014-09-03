#ifndef CODESET_CNCOSOLE_H_H
#define CODESET_CNCOSOLE_H_H


#define QUIT 1
#define PRM 2
#define PCM 3
struct sockets_buffer;
int getcmd(unsigned char* buf, struct sockets_buffer* socketbuf);

#endif

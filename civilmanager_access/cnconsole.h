#ifndef CODESET_CNCOSOLE_H_H
#define CODESET_CNCOSOLE_H_H


#define QUIT 1
#define PRM 2
#define UNPRM 3
#define PCM 4
#define PCS 5
struct sockets_buffer;
int console_parsecmd(unsigned char* buf, struct sockets_buffer* socketbuf);

#endif

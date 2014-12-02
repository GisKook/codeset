#ifndef CODESET_SOCKETS_BUFFER_H_H
#define CODESET_SOCKETS_BUFFER_H_H

struct sockets_buffer;
struct kfifo;
struct list_head;
struct sockets_buffer* sockets_buffer_create(unsigned int slotcount);
struct sockets_buffer* sockets_buffer_add(struct sockets_buffer* sockets_buf, int fd, char* ip, unsigned char* buf, int len);

struct kfifo* sockets_buffer_getrawdata(struct sockets_buffer * sbuf, int fd);
struct list_head* sockets_buffer_gethighlist(struct sockets_buffer * sbuf, int fd);
struct list_head* sockets_buffer_getnormallist(struct sockets_buffer * sbuf, int fd);

int sockets_buffer_del(struct sockets_buffer* buf, int fd);
int sockets_buffer_destroy(struct sockets_buffer* buf);

int sockets_buffer_print(struct sockets_buffer* buf);

void sockets_buffer_signal(struct sockets_buffer * sbuf, int fd);
struct int* sockets_buffer_getsignalfdfifo(struct sockets_buffer * sbuf);


int sockets_buffer_write(struct sockets_buffer * sbuf, int fd; char * buffer); 
char * sockets_buffer_getwritebuffer(struct sockets_buffer * sbuf, int fd);

#endif

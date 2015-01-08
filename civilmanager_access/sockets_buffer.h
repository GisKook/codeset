#ifndef CODESET_SOCKETS_BUFFER_H_H
#define CODESET_SOCKETS_BUFFER_H_H

struct sockets_buffer;
struct kfifo;
struct list_head;
struct mqueue;
struct encodeprotocol_respond;

struct sockets_buffer* sockets_buffer_create(unsigned int slotcount);
struct sockets_buffer* sockets_buffer_add(struct sockets_buffer* sockets_buf, int fd, char* ip, unsigned char* buf, int len);

struct kfifo* sockets_buffer_getrawdata(struct sockets_buffer * sbuf, int fd);
struct list_head* sockets_buffer_gethighlist(struct sockets_buffer * sbuf, int fd);
struct list_head* sockets_buffer_getnormallist(struct sockets_buffer * sbuf, int fd);

int sockets_buffer_del(struct sockets_buffer* buf, int fd);
int sockets_buffer_destroy(struct sockets_buffer* buf);

int sockets_buffer_print(struct sockets_buffer* buf);

void sockets_buffer_signal(struct sockets_buffer * sbuf, int fd);
int* sockets_buffer_getsignalfdfifo(struct sockets_buffer * sbuf);

void sockets_buffer_normaltasksignal(struct sockets_buffer * sbuf, int fd);
int * sockets_buffer_getnormaltasklist(struct sockets_buffer * sbuf);

int sockets_buffer_write(struct sockets_buffer * sbuf, int fd, struct encodeprotocol_respond * epr);

int * sockets_buffer_getdownstreamsignal(struct sockets_buffer * sbuf);

struct mqueue * sockets_buffer_getwritequeue(struct sockets_buffer * sbuf, int fd);
int sockets_buffer_clear(struct sockets_buffer * buf, int fd);

#endif

#ifndef CODE_CIVILMANAGER_ACCESS_ZMQ_BUFFER_H_H
#define CODE_CIVILMANAGER_ACCESS_ZMQ_BUFFER_H_H

struct zmq_buffer_authentication; 
struct zmq_buffer;

struct zmq_buffer* zmq_buffer_create(int capatiy);
int zmq_buffer_push(struct zmq_buffer *, unsigned char * buf, int len);
int zmq_buffer_add(struct zmq_buffer * zmq_buffer, unsigned char * buf, int len, int fd);
int zmq_buffer_destroy(struct zmq_buffer * zmq_buffer); 

#endif

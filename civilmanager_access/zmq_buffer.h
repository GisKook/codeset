#ifndef CODE_CIVILMANAGER_ACCESS_ZMQ_BUFFER_H_H
#define CODE_CIVILMANAGER_ACCESS_ZMQ_BUFFER_H_H

struct zmq_buffer_authentication; 
struct zmq_buffer;
struct fmtreportsockdata;

struct zmq_buffer * zmq_buffer_create(struct sockets_buffer * sockets_buffer, struct cardmanager * cardmanager,  struct loginenterprisemanager * loginenterprisemanager, int capacity);

int zmq_buffer_upstream_add(struct zmq_buffer * zmq_buffer, struct fmtreportsockdata * fmtreportsockdata,  unsigned int messagetype, char * enterpriseid, int fd, unsigned int usersendindex);

int zmq_buffer_destroy(struct zmq_buffer * zmq_buffer); 

#endif

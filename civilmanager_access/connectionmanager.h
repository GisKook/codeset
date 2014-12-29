#ifndef CIVILMANAGER_ACCESS_CONNECTIONMANAGER_H_H
#define CIVILMANAGER_ACCESS_CONNECTIONMANAGER_H_H

struct connection;
struct connectionmanager;

struct connection * connection_create(int fd, char * login, char * loginname, char * enterpriseid);
char * connection_getenterpriseid(struct connection * connection);
void connection_destroy(struct connection * connection);

struct connectionmanager * connectionmanager_create();
void connectionmanager_destroy(struct connectionmanager * manager);

struct connection * connectionmanager_search(struct connectionmanager * manager, int fd);
struct connection * connectionmanager_insert(struct connectionmanager * manager, struct connection * connection); 
struct connection * connectionmanager_delete(struct connectionmanager * manager, int fd);
void connectionmanager_print(struct connectionmanager * manager);

#endif

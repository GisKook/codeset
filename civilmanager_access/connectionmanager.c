// 根据fd得到其他属性 当前接入的链接
#define MAXLOGINNAMELEN 100
#define MAXLOGINLEN 32
#define MAXENTERPRISEIDLEN 32
#define MAXTIMEOUT 32

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "rbtree.h"
#include "cndef.h"

struct connection{
	int fd;
	time_t heartchecktime;
	struct rb_node node;
	char enterpriseid[MAXENTERPRISEIDLEN];
	char login[MAXLOGINLEN];
	char loginname[MAXLOGINNAMELEN];
};

struct connectionmanager{ 
	struct rb_root root;
	int connectioncount;
	int timeout[MAXTIMEOUT];
	int timeoutcount;
};

struct connection * connection_create(int fd, char * login, char * loginname, char * enterpriseid){
	struct connection * connection = (struct connection *)malloc(sizeof(struct connection));
	if(connection != NULL){
		memset(connection, 0, sizeof(struct connection));
		connection->fd = fd;
		memcpy(connection->login, login, MIN(MAXLOGINLEN, strlen(login)));
		memcpy(connection->loginname, loginname, MIN(MAXLOGINNAMELEN, strlen(loginname)));
		memcpy(connection->enterpriseid, enterpriseid, MIN(MAXENTERPRISEIDLEN, strlen(enterpriseid)));
	}

	return connection;
}

void connection_destroy(struct connection * connection){ 
	free(connection);
	connection = NULL;
}

char * connection_getenterpriseid(struct connection * connection){
	if(connection != NULL){
		return connection->enterpriseid;
	}

	return NULL;
}

struct connectionmanager * connectionmanager_create(){
	struct connectionmanager *manager = (struct connectionmanager *)malloc(sizeof(struct connectionmanager));
	memset(manager, 0, sizeof(struct connectionmanager));
	manager->root = RB_ROOT;

	return manager;
}

void connectionmanager_destroy(struct connectionmanager * manager){
	free(manager);
}

struct connection * connectionmanager_search(struct connectionmanager * manager, int fd){
	struct rb_root *root = &manager->root;
	struct rb_node *node = root->rb_node;

	struct connection * connection;
	while (node) {
		connection = rb_entry(node, struct connection, node);

		if (fd < connection->fd)
			node = node->rb_left;
		else if (fd > connection->fd)
			node = node->rb_right;
		else
			return connection;
	}

	return NULL;
}

struct connection * _conntionmanger_insert(struct connectionmanager * manager, struct connection * connection){
	struct rb_node **newnode = &(manager->root.rb_node), *parent = NULL;
	struct connection * conn;

	while (*newnode)
	{
		conn  = rb_entry(*newnode, struct connection, node);
		parent = *newnode;

		if (connection->fd < conn->fd)
			newnode = &((*newnode)->rb_left);
		else if (connection->fd > conn->fd)
			newnode = &((*newnode)->rb_right);
		else
			return conn;
	}

	++manager->connectioncount;
	rb_link_node(&connection->node, parent, newnode);

	return NULL;
}

struct connection * connectionmanager_insert(struct connectionmanager * manager, struct connection * connection){
	struct connection * conn;
	if((conn = _conntionmanger_insert(manager, connection)))
		goto out;
	rb_insert_color(&connection->node, &manager->root);
out:
	return conn; 
}

struct connection * connectionmanager_delete(struct connectionmanager * manager, int fd){
	struct connection * connection = connectionmanager_search(manager, fd);
	if( connection != NULL){
		struct rb_root * root = &manager->root;
		struct rb_node * node = &connection->node;
		rb_erase(node, root);
	}

	return connection;
}

void connectionmanager_print(struct connectionmanager * manager){
	struct rb_root root = manager->root;
	struct rb_node *node = rb_first(&root);
	struct connection *connection; 
	while(node != NULL){
		connection = rb_entry(node, struct connection, node);
		fprintf(stdout, "    fd: %d, enterpriseid: %s, login: %s, loginname: %s\n", connection->fd, connection->enterpriseid, connection->login, connection->loginname);
		node = rb_next(node);
	}
}

void connectionmanager_updateheartcheck(struct connectionmanager * connectionmanager, int fd){ 
	struct connection * connection = connectionmanager_search(connectionmanager, fd);
	if(connection != NULL){
		connection->heartchecktime = time(NULL);
	}
}

void connectionmanager_timeout(struct connectionmanager * connectionmanager, int timeout){
	time_t cur = time(NULL);
	struct rb_root root = connectionmanager->root;
	struct rb_node *node = rb_first(&root);
	struct connection * connection = NULL;
	while( node != NULL ){
		connection = rb_entry(node, struct connection, node); 
		if((long int)cur - (long int)(connection->heartchecktime) > timeout){ 
			connectionmanager->timeout[connectionmanager->timeoutcount] = connection->fd;
			connectionmanager->timeoutcount++;
		}

		node = rb_next(node);
	}
}

int * connectionmanager_gettimeout(struct connectionmanager * manager, int timeout, int *count){
	connectionmanager_timeout(manager, timeout);
	*count = manager->timeoutcount;
	return manager->timeout;
}

void connectionmanager_resettimeout(struct connectionmanager * manager){
	memset(manager->timeout, 0, MAXTIMEOUT);
	manager->timeoutcount = 0;
}

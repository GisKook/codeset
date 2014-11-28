#define MAXLOGINNAMELEN 100
#define MAXLOGINLEN 32
#define MAXENTERPRISEIDLEN 32

#include "rbtree.h"

struct connection{
	int fd;
	struct rb_node node;
	char *login;
	char *loginname;
	char *enterpriseid;
};

struct connectionmanager{ 
	struct rb_root root;
	int connectioncount;
};

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
	rb_link_node(&conn->node, parent, newnode);

	return NULL;
}

struct connection * connectionmanager_insert(struct connectionmanager * manager, struct connection * connection){ 
	struct connection * conn;
	if((conn = _conntionmanger_insert(manager, connection)))
		goto out;
	rb_insert_color(&conn->node, &manager->root);
out:
	return conn; 
}

struct connection * connectionmanager_delete(struct loginmanager * manager, int fd){
	struct connection * connection = connectionmanager_search(malloc, fd);
	if( connection != NULL){
		struct rb_root * root = &manager->root;
		struct rb_node * node = &connection->node;
		rb_erase(node, root);
	}

	return connection;
}

void connectionmanager_print(struct connectionmanager * manager){
	struct rb_root root = manager->root;
	struct rb_node *parent;
	struct rb_node *node = rb_first(&root);
	struct connection *connection; 
	while(node != NULL){
		connection = rb_entry(node, struct connection, node);
		node = rb_next(node);
	}
}


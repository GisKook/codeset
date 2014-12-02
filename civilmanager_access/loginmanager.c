#include <string.h>
#include <stdio.h>

#include "loginmanager.h"
#include "rbtree.h"
#include "toolkit.h"
#include "cndef.h"

struct loginmanager{
	struct rb_root root;
	unsigned int logincount;
};

struct loginmanager * loginmanager_create(){
	struct loginmanager * manager = (struct loginmanager *)malloc(sizeof(struct loginmanager));
	manager->root = RB_ROOT;
	manager->logincount = 0;

	return manager;
}

void loginmanager_destroy(struct loginmanager * lm){
	struct rb_root *root = &lm->root;
	struct login * login = NULL;  
	while( root->rb_node ){ 
		login = rb_entry(root->rb_node, struct login, node);
		free(login);
		login = NULL;

		rb_erase(root->rb_node, root);
	}

	free(lm);
}

struct login * loginmanager_search(struct loginmanager *manager, const char *login){
	struct rb_root *root = &manager->root;
	struct rb_node *node = root->rb_node;

	while (node) {
		struct login *data;
		data = rb_entry(node, struct login, node);
		int result;

		result = strcmp(login, data->login);

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return data;
	}

	return NULL;
}

struct login * _loginmanager_insert(struct loginmanager *manager, struct login *data){
	struct rb_node **newnode = &(manager->root.rb_node), *parent = NULL;
	struct login *ent;
	int result = 0;

	while (*newnode)
	{
		ent = rb_entry(*newnode, struct login, node);
		result = strcmp(data->login, ent->login);
		parent = *newnode;

		if (result < 0)
			newnode = &((*newnode)->rb_left);
		else if (result > 0)
			newnode = &((*newnode)->rb_right);
		else
			return ent;
	}

	++manager->logincount;
	rb_link_node(&data->node, parent, newnode);

	return NULL;
}

struct login * loginmanager_insert(struct loginmanager *manager, struct login *data){
	struct login *ret;
	if ((ret = _loginmanager_insert(manager, data)))
		goto out;
	rb_insert_color(&data->node, &manager->root);
 out:
	return ret;
}

struct login * loginmanager_delete(struct loginmanager *manager, const char *login){
	struct login *data= loginmanager_search(manager, login); 
	if(data != NULL){ 
		struct rb_root * root = &manager->root;
		struct rb_node * node = &data->node;
		rb_erase(node, root);
	}

	return data; 
}

void loginmanager_print(struct loginmanager *manager){ 
	struct rb_root root = manager->root;
	struct rb_node *parent;
	struct rb_node *node = rb_first(&root);
	struct login *login;
	while (node != NULL){
		login = rb_entry(node,struct login, node);
		fprintf(stdout, "    login : %s , password : %s , issuedfrequency : %d , loginname : %s \n", login->login, login->password, login->issuedfrequency, login->loginname);
		node = rb_next(node);
	}
}


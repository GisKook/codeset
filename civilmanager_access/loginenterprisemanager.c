#define MAXENTERPRISEIDLEN 32
#define MAXFDCOUNT 1024

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rbtree.h"

struct loginenterprisemanager{
	struct rb_root root;
	int loginenterprisefdcount;
};

struct loginenterprise{
	struct rb_node node;
	char enterpriseid[MAXENTERPRISEIDLEN];
	int fd[MAXFDCOUNT];
	int maxfdindex;
	int fdcount;
};

struct loginenterprisemanager * loginenterprisemanager_create(){
	struct loginenterprisemanager * manager = (struct loginenterprisemanager *)malloc(sizeof(struct loginenterprisemanager));
	manager->root = RB_ROOT;
	manager->loginenterprisefdcount = 0;

	return manager;
};

void loginenterprisemanager_destroy(struct loginenterprisemanager *manager){

	struct rb_root *root = &manager->root;
	struct loginenterprise *le;
	while( root->rb_node ){ 
		le = rb_entry(root->rb_node, struct loginenterprise, node);
		free(le);
		le = NULL;

		rb_erase(root->rb_node, root);
	}

	free(manager);
};

struct loginenterprise * _loginenterprisemanager_insert( struct loginenterprisemanager * manager, char * enterpriseid, int fd){ 
	struct rb_node **newnode = &(manager->root.rb_node), *parent = NULL; 
	struct loginenterprise * lp;
	int result = 0;

	while( *newnode ){ 
		lp = rb_entey( *newnode, struct loginenterprise, node); 
		result = strcmp(enterpriseid, lp->enterpriseid);
		if(result < 0){
			newnode = &((*newnode)->rb_left);
		}else if(result > 0){
			newnode = &((*newnode)->rb_right);
		}else{
			return lp;
		}
	}

	++manager->loginenterprisefdcount;
	struct loginenterprise * loginenterprise = (struct loginenterprise *)malloc(sizeof(struct loginenterprise));
	memset(loginenterprise, 0, sizeof(struct loginenterprise));
	memcpy(loginenterprise->enterpriseid, enterpriseid, strlen(enterpriseid));
	loginenterprise->fd[0] = fd;
	++loginenterprise->fdcount;
	++loginenterprise->maxfdindex;

	rb_link_node(&loginenterprise->node, parent, newnode);

	return NULL;
};

void loginenterprisemanager_insert( struct loginenterprisemanager * manager, char * enterpriseid, int fd){ 
	struct loginenterprise * le;
	if(le = _loginenterprisemanager_insert(manager, enterpriseid, fd)){
		int i;
		for( i = 0; i < le->maxfdindex; ++i){
			if(fd == le->fd[i]){
				goto out;
			}
		}

		if(le->fdcount < MAXFDCOUNT){
			if(le->maxfdindex < MAXFDCOUNT){
				le->fd[le->maxfdindex] = fd;
			}else{ 
				for( i = 0; i < le->maxfdindex; ++i){
					if(le->fd[i] == 0){
						le->fd[i] = fd;
					}
				}
			}
			++le->maxfdindex;
			++le->fdcount;
		}else{
			fprintf(stderr, " %s has %d connections , server refuse connect.\n", enterpriseid, MAXFDCOUNT);

			return;
		}

		goto out;
	}
	rb_insert_color(&le->node, &manager->root);

out:
	return;
}

struct loginenterprise * loginenterprisemanager_search(struct loginenterprisemanager * manager, char * enterpriseid){ 
	struct rb_root *root = &manager->root;
	struct rb_node *node = root->rb_node;

	int result;
	while(node){
		struct loginenterprise * le;
		le - rb_entry(node, struct loginenterprise, node); 

		result = strcmp(enterpriseid, le->enterpriseid);
		
		if(result < 0){
			node = node->rb_left;
		}else if(result < 0){
			node = node->rb_right;
		}else{
			return le;
		}
	}

	return NULL;
}

int * loginenterprisemanager_getfds(struct loginenterprisemanager *manager, char * enterpriseid){
	struct loginenterprise * le = loginenterprisemanager_search(manager, enterpriseid); 
	int fdcount = le->fdcount;
	int * result = (int *)malloc(sizeof(int)*(fdcount+1));
	int i,j;
	for(i = 0, j= 0; i < le->maxfdindex; ++i){ 
		if(0 != le->fd[i]){
			result[j++] = le->fd[i];
		}
	}

	return result;
}

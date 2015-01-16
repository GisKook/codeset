//管理一个公司下的所有连接
#define MAXENTERPRISEIDLEN 32
#define MAXFDCOUNT 1024
#define MAXLOGINLEN 32

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "rbtree.h"
#include "cndef.h"
#include "toolkit.h"

struct loginenterprisemanager{
	struct rb_root root;
	int loginenterprisefdcount;
	pthread_mutex_t mutex;
};

struct loginfd{
	char login[MAXLOGINLEN];
	int fd;
};

struct loginenterprise{
	struct rb_node node;
	char enterpriseid[MAXENTERPRISEIDLEN];
	struct loginfd * loginfd;
	int loginfdcount;
};

struct loginenterprisemanager * loginenterprisemanager_create(){
	struct loginenterprisemanager * manager = (struct loginenterprisemanager *)malloc(sizeof(struct loginenterprisemanager));
	manager->root = RB_ROOT;
	manager->loginenterprisefdcount = 0;
	pthread_mutex_init(&manager->mutex, NULL);

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

struct loginenterprise * _loginenterprisemanager_insert( struct loginenterprisemanager * manager, char * enterpriseid, char * login, int fd, struct loginenterprise * newloginenterprise){ 
	struct rb_node **newnode = &(manager->root.rb_node), *parent = NULL; 
	struct loginenterprise * lp;
	int result = 0;

	while( *newnode ){ 
		lp = rb_entry( *newnode, struct loginenterprise, node); 
		result = strcmp(enterpriseid, lp->enterpriseid);
		if(result < 0){
			newnode = &((*newnode)->rb_left);
		}else if(result > 0){
			newnode = &((*newnode)->rb_right);
		}else{ 
			++manager->loginenterprisefdcount; 
			lp->loginfd = (struct loginfd*)realloc(lp->loginfd,sizeof(struct loginfd)*(lp->loginfdcount+1)); 
			memcpy(lp->loginfd[lp->loginfdcount].login, login, MIN(MAXLOGINLEN, strlen(login)));
			lp->loginfd[lp->loginfdcount].fd = fd;
			++lp->loginfdcount;
			return lp;
		}
	}

	++manager->loginenterprisefdcount;
	struct loginenterprise * loginenterprise = (struct loginenterprise *)malloc(sizeof(struct loginenterprise));
	memset(loginenterprise, 0, sizeof(struct loginenterprise));
	memcpy(loginenterprise->enterpriseid, enterpriseid, MIN(MAXENTERPRISEIDLEN, strlen(enterpriseid)));
	loginenterprise->loginfd = (struct loginfd *)malloc(sizeof(struct loginfd));
	memcpy(loginenterprise->loginfd->login, login, MIN(MAXLOGINLEN, strlen(login)));
	loginenterprise->loginfd->fd = fd;
	++loginenterprise->loginfdcount;

	rb_link_node(&loginenterprise->node, parent, newnode);
	newloginenterprise = loginenterprise;

	return NULL;
};

void loginenterprisemanager_insert( struct loginenterprisemanager * manager, char * enterpriseid, char * login, int fd){ 
	pthread_mutex_lock(&manager->mutex);
	struct loginenterprise * le;
	struct loginenterprise * newloginenterprise = NULL;
	if((le = _loginenterprisemanager_insert(manager, enterpriseid, login, fd, newloginenterprise))){
		goto out;
	}
	if(le == NULL && newloginenterprise != NULL){
		rb_insert_color(&newloginenterprise->node, &manager->root);
	}

out:
	pthread_mutex_unlock(&manager->mutex);
	return;
}

int loginenterprisemanager_search(struct loginenterprisemanager * manager, char * enterpriseid, char * login){ 
	struct rb_root *root = &manager->root;
	struct rb_node *node = root->rb_node;

	int result;
	int i;

	while(node){
		struct loginenterprise * le;
		le = rb_entry(node, struct loginenterprise, node); 

		result = strcmp(enterpriseid, le->enterpriseid);

		if(result < 0){
			node = node->rb_left;
		}else if(result < 0){
			node = node->rb_right;
		}else{ 
			for( i = 0; i < le->loginfdcount; ++i){
				if((strlen(le->loginfd[i].login) == strlen(login)) && (0 == strcmp(le->loginfd[i].login, login))){
					return 1;
				}
			}
			return 0;
		}
	}

	return 0;
}

struct loginenterprise * _loginenterprisemanager_search(struct loginenterprisemanager * manager, char * enterpriseid){ 
	struct rb_root *root = &manager->root;
	struct rb_node *node = root->rb_node;

	int result;
	while(node){
		struct loginenterprise * le;
		le = rb_entry(node, struct loginenterprise, node); 

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

int * loginenterprisemanager_getfds(struct loginenterprisemanager *manager, char * enterpriseid, int * loginfdcount){
	struct loginenterprise * le = _loginenterprisemanager_search(manager, enterpriseid); 
	if(le != NULL){
		int fdcount = le->loginfdcount;
		*loginfdcount = fdcount;
		int * result = (int *)malloc(sizeof(int)*fdcount);
		if(result == NULL){
			fprintf(stderr, "malloc %d bytes error.%s %s %d\n", fdcount, __FILE__, __FUNCTION__, __LINE__);
			*loginfdcount = 0;

			return NULL;
		}
		int i;
		for(i = 0; i < le->loginfdcount; ++i){ 
			result[i] = le->loginfd[i].fd;
		}

		return result;
	}

	return NULL;
}

void loginenterprisemanager_delete(struct loginenterprisemanager *manager, char * enterpriseid, int fd){ 
	pthread_mutex_lock(&manager->mutex); 
	struct loginenterprise *le = _loginenterprisemanager_search(manager, enterpriseid);
	if(le == NULL){
		pthread_mutex_unlock(&manager->mutex);
		return;
	}
	int i;
	for(i = 0; i < le->loginfdcount; ++i){ 
		if(le->loginfd[i].fd == fd){ 
			le->loginfd[i].fd = 0;
			break;
		}
	}
	--le->loginfdcount;
	if(le->loginfdcount <= 0){ 
		rb_erase(&le->node,&manager->root); 
		free(le);
	}else{
		memmove((le->loginfd+i-1), le->loginfd+i, sizeof(struct loginfd)*(le->loginfdcount-i));
		le->loginfd = (struct loginfd*)realloc(le->loginfd, sizeof(struct loginfd)*le->loginfdcount);
	}
	pthread_mutex_unlock(&manager->mutex);
}

int loginenterprisemanager_check(struct loginenterprisemanager * loginenterprisemanager, char * enterpriseid){
	return _loginenterprisemanager_search(loginenterprisemanager, enterpriseid) != NULL;
}

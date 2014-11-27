#define MAXLOGINLEN 32
#define MAXPASSWORDLEN 32

#include <string.h>
#include <stdio.h>

#include "rbtree.h"
#include "toolkit.h"
#include "cndef.h"
#include "enterprisemanager.h"

struct enterpriseaccount{
	char login[MAXLOGINLEN];
	char password[MAXPASSWORDLEN];
	int issuedfrequency;
	int fd;
};

struct enterprise{
	struct rb_node node;
	char enterpriseid[MAXENTERPRISEIDLEN];
	struct enterpriseaccount * enterpriseaccount;
	int accountcount;
	int accountcapacity;
};

struct enterprisemanager{
	struct rb_root root;
	int enterprisecount;
};

struct enterprisemanager * enterprisemanager_create(){
	struct enterprisemanager * manager = (struct enterprisemanager *)malloc(sizeof(struct enterprisemanager));
	manager->root = RB_ROOT;
	manager->enterprisecount = 0;

	return manager;
}

void enterprisemanager_destroy(struct enterprisemanager * em){
	struct rb_root *root = &em->root;
	struct enterprise * enterprise = NULL;  
	struct enterpriseaccount * ea = NULL;
	while( root->rb_node ){ 
		enterprise = rb_entry(root->rb_node, struct enterprise, node);
		ea = enterprise->enterpriseaccount;
		free(ea);
		ea = NULL;
		free(enterprise);
		enterprise = NULL;

		rb_erase(root->rb_node, root);
	}
}

struct enterprise * enterprisemanager_search(struct enterprisemanager *manager, const char *enterpriseid){
	struct rb_root *root = &manager->root;
	struct rb_node *node = root->rb_node;

	while (node) {
		struct enterprise *data;
		data = rb_entry(node, struct enterprise, node);
		int result;

		result = strcmp(enterpriseid, data->enterpriseid);

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return data;
	}

	return NULL;
}

struct enterprise * _enterprisemanager_insert(struct enterprisemanager *manager, struct enterprise *data){
	struct rb_node **newnode = &(manager->root.rb_node), *parent = NULL;
	struct enterprise *ent;
	int result = 0;

	while (*newnode)
	{
		ent = rb_entry(*newnode, struct enterprise, node);
		int result = strcmp(data->enterpriseid, ent->enterpriseid);
		parent = *newnode;

		if (result > 0)
			newnode = &((*newnode)->rb_left);
		else if (result < 0)
			newnode = &((*newnode)->rb_right);
		else
			return ent;
	}

	++manager->enterprisecount;
	rb_link_node(&data->node, parent, newnode);

	return NULL;
}

struct enterprise * enterprisemanager_insert(struct enterprisemanager *manager, struct enterprise *data){
	struct enterprise *ret;
	if ((ret = _enterprisemanager_insert(manager, data)))
		goto out;
	rb_insert_color(&data->node, &manager->root);
 out:
	return ret;
}

int enterprisemanager_delete(struct enterprisemanager *manager, const char *enterpriseid){
	struct enterprise *enterprise = enterprisemanager_search(manager, enterpriseid); 
	if(enterprise != NULL){ 
		struct rb_root * root = &manager->root;
		struct rb_node * node = &enterprise->node;
		rb_erase(node, root);

		return 1;
	}

	return 0; 
}

struct enterprise * enterprise_create(const char * enterpriseid, int capacity){
	struct enterprise * enterprise = (struct enterprise *)malloc(sizeof(struct enterprise));
	if(enterprise == NULL){
		fprintf(stderr, "malloc enterprise error. %s %s %d\n.", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	memset(enterprise, 0, sizeof(struct enterprise));
	memcpy(enterprise->enterpriseid, enterpriseid, MIN(MAXENTERPRISEIDLEN-1, strlen(enterpriseid))); 
	int chuncksize = capacity * sizeof(struct enterpriseaccount);
	enterprise->enterpriseaccount = (struct enterpriseaccount *)malloc(chuncksize);
	if(enterprise->enterpriseaccount == NULL){
		fprintf(stderr, "enterprise account malloc error. %s %s %d\n.", __FILE__, __FUNCTION__, __LINE__);
		free(enterprise);
		return NULL;
	}
	memset(enterprise->enterpriseaccount, 0, chuncksize);
	enterprise->accountcapacity = capacity;

	return enterprise;
}

void enterprise_destroy(struct enterprise *enterprise){ 
	free(enterprise->enterpriseaccount);
	enterprise->enterpriseaccount = NULL;
	free(enterprise);
	enterprise = NULL;
}

int enterprise_addaccount(struct enterprise *enterprise, const char *login, const char *password, int issuedfrequency){ 
	int i = 0;
	struct enterpriseaccount * ea;
	for(; i < enterprise->accountcount; ++i){
		ea = &enterprise->enterpriseaccount[i];
		if((strlen(ea[i].login) == strlen(login)) && 0 == strcmp(ea[i].login, login)){ 
			ea[i].issuedfrequency = issuedfrequency;
			memcpy(ea[i].password, password, MIN(strlen(password), MAXPASSWORDLEN));
			
			return 1; // 更新信息
		}
	}

	if(unlikely(enterprise->accountcount >= enterprise->accountcapacity && enterprise->accountcount != 0)){ 
		int newcapacity = enterprise->accountcapacity * 2;
		ea = (struct enterpriseaccount*)realloc(enterprise->enterpriseaccount,sizeof(struct enterpriseaccount)*newcapacity);
		if(ea == NULL){
			fprintf(stderr, "enterprise account malloc %d bytes error. %s %s %d.\n", sizeof(struct enterpriseaccount)*newcapacity,__FILE__, __FUNCTION__, __LINE__);
			newcapacity = enterprise->accountcount + 1;
			ea = (struct enterpriseaccount*)malloc(sizeof(struct enterpriseaccount)*newcapacity);
			if(ea == NULL){
				fprintf(stderr, "enterprise account malloc error. %s %s %d.\n", __FILE__, __FUNCTION__, __LINE__);
				return 2; // 失败，没有添加
			}
		}
		memset(&ea[enterprise->accountcount], 0, (newcapacity-enterprise->accountcount)*sizeof(struct enterpriseaccount));
		memcpy(ea[enterprise->accountcount].login, login, MIN(strlen(login), MAXLOGINLEN));
		memcpy(ea[enterprise->accountcount].password, password, MIN(strlen(password), MAXLOGINLEN));
		ea[enterprise->accountcount].issuedfrequency = issuedfrequency;
		++enterprise->accountcount;
		enterprise->accountcapacity = newcapacity;
		enterprise->enterpriseaccount = ea;
		ea = NULL;
		
		return 3;
	}else{ 
		ea = &enterprise->enterpriseaccount[enterprise->accountcount++];
		memcpy(ea->login, login, MIN(strlen(login), MAXLOGINLEN));
		memcpy(ea->password, password, MIN(strlen(password), MAXPASSWORDLEN));
		ea->issuedfrequency = issuedfrequency;

		return 4;
	}

	return 0;
}

int enterprise_delaccount(struct enterprise *enterprise, const char *login){ 
	int i = 0;
	struct enterpriseaccount * ea;
	for(; i < enterprise->accountcount; ++i){
		ea = &enterprise->enterpriseaccount[i];
		if((strlen(ea[i].login) == strlen(login)) && (0 == strcmp(ea[i].login, login))){ 
			memmove(&ea[i], &ea[i+1], (enterprise->accountcount - i - 1)*sizeof(struct enterpriseaccount));
			--enterprise->accountcount;
			memset(&ea[enterprise->accountcount], 0, sizeof(struct enterpriseaccount));

			return 1; // 更新信息
		}
	}

	return 2;
}



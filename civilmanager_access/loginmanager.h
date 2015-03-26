#ifndef CIVILMANAGER_ACCESS_H_H
#define CIVILMANAGER_ACCESS_H_H


#include "rbtree.h"
#include "cndef.h"

struct login{
	struct rb_node node;
	char login[MAXLOGINLEN];
	char enterpriseid[MAXENTERPRISEIDLEN];
	char loginname[MAXLOGINNAMELEN];
	char password[MAXPASSWORDLEN];
	int issuedfrequency;
};

struct loginmanager;

struct loginmanager * loginmanager_create();
void loginmanager_destroy(struct loginmanager * em);

struct login * loginmanager_insert(struct loginmanager *manager, struct login *data);
struct login * loginmanager_search(struct loginmanager *manager, const char *login);
struct login * loginmanager_delete(struct loginmanager *manager, const char *login);

void loginmanager_print(struct loginmanager *manager); 

#endif

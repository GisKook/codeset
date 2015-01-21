#ifndef CIVILMANAGER_ACCESS_LOGINENTERPRISEMANAGER_H_H
#define CIVILMANAGER_ACCESS_LOGINENTERPRISEMANAGER_H_H

struct loginenterprisemanager;

struct loginenterprisemanager * loginenterprisemanager_create();

void loginenterprisemanager_destroy(struct loginenterprisemanager *manager);

void loginenterprisemanager_insert( struct loginenterprisemanager * manager, char * enterpriseid, char * login, int fd); 

int loginenterprisemanager_search(struct loginenterprisemanager * manager, char * enterpriseid, char * login); 

int * loginenterprisemanager_getfds(struct loginenterprisemanager *manager, char * enterpriseid, int * loginfdcount);

void loginenterprisemanager_delete(struct loginenterprisemanager *manager, char * enterpriseid, int fd); 

int loginenterprisemanager_check(struct loginenterprisemanager * loginenterprisemanager, char * enterpriseid);

void loginenterprisemanager_updateheartcheck(struct loginenterprisemanager * loginenterprisemanager, char * enterpriseid, int fd);

#endif

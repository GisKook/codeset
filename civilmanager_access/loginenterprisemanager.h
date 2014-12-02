#ifndef CIVILMANAGER_ACCESS_LOGINENTERPRISEMANAGER_H_H
#define CIVILMANAGER_ACCESS_LOGINENTERPRISEMANAGER_H_H

struct loginenterprisemanager;

struct loginenterprisemanager * loginenterprisemanager_create();

void loginenterprisemanager_destroy(struct loginenterprisemanager *manager);

void loginenterprisemanager_insert( struct loginenterprisemanager * manager, char * enterpriseid, int fd); 

int * loginenterprisemanager_getfds(struct loginenterprisemanager *manager, char * enterpriseid);

#endif


#ifndef CIVILMANAGER_ACCESS_H_H
#define CIVILMANAGER_ACCESS_H_H

struct enterprise;
struct enterprisemanager;

struct enterprisemanager * enterprisemanager_create();
void enterprisemanager_destroy(struct enterprisemanager * em);
struct enterprise * enterprisemanager_search(struct enterprisemanager *manager, const char *enterpriseid);
struct enterprise * enterprisemanager_insert(struct enterprisemanager *manager, struct enterprise *data);
int enterprisemanager_delete(struct enterprisemanager *manager, const char *enterpriseid); 


struct enterprise * enterprise_create(const char * enterpriseid, int capacity);
void enterprise_destroy(struct enterprise *enterprise); 
int enterprise_addaccount(struct enterprise *enterprise, const char *login, const char *password, int issuedfrequency); 
int enterprise_delaccount(struct enterprise *enterprise, const char *login); 

#endif

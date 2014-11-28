#ifndef CIVILMANAGER_ACCESS_DBLOGIN_H_H
#define CIVILMANAGER_ACCESS_DBLOGIN_H_H 

#include"loginmanager.h"

struct dblogin;
struct dblogin * dblogin_start(struct loginmanager * manager); 
void dblogin_end(struct dblogin* dbe); 

#endif

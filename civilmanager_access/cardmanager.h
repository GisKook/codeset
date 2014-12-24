#ifndef CIVIMANAGER_ACCESS_CARDMANAGER_H_H
#define CIVIMANAGER_ACCESS_CARDMANAGER_H_H

struct cardmanager;
struct card;

void card_setenterpriseid(struct card * card, char * enterpriseid);

struct cardmanager * cardmanager_create();
void cardmanager_destroy(struct cardmanager *cardmanager);
struct card * cardmanager_search(struct cardmanager * manager, unsigned int cardid);
struct card * cardmanager_insert(struct cardmanager * manager, unsigned int cardid, char * enterpriseid);
struct card * cardmanager_delete(struct cardmanager * manager, unsigned int cardid);
void cardmanager_print(struct cardmanager *manager); 

#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rbtree.h"
#include "cndef.h"

#define MAXENTERPRISEID 32

struct cardmanager{ 
	struct rb_root root;
	unsigned int cardcount;
};

struct card{
	struct rb_node node;
	unsigned int cardid;
	char enterpriseid[MAXENTERPRISEID];
};


void card_setenterpriseid(struct card * card, char * enterpriseid){
	memset(card->enterpriseid, 0, MAXENTERPRISEID);
	memcpy(card->enterpriseid, enterpriseid, MIN(MAXENTERPRISEID, strlen(enterpriseid)));
}

struct cardmanager * cardmanager_create(){
	struct cardmanager * cardmanager = (struct cardmanager *)malloc(sizeof(struct cardmanager));
	memset(cardmanager, 0, sizeof(struct cardmanager));
	cardmanager->root = RB_ROOT;

	return cardmanager;
}

void cardmanager_destroy(struct cardmanager *cardmanager){
	struct rb_root *root = &cardmanager->root;
	struct card *card = NULL;
	while(root->rb_node){
		card = rb_entry(root->rb_node, struct card, node);
		rb_erase(root->rb_node, root);
		free(card);
		card = NULL; 
	}

	free(cardmanager);
}

struct card * cardmanager_search(struct cardmanager * manager, unsigned int cardid){
	struct rb_root *root = &manager->root;
	struct rb_node *node = root->rb_node;

	struct card * card;
	while(node){ 
		card = rb_entry(node, struct card, node);
		if(cardid > card->cardid){
			node = node->rb_right;
		}else if(cardid < card->cardid){
			node = node->rb_left;
		}else{
			return card;
		}
	}

	return NULL;
}

struct card * _cardmanager_insert(struct cardmanager *manager,struct card *entry){
	unsigned int cardid = entry->cardid;
	struct rb_node **newnode = &(manager->root.rb_node), *parent = NULL;
	struct card * card;
	
	while(*newnode){
		card = rb_entry(*newnode, struct card, node);

		parent = *newnode;

		if( cardid > card->cardid ){
			newnode = &((*newnode)->rb_right);
		}else if( cardid < card->cardid){
			newnode = &((*newnode)->rb_left);
		}else{
			return card;
		}
	}

	++manager->cardcount;

	rb_link_node(&entry->node, parent, newnode);

	return NULL; 
}

struct card * cardmanager_insert(struct cardmanager * manager, unsigned int cardid, char * enterpriseid){
	struct card * card = (struct card *)malloc(sizeof(struct card));
	memset(card, 0, sizeof(struct card));
	card->cardid = cardid;
	memcpy(card->enterpriseid, enterpriseid, MIN(MAXENTERPRISEID - 1, strlen(enterpriseid)));

	struct card * ret;
	if((ret = _cardmanager_insert(manager, card)))
		goto out;
	rb_insert_color(&card->node, &manager->root);
out:
	return ret;
}

struct card * cardmanager_delete(struct cardmanager * manager, unsigned int cardid){
	struct card * card = cardmanager_search(manager, cardid);
	if(card != NULL){
		struct rb_root * root = &manager->root;
		struct rb_node * node = &card->node;
		rb_erase(node, root);
	}

	return card;
}

void cardmanager_print(struct cardmanager *manager){ 
	struct rb_root root = manager->root;
	struct rb_node *node = rb_first(&root);
	struct card * card;
	while (node != NULL){
		card = rb_entry(node,struct card, node);
		fprintf(stdout, "    cardid: %d , enterpriseid: %s  \n", card->cardid, card->enterpriseid);
		node = rb_next(node);
	}
}

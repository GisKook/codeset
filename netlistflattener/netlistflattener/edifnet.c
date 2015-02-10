#include <stdlib.h>
#include <string.h>
#include "edif.h"
#include "ediflibrary.h"

global char * glibrary;
int edifnet_getcount(struct edifnet * edifnet){
	int count = 0;
	struct edifnet * net = NULL;
	if(edifnet != NULL){
		for(net = edifnet; net != NULL; net = edifnet->next, ++count);
	}
	return count;
}

struct edifnet * edifnet_copy(struct edifnet * edifnet){
	struct edifnet * net = NULL;
	struct edifnetportref * portref = NULL, *port = NULL, *newref = NULL;
	if (edifnet != NULL){
		net = (struct edifnet *)malloc(sizeof(struct edifnet));
		memset(net, 0, sizeof(struct edifnet));
		net->net = strdup(edifnet->net);
		portref = edifnet->edifnetportref;
		for(port = portref; port != NULL; port = portref->next){ 
			newref = (struct edifnetportref *)malloc(sizeof(struct edifnetportref));
			memset(newref, 0, sizeof(struct edifnetportref));
			newref->portref = strdup(port->portref);
			newref->instanceref = strdup(port->instanceref);
			newref->next = net->edifnetportref;
			net->edifnetportref = newref;
		}
	}

	return net;
}

char * edifnet_getcellname(struct edifinstance * edifinstance, char * instanceref){
	struct edifinstance * iptrinstance = NULL;
	for(iptrinstance = edifinstance; iptrinstance != NULL; iptrinstance = iptrinstance->next){
		if(strlen(instanceref) == strlen(iptrinstance->instance) && strcmp(instanceref, iptrinstance->instance) == 0 && iptrinstance->libraryref == NULL){
			return iptrinstance->cellref;
		}
	}

	return NULL;
}

struct edifnetportref * edifnet_getnetports(struct edifnet * edifnet, char * szportref){
	struct edifnetportref *tmpportref = NULL, *portref = NULL, *iptrportref = NULL;
	int flag = 0;
	for (tmpportref = edifnet->edifnetportref; tmpportref != NULL; tmpportref = tmpportref->next) {
		if(strlen(tmpportref->portref) == strlen(szportref) && strcmp(tmpportref->portref, szportref) == 0){ 
			flag = 1;
			break;
		}
	}
	if(flag){
		for (tmpportref = edifnet->edifnetportref; tmpportref != NULL; tmpportref = tmpportref->next) { 
			if (tmpportref->instanceref != NULL) {
				iptrportref = (struct edifnetportref *)malloc(sizeof(struct edifnetportref));
				memset(iptrportref, 0, sizeof(struct edifnetportref));
				iptrportref->instanceref = strdup(tmpportref->instanceref);
				iptrportref->portref = strdup(tmpportref->portref);
				iptrportref->next = portref;
				portref = iptrportref;
			}
		}
	}

	return portref;
}

struct edifnetportref * edifnet_addtail(struct edifnetportref * edifnetportref, struct edifnetportref * tail){
	struct edifnetportref * tmpportref = NULL, *preportref = NULL;
	for (tmpportref = edifnetportref; tmpportref != NULL; preportref = tmpportref, tmpportref = tmpportref->next);
	if (preportref == NULL) {
		return tail;
	}else{
		preportref->next = tail;
	}

	return edifnetportref;
}

struct edifnet * edifnet_flatten(struct edifcontents * edifcontents, struct ediflibrary * library, struct edifsubcircuit * eidfsubcircuit){
	struct edifnet * net= NULL, *iptrnet = NULL, *tmpnet = NULL;
	struct edifinstance * instance = NULL;
	struct edifnetportref * edifnetportref = NULL, *portref = NULL, *iptrportref = NULL;
	char * cellname = NULL;
	if (edifcontents == NULL || library == NULL) {
		return NULL;
	}
	for(tmpnet = edifcontents->edifnet; tmpnet != NULL; tmpnet = tmpnet->next){
		iptrnet = (struct edifnet *)malloc(sizeof(struct edifnet));
		memset(iptrnet, 0, sizeof(struct edifnet)); 
		iptrnet->net = strdup(tmpnet->net);
		for(edifnetportref = tmpnet->edifnetportref; edifnetportref != NULL; edifnetportref = edifnetportref->next){ 
			iptrportref = (struct edifnetportref *)malloc(sizeof(struct edifnetportref));
			memset(iptrportref, 0, sizeof(struct edifnetportref)); 
			cellname = edifnet_getcellname(edifcontents->edifinstance, edifnetportref->instanceref);
			if(cellname == NULL){
				iptrportref->instanceref = strdup(edifnetportref->instanceref);
				iptrportref->portref = strdup(edifnetportref->portref);
				iptrportref->next = portref;
				portref = iptrportref;
			}else{
				iptrportref = ediflibrary_getnetportref(library, glibrary, cellname, edifnetportref->portref);
				edifnet_addtail(iptrportref, portref);
				portref = iptrportref;
			}
		}
		iptrnet->next = net;
		net = iptrnet;
	}

	return net;
}
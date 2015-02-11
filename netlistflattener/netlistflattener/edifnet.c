#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "edifwriter.h"
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
		for(port = portref; port != NULL; port = port->next){ 
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

struct edifnet * edifnet_copynets(struct edifnet * edifnet){
	struct edifnet * net = NULL, *iptrnet = NULL, * tmpnet = NULL;
	for(tmpnet = edifnet; tmpnet != NULL; tmpnet = tmpnet->next){
		iptrnet = edifnet_copy(tmpnet);
		iptrnet->next = net;
		net = iptrnet;
	}

	return net;
}

void * edifnet_destroy(struct edifnet * edifnet){
	struct edifnet * iptrnet = NULL, *iptrnextnet = NULL;
	struct edifnetportref * portref = NULL, *nextportref = NULL;
	for(iptrnet = edifnet; iptrnet != NULL; ){
		iptrnextnet = iptrnet->next;
		free(iptrnet->net);
		for(portref = iptrnet->edifnetportref; portref != NULL; ){ 
			nextportref = portref->next; 
			free(portref->instanceref);
			free(portref->portref);
			free(portref);
			portref = nextportref;
		}
		free(iptrnet);
		iptrnet = iptrnextnet;
	}
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
	struct edifnet * net= NULL, *iptrnet = NULL, *tmpnet = NULL, *internet = NULL;
	struct edifinstance * instance = NULL;
	struct edifnetportref * edifnetportref = NULL, *portref = NULL, *iptrportref = NULL;
	char * cellname = NULL;
	char * cellnames[] = {NULL};
	int cellcount = 0, i = 0;
	int alreadhave = 0;
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
				for(i = 0; i < cellcount; ++i){
					if (strlen(cellname) == strlen(cellnames[i]) && strcmp(cellname, cellnames[i]) == 0) {
						alreadhave = 1;
					}
				}
				if(!alreadhave){
					cellnames[cellcount++] = strdup(cellname);
					alreadhave = 0;
				}
				
				edifnet_addtail(iptrportref, portref);
				portref = iptrportref;
			}
		}
		iptrnet->edifnetportref = portref;
		portref = NULL;
		iptrnet->next = net;
		net = iptrnet;
	}
	
	iptrnet = NULL;

	for (i = 0; i < cellcount; ++i) { 
		iptrnet = ediflibrary_getnet(library, glibrary, cellnames[i]);
		iptrnet->next = net;
		net = iptrnet;
	}

	return net;
}

int edifnet_isinteral(struct edifnet * edifnet){
	struct edifnetportref * tmpportref = NULL;
	if(edifnet != NULL){ 
		for (tmpportref = edifnet->edifnetportref; tmpportref != NULL; tmpportref = tmpportref->next) {
			if (tmpportref->instanceref == NULL) {
				return 0;
			}
		}
	}

	return 1;
}

void edifnet_writer(struct edifnet * edifnet, FILE * out){
	struct edifnet * net = NULL, * nextnet = NULL;
	struct edifnetportref * portref = NULL;
	if(edifnet == NULL || out == NULL){
		fprintf(stderr, "edifnet write error.\n");
		return;
	}
	for(net = edifnet; net != NULL; net = net->next){
		nextnet = net->next;
		gkfputs("   (net ");
		gkfputs(net->net);
		gkfputx;
		gkfputs("    (joined"); 
		for (portref = net->edifnetportref; portref != NULL; portref = portref->next) {
			gkfputs("\n    (portRef ");
			if (gkisdigit(portref->portref)) {
				gkfputs("&");
			}
			gkfputs(portref->portref);
			gkfputs(" ");
			if(portref->instanceref != NULL){
				gkfputs("(instanceRef ");
				gkfputs(portref->instanceref);
				gkfputs(")"); 
			}else{
				assert(0);
			}
			gkfputs(")");
		}
		gkfputs("))");
		if(nextnet != NULL){
			gkfputx;
		}
	}
}
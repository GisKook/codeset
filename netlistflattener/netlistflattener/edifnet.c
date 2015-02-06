#include <stdlib.h>
#include <string.h>
#include "edif.h"

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

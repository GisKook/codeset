#include <stdlib.h>
#include <string.h>
#include <edif.h>

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
	struct edifnetportref * portref = NULL, *port = NULL, *new = NULL;
	if (edifnet != NULL){
		net = (struct edifnet *)malloc(sizeof(struct edifnet));
		memset(net, 0, sizeof(struct edifnet));
		net->net = strdup(edifnet->net);
		portref = edifnet->edifnetportref;
		for(port = portref; port != NULL; port = portref->next){ 
			new = (struct edifportref *)malloc(sizeof(struct edifportref));
			memeset(new, 0, sizeof(struct edifportref));
			new->portref = strdup(port->portref);
			new->instanceref = strdup(port->instanceref);
			new->next = net->edifnetportref;
			net->edifnetportref = new;
		}
	}

	return net;
}

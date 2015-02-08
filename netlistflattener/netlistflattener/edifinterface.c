#include <string.h>
#include <stdlib.h>

#include "edif.h"

struct edifinterfaceport * edifinterface_copy(struct edifinterfaceport * interface){
	struct edifinterfaceport * edifinterfaceport = NULL, *iptrport = NULL, *tmpinterface;
	for (tmpinterface = interface; tmpinterface != NULL; tmpinterface = tmpinterface->next) {
		iptrport = (struct edifinterfaceport *)malloc(sizeof(struct edifinterfaceport));
		memset(iptrport, 0, sizeof(struct edifinterfaceport));
		iptrport->port = strdup(tmpinterface->port);
		iptrport->direction = tmpinterface->direction; 
		iptrport->next = edifinterfaceport;
		edifinterfaceport = iptrport;
	}

	return edifinterfaceport;
}


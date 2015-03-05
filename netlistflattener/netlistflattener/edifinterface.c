#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "edif.h"
#include "edifwriter.h"

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

void edifinterface_writer(struct edifinterfaceport * interface, FILE * out){
	struct edifinterfaceport * port = NULL, *nextport = NULL; 
	gkfputs("   (interface");
	for(port = interface; port != NULL; port = port->next){ 
		nextport = port->next;
		gkfputs("\n    (port ");
		if (gkisdigit(port->port)) {
			gkfputs("&");
		}
		gkfputs(port->port);
		gkfputs(" (direction ");
		switch(port->direction){
			case DIRECTIONBIDI:
				gkfputs("INOUT))");
				break;
			case DIRECTIONINPUT:
				gkfputs("INPUT))");
				break;
			case DIRECTIONOUTPUT:
				gkfputs("OUTPUT))");
				break;
		}
///		if(nextport != NULL){ 
///			gkfputx;
///		}
	}
	gkfputy;
	
}
#include <stdlib.h>
#include <string.h>
#include "edif.h"
#include "ediflibrary.h"
#include "edifinterface.h"

struct edifsubcircuit{
	char * ediflibrary;
	char * edifcell;
	struct edifinterfaceport * edifinterfaceport;
	struct edifsubcircuit * next;
	int used;
};

struct edifsubcircuit * edifsubcircuit_create(struct ediflibrary * ediflibrary){
	struct edifsubcircuit * subcircuit = NULL, * iptrsubcircuit = NULL; 
	struct ediflibrary * library = NULL;
	struct edifcell * cell = NULL;
	if(ediflibrary != NULL){
		memset(subcircuit, 0, sizeof(struct edifsubcircuit));
		for(library = ediflibrary; library != NULL; library = ediflibrary->next){
			cell = ediflibrary_getcells(library);
			if(cell != NULL){
				if (cell->edifinterfaceport != NULL) { 
					iptrsubcircuit = (struct edifsubcircuit *)malloc(sizeof(struct edifsubcircuit));
					memset(iptrsubcircuit, 0, sizeof(struct edifsubcircuit));
					iptrsubcircuit->ediflibrary = strdup(library->library);
					iptrsubcircuit->edifcell = strdup(cell->cell);
					iptrsubcircuit->edifinterfaceport = edifinterface_copy(cell->edifinterfaceport);
					iptrsubcircuit->next = subcircuit;
					subcircuit = iptrsubcircuit;
				}
			}
		}
	}

	return subcircuit;
}

#include <stdlib.h>
#include <string.h>
#include "edif.h"
#include "ediflibrary.h"

struct edifsubcircuit{
	char * ediflibrary;
	char * edifcell;
	struct edifinterfaceport * edifinterfaceport;
	struct edifsubcircuit * next;
	int used;
};

struct edifsubcircuit * edifsubcircuit_create(struct ediflibrary * ediflibrary){
	struct edifsubcircuit * subcircuit = NULL; 
	struct ediflibrary * library = NULL;
	struct edifcell * cell = NULL;
	if(ediflibrary != NULL){
		subcircuit = (struct edifsubcircuit *)malloc(sizeof(struct edifsubcircuit));
		memset(subcircuit, 0, sizeof(struct edifsubcircuit));
		for(library = ediflibrary; library != NULL; library = ediflibrary->next){
			edifcell = ediflibrary_getcells(library);
			if(edifcell != NULL){
				if (edifcell->edifinterfaceport != NULL) {

				}
			}
		}
	}
	return subcircuit;
}

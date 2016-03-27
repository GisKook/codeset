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
	struct edifinstance * edifinstance;
	struct edifnet * edifnet;
	int used;
};

struct edifsubcircuit * edifsubcircuit_create(struct ediflibrary * ediflibrary){
	struct edifsubcircuit * subcircuit = NULL, * iptrsubcircuit = NULL; 
	struct ediflibrary * library = NULL;
	struct edifcell * cell = NULL, *tmpcell = NULL;
	if(ediflibrary != NULL){
		for(library = ediflibrary; library != NULL; library = ediflibrary->next){
			cell = ediflibrary_getcells(library);
			for(tmpcell = cell; tmpcell != NULL; tmpcell = tmpcell->next){
	//			if (tmpcell->edifinterfaceport != NULL) { 
					iptrsubcircuit = (struct edifsubcircuit *)malloc(sizeof(struct edifsubcircuit));
					memset(iptrsubcircuit, 0, sizeof(struct edifsubcircuit));
					iptrsubcircuit->ediflibrary = strdup(library->library);
					iptrsubcircuit->edifcell = strdup(tmpcell->cell);
					iptrsubcircuit->edifinterfaceport = edifinterface_copy(tmpcell->edifinterfaceport);
					iptrsubcircuit->edifinstance = edifinstance_copy(tmpcell->edifcontents->edifinstance);
					iptrsubcircuit->edifnet = edifnet_copynets(tmpcell->edifcontents->edifnet);
					iptrsubcircuit->next = subcircuit;
					subcircuit = iptrsubcircuit;
				//}
			}
		}
	}

	return subcircuit;
}

int edifsubcircuit_isreal(struct edifsubcircuit * subcircuit, char * cellname){
	struct edifsubcircuit * iptrsubcircuit = NULL;
	int cellnamelen = strlen(cellname);
	for(iptrsubcircuit = subcircuit; iptrsubcircuit != NULL; iptrsubcircuit = iptrsubcircuit->next){
		if(cellnamelen == strlen(iptrsubcircuit->edifcell) && 0 == strcmp(iptrsubcircuit->edifcell, cellname)){// && iptrsubcircuit->used == 1){
				return 1;
		}
	}

	return 0;
}

int edifsubcircuit_search(struct edifsubcircuit * subcircuit, char * libraryref, char * cellref){ 
	struct edifsubcircuit * tmpsubcircuit = NULL;
	for(tmpsubcircuit = subcircuit; tmpsubcircuit != NULL; tmpsubcircuit = tmpsubcircuit->next){ 
		if(strlen(tmpsubcircuit->ediflibrary) == strlen(libraryref) && strcmp(tmpsubcircuit->ediflibrary, libraryref) == 0 &&
			strlen(tmpsubcircuit->edifcell) == strlen(cellref) && strcmp(tmpsubcircuit->edifcell, cellref) == 0){
				tmpsubcircuit->used = 1;
				return 1;
		}
	}

	return 0;
}

int edifsubcircuit_getcount(struct edifsubcircuit * subcircuit){
	struct edifsubcircuit * circuit = NULL;
	int count;
	for (circuit = subcircuit, count = 0; circuit != NULL; circuit = circuit->next, ++count);

	return count;
}

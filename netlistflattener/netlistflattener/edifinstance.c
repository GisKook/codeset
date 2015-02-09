#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "edifsubcircuit.h"
#include "edif.h"

global char * glibrary;

int edifinstance_getcount(struct edifinstance * edifinstance){
	int count = 0;
	struct edifinstance * instance = NULL;
	for(instance = edifinstance ; instance != NULL; instance = edifinstance->next, ++count);

	return count;
}

struct edifinstance * edifinstance_copy(struct edifinstance * edifinstance){ 
	struct edifinstance * instance = NULL; 
	if(edifinstance != NULL){
		instance = (struct edifinstance *)malloc(sizeof(struct edifinstance));
		memset(instance, 0, sizeof(struct edifinstance));
		instance->instance = strdup(edifinstance->instance);
		instance->viewref = strdup(edifinstance->viewref);
		instance->cellref = strdup(edifinstance->cellref);
		instance->libraryref = strdup(edifinstance->libraryref);
	}

	return instance;
}

struct edifinstance * edifinstance_addtail(struct edifinstance * instance, struct edifinstance * tail){
	struct edifinstance * tmpinstance = NULL, *preinstance = NULL;
	for (tmpinstance = instance; tmpinstance != NULL; preinstance = tmpinstance, tmpinstance = tmpinstance->next);
	if (preinstance == NULL) {
		return tail;
	}else{
		preinstance->next = tail;
	}

	return instance;
}

struct edifinstance * edifinstance_flatten(struct edifinstance * edifinstance, struct ediflibrary * library, struct edifsubcircuit * subcircuit){
	char * szinstance = NULL;
	char * cellref = NULL;
	char * viewref = NULL;
	char * libraryref = NULL; 
	struct edifinstance * tmpinstances = NULL, * iptrinstance = NULL, * flatteninstance = NULL, * instance = NULL;

	if(edifinstance == NULL || subcircuit == NULL){
		fprintf(stderr, "%s error.\n", __FUNCTION__);
		return NULL;
	}
	for (tmpinstances = edifinstance; tmpinstances != NULL; tmpinstances = tmpinstances->next) {
		szinstance = tmpinstances->instance; 
		cellref = tmpinstances->cellref;
		viewref = tmpinstances->viewref;
		libraryref = tmpinstances->libraryref;
		if (libraryref == NULL) {
			libraryref = glibrary;
		}
		if (edifsubcircuit_search(subcircuit, libraryref, cellref)) {
			flatteninstance = ediflibrary_getintance(library, libraryref, cellref);
			iptrinstance = edifinstance_addtail(flatteninstance, instance);
			instance = iptrinstance;
		}else{
			iptrinstance = (struct edifinstance *)malloc(sizeof(struct edifinstance));
			memset(iptrinstance, 0, sizeof(struct edifinstance)); 
			iptrinstance->libraryref = strdup(libraryref);
			iptrinstance->cellref = strdup(cellref);
			iptrinstance->instance = strdup(szinstance);
			iptrinstance->viewref = strdup(viewref);
			iptrinstance->next = instance;
			instance = iptrinstance;
		}
	}

	return instance; 
}

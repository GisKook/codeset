#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "edifwriter.h"
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
	struct edifinstance * instance = NULL, * iptrinstance = NULL, *tmpinstance = NULL; 
	for(tmpinstance = edifinstance; tmpinstance != NULL; tmpinstance = tmpinstance->next){ 
		if(tmpinstance != NULL){
			iptrinstance = (struct edifinstance *)malloc(sizeof(struct edifinstance));
			memset(iptrinstance, 0, sizeof(struct edifinstance));
			iptrinstance->instance = strdup(tmpinstance->instance);
			iptrinstance->viewref = strdup(tmpinstance->viewref);
			iptrinstance->cellref = strdup(tmpinstance->cellref);
			iptrinstance->libraryref = strdup(tmpinstance->libraryref);
			iptrinstance->next = instance; 
			instance = iptrinstance;
		}
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

void edifinstance_destroy(struct edifinstance * edifinstance){
	struct edifinstance * instance = NULL, *nextinstance = NULL;
	for (instance = edifinstance; instance != NULL; ){
		nextinstance = instance->next;
		free(instance->instance);
		free(instance->libraryref);
		free(instance->cellref);
		free(instance->viewref);
		free(instance);
		instance = nextinstance;
	}
}

void edifinstance_writer(struct edifinstance * instance, FILE * out){
	struct edifinstance * edifinstance = NULL;
	if (instance == NULL || out == NULL) {
		fprintf(stderr, "edifinstance writer. \n");
		return;
	}
	for(edifinstance = instance; edifinstance != NULL; edifinstance = edifinstance->next){
		gkfputs("    (instance ");
		gkfputs(edifinstance->instance);
		gkfputx;
		gkfputs("     (veiwRef ");
		gkfputs(edifinstance->viewref);
		gkfputx;
		gkfputs("      (cellRef ");
		gkfputs(edifinstance->cellref);
		gkfputx;
		if (edifinstance->libraryref != NULL) {
			gkfputs("       (libraryRef ");
			gkfputs(edifinstance->libraryref);
			gkfputs(")");
		}
		gkfputs(")))\n");
	}
}
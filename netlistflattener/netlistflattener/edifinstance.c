#include "edif.h"
#include <stdlib.h>
#include <string.h>

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

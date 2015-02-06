#include "ed.h"
#include "edif.h"
#include <stdlib.h>

int edifgetcellcount(struct edifcell * edifcell){
	int count;
	struct edifcell * cell;
	for(cell = edifcell, count = 0; cell != NULL; cell=cell->next, ++count);
	return count;
}

char ** edifgetcellname(struct edifcell * edifcell){
	int count = edifgetcellcount(edifcell);
	struct edifcell *cell;
	char ** cellname = (char **)malloc(sizeof(char *)*(count+1));
	int i;
	for(cell = edifcell, i = 0; cell != NULL; cell=cell->next, ++i){ 
		cellname[i] = cell->cell;
	}
	cellname[i] = NULL;

	return cellname;
}

int edifgetcellinstancecount(struct edifcell * edifcell){
	struct edifinstance * edifinstance = NULL, *instance = NULL;
	int count = 0;
	if(edifcell != NULL && edifcell->edifcontents != NULL && edifcell->edifcontents->edifinstance != NULL){ 
		edifinstance = edifcell->edifcontents->edifinstance;
		for(instance = edifinstance ; instance != NULL; instance = edifinstance->next, ++count);
	} 

	return count;
}

struct edifinstance * edifgetcellinstances(struct edifcell * edifcell){
	if(edifcell != NULL && edifcell->edifcontents != NULL && edifcell->edifcontents->edifinstance != NULL){ 
		return edifcell->edifcontents->edifinstance;
	}
}

struct edifcell * edifcellcopy(struct edifcell * edifcell){ 
	struct edifcell * cell = (struct edifcell *)malloc(sizeof(struct edifcell));
	cell->cell = strdup(edifcell->cell);
	cell->celltype = strdup(edifcell->celltype);
}

struct edifcell * edifcellflatten(struct edifcell * edifcellout, struct edifcell * edifcellin){ 
	int i;
	if (edifcellin->edifinterfaceport != NULL) { 
	}
}

struct edifcell * ediflibraryflatten(struct edifcell * edifcells){
	struct edifcell * cell = (struct edifcell *)malloc(sizeof(struct edifcell));
	char ** cellname ;
	int cellcount = 0;
	int i;
	memset(cell, 0, sizeof(struct edifcell));
	cell->cell = strdup(edifcells->cell);
	cell->celltype = strdup(edifcells->celltype);
	cellname = edifgetcellname(edifcells); 
	cellcount = edifgetcellcount(edifcells); 
	for (i = 0; i < cellcount; ++i) {

	}

	free(cellname);

	return cell;
};

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

struct edifcell * edifflatten(struct edifcell * edifcells){ 
	struct edifcell * cell = (struct edifcell *)malloc(sizeof(struct edifcell));
	char ** cellname ;
	memset(cell, 0, sizeof(struct edifcell));
	cell->cell = strdup(edifcells->cell);
	cell->celltype = strdup(edifcells->celltype);
	cellname = edifgetcellname(edifcells);

	free(cellname);

	return cell;
};
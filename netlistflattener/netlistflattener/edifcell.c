#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "edifwriter.h"
#include "edif.h"
#include "edifsubcircuit.h"
#include "edifinterface.h"
#include "edifcontents.h"

struct edifcell * edifcell_flatten(struct edifcell * cell, struct ediflibrary * library, struct edifsubcircuit * edifsubcircuit){
	struct edifcell * edifcell = NULL, *iptredifcell = NULL, *tmpcell = NULL;
	if(cell == NULL || edifsubcircuit == NULL){ 
		fprintf(stdout, "%s error.\n", __FUNCTION__);
		return NULL;
	} 
	for(tmpcell = cell; tmpcell != NULL; tmpcell = tmpcell->next){ 
		if(!edifsubcircuit_isreal(edifsubcircuit, tmpcell->cell)){ 
			iptredifcell = (struct edifcell *)malloc(sizeof(struct edifcell));
			memset(iptredifcell, 0, sizeof(struct edifcell));
			iptredifcell->cell = strdup(tmpcell->cell);
			iptredifcell->celltype = strdup(tmpcell->celltype);
			iptredifcell->edifinterfaceport = edifinterface_copy(tmpcell->edifinterfaceport);
			iptredifcell->next = edifcell;
			iptredifcell->edifcontents = edifcontents_flatten(tmpcell->edifcontents, library, edifsubcircuit);
			edifcell = iptredifcell;
		}
	}

	return edifcell;
}

void edifcell_writer(struct edifcell * cell, FILE * out){ 
	if(cell != NULL && out != NULL){
		gkfputs("\n (cell ");
		gkfputs(cell->cell);
		gkfputx; 
		gkfputs("  (celltype ");
		gkfputs(cell->celltype);
		gkfputy;
		gkfputz;
		edifinterface_writer(cell->edifinterfaceport, out);
		edifcontents_writer(cell->edifcontents, out);

		gkfputs("))\n");

	}else{ 
		fprintf(stderr, "cell writer error. %s cell is %lx File is %lx\n", __FUNCTION__, cell, out);
	} 
}
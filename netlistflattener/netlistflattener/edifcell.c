#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "edifwriter.h"
#include "edif.h"
#include "edifsubcircuit.h"
#include "edifinterface.h"
#include "edifcontents.h"
#include "ediflibrary.h"

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

		gkfputs("))");

	}else{ 
		fprintf(stderr, "cell writer error. %s cell is %lx File is %lx\n", __FUNCTION__, cell, out);
	}
}

struct edifcell * edifcell_getcell(struct edifcell * edifcell, char * cellname){
	struct edifcell * cell = NULL; 
	if(edifcell == NULL || cellname == NULL){
		fprintf(stderr, "%s error. edifcell :0x%lx, cellname :%s\n", __FUNCTION__, edifcell, cellname);
		return NULL;
	}
	for (cell = edifcell; cell != NULL; cell = cell->next) {
		if(strlen(cell->cell) == strlen(cellname) && strcmp(cell->cell, cellname) == 0){ 
			break;
		}
	}

	return cell;
}

int edifcell_getsubcellcount(struct edifcell * edifcell, char * cellname){
	struct edifinstance * iptredifinstance = NULL, * instance = NULL;
	int count = 0;
	if (edifcell != NULL && edifcell->edifcontents != NULL && edifcell->edifcontents->edifinstance != NULL) { 
		iptredifinstance = edifcell->edifcontents->edifinstance; 
		for(instance = iptredifinstance; instance != NULL; instance = instance->next){
			if (instance->libraryref == NULL) {
				count++;
			}
		}
	}

	return count;
}

char * edifcell_getsubcellname(struct ediflibrary * ediflibrary, char * libraryname, char * cellname, int index){ 
	struct ediflibrary * library = NULL;
	struct edifcell * cell = NULL;
	struct edifinstance * iptrinstance = NULL, * instance = NULL;
	int count = 0;
	
	library = ediflibrary_getlibrary(ediflibrary, libraryname);

	if(library){ 
		cell = edifcell_getcell(library->edifcell, cellname);
		if(cell != NULL && cell->edifcontents != NULL){ 
			for(iptrinstance = cell->edifcontents->edifinstance; iptrinstance != NULL; iptrinstance = iptrinstance->next){ 
				if (iptrinstance->libraryref == NULL && count == index) {
					return iptrinstance->cellref;
				}
				count ++;
			}
		}
	}
}


int edifcell_isinteralinstance(struct edifcell * edifcell, char * instancename){ 
	struct edifinstance * instance = NULL, *tmpinstance = NULL;
	if (edifcell != NULL && edifcell->edifcontents != NULL) {
		for (instance = edifcell->edifcontents->edifinstance; instance != NULL; instance = instance->next){ 
			if (strlen(instancename) == strlen(instance->instance) && 0 == strcmp(instancename, instance->instance)) {
				if (instance->libraryref == NULL) {
					return 1;
				}
			}
		};
	}

	return 0;
}
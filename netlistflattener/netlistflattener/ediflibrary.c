#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "edif.h"
#include "edifsubcircuit.h"
#include "edifcell.h"
#include "edifinstance.h"
#include "edifnet.h"

global char * glibrary = NULL;

int ediflibrary_getcount(struct ediflibrary * ediflibrary){
	int count = 0;
	struct ediflibrary * lib = NULL;
	if(ediflibrary != NULL){ 
		for (lib = ediflibrary; lib != NULL; lib = ediflibrary->next, count++);
	}

	return count;
}

char ** ediflibrary_getnames(struct ediflibrary * ediflibrary){
	int count = 0;
	struct ediflibrary *lib = NULL;
	char ** libname = NULL;
	int i;
	if(ediflibrary != NULL){
		count = ediflibrary_getcount(ediflibrary);
		libname = (char **)malloc(sizeof(char *)*(count+1));
		for(lib = ediflibrary, i = 0; lib != NULL; lib = ediflibrary->next, ++i){ 
			libname[i] = lib->library;
		}
		libname[i] = NULL;
	}
	
	return libname;
}

struct edifcell * ediflibrary_getcells(struct ediflibrary * ediflibrary){ 
	return ediflibrary != NULL?ediflibrary->edifcell:NULL;
}

struct ediflibrary * ediflibrary_singleflatten(struct ediflibrary * edifsinglelibrary, struct ediflibrary * totallibrary, struct edifsubcircuit * edifsubcircuit){ 
	struct ediflibrary * library = NULL, *iptrlibrary = NULL;
	if(totallibrary == NULL || edifsinglelibrary == NULL || edifsubcircuit == NULL){
		fprintf(stderr, "%s error.\n", __FUNCTION__);
		return NULL;
	} 
	library = (struct ediflibrary *)malloc(sizeof(struct ediflibrary));
	memset(library, 0, sizeof(struct ediflibrary));
	library->library = strdup(edifsinglelibrary->library);
	if(edifsinglelibrary->edifcell != NULL){
		glibrary = library->library;
		library->edifcell = edifcell_flatten(edifsinglelibrary->edifcell, totallibrary, edifsubcircuit);
	}

	return library;
}

struct ediflibrary * ediflibrary_flatten(struct ediflibrary * ediflibrarys){
	struct ediflibrary * library = NULL, *iptrlibrary = NULL, *tmplibrary = NULL;
	struct edifsubcircuit * edifsubcircuit = edifsubcircuit_create(ediflibrarys);
	for(tmplibrary = ediflibrarys; tmplibrary != NULL; tmplibrary = ediflibrarys->next){
		iptrlibrary = ediflibrary_singleflatten(tmplibrary, ediflibrarys, edifsubcircuit);
		iptrlibrary->next = library;
		library = iptrlibrary;
	}

	return library;
}

void ediflibrary_writer(struct ediflibrary * ediflibrary, FILE * out){

}

struct edifinstance * ediflibrary_getintance(struct ediflibrary * library, char * libraryname, char * cellname){
	struct ediflibrary *iptrlibrary = NULL;
	struct edifinstance * instance = NULL, *iptrinstance = NULL, *tmpinstance = NULL;
	struct edifcell * cell = NULL;
	for(iptrlibrary = library; iptrlibrary != NULL; iptrlibrary = iptrlibrary->next){
		if (strlen(iptrlibrary->library, libraryname) == strlen(libraryname) && 0 == strcmp(iptrlibrary->library, libraryname)){
			cell = library->edifcell;
			for(cell = library->edifcell; cell != NULL; cell = cell->next){
				if(cell != NULL && strlen(cell->cell) == strlen(cellname) && 0 == strcmp(cell->cell, cellname)){ 
					if(cell->edifcontents != NULL && cell->edifcontents->edifinstance != NULL){ 
						for(tmpinstance = cell->edifcontents->edifinstance; tmpinstance != NULL; tmpinstance = tmpinstance->next) {
							iptrinstance = (struct edifinstance *)malloc(sizeof(struct edifinstance));
							memset(iptrinstance, 0, sizeof(struct edifinstance)); 
							iptrinstance->libraryref = strdup(tmpinstance->libraryref);
							iptrinstance->cellref = strdup(tmpinstance->cellref);
							iptrinstance->viewref = strdup(tmpinstance->viewref);
							iptrinstance->instance = strdup(tmpinstance->instance);
							iptrinstance->next = instance;
							instance = iptrinstance;
						}
						break;
					}
				} 
			}
			
		}
	}

	return instance;
}

struct edifnetportref * ediflibrary_getnetportref(struct ediflibrary * library, char * libraryname, char * cellname, char * portref){
	struct ediflibrary *iptrlibrary = NULL;
	struct edifnet * net = NULL, *iptrnet = NULL, *tmpnet = NULL;
	struct edifcell * cell = NULL;
	struct edifnetportref * tmpnetportref = NULL, *iptrnetportref = NULL, *portrefs = NULL;
	for(iptrlibrary = library; iptrlibrary != NULL; iptrlibrary = iptrlibrary->next){
		if (strlen(iptrlibrary->library, libraryname) == strlen(libraryname) && 0 == strcmp(iptrlibrary->library, libraryname)){
			cell = library->edifcell;
			for(cell = library->edifcell; cell != NULL; cell = cell->next){
				if(cell != NULL && strlen(cell->cell) == strlen(cellname) && 0 == strcmp(cell->cell, cellname)){ 
					if(cell->edifcontents != NULL && cell->edifcontents->edifnet != NULL){ 
						for(tmpnet = cell->edifcontents->edifnet; tmpnet != NULL; tmpnet = tmpnet->next) { 
							portrefs = edifnet_getnetports(tmpnet, portref); 
							if(portrefs != NULL){
								return portrefs;
							}
						}
						break;
					}
				}
			}
		}
	}
	return NULL;
}

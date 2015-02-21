#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "edif.h"
#include "edifsubcircuit.h"
#include "edifcell.h"
#include "edifinstance.h"
#include "edifnet.h"
#include "edifwriter.h"

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
	library->usedinstance = (struct edifinstancename **)malloc(sizeof(struct edifinstancename *)*INSTANCECOUNT);
	library->instancecapacity = INSTANCECOUNT;
	memset(library->usedinstance, 0, sizeof(struct edifinstancename *)*INSTANCECOUNT);
	library->library = strdup(edifsinglelibrary->library);
	if(edifsinglelibrary->edifcell != NULL){
		glibrary = library->library;
		library->edifcell = edifcell_flatten(library, edifsinglelibrary->edifcell, totallibrary, edifsubcircuit);
	}

	return library;
}

struct ediflibrary * ediflibrary_flatten(struct ediflibrary * ediflibrarys){
	struct ediflibrary * library = NULL, *iptrlibrary = NULL, *tmplibrary = NULL;
	struct edifsubcircuit * edifsubcircuit = edifsubcircuit_create(ediflibrarys);
	for(tmplibrary = ediflibrarys; tmplibrary != NULL; tmplibrary = ediflibrarys->next){
		iptrlibrary = ediflibrary_singleflatten(tmplibrary, ediflibrarys, edifsubcircuit);
		if (iptrlibrary != NULL) {
			iptrlibrary->next = library;
			library = iptrlibrary;
		}
	}

	if (library == NULL) {
		return ediflibrarys;
	}
	return library;
}

void ediflibrary_writer(struct ediflibrary * ediflibrary, FILE * out){
	if(ediflibrary == NULL || out == NULL){
		fprintf(stderr, "ediflibrary write error.\n");
		return;
	}
	edifcell_writer(ediflibrary->edifcell, out); 
	gkfputy;
}


struct edifnetportref * ediflibrary_getnetportref(struct ediflibrary * library, char * libraryname, char * cellname, char * portref){
	struct ediflibrary *iptrlibrary = NULL;
	struct edifnet *tmpnet = NULL;
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

struct edifnet * ediflibrary_getnet(struct ediflibrary * library, struct ediflibrary * referlibrary, char * libraryname, char * cellname, char * instancename){
	struct ediflibrary *iptrlibrary = NULL;
	struct edifnet * net = NULL, *iptrnet = NULL, *tmpnet = NULL;
	struct edifcell * cell = NULL;
	struct edifnetportref * tmpnetportref = NULL, *iptrnetportref = NULL, *portrefs = NULL;
	for(iptrlibrary = referlibrary; iptrlibrary != NULL; iptrlibrary = iptrlibrary->next){
		if (strlen(iptrlibrary->library, libraryname) == strlen(libraryname) && 0 == strcmp(iptrlibrary->library, libraryname)){
			cell = referlibrary->edifcell;
			for(cell = referlibrary->edifcell; cell != NULL; cell = cell->next){
				if(cell != NULL && strlen(cell->cell) == strlen(cellname) && 0 == strcmp(cell->cell, cellname)){ 
					if(cell->edifcontents != NULL && cell->edifcontents->edifnet != NULL){ 
						for(tmpnet = cell->edifcontents->edifnet; tmpnet != NULL; tmpnet = tmpnet->next) { 
							if(edifnet_isinteral(tmpnet)){ 
								iptrnet = edifnet_copyrename(library, tmpnet, instancename);
								iptrnet->next = net;
								net = iptrnet;
							}
						}
						return net;
					}
				}
			}
		}
	}
	
	return NULL;
}

struct ediflibrary * ediflibrary_getlibrary(struct ediflibrary * library, char * libraryname){
	struct ediflibrary * iptrlibrary = NULL;
	if (library == NULL || libraryname == NULL) {
		fprintf(stderr, "%s error. library: 0x%lx libraryname: %s\n", __FUNCTION__, library, libraryname);
		return NULL;
	}
	for (iptrlibrary = library; iptrlibrary != NULL; iptrlibrary = iptrlibrary->next) {
		if(strlen(library->library) == strlen(libraryname) && 0 == strcmp(libraryname, library->library)){
			return iptrlibrary;
		}
	}

	return NULL;
}

struct edifcell * ediflibrary_getcell(struct ediflibrary * library, char * cellname){
	struct edifcell * cells = NULL, *iptrcell = NULL;
	cells = ediflibrary_getcells(library); 
	for(iptrcell = cells; iptrcell != NULL; iptrcell = iptrcell->next){
		if(strlen(iptrcell->cell) == strlen(cellname) && strcmp(iptrcell->cell, cellname) == 0){
			return iptrcell;
		}
	}
	return NULL;
}
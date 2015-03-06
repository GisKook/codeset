#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "edifwriter.h"
#include "edifsubcircuit.h"
#include "edif.h"

global char * glibrary;

struct edifinstance * edifinstance_getinternalintance(struct ediflibrary * library, struct ediflibrary * referlibrary, char * libraryname, char * cellname, char * szinstance);
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

struct edifinstance * edifinstance_copysingle(struct edifinstance * edifinstance){
	struct edifinstance * instance = NULL;
	if(edifinstance == NULL){
		return NULL;
	}
	instance = (struct edifinstance *)malloc(sizeof(struct edifinstance));
	memset(instance, 0, sizeof(struct edifinstance));
	instance->instance = strdup(edifinstance->instance);
	instance->cellref = strdup(edifinstance->cellref);
	instance->libraryref = strdup(edifinstance->libraryref);
	instance->viewref = strdup(edifinstance->viewref);

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

int edifinstance_hasused(struct ediflibrary * library, char * instancename){
	struct edifinstancename ** names = NULL;
	int namecount = 0;
	int i;
	names = library->usedinstance;
	namecount = library->instancecount;
	for(i = 0; i < namecount; ++i){
		if(strlen(names[i]->instancename) == strlen(instancename) && 0 == strcmp(names[i]->instancename, instancename)){
			return 1;
		}
	}

	return 0;
}

void edifinstance_addusedinstance(struct ediflibrary * library, char * instancename, char * originalname, char * uidname){
	struct edifinstancename * name = NULL;
	struct edifinstancename ** names = NULL;
	int i;
	name = (struct edifinstancename *)malloc(sizeof(struct edifinstancename));
	memset(name, 0, sizeof(struct edifinstancename));
	name->instancename = strdup(instancename);
	name->originalname = strdup(originalname);
	name->uidname = strdup(uidname);
	if (library->instancecount >= library->instancecapacity) {
		names = (struct edifinstancename **)malloc(sizeof(struct edifinstancename *) * library->instancecapacity * 2);
		memset(names, 0, sizeof(struct edifinstancename *) * library->instancecapacity * 2);
		library->instancecapacity = library->instancecapacity * 2;
		for(i = 0; i < library->instancecount; ++i){
			names[i] = (struct edifinstancename *)malloc(sizeof(struct edifinstancename));
			memset(names[i], 0, sizeof(struct edifinstancename));
			names[i]->instancename = strdup(library->usedinstance[i]->instancename);
			names[i]->originalname = strdup(library->usedinstance[i]->originalname);
			names[i]->uidname = strdup(library->usedinstance[i]->uidname);
			free(library->usedinstance[i]->instancename);
			free(library->usedinstance[i]->originalname);
			free(library->usedinstance[i]->uidname);
			free(library->usedinstance[i]);
		}
		library->usedinstance[library->instancecount] = name;
		library->instancecount++;
	}else{
		library->usedinstance[library->instancecount] = name;
		library->instancecount++;
	}
}

char * edifinstance_rename(struct ediflibrary * library, char * instancename){
	char * newname = NULL, iptrname = NULL;
	char c = 0;
	int i = 0, num = 0;
	char tmpname[16] = {0};
	newname = strdup(instancename); 
	while(edifinstance_hasused(library, newname) == 1){
		i = strlen(newname)-1;
		for(;i != 0; i--){
			if(newname[i] == '_'){
				if(gkisdigit(newname + i + 1)){
					num = atoi(newname + i + 1);
					num ++;
					sprintf(tmpname, "%d", num);
					iptrname = (char *)malloc(i+strlen(num)+2);
					memset(iptrname, 0, i + strlen(num) + 2);
					memcpy(iptrname, newname, i + 1);
					memcpy(iptrname + i + 1, tmpname, strlen(tmpname));
					free(newname);
					newname = iptrname;
					break;
				}else{
					free(newname);
					newname = (char *)malloc(strlen(instancename)+3);
					memset(newname, 0, strlen(instancename) + 3);
					memcpy(newname, instancename, strlen(instancename));
					newname[strlen(instancename)] = '_';
					newname[strlen(instancename) + 1] = '1';
				}
			}
		}
		if(i == 0){
			free(newname);
			newname = (char *)malloc(strlen(instancename)+3);
			memset(newname, 0, strlen(instancename) + 3);
			memcpy(newname, instancename, strlen(instancename));
			newname[strlen(instancename)] = '_';
			newname[strlen(instancename) + 1] = '1';
		}
	}

	return newname;
}

struct edifinstance * edifinstance_flatten(struct ediflibrary * library, struct edifinstance * edifinstance, struct ediflibrary * referlibrary, struct edifsubcircuit * subcircuit){
	char * szinstance = NULL;
	char * cellref = NULL;
	char * viewref = NULL;
	char * libraryref = NULL; 
	struct edifinstance * tmpinstances = NULL, * iptrinstance = NULL, * flatteninstance = NULL, * instance = NULL;

	char * newname = NULL;

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
			flatteninstance = edifinstance_getinternalintance(library, referlibrary, libraryref, cellref, szinstance);
			iptrinstance = edifinstance_addtail(flatteninstance, instance);
			instance = iptrinstance;
		}else{
			iptrinstance = (struct edifinstance *)malloc(sizeof(struct edifinstance));
			memset(iptrinstance, 0, sizeof(struct edifinstance)); 
			if(edifinstance_hasused(library, szinstance) == 1){ 
				newname = edifinstance_rename(library, szinstance);
				edifinstance_addusedinstance(library, newname, szinstance, newname);
			}else{
				newname = strdup(szinstance);
				edifinstance_addusedinstance(library, newname, szinstance, newname);
			}
			iptrinstance->libraryref = strdup(libraryref);
			iptrinstance->cellref = strdup(cellref);
			iptrinstance->instance = newname;
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
		gkfputs("     (viewRef ");
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

int edifinstance_needflatten(struct edifinstance * instance, char * instancename){
	struct edifinstance * tmpinstance = NULL;
	if(instancename == NULL){
		return 0;
	}
	if(instance){
		for(tmpinstance = instance; tmpinstance != NULL; tmpinstance = tmpinstance->next){
			if (strlen(tmpinstance->instance) == strlen(instancename) && strcmp(tmpinstance->instance, instancename) == 0) {
				if(tmpinstance->libraryref == NULL){
					return 1;
				}
			}
		}
	}

	return 0;
}

char * edifinstance_getsubcircuitname(struct edifinstance * instance){
	return instance->libraryref == NULL ? instance->cellref : NULL;
}

struct edifinstance * edifinstance_getinstance(struct edifinstance * instance, char * instancename){
	struct edifinstance * edifinstance = NULL, * tmpinstance = NULL;
	for(tmpinstance = instance; tmpinstance != NULL; tmpinstance = tmpinstance->next){
		if(strlen(instancename) == strlen(tmpinstance->instance) && 0 == strcmp(instancename, tmpinstance->instance)){
			return tmpinstance;
		}
	}

	return NULL;
}

void edifinstance_addnames(char ** instancenames, char * instancename){
	int i;
	int hasinstance = 0;
	for(i = 0 ;instancenames[i] != NULL;i++){ 
		if(strlen(instancenames[i]) == strlen(instancename) && strcmp(instancenames[i], instancename) == 0){
			hasinstance = 1;
			break;
		}
	}
	if(!hasinstance){
		for(i = 0; instancenames[i] != NULL; i++);
		instancenames[i] = strdup(instancename);
	}
}

//struct edifnet * edifnet_getallinternalnets(struct ediflibrary * library){
//	int subcellcount = 0;
//	char * subcellname = NULL;
//	int i = 0;
//	subcellcount = edifcell_getsubcellcount(library->edifcell, ""); 
//	for(i = 0; i < subcellcount; ++i){
//}
struct edifinstance * edifinstance_getinternalintance(struct ediflibrary * library, struct ediflibrary * referlibrary, char * libraryname, char * cellname, char * szinstance){
	struct ediflibrary *iptrlibrary = NULL;
	struct edifinstance * instance = NULL, *iptrinstance = NULL, *tmpinstance = NULL;
	struct edifcell * cell = NULL;
	char * instancename = NULL;
	int instancepart1len, instancepart2len;
	char * name = NULL;
	for(iptrlibrary = referlibrary; iptrlibrary != NULL; iptrlibrary = iptrlibrary->next){
		if (strlen(iptrlibrary->library) == strlen(libraryname) && 0 == strcmp(iptrlibrary->library, libraryname)){
			cell = referlibrary->edifcell;
			for(cell = referlibrary->edifcell; cell != NULL; cell = cell->next){
				if(cell != NULL && strlen(cell->cell) == strlen(cellname) && 0 == strcmp(cell->cell, cellname)){ 
					if(cell->edifcontents != NULL && cell->edifcontents->edifinstance != NULL){ 
						for(tmpinstance = cell->edifcontents->edifinstance; tmpinstance != NULL; tmpinstance = tmpinstance->next) {
							iptrinstance = (struct edifinstance *)malloc(sizeof(struct edifinstance));
							memset(iptrinstance, 0, sizeof(struct edifinstance)); 
							iptrinstance->libraryref = strdup(tmpinstance->libraryref);
							iptrinstance->cellref = strdup(tmpinstance->cellref);
							iptrinstance->viewref = strdup(tmpinstance->viewref);
							instancepart1len = strlen(szinstance);
							instancepart2len = strlen(tmpinstance->instance);
							instancename = (char *)malloc(instancepart1len+instancepart2len+1);
							memset(instancename, 0, instancepart1len+instancepart2len+1);
							memcpy(instancename, szinstance, instancepart1len);
							memcpy(instancename+instancepart1len, tmpinstance->instance, instancepart2len); 
							if(edifinstance_hasused(library, tmpinstance->instance)){
								name = edifinstance_rename(library, tmpinstance->instance);
								edifinstance_addusedinstance(library, name, tmpinstance->instance, instancename);
							}else{
								name = strdup(tmpinstance->instance);
								edifinstance_addusedinstance(library, name, tmpinstance->instance, instancename);
							}
							iptrinstance->instance = name;
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

char * edifinstance_getrealname(struct ediflibrary * library, char * uidname){
	int i;
	char * realname = NULL;
	for(i = 0; i < library->instancecount; ++i){
		if(library->usedinstance[i]->uidname != NULL){
			if(strlen(library->usedinstance[i]->uidname) == strlen(uidname) && strcmp(library->usedinstance[i]->uidname, uidname) == 0){
				realname = strdup(library->usedinstance[i]->instancename);
				return realname;
			}
		}
	}

	return NULL;
}

int edifinstance_isflat(struct edifinstance * edifinstance, struct edifsubcircuit * edifsubcircuit){
	struct edifinstance * tmpinstance = NULL, * iptrinstance = NULL;
	if(edifinstance == NULL || edifsubcircuit == NULL){
		return 1;
	}
	for(tmpinstance = edifinstance; tmpinstance != NULL; tmpinstance = tmpinstance->next){
		if(tmpinstance->libraryref == NULL){
			if(edifsubcircuit_search(edifsubcircuit, glibrary, tmpinstance->cellref)){
				return 0;
			}
		}
	}

	return 1;
}

struct edifinstance * edifinstance_getflatinstance(struct edifinstance * edifinstance){
	struct edifinstance * tmpinstance = NULL, *instance = NULL, *iptrinstance = NULL;
	if(edifinstance == NULL){
		return NULL;
	}
	for(tmpinstance = edifinstance; tmpinstance != NULL; tmpinstance = tmpinstance->next){ 
		if(tmpinstance->libraryref != NULL){
			iptrinstance = edifinstance_copysingle(tmpinstance);
			iptrinstance->next = instance;
			instance = iptrinstance;
		}
	}

	return instance;
}

struct edifinstance * edifinstance_getfoldinstance(struct edifinstance * edifinstance){
	struct edifinstance * tmpinstance = NULL, *instance = NULL, *iptrinstance = NULL;
	if(edifinstance == NULL){
		return NULL;
	}
	for(tmpinstance = edifinstance; tmpinstance != NULL; tmpinstance = tmpinstance->next){ 
		if(tmpinstance->libraryref == NULL){
			iptrinstance = edifinstance_copysingle(tmpinstance);
			iptrinstance->next = instance;
			instance = iptrinstance;
		}
	}

	return instance;
}

struct edifinstance * edifinstance_flattenonce(struct ediflibrary * library, struct edifinstance * edifinstance, struct ediflibrary * referlibrary, struct edifsubcircuit * subcircuit){

	struct edifinstance * iptrflatinstance = NULL, * iptrfoldinstance = NULL, *instance = NULL;
	iptrflatinstance = edifinstance_getflatinstance(edifinstance);
	iptrfoldinstance = edifinstance_getfoldinstance(edifinstance);
	instance = edifinstance_flatten(library, iptrfoldinstance, referlibrary, subcircuit);
	iptrflatinstance = edifinstance_addtail(iptrflatinstance, instance);
	edifinstance_destroy(iptrfoldinstance);

	return iptrflatinstance;
}
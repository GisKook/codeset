#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "edifwriter.h"
#include "edif.h"
#include "ediflibrary.h"
#include "edifinstance.h"
#include "edifcontents.h"

global char * glibrary;
global int blogicalerror;
global struct ediflibrary * gs_lib;
global int HasGlobal;

struct edifnetportref * edifnet_getportrefs(struct ediflibrary * library, struct edifnet * edifnet, char * instancename, char * portref);
struct edifnetportref * edifnetportref_copy(struct edifnetportref * portref);
struct edifnet *edifnet_add(struct edifnet * edifnet, struct edifnet * net); 
int edifnet_checkglobalnetname(char * netname); 
int edifnet_getcount(struct edifnet * edifnet){
	int count = 0;
	struct edifnet * net = NULL;
	if(edifnet != NULL){
		for(net = edifnet; net != NULL; net = edifnet->next, ++count);
	}
	return count;
}

struct edifnet * edifnet_copy(struct edifnet * edifnet){
	struct edifnet * net = NULL;
	struct edifnetportref * portref = NULL, *port = NULL, *newref = NULL;
	if (edifnet != NULL){
		net = (struct edifnet *)malloc(sizeof(struct edifnet));
		memset(net, 0, sizeof(struct edifnet));
		net->net = strdup(edifnet->net);
		portref = edifnet->edifnetportref;
		for(port = portref; port != NULL; port = port->next){ 
			newref = (struct edifnetportref *)malloc(sizeof(struct edifnetportref));
			memset(newref, 0, sizeof(struct edifnetportref));
			newref->portref = strdup(port->portref);
			newref->instanceref = strdup(port->instanceref);
			newref->next = net->edifnetportref;
			net->edifnetportref = newref;
		}
	}

	return net;
}

struct edifnet * edifnet_copyrename(struct ediflibrary * library, struct edifnet * edifnet, char * instance){
	struct edifnet * net = NULL;
	struct edifnetportref * portref = NULL, *port = NULL, *newref = NULL;
	char * sznewinstanceref = NULL;
	int instancerefpart1len = 0;
	int instancerefpart2len = 0;
	char * sznet = NULL;
	int netpart1len = 0;
	int netpart2len = 0;
	char * realname = NULL;
	instancerefpart1len = strlen(instance);
	netpart1len = strlen(instance); 

	if (edifnet != NULL){
		net = (struct edifnet *)malloc(sizeof(struct edifnet));
		memset(net, 0, sizeof(struct edifnet));
		netpart2len = strlen(edifnet->net);
		if(edifnet_checkglobalnetname(edifnet->net)){
			sznet = (char *)malloc(netpart2len + 1);
			memset(sznet, 0, netpart2len + 1);
			memcpy(sznet, edifnet->net, netpart2len);
		}else{
			sznet = (char *)malloc(netpart1len + netpart2len + 1);
			memset(sznet, 0, netpart1len + netpart2len + 1);
			memcpy(sznet, instance, netpart1len);
			memcpy(sznet + netpart1len, edifnet->net, netpart2len);
		}
	
		net->net = sznet;
		portref = edifnet->edifnetportref;
		for(port = portref; port != NULL; port = port->next){ 
			newref = (struct edifnetportref *)malloc(sizeof(struct edifnetportref));
			memset(newref, 0, sizeof(struct edifnetportref));
			newref->portref = strdup(port->portref); 
			instancerefpart2len = strlen(port->instanceref);
			if(instancerefpart1len > 1024 || instancerefpart2len > 1024){
				return NULL;
			}
			sznewinstanceref = (char *)malloc(instancerefpart1len + instancerefpart2len + 1);
			memset(sznewinstanceref, 0, instancerefpart1len+instancerefpart2len+1);
			memcpy(sznewinstanceref, instance, instancerefpart1len);
			memcpy(sznewinstanceref+instancerefpart1len, port->instanceref, instancerefpart2len);
			if(strlen(instance) != 0){
				realname = edifinstance_getrealname(library, sznewinstanceref);
			}else{
				realname = strdup(sznewinstanceref);
			}
			
			free(sznewinstanceref);
			newref->instanceref = realname;
			newref->next = net->edifnetportref;
			net->edifnetportref = newref;
		}
	}

	return net;
}

struct edifnet * edifnet_copynets(struct edifnet * edifnet){
	struct edifnet * net = NULL, *iptrnet = NULL, * tmpnet = NULL;
	for(tmpnet = edifnet; tmpnet != NULL; tmpnet = tmpnet->next){
		iptrnet = edifnet_copy(tmpnet);
		iptrnet->next = net;
		net = iptrnet;
	}

	return net;
}

void * edifnet_destroy(struct edifnet * edifnet){
	struct edifnet * iptrnet = NULL, *iptrnextnet = NULL;
	struct edifnetportref * portref = NULL, *nextportref = NULL;
	for(iptrnet = edifnet; iptrnet != NULL; ){
		iptrnextnet = iptrnet->next;
		free(iptrnet->net);
		for(portref = iptrnet->edifnetportref; portref != NULL; ){ 
			nextportref = portref->next; 
			free(portref->instanceref);
			free(portref->portref);
			free(portref);
			portref = nextportref;
		}
		free(iptrnet);
		iptrnet = iptrnextnet;
	}
}

char * edifnet_getcellname(struct edifinstance * edifinstance, char * instanceref){
	struct edifinstance * iptrinstance = NULL;
	for(iptrinstance = edifinstance; iptrinstance != NULL; iptrinstance = iptrinstance->next){
		if(strlen(instanceref) == strlen(iptrinstance->instance) && strcmp(instanceref, iptrinstance->instance) == 0 && iptrinstance->libraryref == NULL){
			return iptrinstance->cellref;
		}
	}

	return NULL;
}

struct edifnetportref * edifnet_getnetports(struct edifnet * edifnet, char * szportref){
	struct edifnetportref *tmpportref = NULL, *portref = NULL, *iptrportref = NULL;
	int flag = 0;
	for (tmpportref = edifnet->edifnetportref; tmpportref != NULL; tmpportref = tmpportref->next) {
		if(strlen(tmpportref->portref) == strlen(szportref) && strcmp(tmpportref->portref, szportref) == 0){ 
			flag = 1;
			break;
		}
	}
	if(flag){
		for (tmpportref = edifnet->edifnetportref; tmpportref != NULL; tmpportref = tmpportref->next) { 
			if (tmpportref->instanceref != NULL) {
				iptrportref = (struct edifnetportref *)malloc(sizeof(struct edifnetportref));
				memset(iptrportref, 0, sizeof(struct edifnetportref));
				iptrportref->instanceref = strdup(tmpportref->instanceref);
				iptrportref->portref = strdup(tmpportref->portref);
				iptrportref->next = portref;
				portref = iptrportref;
			}
		}
	}

	return portref;
}

struct edifnetportref * edifnet_addtail(struct edifnetportref * edifnetportref, struct edifnetportref * tail){
	struct edifnetportref * tmpportref = NULL, *preportref = NULL;
	for (tmpportref = edifnetportref; tmpportref != NULL; preportref = tmpportref, tmpportref = tmpportref->next);
	if (preportref == NULL) {
		return tail;
	}else{
		preportref->next = tail;
	}

	return edifnetportref;
}

#define MAXCELLNAMELEN 128
struct edifnet * edifnet_flatten(struct edifcontents * edifcontents, struct ediflibrary * library, struct edifsubcircuit * eidfsubcircuit){
	struct edifnet * net= NULL, *iptrnet = NULL, *tmpnet = NULL, *internet = NULL;
	struct edifinstance * instance = NULL;
	struct edifnetportref * edifnetportref = NULL, *portref = NULL, *iptrportref = NULL;
	char * cellname = NULL, *subcellname = NULL, *tmpcellname = NULL;
	char * cellnames[128] = {NULL};
	int cellcount = 0, i = 0;
	int alreadhave = 0;
	int flatten = 0;
	int subcellcount = 0;
	struct edifcell * cell = NULL, *iptrcell = NULL;
	struct ediflibrary * ediflibrary = NULL;
	if (edifcontents == NULL || library == NULL) {
		return NULL;
	}
	for(tmpnet = edifcontents->edifnet; tmpnet != NULL; tmpnet = tmpnet->next){
//		iptrnet = (struct edifnet *)malloc(sizeof(struct edifnet));
//		memset(iptrnet, 0, sizeof(struct edifnet)); 
//		iptrnet->net = strdup(tmpnet->net);
//		for(edifnetportref = tmpnet->edifnetportref; edifnetportref != NULL; edifnetportref = edifnetportref->next){
//			iptrportref = (struct edifnetportref *)malloc(sizeof(struct edifnetportref));
//			memset(iptrportref, 0, sizeof(struct edifnetportref)); 
//			cellname = edifnet_getcellname(edifcontents->edifinstance, edifnetportref->instanceref);
//			if(cellname == NULL){
//				iptrportref->instanceref = strdup(edifnetportref->instanceref);
//				iptrportref->portref = strdup(edifnetportref->portref);
//				iptrportref->next = portref;
//				portref = iptrportref;
//			}else{ 
//				flatten = 1;
//				iptrportref = ediflibrary_getnetportref(library, glibrary, cellname, edifnetportref->portref); 
//				tmpcellname = cellname;
//				ediflibrary = ediflibrary_getlibrary(library, glibrary);
//				iptrcell= ediflibrary->edifcell;
//				while(iptrportref == NULL){
//					cell = edifcell_getcell(iptrcell, tmpcellname);
//					subcellcount = edifcell_getsubcellcount(cell, tmpcellname);
//					if(subcellcount == 0){
//						break;
//					}
//					for(i = 0; i < subcellcount; ++i){
//						subcellname = edifcell_getsubcellname(library, glibrary, tmpcellname, i);
//						iptrportref = ediflibrary_getnetportref(library, glibrary, subcellname, edifnetportref->portref);
//						if (iptrportref != NULL) {
//							break;
//						}
//					}
//					tmpcellname = subcellname;
//					iptrcell = cell;
//				}
//				for(i = 0; i < cellcount; ++i){
//					if (strlen(cellname) == strlen(cellnames[i]) && strcmp(cellname, cellnames[i]) == 0) {
//						alreadhave = 1;
//					}
//				}
//				if(!alreadhave){
//					cellnames[cellcount++] = strdup(cellname);
//					alreadhave = 0;
//				}
//				
//				edifnet_addtail(iptrportref, portref);
//				portref = iptrportref;
//			}
//		}
//		iptrnet->edifnetportref = portref;
//		portref = NULL;
//		iptrnet->next = net;
//		net = iptrnet;
	}
	
//	iptrnet = NULL;
//
//	for (i = 0; i < cellcount; ++i) { 
//		iptrnet = ediflibrary_getnet(library, glibrary, cellnames[i]);
//		iptrnet->next = net;
//		net = iptrnet;
//	}
//	if(flatten){
//		instance = edifinstance_flatten(edifcontents->edifinstance, library, eidfsubcircuit);
//		edifinstance_destroy(edifcontents->edifinstance);
//		edifcontents->edifinstance = instance; 
//	}
	
	return net;
}

int edifnet_needflatten(struct edifcontents * edifcontents, struct edifnet * net){
	struct edifnetportref * portref = NULL, * tmpportref = NULL;
	struct edifinstance * instance = NULL;
	if(edifcontents == NULL || edifcontents->edifinstance == NULL || edifcontents->edifnet == NULL){
		return NULL;
	}
	instance = edifcontents->edifinstance;
	for (tmpportref = net->edifnetportref; tmpportref != NULL; tmpportref = tmpportref->next) {
		if(edifinstance_needflatten(instance, tmpportref->instanceref)){
			return 1;
		}
	}

	return 0;
}

void edifnet_addnames(struct ediflibrary * library, char * orignalname, char * newname){
	struct edifnetnames ** iptrnetnames;
	if(library->netcount >= library->netcapacity){
		iptrnetnames = (struct edifnetnames **)malloc(sizeof(struct edifnetnames *)*library->netcapacity*2);
		memset(iptrnetnames, 0, sizeof(struct edifnetnames *)*library->netcapacity*2);
		memcpy(iptrnetnames, library->netnames, sizeof(struct edifnetnames *)*library->netcount);
		free(library->netnames);
		library->netnames = iptrnetnames;
		library->netcapacity = library->netcapacity * 2;
	}
	library->netnames[library->netcount]  = (struct edifnetnames *)malloc(sizeof(struct edifnetnames));
	memset(library->netnames[library->netcount], 0, sizeof(struct edifnetnames));
	library->netnames[library->netcount]->netname = strdup(newname);
	library->netnames[library->netcount]->originalname = strdup(orignalname);
	++library->netcount;
}


struct edifnet * edifnet_getinternalnets(struct ediflibrary * library, struct edifnet * edifnet, char * instancename){
	struct edifnet * nets = NULL, * tmpnets= NULL, * iptrnets = NULL;
	char * newnetname = NULL;
	int newnamepart1len = 0;
	int newnamepart2len = 0;
	newnamepart1len = strlen(instancename);
	if ( edifnet != NULL) { 
		for (tmpnets = edifnet; tmpnets != NULL; tmpnets = tmpnets->next) {
			if(edifnet_isinteral(tmpnets)){ 
				iptrnets = edifnet_copyrename(library, tmpnets, instancename);
				// 
				//
				iptrnets->next = nets;
				newnamepart2len = strlen(tmpnets->net);
				newnetname = (char *)malloc(sizeof(newnamepart1len + newnamepart2len + 1));
				memset(newnetname, 0, newnamepart1len + newnamepart2len + 1);
				memcpy(newnetname, instancename, newnamepart1len);
				memcpy(newnetname + newnamepart1len, tmpnets->net, newnamepart2len);
				nets->net = newnetname;
				nets = iptrnets;
			}
		}
	}

	return nets;
}

struct edifnet * edifnet_getnet(struct ediflibrary * referlibrary, char * libraryname, char * cellname){ 
	struct ediflibrary * library = NULL;
	struct edifcell * cell = NULL;
	library = ediflibrary_getlibrary(referlibrary, libraryname);
	cell = ediflibrary_getcell(library, cellname);

	return cell->edifcontents->edifnet;
}

struct edifnet * edifnet_flattensingle(struct ediflibrary * library, struct edifnet * edifnet, struct edifcontents * edifcontents, struct ediflibrary * referlibrary, char ** instancenames){
	struct edifnetportref * netportref = NULL, *iptrnetportref = NULL, *tmpnetportref = NULL;
	struct edifinstance * instance = NULL;
	struct edifnet * net = NULL, *iptrnet = NULL;
	char * cellname = NULL;
	if(edifnet == NULL || edifnet->edifnetportref == NULL || edifcontents == NULL){
		return NULL;
	}
	instance = edifcontents->edifinstance;

	for (tmpnetportref = edifnet->edifnetportref; tmpnetportref != NULL; tmpnetportref = tmpnetportref->next) {
		if(edifinstance_needflatten(instance, tmpnetportref->instanceref)){ 
			cellname = edifnet_getcellname(instance, tmpnetportref->instanceref);
			iptrnet = edifnet_getnet(referlibrary, glibrary, cellname);
			iptrnetportref = edifnet_getportrefs(library, iptrnet, tmpnetportref->instanceref, tmpnetportref->portref);
			if(iptrnetportref != NULL){ 
				edifnet_addtail(iptrnetportref, netportref);
				netportref = iptrnetportref;
				edifinstance_addnames(instancenames, tmpnetportref->instanceref);
			}else{
				blogicalerror = 1;
				fprintf(stdout, "[need check]can not find instance %s's port %s\n", tmpnetportref->instanceref, tmpnetportref->portref);
			}
		}else{
			iptrnetportref = edifnetportref_copy(tmpnetportref); 
			iptrnetportref->next = netportref;
			netportref = iptrnetportref;
		}
	} 
	net = (struct edifnet *)malloc(sizeof(struct edifnet));
	memset(net, 0, sizeof(struct edifnet));
	net->net = strdup(edifnet->net);
	net->edifnetportref = netportref;

	return net;
}

struct edifnet * edifnet_getallinternalnets(struct ediflibrary * library,  struct edifcontents * edifcontents, struct ediflibrary * referlibrary, char ** instancenames){
	struct edifinstance * instance = NULL;
	char * tmpinstance = NULL, * subcircuit = NULL;
	int i = 0;
	struct edifnet * tmpnet, *iptrnet = NULL, * net = NULL;
	for(tmpinstance = instancenames[i]; tmpinstance!=NULL; i++,tmpinstance=instancenames[i] ){ 
		instance = edifinstance_getinstance(edifcontents->edifinstance, tmpinstance);
		subcircuit = edifinstance_getsubcircuitname(instance);
		iptrnet = ediflibrary_getnet(library, referlibrary, glibrary, subcircuit, tmpinstance);
	//	iptrnet = edifnet_getinternalnets(net, tmpinstance);
		if(iptrnet){
			iptrnet = edifnet_add(iptrnet, net);
			net = iptrnet;
		}
		
	//	edifnet_destroy(tmpnet);
	}

	return net;
}

struct edifnet *edifnet_add(struct edifnet * edifnet, struct edifnet * net){ 
	struct edifnet * tmpnet = NULL, *prenet = NULL;
	for (tmpnet = edifnet; tmpnet != NULL; prenet = tmpnet, tmpnet = tmpnet->next);
	if (prenet == NULL) {
		return net;
	}else{
		prenet->next = net;
	}

	return edifnet;
}

struct edifnet * edifnet_flattenex(struct ediflibrary * library, struct edifcontents * edifcontents, struct ediflibrary * referlibrary, struct edifsubcircuit * eidfsubcircuit){
	struct edifnet * originalnet = NULL, * iptrnet = NULL, * net = NULL, * tmpnet = NULL;
	char * instancenames[128];
	char * netnames[128];
	struct edifinstance * instance = NULL;
	memset(instancenames, 0, sizeof(char *) * 128);
	memset(netnames, 0, sizeof(char *) * 128);
	if(edifcontents == NULL){
		return NULL;
	}
	originalnet = edifcontents->edifnet;
	for(tmpnet = originalnet; tmpnet != NULL; tmpnet = tmpnet->next){
		if (edifnet_needflatten(edifcontents, tmpnet)) {
			iptrnet = edifnet_flattensingle(library, tmpnet, edifcontents, referlibrary, instancenames);
			iptrnet->next = net;
			net = iptrnet;
		}else{ 
			iptrnet = edifnet_copy(tmpnet);
			iptrnet->next = net;
			net = iptrnet;
		}
	}
	iptrnet = edifnet_getallinternalnets(library, edifcontents, referlibrary, instancenames);
	if (iptrnet) {
		iptrnet = edifnet_add(iptrnet, net);
		net = iptrnet;
	}

	return net;
}

int edifnet_isinteral(struct edifnet * edifnet){
	struct edifnetportref * tmpportref = NULL;
	if(edifnet != NULL){ 
		for (tmpportref = edifnet->edifnetportref; tmpportref != NULL; tmpportref = tmpportref->next) {
			if (tmpportref->instanceref == NULL) {
				return 0;
			}
		}
	}

	return 1;
}

void edifnet_writer(struct edifnet * edifnet, FILE * out){
	struct edifnet * net = NULL, * nextnet = NULL;
	struct edifnetportref * portref = NULL;
	if(edifnet == NULL || out == NULL){
		fprintf(stderr, "edifnet write error.\n");
		return;
	}
	for(net = edifnet; net != NULL; net = net->next){
		nextnet = net->next;
		gkfputs("   (net ");
		gkfputs(net->net);
		gkfputx;
		gkfputs("    (joined"); 
		for (portref = net->edifnetportref; portref != NULL; portref = portref->next) {
			gkfputs("\n    (portRef ");
			if (gkisdigit(portref->portref)) {
				gkfputs("&");
			}
			gkfputs(portref->portref);
			gkfputs(" ");
			if(portref->instanceref != NULL){
				gkfputs("(instanceRef ");
				gkfputs(portref->instanceref);
				gkfputs(")"); 
			}
			gkfputs(")");
		}
		gkfputs("))");
		if(nextnet != NULL){
			gkfputx;
		}
	}
}

struct edifnet * edifnet_getportnet(struct edifnet * net, char * portname){
	struct edifnet * iptrnet = NULL;
	struct edifnetportref * iptrportref = NULL;
	for(iptrnet = net; iptrnet != NULL; iptrnet = iptrnet->next){
		for(iptrportref = iptrnet->edifnetportref; iptrportref != NULL; iptrportref = iptrportref->next){
			if (strlen(iptrportref->portref)  == strlen(portname) && (stricmp(iptrportref->portref, portname, strlen(portname)) == 0)){
				return iptrnet;
			}
		}
	}
	return NULL;
}

struct edifnetportref * edifnet_getportrefs(struct ediflibrary * library, struct edifnet * net, char * instancename, char * portref){
	struct edifnetportref * netportref = NULL, *iptrnetportref = NULL, * tmpnetportref = NULL;
	char * newinstanceref = NULL;
	int instancerefpart1len = 0;
	int instancerefpart2len = 0;
	char * name = NULL;
	struct edifnet * iptrnet = NULL;
	instancerefpart1len = strlen(instancename);
	iptrnet = edifnet_getportnet(net, portref);
	if(iptrnet != NULL && iptrnet->edifnetportref != NULL){
		for(tmpnetportref = iptrnet->edifnetportref; tmpnetportref != NULL; tmpnetportref = tmpnetportref->next){
			if(tmpnetportref->instanceref != NULL){
				iptrnetportref = (struct edifnetportref *)malloc(sizeof(struct edifnetportref));
				memset(iptrnetportref, 0, sizeof(struct edifnetportref));
				iptrnetportref->portref = strdup(tmpnetportref->portref);
				instancerefpart2len = strlen(tmpnetportref->instanceref);
				newinstanceref = (char *)malloc(instancerefpart1len + instancerefpart2len + 1);
				memset(newinstanceref, 0, instancerefpart1len + instancerefpart2len + 1);
				memcpy(newinstanceref, instancename, instancerefpart1len);
				memcpy(newinstanceref + instancerefpart1len, tmpnetportref->instanceref, instancerefpart2len);
				name = edifinstance_getrealname(library, newinstanceref);
				free(newinstanceref);
				iptrnetportref->instanceref = name;
				iptrnetportref->next = netportref;
				netportref = iptrnetportref;
			}
		}
	}

	return netportref;
}


struct edifnetportref * edifnetportref_copy(struct edifnetportref * portref){ 
	struct edifnetportref * netportref = NULL;
	if(portref){ 
		netportref = (struct edifnetportref *)malloc(sizeof(struct edifnetportref));
		memset(netportref, 0, sizeof(struct edifnetportref));
		netportref->instanceref = strdup(portref->instanceref);
		netportref->portref = strdup(portref->portref);
	}

	return netportref;
}

struct edifnetportref * edifnet_getinternalportrefs(struct edifnet * edifnet, struct edifcell * edifcell){
	struct edifnetportref * edifnetportref = NULL, *iptrnetportref = NULL, * tmpnetportref = NULL;
	if(edifnet != NULL && edifnet->edifnetportref != NULL){
		for (tmpnetportref = edifnet->edifnetportref; tmpnetportref != NULL; tmpnetportref = tmpnetportref->next) {
			if(edifcell_isinteralinstance(edifcell, tmpnetportref->instanceref)){
				iptrnetportref = edifnetportref_copy(tmpnetportref);
				iptrnetportref->next = edifnetportref; 
				edifnetportref = iptrnetportref;
			}
		}
	}

	return edifnetportref;
}

void edifnet_dumplog(struct ediflibrary * library){
	int i,j,spacecount;
	fprintf(stdout, "\n--------------------------net--------------------------------\n");
	fprintf(stdout, "    original name                               new name\n");
	for(i = 0; i < library->netcount; ++i){
		fprintf(stdout, "    %s", library->netnames[i]->originalname);
		spacecount = 40 - strlen(library->netnames[i]->originalname);
		for(j = 0; j < spacecount; ++j)
			fprintf(stdout, " ");
		fprintf(stdout, "    %s", library->netnames[i]->netname);
		spacecount = 25 - strlen(library->netnames[i]->netname);
		for(j = 0; j < spacecount; ++j)
			fprintf(stdout, " ");
		fprintf(stdout, "\n");
	}

	fprintf(stdout, "-------------------------------------------------------------\n");
	fprintf(stdout, "\n");
}

struct edifnet * edifnet_flattenrecursive(struct edifcontents * edifcontents, struct ediflibrary * referlibrary, struct edifsubcircuit * subcircuit){ 
	struct edifcontents * contents = NULL;
	struct edifnet * net = NULL, *iptrnet = NULL;
	struct edifinstance * instance = NULL;
	struct ediflibrary * library = NULL;
	contents = edifcontents_copy(edifcontents);
	library = ediflibrary_create(referlibrary);
	while (!edifinstance_isflat(contents->edifinstance, subcircuit)) {
		instance = edifinstance_flattenonce(library, contents->edifinstance, referlibrary, subcircuit);
		net = edifnet_flattenex(library, contents, referlibrary, subcircuit); 
		edifnet_destroy(contents->edifnet);
		contents->edifnet = net;

		edifinstance_destroy(contents->edifinstance);
		contents->edifinstance = instance;
	}

	edifnet_dumplog(library);
	iptrnet = edifnet_copynets(net);

	edifcontents_destroy(contents);

	return iptrnet;
}

int edifnetport_samepin(struct edifnet * net, struct edifnetportref * port){
	struct edifnetportref * iptrport, * tmpport;
	if(port->instanceref == NULL || port->portref == NULL){
		return 0;
	}

	for (tmpport = net->edifnetportref; tmpport != NULL; tmpport = tmpport->next){ 
		if(tmpport->instanceref == NULL || port->portref == NULL){
			continue;
		}
		if(strlen(tmpport->instanceref) == strlen(port->instanceref) && strcmp(tmpport->instanceref, port->instanceref) == 0 && 
			strlen(tmpport->portref) == strlen(port->portref) && strcmp(tmpport->portref, port->portref) == 0){
				return 1;
		}
	}

	return 0;
}

int edifnet_samepin(struct edifnet * refernet, struct edifnet * testnet){
	struct edifnetportref * iptrport, * tmpport;
	for(tmpport = testnet->edifnetportref; tmpport != NULL; tmpport = tmpport->next){
		if(1 == edifnetport_samepin(refernet, tmpport)){
			return 1;
		}
	}
}

void edifnet_checkpins(struct edifnet * net){
	struct edifnet * iptrnet, * tmpnet, * nextnet;
	struct edifnetportref * iptrport, * tmpport, * nextport;
	int spacecount,i;
	for(tmpnet = net; tmpnet != NULL; tmpnet = tmpnet->next){
		nextnet = tmpnet->next;
		for(iptrnet = nextnet;  iptrnet != NULL; iptrnet = iptrnet->next){
			if(edifnet_samepin(iptrnet, tmpnet) == 1){
				fprintf(stdout, "    %s", tmpnet->net);
				spacecount = 42 - strlen(tmpnet->net);
				for(i = 0; i < spacecount; ++i){
					fprintf(stdout, " ");
				}
				fprintf(stdout, "%s\n", iptrnet->net);
				free(tmpnet->net);
				tmpnet->net = strdup(iptrnet->net);
			}
		}
	}
}

int edifnet_checkglobalnetname(char * netname){ 
	struct edifnet * tmpnet;
	if(HasGlobal){ 
		tmpnet=gs_lib->edifcell->edifcontents->edifnet;
		while(tmpnet){
			if (strcmp(tmpnet->net, netname) == 0){
				return 1;
			}
			tmpnet = tmpnet->next;
		}
	}
	return 0;
}
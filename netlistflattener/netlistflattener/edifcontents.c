#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "edif.h"
#include "edifsubcircuit.h"
#include "edifinstance.h"
#include "edifnet.h"
#include "edifwriter.h"

struct edifcontents * edifcontents_copy(struct edifcontents * edifcontents){
	struct edifcontents * contents = NULL;
	if(edifcontents == NULL){
		fprintf(stderr, "%s error. \n", __FUNCTION__);

		return NULL;
	}
	contents = (struct edifcontents *)malloc(sizeof(struct edifcontents));
	memset(contents, 0, sizeof(struct edifcontents));
	contents->edifinstance = edifinstance_copy(edifcontents->edifinstance);
	contents->edifnet = edifnet_copynets(edifcontents->edifnet);

	return contents;
}

void edifcontents_destroy(struct edifcontents * edifcontents){ 
	if (edifcontents) {
		edifinstance_destroy(edifcontents->edifinstance);
		edifnet_destroy(edifcontents->edifnet);
		free(edifcontents);
	}
}

struct edifcontents * edifcontents_flatten(struct ediflibrary * library, struct edifcontents * edifcontents, struct ediflibrary * referlibrary, struct edifsubcircuit * subcircuit){
	struct edifcontents * con = NULL;
	int i = 0;
	struct edifinstance * instance = NULL, * iptrinstance = NULL, *iptrflatinstance = NULL, *iptrfoldinstance = NULL;
	struct edifnet * net = NULL, *iptrnet = NULL, *iptrflatnet = NULL, *iptrfoldnet = NULL;
	struct edifcontents * contents = NULL, * iptrcontents = NULL;
	if(edifcontents == NULL || referlibrary == NULL || subcircuit == NULL){
		fprintf(stderr, "%s error.\n", __FUNCTION__);
	}
	con = (struct edifcontents *)malloc(sizeof(struct edifcontents));
	memset(con, 0, sizeof(struct edifcontents));

	iptrflatinstance = edifinstance_getflatinstance(edifcontents->edifinstance);
	iptrfoldinstance = edifinstance_getfoldinstance(edifcontents->edifinstance);
	while(iptrfoldinstance != NULL){
		instance = edifinstance_flatten(library, iptrfoldinstance, referlibrary, subcircuit);
		iptrinstance = edifinstance_getflatinstance(instance);
		iptrflatinstance = edifinstance_addtail(iptrflatinstance, iptrinstance);
		edifinstance_destroy(iptrfoldinstance);
		iptrfoldinstance = edifinstance_getfoldinstance(instance);
	}

	con->edifinstance = iptrflatinstance;
	
	con->edifnet = edifnet_flattenex(library, edifcontents, referlibrary, subcircuit);

	return con;
}


void edifcontents_writer(struct edifcontents * edifcontents, FILE * out){
	if(edifcontents == NULL || out == NULL){
		fprintf(stderr, "contents error.\n");
		return;
	} 
	gkfputs("   (contents");
	gkfputx;
	edifinstance_writer(edifcontents->edifinstance, out);
	edifnet_writer(edifcontents->edifnet, out);
	gkfputs(")");
}

struct edifinstance * edifcontents_getinstance(struct edifcontents * edifcontents){
	if(edifcontents != NULL && edifcontents->edifinstance != NULL){ 
		return edifcontents->edifinstance;
	}

	return NULL;
}
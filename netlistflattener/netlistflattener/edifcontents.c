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

struct edifcontents * edifcontents_flatten(struct edifcontents * edifcontents, struct ediflibrary * library, struct edifsubcircuit * subcircuit){
	struct edifcontents * con = NULL;
	int subcircuitcount = 0, i = 0;
	struct edifinstance * instance = NULL, * iptrinstance = NULL;
	struct edifnet * net = NULL, *iptrnet = NULL;
	struct edifcontents * contents = NULL, * iptrcontents = NULL;
	if(edifcontents == NULL || library == NULL || subcircuit == NULL){
		fprintf(stderr, "%s error.\n", __FUNCTION__);
	}
	con = (struct edifcontents *)malloc(sizeof(struct edifcontents));
	memset(con, 0, sizeof(struct edifcontents));

	subcircuitcount = edifsubcircuit_getcount(subcircuit);
	iptrinstance = edifinstance_copy(edifcontents->edifinstance);
	for(i = 0; i < subcircuitcount; ++i){ 
		instance = edifinstance_flatten(iptrinstance, library, subcircuit);
		edifinstance_destroy(iptrinstance);
		iptrinstance = instance;
	}
	con->edifinstance = instance;
	
//	contents = edifcontents_copy(edifcontents);
//	for(i = 0; i < subcircuitcount; ++i){
//		net = edifnet_flattenex(contents, library, subcircuit); 
//		edifnet_destroy(contents->edifnet);
//		contents->edifnet = net;
//	}
//	iptrnet = edifnet_copynets(net);
//	edifcontents_destroy(contents);
//	con->edifnet = iptrnet;
	con->edifnet = edifnet_flattenex(edifcontents, library, subcircuit);

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
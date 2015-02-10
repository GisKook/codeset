#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "edif.h"
#include "edifsubcircuit.h"
#include "edifinstance.h"
#include "edifnet.h"

struct edifcontents * edifcontens_flatten(struct edifcontents * edifcontents, struct ediflibrary * library, struct edifsubcircuit * subcircuit){
	struct edifcontents * con = NULL;
	int subcircuitcount = 0, i = 0;
	struct edifinstance * instance = NULL, * iptrinstance = NULL;
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
	con->edifnet = edifnet_flatten(edifcontents, library, subcircuit);

	return con;
}


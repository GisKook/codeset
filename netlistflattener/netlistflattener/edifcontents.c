#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "edif.h"
#include "edifsubcircuit.h"
#include "edifinstance.h"

struct edifcontents * edifcontens_flatten(struct edifcontents * edifcontents, struct ediflibrary * library, struct edifsubcircuit * subcircuit){
	struct edifcontents * con = NULL;
	if(edifcontents == NULL || library == NULL || subcircuit == NULL){
		fprintf(stderr, "%s error.\n", __FUNCTION__);
	}
	con = (struct edifcontents *)malloc(sizeof(struct edifcontents));
	memset(con, 0, sizeof(struct edifcontents));
	con->edifinstance = edifinstance_flatten(edifcontents->edifinstance, library, subcircuit);
	con->edifnet = NULL; 

	return con;
}


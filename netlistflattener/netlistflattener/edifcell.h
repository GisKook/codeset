#ifndef EDIFCELL_H_H
#define EDIFCELL_H_H

struct edifcell;
struct edifsubcircuit;
struct ediflibrary;
struct edifcontents;
struct edifcell * edifcell_flatten(struct ediflibrary * library, struct edifcell * cell, struct ediflibrary * referlibrary, struct edifsubcircuit * edifsubcircuit);
void edifcell_writer(struct edifcell * cell, FILE * out);
char * edifcell_getsubcellname(struct ediflibrary * ediflibrary, char * libraryname, char * cellname, int index);
int edifcell_getsubcellcount(struct edifcell * edifcell, char * cellname);
struct edifcell * edifcell_getcell(struct edifcell * edifcell, char * cellname);
int edifcell_isinteralinstance(struct edifcell * edifcell, char * instancename); 

#endif
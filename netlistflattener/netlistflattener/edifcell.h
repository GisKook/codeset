#ifndef EDIFCELL_H_H
#define EDIFCELL_H_H

struct edifcell;
struct edifsubcircuit;
struct ediflibrary;
struct edifcell * edifcell_flatten(struct edifcell * cell, struct ediflibrary * library, struct edifsubcircuit * edifsubcircuit);
void edifcell_writer(struct edifcell * cell, FILE * out);
char * edifcell_getsubcellname(struct ediflibrary * ediflibrary, char * libraryname, char * cellname, int index);
int edifcell_getsubcellcount(struct edifcell * edifcell, char * cellname);
struct edifcell * edifcell_getcell(struct edifcell * edifcell, char * cellname);

#endif
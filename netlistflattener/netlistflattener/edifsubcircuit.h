#ifndef EDIFSUBSIRCUIT_H_H
#define EDIFSUBSIRCUIT_H_H

struct edifsubcircuit;
struct edifsubcircuit * edifsubcircuit_create(struct ediflibrary * ediflibrary);
int edifsubcircuit_isreal(struct edifsubcircuit * subcircuit, char * cellname);
int edifsubcircuit_search(struct edifsubcircuit * subcircuit, char * libraryref, char * cellref);
int edifsubcircuit_getcount(struct edifsubcircuit * subcircuit);

#endif
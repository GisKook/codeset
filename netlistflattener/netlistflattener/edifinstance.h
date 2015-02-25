#ifndef EDIFINSTANCE_H_H
#define EDIFINSTANCE_H_H
#include <stdio.h>

struct edifinstance;
struct edifsubcircuit;
struct ediflibrary;
struct edifinstance * edifinstance_flatten(struct ediflibrary * library, struct edifinstance * instance, struct ediflibrary * referlibrary, struct edifsubcircuit * subcircuit); 
struct edifinstance * edifinstance_copy(struct edifinstance * edifinstance);
void edifinstance_destroy(struct edifinstance * edifinstance);
void edifinstance_writer(struct edifinstance * instance, FILE * out);
int edifinstance_needflatten(struct edifinstance * instance, char * instancename);
char * edifinstance_getsubcircuitname(struct edifinstance * instance);
void edifinstance_addnames(char ** instancenames, char * instancename);
struct edifinstance * edifinstance_getinstance(struct edifinstance * instance, char * instancename);
struct edifinstance * edifinstance_getinternalintance(struct ediflibrary * library, struct ediflibrary * referlibrary, char * libraryname, char * cellname, char * szinstance);
char * edifinstance_getrealname(struct ediflibrary * library, char * uidname);
int edifinstance_isflat(struct edifinstance * edifinstance, struct edifsubcircuit * edifsubcircuit);
struct edifinstance * edifinstance_getflatinstance(struct edifinstance * edifinstance);
struct edifinstance * edifinstance_getfoldinstance(struct edifinstance * edifinstance);
struct edifinstance * edifinstance_addtail(struct edifinstance * instance, struct edifinstance * tail);
struct edifinstance * edifinstance_copysingle(struct edifinstance * edifinstance);
struct edifinstance * edifinstance_flattenonce(struct ediflibrary * library, struct edifinstance * edifinstance, struct ediflibrary * referlibrary, struct edifsubcircuit * subcircuit);

#endif

#ifndef EDIFINSTANCE_H_H
#define EDIFINSTANCE_H_H
#include <stdio.h>

struct edifinstance;
struct edifsubcircuit;
struct ediflibrary;
struct edifinstance * edifinstance_flatten(struct edifinstance * instance, struct ediflibrary * library, struct edifsubcircuit * subcircuit); 
struct edifinstance * edifinstance_copy(struct edifinstance * edifinstance);
void edifinstance_destroy(struct edifinstance * edifinstance);
void edifinstance_writer(struct edifinstance * instance, FILE * out);
int edifinstance_needflatten(struct edifinstance * instance, char * instancename);
char * edifinstance_getsubcircuitname(struct edifinstance * instance);
void edifinstance_addnames(char ** instancenames, char * instancename);
struct edifinstance * edifinstance_getinstance(struct edifinstance * instance, char * instancename);

#endif

#ifndef EDIFNET_H_H
#define EDIFNET_H_H
#include <stdio.h>

struct edifnet;
struct ediflibrary;
struct edifsubcircuit;
struct edifnet * edifnet_flatten(struct edifcontents * edifcontents, struct ediflibrary * library, struct edifsubcircuit * eidfsubcircuit);
struct edifnetportref * edifnet_getnetports(struct edifnet * edifnet, char * szportref);
struct edifnet * edifnet_copy(struct edifnet * edifnet);
int edifnet_isinteral(struct edifnet * edifnet);
void edifnet_writer(struct edifnet * net, FILE * out);
struct edifnet * edifnet_copynets(struct edifnet * edifnet);
void * edifnet_destroy(struct edifnet * edifnet);
struct edifnet * edifnet_copyrename(struct ediflibrary * library, struct edifnet * edifnet, char * instance);
struct edifnetportref * edifnet_getportrefs(struct ediflibrary * library, struct edifnet * edifnet, char * instancename, char * portref);
struct edifnet * edifnet_getinternalnets(struct ediflibrary * library, struct edifcontents * edifcontents, char * instancename);
struct edifnet * edifnet_flattenex(struct ediflibrary * library ,struct edifcontents * edifcontents, struct ediflibrary * referlibrary, struct edifsubcircuit * eidfsubcircuit);
struct edifnet * edifnet_flattenrecursive(struct edifcontents * edifcontents, struct ediflibrary * referlibrary, struct edifsubcircuit * subcircuit);

#endif
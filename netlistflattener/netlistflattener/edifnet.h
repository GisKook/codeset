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

#endif
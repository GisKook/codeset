#ifndef EDIFCONTENTS_H_H
#define EDIFCONTENTS_H_H
#include "stdio.h"

struct edifcontents;
struct edifsubcircuit;
struct ediflibrary;
struct edifcontents * edifcontents_flatten(struct edifcontents * edifcontents, struct ediflibrary * library, struct edifsubcircuit * subcircuit);
void edifcontents_writer(struct edifcontents * edifcontents, FILE * out);

#endif
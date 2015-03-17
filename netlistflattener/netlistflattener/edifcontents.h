#ifndef EDIFCONTENTS_H_H
#define EDIFCONTENTS_H_H
#include "stdio.h"

struct edifcontents;
struct edifsubcircuit;
struct ediflibrary;
struct edifinstance;
struct edifcontents * edifcontents_flatten(struct ediflibrary * library, struct edifcontents * edifcontents, struct ediflibrary * referlibrary, struct edifsubcircuit * subcircuit);
void edifcontents_writer(struct edifcontents * edifcontents, FILE * out);
struct edifinstance * edifcontents_getinstance(struct edifcontents * edifcontents);
struct edifcontents * edifcontents_copy(struct edifcontents * edifcontents);
void edifcontents_destroy(struct edifcontents * edifcontents);
struct edifnet * edifcontents_getnets(struct edifcontents * contents);

#endif
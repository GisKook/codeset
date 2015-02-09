#ifndef EDIFCONTENTS_H_H
#define EDIFCONTENTS_H_H

struct edifcontents;
struct edifsubcircuit;
struct ediflibrary;
struct edifcontents * edifcontens_flatten(struct edifcontents * edifcontents, struct ediflibrary * library, struct edifsubcircuit * subcircuit);

#endif
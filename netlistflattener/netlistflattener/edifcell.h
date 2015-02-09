#ifndef EDIFCELL_H_H
#define EDIFCELL_H_H

struct edifcell;
struct edifsubcircuit;
struct ediflibrary;
struct edifcell * edifcell_flatten(struct edifcell * cell, struct ediflibrary * library, struct edifsubcircuit * edifsubcircuit);

#endif
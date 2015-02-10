#ifndef EDIFINSTANCE_H_H
#define EDIFINSTANCE_H_H

struct edifinstance;
struct edifsubcircuit;
struct ediflibrary;
struct edifinstance * edifinstance_flatten(struct edifinstance * instance, struct ediflibrary * library, struct edifsubcircuit * subcircuit); 
struct edifinstance * edifinstance_copy(struct edifinstance * edifinstance);
void edifinstance_destroy(struct edifinstance * edifinstance);

#endif

#ifndef EDIFNET_H_H

struct edifnet;
struct ediflibrary;
struct edifsubcircuit;
struct edifnet * edifnet_flatten(struct edifcontents * edifcontents, struct ediflibrary * library, struct edifsubcircuit * eidfsubcircuit);

#endif
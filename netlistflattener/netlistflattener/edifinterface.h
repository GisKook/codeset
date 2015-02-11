#ifndef EDIFINTERFACE_H_H
#define EDIFINTERFACE_H_H
#include <stdio.h>

struct edifinterface;
struct edifinterfaceport * edifinterface_copy(struct edifinterfaceport * interface);
void edifinterface_writer(struct edifinterfaceport * interface, FILE * out);

#endif
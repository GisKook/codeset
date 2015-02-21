#ifndef EDIFLIBRARY_H_H
#define EDIFLIBRARY_H_H
#include <stdio.h>

struct edifcell;
struct ediflibrary;
struct edifinstance;

struct edifcell * ediflibrary_getcells(struct ediflibrary * ediflibrary); 
struct edifnetportref * ediflibrary_getnetportref(struct ediflibrary * library, char * libraryname, char * cellname, char * portref);
struct edifnet * ediflibrary_getnet(struct ediflibrary * library, struct ediflibrary * referlibrary, char * libraryname, char * cellname, char * instancename);
void ediflibrary_writer(struct ediflibrary * ediflibrary, FILE * out);
struct ediflibrary * ediflibrary_getlibrary(struct ediflibrary * library, char * libraryname);
struct edifcell * ediflibrary_getcell(struct ediflibrary * library, char * cellname);


#endif
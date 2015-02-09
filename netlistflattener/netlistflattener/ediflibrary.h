#ifndef EDIFLIBRARY_H_H
#define EDIFLIBRARY_H_H

struct edifcell;
struct ediflibrary;
struct edifinstance;

struct edifcell * ediflibrary_getcells(struct ediflibrary * ediflibrary); 
struct edifinstance * ediflibrary_getintance(struct ediflibrary * library, char * libraryname, char * cellname);

#endif
#ifndef GISKOOK_EDIF_H_H
#define GISKOOK_EDIF_H_H

struct edifinstance{
	char * instance;
	char * viewref;
	char * cellref;
	char * libraryref;
	struct edifinstance * next;
};

struct edifport{
	char * portref;
	struct edifinstance * instance;
};

struct edifnet{
	char * net;
	struct edifport * edifport;
	unsigned int portcount;
	struct edifnet * next;
};

struct edifcontents{ 
	struct edifinstance * edifinstance;
	struct edifnet * edifnet; 
};

struct edifinterface{
	char * port;
	char * direction;
	struct edifinterface * edifinterface;
};

struct edifcell{
	char * cell;
	char * celltype;
	char * comment; 
	struct edifinterface * edifinterface;
	struct edifcontents * edifcontents;
	struct edifcell * next;
};

#endif
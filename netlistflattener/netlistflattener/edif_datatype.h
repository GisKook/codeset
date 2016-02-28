#ifndef __EDIF_DATATYPE__H_H
#define __EDIF_DATATYPE__H_H
struct edifinstance{
	char * instance;
	char * viewref;
	char * cellref;
	char * libraryref;
	struct edifinstance * next;
};

struct edifnetportref{
	char * portref;
	char * instanceref;
	struct edifnetportref * next;
};

struct edifnet{
	char * net;
	struct edifnetportref * edifnetportref;
	struct edifnet * next;
};

struct edifcontents{ 
	struct edifinstance * edifinstance;
	struct edifnet * edifnet; 
};

#define DIRECTIONBIDI 0
#define DIRECTIONINPUT 1
#define DIRECTIONOUTPUT 2
struct edifinterfaceport{
	char * port;
	int direction;
	struct edifinterfaceport * next;
};

struct edifcell{
	char * cell;
	char * celltype;
	struct edifinterfaceport * edifinterfaceport;
	struct edifcontents * edifcontents;
	struct edifcell * next;
};

struct edifinstancename{
	char * instancename;
	char * originalname;
	char * uidname;
};

struct edifnetnames{
	char * originalname;
	char * netname;
	int binter;
};

struct ediflibrary{
	char * library;
	struct edifcell * edifcell;
	struct ediflibrary * next;
	struct edifinstancename ** usedinstance;
	int instancecount;
	int instancecapacity;
	struct edifnetnames ** netnames;
	int netcount;
	int netcapacity;
};
#endif
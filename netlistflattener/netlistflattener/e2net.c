/*
 * e2net - EDIF to KiCad netlist
 */
#define global

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "ed.h"
#include "eelibsl.h"

int yydebug=0;
int bug=0;  		// debug level: >2 netlist, >5 schematic, >8 all

char *InFile = "-";

char FileNameEdn[64], FileNameSdtLib[64], FileNameEESchema[64], FileNameKiPro[64];
FILE * FileEdf, * FileEdn, * FileEESchema, * FileSdtLib=NULL, * FileKiPro=NULL;

global char                      *cur_nnam=NULL;
global struct inst               *insts=NULL, *iptr=NULL;
global struct con                *cons=NULL,  *cptr=NULL;
global float scale;

main(int argc, char *argv[])
{
	char FileNameEdn[64];
	char *InFile = "-";
	char * version      = "0.96";
	char * progname;
	// bubble sort cons by ref
	progname = strrchr(argv[0],'/');
	if (progname)
		progname++;
	else
		progname = argv[0];

	fprintf(stderr, "*** %s Version %s ***\n", progname, version);

	if(argc != 2){
		return 0;
	}else{
		InFile = argv[1];
		sprintf(FileNameEdn, "%s.edn", argv[1]);
	}
	ParseEDIF(InFile, "stderr", FileNameEdn);
	fprintf(stderr,"Parse Complete\n");
	CloseEDIF();


	fprintf(stderr,"  output is %s \n", FileNameEdn);

	system("pause");
	exit(0);
}

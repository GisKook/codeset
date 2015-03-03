#include "edifheader.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char FileNameEdn[64];
	char *InFile = "-";
	char * version      = "0.98";
	char * progname;
	// bubble sort cons by ref
	progname = strrchr(argv[0],'/');
	if (progname)
		progname++;
	else
		progname = argv[0];

	fprintf(stderr, "*** %s Version %s ***\n", progname, version);

	// if( argc != 2 ) {
	//    fprintf(stderr,  " usage: %s EDIDsrc \n") ; return(1);
	// }

///	if( argc != 2 ){
///		FileEdf = stdin;
///		FileEdn = stdout;
///	}else{
///		InFile= argv[1];
///		sprintf(FileNameEdn,"%s.edn",argv[1]);
///		fprintf(stderr, "Parsing %s\n", InFile);
///		if( (FileEdf = fopen( InFile, "rt" )) == NULL ) {
///			fprintf(stderr, " %s non trouve\n", InFile);
///			return(-1);
///		}
///
///		if( (FileEdn = fopen( FileNameEdn, "w" )) == NULL ) {
///			fprintf(stderr, " %s impossible a creer\n", FileNameEdn);
///			return(-1);
///		}
///	}

	if(argc != 2){
		return 0;
	}else{
		InFile = argv[1];
		sprintf(FileNameEdn, "%s.edn", argv[1]);
	}
	fprintf(stdout, "Parse start...\n");
	ParseEDIF(InFile, "stderr", FileNameEdn);
	fprintf(stderr,"Parse Complete\n");
	CloseEDIF();

	fprintf(stderr,"  output is %s \n", FileNameEdn);

	system("pause");
	exit(0);
}

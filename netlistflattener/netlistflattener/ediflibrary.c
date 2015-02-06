#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "edif.h"

int ediflibrary_getcount(struct ediflibrary * ediflibrary){
	int count = 0;
	struct ediflibrary * lib = NULL;
	if(ediflibrary != NULL){ 
		for (lib = ediflibrary; lib != NULL; lib = ediflibrary->next, count++);
	}

	return count;
}

char ** ediflibrary_getnames(struct ediflibrary * ediflibrary){
	int count = 0;
	struct ediflibrary *lib = NULL;
	char ** libname = NULL;
	int i;
	if(ediflibrary != NULL){
		count = ediflibrary_getcount(ediflibrary);
		libname = (char **)malloc(sizeof(char *)*(count+1));
		for(lib = ediflibrary, i = 0; lib != NULL; lib = ediflibrary->next, ++i){ 
			libname[i] = lib->library;
		}
		libname[i] = NULL;
	}
	
	return libname;
}

struct ediflibrary * ediflibrary_flatten(struct ediflibrary * ediflibrarys){

}

void ediflibrary_writer(struct ediflibrary * ediflibrary, FILE * out){

}
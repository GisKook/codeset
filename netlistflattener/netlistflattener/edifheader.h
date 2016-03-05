#ifndef EDIFHEARDER_H_H
#define EDIFHEARDER_H_H

int ParseEDIF(char * inp, char *err ,char *outp);
int ParseEDIF2(char * inp);
int CloseEDIF();
int IsLogicalerror();
int SetAction(int action);


#endif


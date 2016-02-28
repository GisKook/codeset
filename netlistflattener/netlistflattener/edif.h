#ifndef GISKOOK_EDIF_H_H
#define GISKOOK_EDIF_H_H

#include "edif_datatype.h"
#ifndef global
#define global extern
#endif



#define INSTANCECOUNT 256
#define NETCOUNT 2

#define EDIFAPI __declspec(dllexport)

#define PRINTFUNC fprintf(stdout, "%s\n", __FUNCTION__);

static int strcicmp(char *a, char *b)
{
    for (;; a++, b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
}
#endif
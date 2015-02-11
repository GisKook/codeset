#ifndef EDIFWRITER_H_H
#define EDIFWRITER_H_H
#define gkfputs(x) fputs(x, out); fflush(out)
#define gkfputx gkfputs("\n")
#define gkfputy gkfputs(")\n")
#define gkfputz gkfputs("  (view NetlistView\n   (viewType netlist)\n")

static int gkisdigit(char * sz){
	char * tmp = sz;
	while(*tmp){
		if(!isdigit(*tmp))
			return 0;
		tmp++;
	}
	return sz != NULL?1:0;
}

#endif
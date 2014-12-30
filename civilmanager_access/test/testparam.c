#include <stdio.h>
#include <stdlib.h>

struct testparam{
	char testparam[21];
};

//typedef unsigned char (&test)[40];
void testparam( unsigned char (&test)[40]){
//void testparam( test test){
	fprintf(stdout, "the param's size is: %d\n", sizeof(test));
	struct testparam testparam;
	fprintf(stdout, "the struct's size is: %d\n", sizeof(testparam.testparam));
}

int main(){
	unsigned char test[40];
	testparam(test);
	return 0;
}

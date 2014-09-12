
#include <unistd.h>
#include <stdio.h>

int main(){
//	int i;
//	for(i = 0; i< 100; ++i){
//		if(i == 30){
//			break;
//		}
//		printf("%d\n",i);
//	}
	printf("_SC_P_S %d \n", sysconf(_SC_PAGE_SIZE)); 
	return 0;
	
}

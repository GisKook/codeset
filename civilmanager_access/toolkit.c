#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>

void toolkit_printbytes(unsigned char* buf, unsigned int len){
	unsigned int i;
	for (i = 0; i < len; i++)
	{
		fprintf(stdout,"%02X", buf[i]);
	}
	fprintf(stdout,"\n");
}

const char* byteliterals[] = {"00","01","02","03","04","05","06","07","08","09","0A","0B","0C","0D","0E","0F","10","11","12","13","14","15","16","17","18","19","1A","1B","1C","1D","1E","1F","20","21","22","23","24","25","26","27","28","29","2A","2B","2C","2D","2E","2F","30","31","32","33","34","35","36","37","38","39","3A","3B","3C","3D","3E","3F","40","41","42","43","44","45","46","47","48","49","4A","4B","4C","4D","4E","4F","50","51","52","53","54","55","56","57","58","59","5A","5B","5C","5D","5E","5F","60","61","62","63","64","65","66","67","68","69","6A","6B","6C","6D","6E","6F","70","71","72","73","74","75","76","77","78","79","7A","7B","7C","7D","7E","7F","80","81","82","83","84","85","86","87","88","89","8A","8B","8C","8D","8E","8F","90","91","92","93","94","95","96","97","98","99","9A","9B","9C","9D","9E","9F","A0","A1","A2","A3","A4","A5","A6","A7","A8","A9","AA","AB","AC","AD","AE","AF","B0","B1","B2","B3","B4","B5","B6","B7","B8","B9","BA","BB","BC","BD","BE","BF","C0","C1","C2","C3","C4","C5","C6","C7","C8","C9","CA","CB","CC","CD","CE","CF","D0","D1","D2","D3","D4","D5","D6","D7","D8","D9","DA","DB","DC","DD","DE","DF","E0","E1","E2","E3","E4","E5","E6","E7","E8","E9","EA","EB","EC","ED","EE","EF","F0","F1","F2","F3","F4","F5","F6","F7","F8","F9","FA","FB","FC","FD","FE","FF"};

void hex2char(char* charbuf, unsigned char* hexbuf, unsigned int len){
	unsigned int i = 0, j = 0;
	for( ;i < len; ++i,j+=2){ 
		charbuf[j] = byteliterals[*(hexbuf+i)][0];
		charbuf[j+1] = byteliterals[*(hexbuf+i)][1];
	}
}

unsigned char * toolkit_cmdsep(unsigned char **cmdp, unsigned int *totallen, unsigned int * len, unsigned char delim){
	unsigned char * s, *tok;
	if(*totallen == 0)
		return NULL;

	s = *cmdp;
	while(*s++ != delim){
		(*totallen)--;
		if((*totallen) == 0){
			return NULL;
		}
	}
	*cmdp = --s;

	for( tok = s;;){
		if(*s++ == delim){
			for( ; *totallen>0;){
				(*len)++;
				(*totallen)--;
				if(*s++ == delim){ 
					*cmdp = --s;
					break;
				}
			}
			return tok; 
		}

		*len = 0;

		return NULL;
	}
}

char* toolkit_strsep(char** stringp, char delim){
	char*s,*tok;
	if(((s=*stringp)==NULL))
		return NULL;
	for (tok=s;;){
		if (*s++==delim ) {
			s[-1]=0;
			*stringp = s;
			return tok;
		}
		if(*s==0){*stringp=tok; return NULL;}
	}
}

void
toolkit_backtrace(void)
{
	int j, nptrs;
#define SIZE 100
	void *buffer[100];
	char **strings;

	nptrs = backtrace(buffer, SIZE);
	printf("backtrace() returned %d addresses\n", nptrs);

	/* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
	 *        would produce similar output to the following: */

	strings = backtrace_symbols(buffer, nptrs);
	if (strings == NULL) {
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}

	for (j = 0; j < nptrs; j++)
		printf("%s\n", strings[j]);

	free(strings);
}

//int main(){
//	unsigned char * tmp = (unsigned char*)malloc(512);
//	unsigned int totallen = 100;
//	unsigned int len = 0;
//
//	tmp[3] = '$';
//	tmp[25] = '$';
//	tmp[50] = '$';
//	tmp[60] = '$';
//	unsigned char * res ;
//	while( totallen != 0){
//		//res = parseprotocol_pickcmd(&tmp, &totallen, &len);
//		toolkit_printbytes(res, len);
//		len = 0;
//	}
//	
//	return 0; 
//}

/*
 * brief: 工具类。
 * function list:
 * 
 * author: zhangkai
 * date: 2014年8月22日
 */
#ifndef CNTOOLKIT_H_H
#define CNTOOLKIT_H_H


void debug_printbytes(unsigned char* buf, unsigned int len);

void hex2char(char* charbuf, unsigned char* hexbuf, unsigned int len);

unsigned char * toolkit_cmdsep(unsigned char **cmd, unsigned int * totallen, unsigned int * len, unsigned char delim);
char * toolkit_strsep(char** stringp, char delim);

#define TOOLKIT_BYTE2INT(x) ((*x*256*256*256)+(*(x+1)*256*256)+(*(x+2)*256)+(*(x+3)))
#define TOOLKIT_BYTE2SHORT(x) ((*x*256)+(*(x+1)))


#endif

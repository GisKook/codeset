/*
 * brief: 定义一些常用的宏全局函数
 * function list:
 * 
 * author: a staff of CETCNAV
 * date: 2014年8月5日
 */
#ifndef CNDEF_H_H
#define CNDEF_H_H

#if defined __GNUC__
#define likely(x) __builtin_expect ((x),1)
#define unlikely(x) __builtin_expect((x),0)
#else
#define likely(x)
#define unlikely(x)
#endif

#define MIN(a,b) a>b?b:a

#define BUSINESSSOFTWARE "business"
#define RECVSOFTWARE "北斗信息接收"
#define SENDSOFTWARE "send"
#define BILLINGSOFTWARE "bill"


#define MAXENTERPRISEIDLEN 33
#define MAXLOGINLEN 33
#define MAXLOGINNAMELEN 100
#define MAXLOGINNAMELEN 100
#define MAXPASSWORDLEN 33

#endif

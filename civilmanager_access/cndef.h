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


#define BUSINESSSOFTWARE "业务服务器"
#define RECVSOFTWARE "接收软件"
#define SENDSOFTWARE "发送软件"
#define BILLINGSOFTWARE "计费服务器"
#endif

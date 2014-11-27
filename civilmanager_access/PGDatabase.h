/*
 * brief: libpq.lib的wrapper,数据库级别的操作
 * function list:
 * 
 * author: a staff of CETCNAV
 * date: 2014年5月7日
 */
#ifndef GKDATABASEPG_H_H
#define GKDATABASEPG_H_H
#include "PGRecordset.h"

typedef struct{
	char* pghost;
	char* pgport;
	char* dbName;
	char* login;
	char* passwd; 
}PGConnInfo;

struct pg_conn;
typedef struct pg_conn PGconn;

#define MAXTABLENAMELEN 256
#define MAXOPVALUELEN 1024
struct pgdb_monitor_result{
	char tablename[MAXTABLENAMELEN];
	char opvalues[MAXOPVALUELEN]; 
};

typedef void (*pgdb_monitor_callback)(struct pgdb_monitor_result *, void*);

class PGDatabase{
public:
	// brief 连接数据库
	// param[in] 连接内容
	int Connect(const PGConnInfo& dbConnInfo); 

	// 断开数据库连接
	int DisConnect(); 

	// brief 执行Select操作 异步操作
	// param[in] sql 要被执行的sql
	//bool SendQuery(const char* strQuery);

	// brief 收集由SendQuery发送的命令的结果
	//PGRecordset* GetRecordset();

	// brief 执行阻塞的insert/update操作
	bool Exec(const char* strSQL);

	// brief 执行阻塞的select 操作
	PGRecordset* Query(const char* strSQL);

	// brief 开启事务
	bool BeginTransaction();

	// brief 结束事务
	bool Commit();

	// brief 对表添加监听
	bool AddListener(const char* strTablename); 

	// brief 对表删除监听
	bool RemoveListener(const char* strTablename);

	// brief 得到修改的数据
	void GetNotify(pgdb_monitor_callback pmc, void*);
	
private:
	PGconn* m_pConnect;
	
};


#endif
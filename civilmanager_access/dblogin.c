#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cndef.h"
#include "toolkit.h"
#include "loginmanager.h"
#include "cnconfig.h"
#include "PGDatabase.h"

char tablename[] = "qhsrvaccount";
char sqltable[] = "select qtsserverid, qtsentId, qtsloginname, qtspassword, qtssendfreq from qhsrvaccount order by qtsentId";
#define INITLOGINCOUNT 5
#define MAXATTRCOUNT 128

struct dblogin{
	PGDatabase *db; 
	struct loginmanager *manager;
	pthread_t tidmonitor;
};

void * pgdbmonitorcallback(void *par, void* par2){
	//fprintf(stderr, "%s %s.\n", pmr->tablename, pmr->opvalues);
	//qhsrvaccount 
	//pmr->tablename 不解释
	//pmr->opvalues I^电科导航^电科导航^dkdh11^dkdh11^1^2004-10-19 10:23:54 . 
	//内容解析 所有属性由空格隔开 I(insert) U(update) D(delete) X(unknow) 如果得到的属性为空用$替代
	struct pgdb_monitor_result *pmr = (struct pgdb_monitor_result *)par;
	struct loginmanager *manager = (struct loginmanager *)par2;
	char * attr[MAXATTRCOUNT] = {0};
	int i = 0;
	char* tmp = (char*)pmr->opvalues;
	while((attr[i] = toolkit_strsep(&tmp, '^')) != NULL){
		++i;
	}
	attr[i] = tmp;
	char opcode = *attr[0];
	char * login = attr[1];
	char * enterpriseid = attr[2];
	char * loginname = attr[3];
	char * password = attr[4];
	char * issuedfrequency = attr[5];

	struct login *data;
	switch(opcode){
		case 'I':
			data = (struct login*)malloc(sizeof(struct login));
			memcpy(data->login, login, MIN(MAXLOGINLEN, strlen(login)));
			memcpy(data->enterpriseid, enterpriseid, MIN(MAXENTERPRISEIDLEN, strlen(enterpriseid)));
			memcpy(data->loginname, loginname, MIN(MAXLOGINNAMELEN, strlen(loginname)));
			memcpy(data->password, password, MIN(MAXPASSWORDLEN, strlen(password)));
			data->issuedfrequency = atoi(issuedfrequency);
			loginmanager_insert(manager, data); 
			break;
		case 'U':
			data = loginmanager_search(manager, login); 
			if(data = NULL){
				data = (struct login*)malloc(sizeof(struct login));
				memcpy(data->login, login, MIN(MAXLOGINLEN, strlen(login)));
				memcpy(data->enterpriseid, enterpriseid, MIN(MAXENTERPRISEIDLEN, strlen(enterpriseid)));
				memcpy(data->loginname, loginname, MIN(MAXLOGINNAMELEN, strlen(loginname)));
				memcpy(data->password, password, MIN(MAXPASSWORDLEN, strlen(password)));
				data->issuedfrequency = atoi(issuedfrequency);
			}else{
				memcpy(data->login, login, MIN(MAXLOGINLEN, strlen(login)));
				memcpy(data->enterpriseid, enterpriseid, MIN(MAXENTERPRISEIDLEN, strlen(enterpriseid)));
				memcpy(data->loginname, loginname, MIN(MAXLOGINNAMELEN, strlen(loginname)));
				memcpy(data->password, password, MIN(MAXPASSWORDLEN, strlen(password)));
				data->issuedfrequency = atoi(issuedfrequency);
			}
			break;
		case 'D':
			data = loginmanager_delete(manager, login);
			if(data != NULL){
				free(data);
			}
			break;
		default:
			assert(0);
			break;
	}

	//loginmanager_print(manager);
}

void * dblogin_monitor(void * dblogin){ 
	struct dblogin * dbe = (struct dblogin*)dblogin;
	PGDatabase *db = dbe->db;
	struct loginmanager *manager = dbe->manager;
	db->AddListener(tablename);
	db->GetNotify((pgdb_monitor_callback)pgdbmonitorcallback, manager);
};

struct dblogin * dblogin_start(struct loginmanager * manager){ 
	PGConnInfo conn;
	conn.pghost = (char*)(cnconfig_getvalue(DBHOST));
	conn.pgport = (char*)(cnconfig_getvalue(DBPORT));
	conn.dbName = (char*)(cnconfig_getvalue(DBNAME));
	conn.login =  (char*)(cnconfig_getvalue(DBUSER));
	conn.passwd = (char*)(cnconfig_getvalue(DBPWD));

	PGDatabase * db = new PGDatabase;
	if(unlikely(0 != db->Connect(conn))){
		fprintf(stderr, "conn database error. host: %s port: %s dbname: %s dblogin: %s dbpassword: %s %s %s %d\n.", conn.pghost, conn.pgport, conn.dbName, conn.login, conn.passwd, __FILE__, __FUNCTION__, __LINE__);

		return NULL;
	}

	PGRecordset *res = db->Query(sqltable); 
	int tuplecount = res->GetTupleCount();
	int i;
	char preenterpriseid[MAXENTERPRISEIDLEN] ={0};
	char * login = NULL;
	char * enterpriseid = NULL;
	char * loginname = NULL;
	char * password = NULL;
	char * issuedfrequency = NULL;
	struct login *logindata = NULL;
	for (i = 0;i < tuplecount; ++i){
		login = res->GetValue(i, 0);
		enterpriseid = res->GetValue(i,1); 
		loginname = res->GetValue(i,2);
		password = res->GetValue(i, 3);
		issuedfrequency = res->GetValue(i,4);
		logindata = (struct login *)malloc(sizeof(struct login));
		memcpy(logindata->login, login, MIN(MAXLOGINLEN, strlen(login)));
		memcpy(logindata->enterpriseid, enterpriseid, MIN(MAXENTERPRISEIDLEN, strlen(enterpriseid)));
		memcpy(logindata->loginname, loginname, MIN(MAXLOGINNAMELEN, strlen(loginname)));
		memcpy(logindata->password, password, MIN(MAXPASSWORDLEN, strlen(password)));
		logindata->issuedfrequency = atoi(issuedfrequency);
		loginmanager_insert(manager, logindata);
	}
	
	fprintf(stdout, "table %s load ok.\n", tablename); 
	res->Destroy();

	struct dblogin * dblogin = (struct dblogin*)malloc(sizeof(struct dblogin));
	dblogin->db = db;
	dblogin->manager = manager;
	if( 0 != pthread_create(&dblogin->tidmonitor, NULL, dblogin_monitor,dblogin)){
		fprintf(stderr, "create dblogin monitor error. %s %s %d.\n", __FILE__, __FUNCTION__, __LINE__); 
		db->DisConnect();
		free(db);
		free(dblogin);

		return NULL;
	}

	//loginmanager_print(manager);
	return dblogin;
}

void dblogin_end(struct dblogin* dbe){ 
	dbe->db->DisConnect();
	pthread_join(dbe->tidmonitor, NULL);
	free(dbe);
}

//int main(){
//	cnconfig_loadfile("./conf.json");
//	struct loginmanager * em = loginmanager_create();
//	struct dblogin *dbe = dblogin_start(em);
//	pthread_join(dbe->tidmonitor, NULL);
//
//	return 0;
//}

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cndef.h"
#include "toolkit.h"
#include "enterprisemanager.h"
#include "cnconfig.h"
#include "PGDatabase.h"

char tablename[] = "qhsrvaccount";
char sqltable[] = "select qtsentId, qtsloginname, qtspassword, qtssendfreq from qhsrvaccount order by qtsentId";
#define INITLOGINCOUNT 5

struct dbenterprise{
	PGDatabase *db; 
	struct enterprisemanager *manager;
	pthread_t tidmonitor;
};

void * pgdbmonitorcallback(void *par, void* par2){
	//fprintf(stderr, "%s %s.\n", pmr->tablename, pmr->opvalues);
	//qhsrvaccount 
	//pmr->tablename 不解释
	//pmr->opvalues I 电科导航 电科导航 dkdh11 dkdh11 1 2004-10-19 10:23:54 . 
	//内容解析 所有属性由空格隔开 I(insert) U(update) D(delete) X(unknow) 如果得到的属性为空用$替代
	struct pgdb_monitor_result *pmr = (struct pgdb_monitor_result *)par;
	struct enterprisemanager *manager = (struct enterprisemanager *)par2;
	char opcode;
	char * value;
	while((value = toolkit_strsep(pmr->opvalues, ' ')) != NULL){ 
		
	}
}

void * dbenterprise_monitor(void * dbenterprise){ 
	struct dbenterprise * dbe = (struct dbenterprise*)dbenterprise;
	PGDatabase *db = dbe->db;
	struct enterprisemanager *manager = dbe->manager;
	db->AddListener(tablename);
	db->GetNotify((pgdb_monitor_callback)pgdbmonitorcallback, manager);
};

struct dbenterprise * dbenterprise_start(struct enterprisemanager * manager){ 
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
	char * enterpriseid = NULL;
	char * login = NULL;
	char * password = NULL;
	char * issuedfrequency = NULL;
	struct enterprise * enterprise = NULL;
	bool isequal = false;
	if(tuplecount > 0){
		enterpriseid = res->GetValue(0,0);
		login = res->GetValue(0, 1);
		password = res->GetValue(0, 2);
		issuedfrequency = res->GetValue(0,3);

		memcpy(preenterpriseid, enterpriseid, strlen(enterpriseid));
		enterprise = enterprise_create(enterpriseid, INITLOGINCOUNT);
		enterprise_addaccount(enterprise, login, password, atoi(issuedfrequency));
	}
	for (i = 1;i < tuplecount; ++i){
		enterpriseid = res->GetValue(i,0); 
		login = res->GetValue(i, 1);
		password = res->GetValue(i, 2);
		issuedfrequency = res->GetValue(i,3);
		isequal = (strlen(preenterpriseid) == strlen(enterpriseid)) && (0 == strcmp(preenterpriseid, enterpriseid));
		if(!isequal){
			enterprisemanager_insert(manager, enterprise);
			enterprise = enterprise_create(enterpriseid, INITLOGINCOUNT);
		}else{
			enterprise_addaccount(enterprise, login, password, atoi(issuedfrequency));
		}
		memset(preenterpriseid, 0, MAXENTERPRISEIDLEN);
		memcpy(preenterpriseid, enterpriseid, strlen(enterpriseid));
	}
	
	if(enterprise != NULL){
		enterprisemanager_insert(manager, enterprise);
	}

	fprintf(stdout, "table %s load ok.\n", tablename); 
	res->Destroy();

	struct dbenterprise * dbenterprise = (struct dbenterprise*)malloc(sizeof(struct dbenterprise));
	dbenterprise->db = db;
	dbenterprise->manager = manager;
	if( 0 != pthread_create(&dbenterprise->tidmonitor, NULL, dbenterprise_monitor,dbenterprise)){
		fprintf(stderr, "create dbenterprise monitor error. %s %s %d.\n", __FILE__, __FUNCTION__, __LINE__); 
		db->DisConnect();
		free(db);
		free(dbenterprise);

		return NULL;
	}

	return dbenterprise;
}

void dbenterprise_end(struct dbenterprise* dbe){ 
	dbe->db->DisConnect();
	pthread_join(dbe->tidmonitor, NULL);
	free(dbe);
}

int main(){
	cnconfig_loadfile("./conf.json");
	struct enterprisemanager * em = enterprisemanager_create();
	struct dbenterprise *dbe = dbenterprise_start(em);
	pthread_join(dbe->tidmonitor, NULL);

	return 0;
}

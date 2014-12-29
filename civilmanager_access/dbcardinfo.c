// 监听数据库的变化，动态修改内存，服务cardmanager模块
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "PGDatabase.h"
#include "PGRecordset.h"
#include "cardmanager.h"
#include "toolkit.h"
#include "cnconfig.h"
#include "cndef.h"

char tablename[] = "qhcardinfo";
char sqltable[] = "select qtsentId,qtscardnumber from qhcardinfo";

#define MAXATTRCOUNT 128

struct dbcardinfo{
	PGDatabase * db;
	struct cardmanager *cardmanager;
	pthread_t tidmonitor;
};

void * pgdbmonitorcardcallback(void *par, void *par2){
	//pmr->tablename 不解释
	//pmr->opvalues I^电科导航^电科导航^dkdh11^dkdh11^1^2004-10-19 10:23:54 . 
	//内容解析 所有属性由空格隔开 I(insert) U(update) D(delete) X(unknow) 如果得到的属性为空用$替代
	struct pgdb_monitor_result *pgdb_monitor_result = (struct pgdb_monitor_result *)par;
	struct cardmanager *cardmanager = (struct cardmanager *)par2;
	char * attr[MAXATTRCOUNT] = {0};
	int i = 0;
	char * tmp = (char *)pgdb_monitor_result->opvalues;
	while((attr[i] = toolkit_strsep(&tmp, '^')) != NULL){
		++i;
	}
	attr[i] = tmp;
	char opcode = *attr[0];
	char * enterpriseid = attr[2];
	char * szcardnumber = attr[3];

	struct card * card;
	switch(opcode){
		case 'I':
			cardmanager_insert(cardmanager, atoi(szcardnumber), enterpriseid);
			break;
		case 'U':
			card = cardmanager_search(cardmanager, atoi(szcardnumber));
			if(card != NULL){
				card_setenterpriseid(card, enterpriseid);
			}
			break;
		case 'D':
			card = cardmanager_delete(cardmanager, atoi(szcardnumber));
			free(card);
			break;
		default:
			fprintf(stderr, "recv a unknow message from datebase. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			break;
	}
	cardmanager_print(cardmanager);

	return NULL;
}

void * dbcardinfo_monitor(void * param){ 
	struct dbcardinfo * cardinfo = (struct dbcardinfo *)param;
	PGDatabase *db = cardinfo->db;
	struct cardmanager *manager = cardinfo->cardmanager;
	db->AddListener(tablename);
	db->GetNotify((pgdb_monitor_callback)pgdbmonitorcardcallback, manager);

	return NULL;
}

struct dbcardinfo * dbcardinfo_start(struct cardmanager * cardmanager){
	PGConnInfo conn;
	conn.pghost = (char*)(cnconfig_getvalue(DBHOST));
	conn.pgport = (char*)(cnconfig_getvalue(DBPORT));
	conn.dbName = (char*)(cnconfig_getvalue(DBNAME));
	conn.login =  (char*)(cnconfig_getvalue(DBUSER));
	conn.passwd = (char*)(cnconfig_getvalue(DBPWD));

	PGDatabase *db = new PGDatabase;
	if(unlikely(0 != db->Connect(conn))){
		fprintf(stderr, "conn database error. host: %s dbname: %s dblogin: %s dbpassword %s %s %s %d\n.", conn.pghost, conn.dbName, conn.login, conn.passwd, __FILE__, __FUNCTION__, __LINE__);

		return NULL;
	}

	PGRecordset *res = db->Query(sqltable);
	int tuplecount  = res->GetTupleCount();
	int i;
	char * enterpriseid;
	unsigned int cardid;
	for(i = 0; i < tuplecount; ++i){ 
		enterpriseid = res->GetValue(i, 0);
		cardid = atoi(res->GetValue(i, 1));
		cardmanager_insert(cardmanager, cardid, enterpriseid);
	}

	fprintf(stdout, "****table %s load ok.\n", tablename);
	res->Destroy();

	struct dbcardinfo * dbcardinfo = (struct dbcardinfo *)malloc(sizeof(struct dbcardinfo));
	dbcardinfo->db = db;
	dbcardinfo->cardmanager = cardmanager;
	if( 0 != pthread_create(&dbcardinfo->tidmonitor, NULL, dbcardinfo_monitor, dbcardinfo)){
		fprintf(stderr, "create dbcardinfo monitor error. %s %s %d.\n", __FILE__, __FUNCTION__, __LINE__);
		db->DisConnect();
		delete db;
		free(dbcardinfo);

		return NULL;
	}
	fprintf(stdout, "thread 0x%lx monitor table qhcardinfo create sucessfully.\n", dbcardinfo->tidmonitor);

	return dbcardinfo;
}

void dbcardinfo_end(struct dbcardinfo * dbcardinfo){
	dbcardinfo->db->DisConnect();
	pthread_join(dbcardinfo->tidmonitor, NULL);
	free(dbcardinfo);
}

//int main(){
//	cnconfig_loadfile("./conf.json");
//	struct cardmanager * cardmanager = cardmanager_create();
//	struct dbcardinfo * dbcardinfo = dbcardinfo_start(cardmanager);
//	pthread_join(dbcardinfo->tidmonitor, NULL);
//
//	return 0;
//}

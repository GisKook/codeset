#include "libpq-fe.h"
#include "PGDatabase.h" 
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <WinSock2.h>
#endif

#ifdef __linux__
#include <errno.h>
#endif

int PGDatabase::Connect( const PGConnInfo& dbConnInfo ) {
	m_pConnect= PQsetdbLogin(dbConnInfo.pghost, dbConnInfo.pgport,
		"", "", dbConnInfo.dbName, 
		dbConnInfo.login, dbConnInfo.passwd);
	if (PQstatus(m_pConnect) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
			PQerrorMessage(m_pConnect));
	    PQfinish(m_pConnect);

		return 1;
	}

	return 0;
}

int PGDatabase::DisConnect() {
	PQfinish(m_pConnect); 
	m_pConnect= NULL;

	return 0;
}

//bool PGDatabase::SendQuery( const char* strQuery){ 
//	int retval = PQsendQuery(m_pConnect, strQuery);
//	if (retval == 0) {
//		fprintf(stderr, "Send Query to database failed: %s",
//			PQerrorMessage(m_pConnect));
//	}
//	return retval != 0;
//}

//PGRecordset* PGDatabase::GetRecordset() { 
//	PGresult* pResult= PQgetResult(m_pConnect);
//
//	PGRecordset *pRecord = NULL;
//	if (pResult != NULL) {
//		pRecord = new PGRecordset;
//		pRecord->Create(pResult);
//	}
//
//	return pRecord;
//}

bool PGDatabase::BeginTransaction() {
	if (m_pConnect == NULL) {
		return false;
	}
	PGresult* res = PQexec(m_pConnect, "begin transaction");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "BEGIN command failed: %s\n", PQerrorMessage(m_pConnect));
		PQclear(res);
		
		return false;
	}
	PQclear(res);
	return true;
}

bool PGDatabase::Commit() {
	if (m_pConnect == NULL) {
		return false;
	}
	PGresult* res = PQexec(m_pConnect, "commit"); 
	if(PQresultStatus(res) != PGRES_COMMAND_OK){
		fprintf(stderr, "END command failed: %s\n", PQerrorMessage(m_pConnect));
		PQclear(res);

		return false;
	}
	PQclear(res);

	return true;
}

bool PGDatabase::Exec( const char* strSQL ) {
	if (m_pConnect == NULL) {
		return false;
	}
	assert(m_pConnect != NULL); 
	PGresult* res = PQexec(m_pConnect, strSQL);
	if (PQresultStatus(res) != PGRES_COMMAND_OK ) {
		fprintf(stderr,"%s failed!", strSQL);
		fprintf(stderr, " %s\n", PQerrorMessage(m_pConnect));
		PQclear(res);

		return false;
	} 

	PQclear(res);
	return true;
}

PGRecordset* PGDatabase::Query( const char* strSQL ) {
	if (m_pConnect == NULL) {
		return NULL;
	}
	assert(m_pConnect != NULL);
	PGresult* res = PQexec(m_pConnect, strSQL);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr,"%s exec failed!\n", strSQL);

		PQclear(res);
		return NULL;
	} 

	PGRecordset* pRecordset = new PGRecordset;
	pRecordset->Create(res);

	return pRecordset;
}

bool PGDatabase::AddListener( const char* strTablename )
{
	char listentable[256] = {0};
	memcpy(listentable, "LISTEN ", sizeof("LISTEN ")-1);
	memcpy(listentable + sizeof("LISTEN ")-1, strTablename, strlen(strTablename));
	PGresult *res = PQexec(m_pConnect, listentable);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "LISTEN command failed: %s\n", PQerrorMessage(m_pConnect));
        PQclear(res);
		return false;
    }

    /*
     * should PQclear PGresult whenever it is no longer needed to avoid memory
     * leaks
     */
    PQclear(res);

	return true;
}

bool PGDatabase::RemoveListener( const char* strTablename )
{
	char listentable[256] = {0};
	memcpy(listentable, "UNLISTEN ", sizeof("UNLISTEN "));
	memcpy(listentable + sizeof("UNLISTEN "), strTablename, strlen(strTablename));
	PGresult *res = PQexec(m_pConnect, listentable);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "UNLISTEN command failed: %s\n", PQerrorMessage(m_pConnect));
        PQclear(res);
		return false;
    }
	
    /*
     * should PQclear PGresult whenever it is no longer needed to avoid memory
     * leaks
     */
    PQclear(res);

	return true;
}

void PGDatabase::GetNotify(pgdb_monitor_callback pmc, void* param)
{
	PGnotify   *notify;
	struct pgdb_monitor_result pmr;
    while (true)
    {
        /*
         * Sleep until something happens on the connection.  We use select(2)
         * to wait for input, but you could also use poll() or similar
         * facilities.
         */
        int         sock;
        fd_set      input_mask;

        sock = PQsocket(m_pConnect);

        if (sock < 0)
            break;              /* shouldn't happen */

        FD_ZERO(&input_mask);
        FD_SET(sock, &input_mask);

        if (select(sock + 1, &input_mask, NULL, NULL, NULL) < 0)
        {
            fprintf(stderr, "select() failed: %s\n", strerror(errno));
        }

        /* Now check for input */
        PQconsumeInput(m_pConnect);
        while ((notify = PQnotifies(m_pConnect)) != NULL)
        {
			memset(pmr.tablename, 0 , MAXTABLENAMELEN);
			memset(pmr.opvalues, 0 , MAXOPVALUELEN);
			memcpy(pmr.tablename, notify->relname, strlen(notify->relname));
			memcpy(pmr.opvalues, notify->extra, strlen(notify->extra));
			pmc(&pmr,param);

            PQfreemem(notify);
        }
    }
}

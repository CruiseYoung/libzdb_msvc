/*
* Copyright (C) Tildeslash Ltd. All rights reserved.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 3.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "Config.h"

#include <stdio.h>


#include "URL.h"
#include "ResultSet.h"
#include "StringBuffer.h"
#include "system/Time.h"
#include "PreparedStatement.h"
#include "SqlServerResultSet.h"
#include "SqlServerPreparedStatement.h"
#include "ConnectionDelegate.h"
#include "SqlServerConnection.h"


/**
* Implementation of the Connection/Delegate interface for SqlServer 
*
* @file
*/


/* ----------------------------------------------------------- Definitions */


#define T ConnectionDelegate_T
struct T {
	URL_T url;
	SqlServer_T db;
	int maxRows;
	int timeout;
	int lastError;
	StringBuffer_T sb;
		StringBuffer_T err;
};

extern const struct Rop_T sqlserverrops;
extern const struct Pop_T sqlserverpops;


/* ------------------------------------------------------- Private methods */
static void getSqlErr(T C,SQLHSTMT hstmt) {
	unsigned char szSQLSTATE[10];
	SDWORD nErr;
	unsigned char msg[SQL_MAX_MESSAGE_LENGTH+1];
	SWORD cbmsg;

	unsigned char szData[256];   // Returned data storage

	while(SQLError(0,0,hstmt,szSQLSTATE,&nErr,msg,sizeof(msg),&cbmsg)==
		SQL_SUCCESS)
	{
        snprintf((char *)szData, sizeof(szData), "Error:\nSQLSTATE=%s,Native error=%ld,msg='%s'", szSQLSTATE, nErr, msg);
		//MessageBox(NULL,(const char *)szData,"ODBC Error",MB_OK);
		//return NULL;
		//return strdup(szData);
	}
	StringBuffer_append(C->err,"%s",szData);

}

/* SqlServer3 client library finalization */
static void onstop(void) {
	//SqlServer3_close();
	// SqlServer3_shutdown();
}



static SqlServer_T doConnect(URL_T url, char **error) {
	RETCODE retcode;
	SqlServer_T db = (SqlServer_T)malloc(sizeof(struct SqlServer_S));

	/*retcode   =   SQLAllocHandle   (SQL_HANDLE_ENV,   NULL,   &db->henv);      
	retcode   =   SQLSetEnvAttr(db->henv,   SQL_ATTR_ODBC_VERSION,      
		(SQLPOINTER)SQL_OV_ODBC3,      
		SQL_IS_INTEGER);    */  
	retcode = SQLAllocEnv(&db->henv);
	//2.Á¬½Ó¾ä±ú     
	retcode   =   SQLAllocConnect(db->henv,   &db->hdbc);
	//retcode   =   SQLAllocHandle(SQL_HANDLE_DBC,   db->henv,   &db->hdbc);      
	retcode   =   SQLConnect(db->hdbc,  
		URL_getParameter(url, "db"),   
		strlen( URL_getParameter(url, "db") ),   
		URL_getUser(url),   
		strlen(URL_getUser(url)),   
		URL_getPassword(url),   
		strlen(URL_getPassword(url)));  
	if (SQLSERVERSUCCESS(retcode))
	{
		return db;
	}
	else {
		SQLFreeHandle(SQL_HANDLE_DBC, db->hdbc);     
		SQLFreeHandle(SQL_HANDLE_ENV, db->henv);
		free(db);
		return NULL;
	}
}


static _inline void executeSQL(T C, const char *sql) {
	SQLHSTMT hstmt;
	SQLAllocStmt(C->db->hdbc,&hstmt);
	C->lastError = SQLExecDirect(hstmt,sql,strlen(sql));
	SQLFreeStmt(hstmt,SQL_DROP);
}


static int setProperties(T C, char **error) {
	/*int i = 0;
	const char **properties = URL_getParameterNames(C->url);
	if (properties) {
	StringBuffer_clear(C->sb);
	for ( i = 0; properties[i]; i++) {
	if (IS(properties[i], "heap_limit")) // There is no PRAGMA for heap limit as of SqlServer-3.7.0, so we make it a configurable property using "heap_limit" [kB]
	#if defined(HAVE_SqlServer3_SOFT_HEAP_LIMIT64)
	SqlServer3_soft_heap_limit64(Str_parseInt(URL_getParameter(C->url, properties[i])) * 1024);
	#elif defined(HAVE_SqlServer3_SOFT_HEAP_LIMIT)
	sqlite3_soft_heap_limit(Str_parseInt(URL_getParameter(C->url, properties[i])) * 1024);
	#else
	DEBUG("heap_limit not supported by your SqlServer3 version, please consider upgrading SqlServer3\n");
	#endif
	else
	StringBuffer_append(C->sb, "PRAGMA %s = %s; ", properties[i], URL_getParameter(C->url, properties[i]));
	}
	executeSQL(C, StringBuffer_toString(C->sb));
	if (C->lastError != SqlServer_OK) {
	*error = Str_cat("unable to set database pragmas -- %s", SqlServer3_errmsg(C->db));
	return false;
	}
	}*/
	return true;
}


/* ------------------------------------------------------------ Operations */


const struct Cop_T sqlservercops = {
    .name = "odbc",
    .onstop = onstop,
    .new = SqlServerConnection_new,
    .free = SqlServerConnection_free,
    .setQueryTimeout = SqlServerConnection_setQueryTimeout,
    .setMaxRows = SqlServerConnection_setMaxRows,
    .ping = SqlServerConnection_ping,
    .beginTransaction = SqlServerConnection_beginTransaction,
    .commit = SqlServerConnection_commit,
    .rollback = SqlServerConnection_rollback,
    .lastRowId = SqlServerConnection_lastRowId,
    .rowsChanged = SqlServerConnection_rowsChanged,
    .execute = SqlServerConnection_execute,
    .executeQuery = SqlServerConnection_executeQuery,
    .prepareStatement = SqlServerConnection_prepareStatement,
    .getLastError = SqlServerConnection_getLastError
};


/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T SqlServerConnection_new(URL_T url, char **error) {
	T C;
	SqlServer_T db;
	assert(url);
	assert(error);
	if (! (db = doConnect(url, error)))
		return NULL;
	NEW(C);
	C->db = db;
	C->url = url;
	C->timeout = SQL_DEFAULT_TIMEOUT;
	C->sb = StringBuffer_create(STRLEN);
	C->err = StringBuffer_create(STRLEN);
	//  if (! setProperties(C, error))
	//         SqlServerConnection_free(&C);
	return C;
}


void SqlServerConnection_free(T *C) {
	StringBuffer_free(&(*C)->sb);
	SQLDisconnect((*C)->db->hdbc);      
	SQLFreeHandle(SQL_HANDLE_DBC, (*C)->db->hdbc);     
	SQLFreeHandle(SQL_HANDLE_ENV,(*C)->db->henv);  
	
	free((*C)->db);
	FREE(*C);
}


void SqlServerConnection_setQueryTimeout(T C, int ms) {
	assert(C);
	C->timeout = ms;
}


void SqlServerConnection_setMaxRows(T C, int max) {
	assert(C);
	C->maxRows = max;
}


int SqlServerConnection_ping(T C) {
	assert(C);
	//executeSQL(C, "select 1;");
	//SQLCloseCursor(C->db->hstmt);
	return (SQLSERVERSUCCESS(C->lastError));

}


int SqlServerConnection_beginTransaction(T C) {
	assert(C);
	executeSQL(C, "BEGIN TRANSACTION;");
	return (SQLSERVERSUCCESS(C->lastError));

}


int SqlServerConnection_commit(T C) {
	assert(C);
	executeSQL(C, "COMMIT TRANSACTION;");
	return (SQLSERVERSUCCESS(C->lastError));

}


int SqlServerConnection_rollback(T C) {
	assert(C);
	executeSQL(C, "ROLLBACK TRANSACTION;");
	return (SQLSERVERSUCCESS(C->lastError));

}


long long int SqlServerConnection_lastRowId(T C) {
	assert(C);
	executeSQL(C, "select scope_identity()");
	return (SQLSERVERSUCCESS(C->lastError));
	//return SqlServer3_last_insert_rowid(C->db);
}


long long int SqlServerConnection_rowsChanged(T C) {
	assert(C);
	return (SQLSERVERSUCCESS(C->lastError));
	// return (long long int)SqlServer3_changes(C->db);
}


int SqlServerConnection_execute(T C, const char *sql, va_list ap) {
	va_list ap_copy;
	assert(C);
	StringBuffer_clear(C->sb);
	va_copy(ap_copy, ap);
	StringBuffer_vappend(C->sb, sql, ap_copy);
	va_end(ap_copy);
	executeSQL(C, StringBuffer_toString(C->sb));
	return (SQLSERVERSUCCESS(C->lastError));

}


ResultSet_T SqlServerConnection_executeQuery(T C, const char *sql, va_list ap) {
	va_list ap_copy;
	const char *tail;
	SQLHSTMT hstmt;

	assert(C);
	StringBuffer_clear(C->sb);
	va_copy(ap_copy, ap);
	StringBuffer_vappend(C->sb, sql, ap_copy);
	va_end(ap_copy);

	C->lastError = SQLAllocStmt(C->db->hdbc,&hstmt);


	C->lastError = SQLPrepare(hstmt,
		StringBuffer_toString(C->sb), StringBuffer_length(C->sb)); 
	if(!SQLSERVERSUCCESS(C->lastError)) {
		getSqlErr(C,hstmt);
		return NULL;
	}
	C->lastError = SQLExecute(hstmt);
	if(SQLSERVERSUCCESS(C->lastError))
		return ResultSet_new(SqlServerResultSet_new(hstmt, C->maxRows, false), (Rop_T)&sqlserverrops);
	else {
		getSqlErr(C,hstmt);
	}
	return NULL;
}


PreparedStatement_T SqlServerConnection_prepareStatement(T C, const char *sql, va_list ap) {
	va_list ap_copy;
	const char *tail;
	HSTMT hstmt;

	assert(C);
	StringBuffer_clear(C->sb);
	va_copy(ap_copy, ap);
	StringBuffer_vappend(C->sb, sql, ap_copy);
	va_end(ap_copy);

	C->lastError = SQLAllocStmt(C->db->hdbc,&hstmt);
	C->lastError = SQLPrepare(hstmt,StringBuffer_toString(C->sb),strlen(StringBuffer_toString(C->sb))); 
	//The third argument with an array of the same size , but not the same database column  

    if (SQLSERVERSUCCESS(C->lastError)) {
        int paramCount = 0;
        return PreparedStatement_new(SqlServerPreparedStatement_new(C->db, hstmt, C->maxRows), (Pop_T)&sqlserverpops, paramCount);
    }
	else {
		getSqlErr(C,hstmt);
	}
	return NULL;
}



const char *SqlServerConnection_getLastError(T C) {
	return C->err;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

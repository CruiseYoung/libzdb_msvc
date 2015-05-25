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


#include "system/Time.h"
#include "ResultSet.h"
#include "SqlServerResultSet.h"
#include "PreparedStatementDelegate.h"
#include "SqlServerPreparedStatement.h"


/**
 * Implementation of the PreparedStatement/Delegate interface for SqlServer 
 *
 * @file
 */


/* ----------------------------------------------------------- Definitions */

const struct Pop_S sqlserverpops = {
    .name = "odbc",
    .free = SqlServerPreparedStatement_free,
    .setString = SqlServerPreparedStatement_setString,
    .setInt = SqlServerPreparedStatement_setInt,
    .setLLong = SqlServerPreparedStatement_setLLong,
    .setDouble = SqlServerPreparedStatement_setDouble,
    /*.setTimestamp = SqlServerPreparedStatement_setTimestamp,*/
    .setBlob = SqlServerPreparedStatement_setBlob,
    .execute = SqlServerPreparedStatement_execute,
    .executeQuery = SqlServerPreparedStatement_executeQuery/*,
    .rowsChanged = SqlServerPreparedStatement_rowsChanged*/
};

typedef struct param_t {
	union {
		long integer;
		long long int llong;
		double real;
	} type;
	long length;
} *param_t;

#define T PreparedStatementDelegate_T
struct T {
        SqlServer_T db;
        int maxRows;
        int lastError;
		SQLSMALLINT paramCount;
		param_t params;
		SQLHSTMT stmt;
};

extern const struct Rop_T sqlserverrops;


#define TEST_INDEX \
	int i; assert(P); i = parameterIndex - 1; if (P->paramCount <= 0 || \
	i < 0 || i >= P->paramCount) THROW(SQLException, "Parameter index is out of range"); 

/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif
#if _DEBUG
static const char *SqlServerConnection_getLastError(void *stmt) {


	unsigned char szSQLSTATE[10];
	SDWORD nErr;
	unsigned char msg[SQL_MAX_MESSAGE_LENGTH+1];
	SWORD cbmsg;

	unsigned char szData[256];   // Returned data storage

	while(SQLError(0,0,stmt,szSQLSTATE,&nErr,msg,sizeof(msg),&cbmsg)==
		SQL_SUCCESS)
	{
        snprintf((char *)szData, sizeof(szData), "Error:\nSQLSTATE=%s,Native error=%ld,msg='%s'", szSQLSTATE, nErr, msg);
		//MessageBox(NULL,(const char *)szData,"ODBC Error",MB_OK);
		//return NULL;
		//return strdup(szData);
	}

	return NULL;
}
#endif

T SqlServerPreparedStatement_new(SqlServer_T db, void *stmt, int maxRows) {
        T P;
        assert(stmt);
        NEW(P);
        P->db = db;
        P->stmt = stmt;
        P->maxRows = maxRows;
        P->lastError = SQL_SUCCESS;
		P->lastError = SQLNumParams(stmt,&P->paramCount);
		if (P->paramCount>0) {
			P->params = CALLOC(P->paramCount, sizeof(struct param_t));
		}
        return P;
}


void SqlServerPreparedStatement_free(T *P) {
	assert(P && *P);
	SQLFreeStmt((*P)->stmt,SQL_DROP);
	FREE((*P)->params);
	FREE(*P);
}


void SqlServerPreparedStatement_setString(T P, int parameterIndex, const char *x) {
        int size = 0;
		assert(P);
         size = x ? (int)strlen(x) : 0; 
		P->lastError = SQLBindParameter(P->stmt,
			 parameterIndex,
			 SQL_PARAM_INPUT,
			 SQL_C_CHAR,
			 SQL_CHAR,
			 size,
			 0,
			 x,
			 0,
			 NULL); 
		 if (!SQLSERVERSUCCESS(P->lastError))
		 {
			 THROW(SQLException, "SQLBindParameter error");
		 }
		 

}


void SqlServerPreparedStatement_setInt(T P, int parameterIndex, int x) {
        TEST_INDEX
		P->params[i].type.integer = x;
		P->lastError = SQLBindParameter(P->stmt,
			parameterIndex,
			SQL_PARAM_INPUT,
			SQL_C_LONG,
			SQL_INTEGER,
			0,
			0,
			&P->params[i].type.integer,
			0,
			NULL); 
		if (!SQLSERVERSUCCESS(P->lastError))
		{
			THROW(SQLException, "SQLBindParameter int error");
		}
}


void SqlServerPreparedStatement_setLLong(T P, int parameterIndex, long long int x) {
	TEST_INDEX
		P->params[i].type.llong = x;
		P->lastError = SQLBindParameter(P->stmt,
			parameterIndex,
			SQL_PARAM_INPUT,
			SQL_C_SBIGINT,
			SQL_BIGINT,
			0,
			0,
			&P->params[i].type.llong,
			0,
			NULL); 
		if (!SQLSERVERSUCCESS(P->lastError))
		{
			THROW(SQLException, "SQLBindParameter int error");
		}
		//SQL_BIGINT
		//
		//
    //    SqlServer3_reset(P->stmt);
    //    P->lastError = SqlServer3_bind_int64(P->stmt, parameterIndex, x);
    //    if (P->lastError == SqlServer_RANGE)
    //            THROW(SQLException, "Parameter index is out of range");
}


void SqlServerPreparedStatement_setDouble(T P, int parameterIndex, double x) {
	TEST_INDEX
		P->params[i].type.real = x;
		P->lastError = SQLBindParameter(P->stmt,
			parameterIndex,
			SQL_PARAM_INPUT,
			SQL_C_DOUBLE,
			SQL_DOUBLE,
			0,
			0,
			&P->params[i].type.real,
			0,
			NULL); 
		if (!SQLSERVERSUCCESS(P->lastError))
		{
			THROW(SQLException, "SQLBindParameter double error");
		}
}


void SqlServerPreparedStatement_setBlob(T P, int parameterIndex, const void *x, int size) {
        assert(P);
		P->lastError = SQLBindParameter(P->stmt,
			parameterIndex,
			SQL_PARAM_INPUT,
			SQL_C_FLOAT,
			SQL_REAL,
			size,
			0,
			x,
			0,
			NULL); 
		if (!SQLSERVERSUCCESS(P->lastError))
		{
			THROW(SQLException, "SQLBindParameter Blob error");
		}
}


void SqlServerPreparedStatement_execute(T P) {
	P->lastError = SQLExecute(P->stmt);
	if (!SQLSERVERSUCCESS(P->lastError))
	{
#if _DEBUG
		SqlServerConnection_getLastError(P->stmt);
#endif
		THROW(SQLException, "SqlServerPreparedStatement_execute error");
	}
}


ResultSet_T SqlServerPreparedStatement_executeQuery(T P) {
        assert(P);
		P->lastError = SQLExecute(P->stmt);
        if (SQLSERVERSUCCESS(P->lastError))
               return ResultSet_new(SqlServerResultSet_new(P->stmt, P->maxRows, true), (Rop_T)&sqlserverrops);
       

	
        return NULL;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

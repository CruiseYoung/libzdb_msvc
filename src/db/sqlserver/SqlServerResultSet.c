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
#include <string.h>


#include "system/Time.h"
#include "ResultSetDelegate.h"
#include "SqlServerResultSet.h"


/**
* Implementation of the ResultSet/Delegate interface for SqlServer. 
* Accessing columns with index outside range throws SQLException
*
* @file
*/


/* ------------------------------------------------------------- Definitions */


const struct Rop_S sqlserverrops = {
    .name = "odbc",
    .free = SqlServerResultSet_free,
    .getColumnCount = SqlServerResultSet_getColumnCount,
    .getColumnName = SqlServerResultSet_getColumnName,
    .getColumnSize = SqlServerResultSet_getColumnSize,
    .next = SqlServerResultSet_next,
    /*.isnull = SqlServerResultSet_isnull,*/
    .getString = SqlServerResultSet_getString,
    .getBlob = SqlServerResultSet_getBlob
    // getTimestamp and getDateTime is handled in ResultSet
};


enum enum_field_types {
SQL_TYPE_UNKNOWN_TYPE = 0,
SQL_TYPE_CHAR = 1,
SQL_TYPE_NUMERIC = 2,
SQL_TYPE_DECIMAL = 3,
SQL_TYPE_INTEGER = 4,
SQL_TYPE_SMALLINT = 5,
SQL_TYPE_FLOAT = 6,
SQL_TYPE_REAL = 7,
SQL_TYPE_DOUBLE = 8,
#if (ODBCVER >= 0x0300)
SQL_TYPE_DATETIME = 9,
#endif
SQL_TYPE_VARCHAR = 12,

#if (ODBCVER >= 0x0300)
SQL_TYPE_TYPE_DATE = 91,
SQL_TYPE_TYPE_TIME = 92,
SQL_TYPE_TYPE_TIMESTAMP = 93,
SQL_TYPE_BIGINT = -5,
#endif
};

typedef struct SqlServer_Field_S {
	char name[256];
	SQLUINTEGER  max_length;
	SQLSMALLINT  name_length;
	enum enum_field_types type;
	char cannull;
	SQLSMALLINT decimals;//Decimal point
	void *extension;
} ;

typedef struct column_s {
    struct SqlServer_Field_S field;
	SQLLEN  real_length;
	char *buffer;
} *column_t;

#define T ResultSetDelegate_T
struct ResultSetDelegate_S {
	int keep;
	int maxRows;
	int currentRow;
	int columnCount;
	SQLHSTMT stmt;
    column_t columns;
};

#define TEST_INDEX \
	int i; assert(R); i = columnIndex - 1; if (R->columnCount <= 0 || \
	i < 0 || i >= R->columnCount) THROW(SQLException, "Column index is out of range");


/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

static const char *SqlServerResultSet_getLastError(void *stmt) {


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
		return strdup(szData);
	}

	return NULL;
}

T SqlServerResultSet_new(void *stmt, int maxRows, int keep) {
	T R;
	WORD wclos = 0;
	RETCODE retcode = 0;
	int i = 0;
	assert(stmt);
	NEW(R);
	R->stmt = stmt;
	R->keep = keep;
	R->maxRows = maxRows;
	retcode = SQLNumResultCols(R->stmt,&wclos);
	R->columnCount = wclos;
	if (!SQLSERVERSUCCESS(retcode))
	{
        SqlServerResultSet_getLastError(stmt);
		SqlServerResultSet_free(&R);
		return NULL;
	}
    R->columns = CALLOC(R->columnCount, sizeof(struct column_s));

	for(i = 1; i <= R->columnCount; i++) {
		SQLSMALLINT ftype;
		SQLSMALLINT ty;
		retcode = SQLDescribeCol(
			stmt,
			i, 
			R->columns[i-1].field.name, 
			sizeof(R->columns[i-1].field.name),
			&R->columns[i-1].field.name_length,
			&ty,
			&R->columns[i-1].field.max_length,
			&R->columns[i-1].field.decimals,
			&R->columns[i-1].field.cannull); 
		if (!SQLSERVERSUCCESS(retcode)) {
            SqlServerResultSet_getLastError(stmt);
			SqlServerResultSet_free(&R);
			return NULL;
		}
		R->columns[i-1].field.type = ty;
		switch (R->columns[i-1].field.type)
		{
		case SQL_CHAR:
		case SQL_VARCHAR:
		case SQL_DECIMAL:
		case SQL_NUMERIC:
			ftype = SQL_C_CHAR;
			break;
		case SQL_INTEGER:
			ftype =SQL_C_LONG;
			break;
		case SQL_SMALLINT:
			ftype =SQL_C_SHORT;
			break;
		case SQL_REAL:
			ftype =SQL_C_FLOAT;
			break;
		case SQL_FLOAT:
		case SQL_DOUBLE:
			ftype = SQL_C_DOUBLE;
			break;
		case SQL_TYPE_BIGINT:
			ftype = SQL_C_SBIGINT;
			break;
		default:
			//SqlServerResultSet_getLastError(stmt);
			//SqlServerResultSet_free(&R);
			//THROW(SQLException, "Column type unknow ");
			//return NULL;
			break;
		}
		ftype = SQL_C_CHAR;

		R->columns[i-1].buffer = ALLOC(STRLEN + 1);
		retcode = SQLBindCol(stmt,
			i,
			ftype,
			R->columns[i-1].buffer,
			STRLEN,
			&R->columns[i-1].real_length);
		if (!SQLSERVERSUCCESS(retcode)) {
            SqlServerResultSet_getLastError(stmt);
			SqlServerResultSet_free(&R);
			return NULL;
		}
		
	}


	return R;
}


void SqlServerResultSet_free(T *R) {
	int i = 0;
	SQLFreeStmt((*R)->stmt,SQL_DROP);
	for ( i = 0; i < (*R)->columnCount; i++)
		FREE((*R)->columns[i].buffer);
	 FREE((*R)->columns);
	/*	assert(R && *R);
	if ((*R)->keep)
	SqlServer3_reset((*R)->stmt);
	else
	SqlServer3_finalize((*R)->stmt);*/
	FREE(*R);
}


int SqlServerResultSet_getColumnCount(T R) {
	assert(R);
	return R->columnCount;
}


const char *SqlServerResultSet_getColumnName(T R, int column) {
	assert(R);
	column--;
	if (R->columnCount <= 0 ||
		column < 0           ||
		column > R->columnCount)
		return NULL;
	//	return SqlServer3_column_name(R->stmt, column);
	return NULL;
}


int SqlServerResultSet_next(T R) {
	int status = 0;
	RETCODE retcode;  
	assert(R);
	if (R->maxRows && (R->currentRow++ >= R->maxRows))
		return false;

	retcode = SQLFetch(R->stmt);
	return (retcode  != SQL_NO_DATA);
}


long SqlServerResultSet_getColumnSize(T R, int columnIndex) {
    column_t col;
	TEST_INDEX
	col =  &R->columns[i];
	//	SQLNumResultCols(R->stmt,);
	//        return SqlServer3_column_bytes(R->stmt, i);
	return col->real_length;
}


const char *SqlServerResultSet_getString(T R, int columnIndex) {
    column_t col;
	TEST_INDEX
		//return (const char*)SqlServer3_column_text(R->stmt, i);
		col =  &R->columns[i];
	if (col->real_length < 0)
	{
		return NULL;
	}
	return col->buffer;
	switch (col->field.type)
	{
	case SQL_SMALLINT:
		sprintf(col->buffer,"%d",*(SQLSMALLINT*)col->buffer);
		break;
	case SQL_TYPE_INTEGER:
		sprintf(col->buffer,"%d",*(int*)col->buffer);
		break;
	case SQL_DECIMAL://DECIMAL(p,s) (1 <= p <= 15; s <= p).
	case SQL_NUMERIC://NUMERIC(p,s) (1 <= p <= 15; s <= p).
		break;
	case SQL_FLOAT://numeral '1.1234567890123456789012345678901234567890' Beyond the numbers indicate the range ( maximum precision of 38 significant digits )¡£
	case SQL_DOUBLE:{
		//double dd = *(double*)col->buffer;
		
		gcvt(*(double*)col->buffer,col->field.decimals > 0 ? col->field.decimals : 38,col->buffer);
		//sprintf(col->buffer,"%s",*(double*)col->buffer);
					}
		break;

	case SQL_TYPE_BIGINT:
		sprintf(col->buffer,"%d",*(long long int*)col->buffer);
		break;
		//memccpy(col->buffer,col->real_length,)
		//col->buffer
	}
	
	return col->buffer;
}


const void *SqlServerResultSet_getBlob(T R, int columnIndex, int *size) {
	const void *blob = NULL;
	TEST_INDEX

		/*        blob = SqlServer3_column_blob(R->stmt, i);
		*size = SqlServer3_column_bytes(R->stmt, i);*/
		return blob;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif


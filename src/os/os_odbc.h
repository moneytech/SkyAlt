SQLHENV g_odbc_env;

BOOL OsODBC_initGlobal(void)
{
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &g_odbc_env);
	SQLSetEnvAttr(g_odbc_env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);	//use ODBC 3 support
	return TRUE;
}

void OsODBC_freeGlobal(void)
{
	SQLFreeHandle(SQL_HANDLE_ENV, g_odbc_env);
}

BIG OsODBC_getDriversList(UNI*** list)
{
	StdArr arr = StdArr_init();

	char driver[256];
	char attr[256];
	SQLSMALLINT driver_ret;
	SQLSMALLINT attr_ret;
	int direction = SQL_FETCH_FIRST;
	SQLRETURN retcode;
	while (SQL_SUCCEEDED(retcode = SQLDrivers(g_odbc_env, direction, (SQLCHAR*)driver, sizeof(driver), &driver_ret, (SQLCHAR*)attr, sizeof(attr), &attr_ret)))
	{
		direction = SQL_FETCH_NEXT;
		StdArr_add(&arr, Std_newUNI_char(driver));
		//printf("%s - %s\n", driver, attr);
	}

	*list = (UNI**)arr.ptrs;
	return arr.num;
}

BIG OsODBC_getDataSourcesList(UNI*** list)
{
	StdArr arr = StdArr_init();

	char dsnName[256];
	char attr[256];
	SQLSMALLINT dsnName_ret;
	SQLSMALLINT attr_ret;
	int direction = SQL_FETCH_FIRST;
	SQLRETURN retcode;
	while (SQL_SUCCEEDED(retcode = SQLDataSources(g_odbc_env, direction, (SQLCHAR*)dsnName, sizeof(dsnName), &dsnName_ret, (SQLCHAR*)attr, sizeof(attr), &attr_ret)))
	{
		direction = SQL_FETCH_NEXT;
		StdArr_add(&arr, Std_newUNI_char(dsnName));
		//printf("%s - %s\n", dsnName, attr);
	}

	*list = (UNI**)arr.ptrs;
	return arr.num;
}

typedef struct OsODBC_s
{
	SQLHDBC dbc;
}OsODBC;

typedef struct OsODBCQuery_s
{
	SQLHSTMT hstmt;
	SQLLEN prm_inds[16];
}OsODBCQuery;

void OsODBC_delete(OsODBC* self)
{
	SQLFreeHandle(SQL_HANDLE_DBC, self->dbc);
	Os_free(self, sizeof(OsODBC));
}

static char* _OsODBC_addParam(char* str, const char* name, const char* var)
{
	str = Std_addAfterCHAR(str, name);
	str = Std_addAfterCHAR(str, "=");
	str = Std_addAfterCHAR(str, var);
	str = Std_addAfterCHAR(str, ";");
	return str;
}

OsODBC* OsODBC_new(const char* connectionName, const char* server, USHORT port, const char* userName, const char* password, const char* driver)
{
	OsODBC* self = calloc(1, sizeof(OsODBC));

	SQLAllocHandle(SQL_HANDLE_DBC, g_odbc_env, &self->dbc);

	char portStr[16];
	snprintf(portStr, 16, "%d", port);

	char* dst = 0;
	if (connectionName)	dst = _OsODBC_addParam(dst, "DSN", connectionName);
	if (server)			dst = _OsODBC_addParam(dst, "SERVER", server);
	if (port)			dst = _OsODBC_addParam(dst, "PORT", portStr);
	if (userName)		dst = _OsODBC_addParam(dst, "UID", userName);
	if (password)		dst = _OsODBC_addParam(dst, "PWD", password);
	if (driver)			dst = _OsODBC_addParam(dst, "DRIVER", driver);

	SQLRETURN ret = SQLDriverConnect(self->dbc, NULL, (SQLCHAR*)dst, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	Std_deleteCHAR(dst);

	if (!SQL_SUCCEEDED(ret))
	{
		OsODBC_delete(self);
		self = 0;
	}

	return self;
}

OsODBCQuery* OsODBC_createQuery(OsODBC* self, char* statement)	//statement = "UPDATE Parts SET Price = ? WHERE PartID = ?"
{
	SQLHSTMT hstmt;

	OsODBCQuery* query = 0;
	if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, self->dbc, &hstmt)))
	{
		if (SQL_SUCCEEDED(SQLPrepare(hstmt, (SQLCHAR*)statement, SQL_NTS)))
		{
			query = Os_malloc(sizeof(OsODBCQuery));
			query->hstmt = hstmt;
		}
		else
			SQLCloseCursor(hstmt);
	}

	return query;
}

char* OsODBC_getDbName(OsODBC* self)
{
	char dbName[256];
	SQLLEN dbNameSize;
	SQLRETURN ret = SQLGetInfo(self->dbc, SQL_DATABASE_NAME, dbName, (SQLSMALLINT)255, (SQLSMALLINT*)&dbNameSize);
	return SQL_SUCCEEDED(ret) ? Std_newCHAR(dbName) : 0;
}

char* OsODBC_getUserName(OsODBC* self)
{
	char dbName[256];
	SQLLEN dbNameSize;
	SQLRETURN ret = SQLGetInfo(self->dbc, SQL_USER_NAME, dbName, (SQLSMALLINT)255, (SQLSMALLINT*)&dbNameSize);
	return SQL_SUCCEEDED(ret) ? Std_newCHAR(dbName) : 0;
}

struct DataBinding
{
	SQLSMALLINT TargetType;
	SQLPOINTER TargetValuePtr;
	SQLINTEGER BufferLength;
	SQLLEN StrLen_or_Ind;
};

BIG OsODBC_getTablesList(OsODBC* self, UNI*** list)
{
	//char* namesss[10];
	//getTheseTables(self->dbc, namesss);

	StdArr names = StdArr_init();

	char* dbName = OsODBC_getDbName(self);
	//char* userName = OsODBC_getUserName(self);

	SQLHSTMT hstmt;
	if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, self->dbc, &hstmt)))
	{
		char name[256];
		SQLLEN nameSize;
		SQLBindCol(hstmt, 3, SQL_C_CHAR, name, sizeof(name) - 1, &nameSize);	//3 = TABLE_NAME

		if (SQL_SUCCEEDED(SQLTables(hstmt, (SQLCHAR*)dbName, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS)))
		{
			while (SQLFetch(hstmt) != SQL_NO_DATA)
			{
				StdArr_add(&names, Std_newUNI_char(name));
			}
		}
		SQLCloseCursor(hstmt);
	}

	Std_deleteCHAR(dbName);

	*list = (UNI**)names.ptrs;
	return names.num;
}

BIG OsODBC_getColumnsList(OsODBC* self, const UNI* tableName, UNI*** out_names, BIG** out_types)
{
	StdArr names = StdArr_init();
	StdBigs types = StdBigs_init();

	SQLHSTMT hstmt;
	if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, self->dbc, &hstmt)))
	{
		char name[256];
		SQLLEN nameSize;
		SQLBindCol(hstmt, 4, SQL_C_CHAR, name, sizeof(name) - 1, &nameSize);

		SQLSMALLINT type;	//https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/sql-data-types?view=sql-server-ver15
		SQLLEN typeSize;
		SQLBindCol(hstmt, 5, SQL_C_SSHORT, &type, 0, &typeSize);

		char* nm = Std_newCHAR_uni(tableName);
		if (SQL_SUCCEEDED(SQLColumns(hstmt, NULL, 0, NULL, 0, (SQLCHAR*)nm, SQL_NTS, NULL, 0)))
		{
			while (SQLFetch(hstmt) != SQL_NO_DATA)
			{
				StdArr_add(&names, Std_newUNI_char(name));

				OsODBCType tp = SQL_UNKNOWN_TYPE;
				switch (type)
				{
					case SQL_CHAR:
					case SQL_VARCHAR:
					tp = OsODBC_STRING;
					break;

					case SQL_DATETIME:
					tp = OsODBC_DATE;
					break;

					case SQL_NUMERIC:
					case SQL_DECIMAL:
					case SQL_INTEGER:
					case SQL_SMALLINT:
					case SQL_FLOAT:
					case SQL_REAL:
					case SQL_DOUBLE:
					tp = OsODBC_NUMBER;
					break;
				}

				StdBigs_add(&types, tp);
			}
		}
		Std_deleteCHAR(nm);

		SQLCloseCursor(hstmt);
	}

	*out_names = (UNI**)names.ptrs;
	*out_types = types.ptrs;
	return names.num;
}

BIG OsODBC_getPrimaryColumnList(OsODBC* self, const UNI* tableName, UNI*** out_names)
{
	StdArr names = StdArr_init();

	SQLHSTMT hstmt;
	if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, self->dbc, &hstmt)))
	{
		char name[256];
		SQLLEN nameSize;
		SQLBindCol(hstmt, 4, SQL_C_CHAR, name, sizeof(name) - 1, &nameSize);

		char* nm = Std_newCHAR_uni(tableName);
		if (SQL_SUCCEEDED(SQLPrimaryKeys(hstmt, NULL, 0, NULL, 0, (SQLCHAR*)nm, SQL_NTS)))
		{
			while (SQLFetch(hstmt) != SQL_NO_DATA)
			{
				StdArr_add(&names, Std_newUNI_char(name));
			}
		}
		Std_deleteCHAR(nm);

		SQLCloseCursor(hstmt);
	}

	*out_names = (UNI**)names.ptrs;

	return names.num;
}

BIG OsODBC_getForeignColumnList(OsODBC* self, const UNI* tableName, UNI*** out_srcColumnNames, UNI*** out_dstTableNames)
{
	StdArr srcNames = StdArr_init();
	StdArr dstNames = StdArr_init();

	SQLHSTMT hstmt;
	if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, self->dbc, &hstmt)))
	{
		char srcName[256];
		SQLLEN srcNameSize;
		SQLBindCol(hstmt, 8, SQL_C_CHAR, srcName, sizeof(srcName) - 1, &srcNameSize);	//src column

		char dstName[256];
		SQLLEN dstNameSize;
		SQLBindCol(hstmt, 3, SQL_C_CHAR, dstName, sizeof(dstName) - 1, &dstNameSize);	//fk. table

		char* nm = Std_newCHAR_uni(tableName);
		if (SQL_SUCCEEDED(SQLForeignKeys(hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, (SQLCHAR*)nm, SQL_NTS)))
		{
			while (SQLFetch(hstmt) != SQL_NO_DATA)
			{
				StdArr_add(&srcNames, Std_newUNI_char(srcName));
				StdArr_add(&dstNames, Std_newUNI_char(dstName));
			}
		}
		Std_deleteCHAR(nm);

		SQLCloseCursor(hstmt);
	}

	*out_srcColumnNames = (UNI**)srcNames.ptrs;
	*out_dstTableNames = (UNI**)dstNames.ptrs;

	return srcNames.num;
}

void OsODBCQuery_delete(OsODBCQuery* self)
{
	SQLCloseCursor(self->hstmt);
	Os_free(self, sizeof(OsODBCQuery));
}

BOOL OsODBCQuery_addVarDouble(OsODBCQuery* self, int index, double* value)	//'prm_index' starts with 1
{
	SQLRETURN ret = SQLBindParameter(self->hstmt, index, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 10, 0, value, 0, &self->prm_inds[index - 1]);
	return SQL_SUCCEEDED(ret);
}

BOOL OsODBCQuery_addVarString(OsODBCQuery* self, int index, char* str, const int max_size)
{
	SQLRETURN ret = SQLBindParameter(self->hstmt, index, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, max_size, 0, str, max_size, &self->prm_inds[index - 1]);
	return SQL_SUCCEEDED(ret);
}

BOOL OsODBCQuery_addColumnDouble(OsODBCQuery* self, int index, double* value)//, BIG* value_len)
{
	SQLRETURN ret = SQLBindCol(self->hstmt, index, SQL_C_DOUBLE, value, 0, 0);
	return SQL_SUCCEEDED(ret);
}

BOOL OsODBCQuery_addColumnString(OsODBCQuery* self, int index, char* str, BIG max_len)//, BIG* value_len)
{
	SQLRETURN ret = SQLBindCol(self->hstmt, index, SQL_C_CHAR, str, max_len, 0);
	return SQL_SUCCEEDED(ret);
}

BOOL OsODBCQuery_execute(OsODBCQuery* self)
{
	SQLRETURN ret = SQLExecute(self->hstmt);
	return SQL_SUCCEEDED(ret);
}

BOOL OsODBCQuery_fetch(OsODBCQuery* self)
{
	return SQLFetch(self->hstmt) != SQL_NO_DATA;
}

UBIG OsODBCQuery_count(OsODBCQuery* self)
{
	SQLLEN count;
	SQLGetDiagField(SQL_HANDLE_STMT, self->hstmt, 0, SQL_DIAG_ROW_COUNT, &count, sizeof(count), 0);
	return count;
}

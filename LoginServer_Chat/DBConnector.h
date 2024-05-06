#pragma once
#if _MSC_VER >= 1932 // Visual Studio 2022 version 17.2+
#pragma comment(linker, "/alternatename:__imp___std_init_once_complete=__imp_InitOnceComplete")
#pragma comment(linker, "/alternatename:__imp___std_init_once_begin_initialize=__imp_InitOnceBeginInitialize")
#endif
#include <strsafe.h>
#pragma comment(lib,"mysqlclient.lib")
#include "include/mysql.h"
#include "include/errmsg.h"
#define MAX_QUERY_LEN	1024

class DBConnector
{
public:
	DBConnector() = delete;
	DBConnector(const char* ip, unsigned int port, const char* user, const char* passwd, const char* schema);
	bool ReqQuery(const char* query, ...); // 추후 WCHAR 지원 오버로딩
	MYSQL_ROW FetchRow();
	void FreeQueryResult();
	void CloseConnection();
private:
	MYSQL		_conn;
	MYSQL_RES*	_sql_result;
	char		_query[MAX_QUERY_LEN];
};
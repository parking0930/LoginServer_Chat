#include "DBConnector.h"

DBConnector::DBConnector(const char* ip, unsigned int port, const char* user, const char* passwd, const char* schema) :_sql_result(nullptr)
{
	_query[0] = '\0';

	// 초기화
	mysql_init(&_conn);

	// DB 연결
	MYSQL* connection = NULL;
	connection = mysql_real_connect(&_conn, ip, user, passwd, schema, port, (char*)NULL, 0);
	if (connection == NULL)
	{
		fprintf(stderr, "Mysql connection error : %s", mysql_error(&_conn));
		throw 1;
	}
}

bool DBConnector::ReqQuery(const char* query, ...)
{
	va_list argList;
	va_start(argList, query);
	StringCchVPrintfA(_query, MAX_QUERY_LEN, query, argList);
	if (mysql_query(&_conn, _query) != 0)
		return false;
	_sql_result = mysql_store_result(&_conn);
	return true;
}

MYSQL_ROW DBConnector::FetchRow()
{
	return mysql_fetch_row(_sql_result);
}

void DBConnector::FreeQueryResult()
{
	mysql_free_result(_sql_result);
}

void DBConnector::CloseConnection()
{
	mysql_close(&_conn);
}
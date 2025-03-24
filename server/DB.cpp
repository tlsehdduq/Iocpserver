#include "pch.h"
#include "DB.h"
#include"Session.h"
DB::DB()
{
	setlocale(LC_ALL, "korean");

	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	// 사용자 이름과 비밀번호 추가 (예: root, password)
	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2drpgodbc", SQL_NTS,
		(SQLWCHAR*)L"root", SQL_NTS,
		(SQLWCHAR*)L"Sch951639@", SQL_NTS);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLWCHAR sqlState[6], errorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER nativeError;
		SQLSMALLINT msgLen;
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, sqlState, &nativeError, errorMsg, SQL_MAX_MESSAGE_LENGTH, &msgLen);
		std::wcerr << L"Connection failed: " << errorMsg << std::endl;
		throw std::runtime_error("DB connection failed");
	}

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	std::cout << "DB Access OK\n";
}

DB::~DB()
{
}

bool DB::isAllowAccess(char* name,int id)
{
	wstring playername = charToWstring(name);
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	wstring storedProcedure = L"CALL selectinfo('" + playername + L"')";

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)storedProcedure.c_str(), SQL_NTS);
	if (retcode == SQL_ERROR) {
		SQLWCHAR sqlState[6], errorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER nativeError;
		SQLSMALLINT msgLen;
		SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlState, &nativeError, errorMsg, SQL_MAX_MESSAGE_LENGTH, &msgLen);
		std::wcerr << L"SQL Error: " << errorMsg << std::endl;
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &user_name, 10, &cbuser_name);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &user_xpos, 0, &cbuser_xpos);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &user_ypos, 0, &cbuser_ypos);
		retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &user_hp, 0, &cbuser_hp);
		retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &user_monstercnt, 0, &cbuser_monstercnt);
		retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &user_level, 0, &cbuser_level);

		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			char* userName = reinterpret_cast<char*>(&user_name);
			SessionManager::GetInstance()._clients[id].setPosX(user_xpos);
			SessionManager::GetInstance()._clients[id].setPosY(user_ypos);
			SessionManager::GetInstance()._clients[id].setName(userName);
			SessionManager::GetInstance()._clients[id].setHp(user_hp);
			SessionManager::GetInstance()._clients[id]._monstercnt = user_monstercnt;
			SessionManager::GetInstance()._clients[id].setLevel(user_level);
			std::cout << "DB 정보를 찾았습니다. 정보를 불러옵니다\n";
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return true;
		}
		else {
			std::cout << "정보를 찾지 못했습니다. 정보를 추가합니다.\n";
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return false;
		}
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return false;
}

void DB::saveUserInfo(int id)
{
	auto& instance = SessionManager::GetInstance();
	SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	wstring playername = charToWstring(instance._clients[id].getName());
	wstring xpos = to_wstring(instance._clients[id].getPosX());
	wstring ypos = to_wstring(instance._clients[id].getPosY());
	wstring playerhp = to_wstring(instance._clients[id].getHp());
	wstring monstercnt = to_wstring(instance._clients[id]._monstercnt);
	wstring level = to_wstring(instance._clients[id].getLevel());

	wstring storedProcedure = L"CALL update_userinfo('" + playername + L"', " + xpos + L", " + ypos + L", " + playerhp + L", " + monstercnt + L", " + level + L")";

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)storedProcedure.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLLEN rowCount;
		retcode = SQLRowCount(hstmt, &rowCount);
		if (rowCount > 0) {
			std::wcout << L"업데이트 성공: " << rowCount << L"개의 행이 업데이트되었습니다." << std::endl;
		}
		else {
			std::wcout << L"업데이트 실패: 해당 사용자가 존재하지 않습니다." << std::endl;
		}
	}
	else if (retcode == SQL_ERROR) {
		SQLWCHAR sqlState[6], errorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER nativeError;
		SQLSMALLINT msgLen;
		SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlState, &nativeError, errorMsg, SQL_MAX_MESSAGE_LENGTH, &msgLen);
		std::wcerr << L"SQL Error: " << errorMsg << std::endl;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

void DB::showError()
{
}

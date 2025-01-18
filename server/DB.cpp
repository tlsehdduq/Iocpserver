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

	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"mygame", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	std::cout << "DB Access OK\n";
}

DB::~DB()
{
}

bool DB::isAllowAccess(char* name,int id)
{
	wstring playername = charToWstring(name);

	auto& instance = SessionManager::GetInstance();
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	wstring _dbname = playername;

	wstring storedProcedure = L"EXEC select_info ";
	storedProcedure += _dbname;

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)storedProcedure.c_str(), SQL_NTS);
	//ShowError(hstmt, SQL_HANDLE_STMT, retcode);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &user_name,20, &cbuser_name);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &user_xpos, 10, &cbuser_xpos);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &user_ypos, 10, &cbuser_ypos);
		retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &user_hp, 10, &cbuser_hp);
		retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &user_monstercnt, 10, &cbuser_monstercnt);
		for (int i = 0; ; i++) {
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				char* userName = reinterpret_cast<char*>(&user_name);
				instance._clients[id].setPosX(user_xpos);
				instance._clients[id].setPosY(user_ypos);	
				instance._clients[id].setName(userName);
				instance._clients[id].setHp(user_hp);
				instance._clients[id]._monstercnt = user_monstercnt;
				std::cout << "DB 정보를 찾았습니다. 정보를 불러옵니다 ";
				return true;
			}
			else
			{
				cout << " 정보를 찾지 못했습니다 " << endl;
				cout << "DB에 정보를 추가하였습니다. ";
				return false;
				// 정보 추가 
			}
		}
	}
}

void DB::saveUserInfo(int id)
{
	auto& instance = SessionManager::GetInstance();
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	wstring playername = charToWstring(instance._clients[id].getName());
	wstring xpos = to_wstring(instance._clients[id].getPosX());
	wstring ypos = to_wstring(instance._clients[id].getPosY());
	wstring playerhp = to_wstring(instance._clients[id].getHp());
	wstring monstercnt = to_wstring(instance._clients[id]._monstercnt);
	wstring storedProcedure = L"EXEC update_userinfo ";
	storedProcedure += playername;
	storedProcedure += L", ";
	storedProcedure += xpos;
	storedProcedure += L", ";
	storedProcedure += ypos;
	storedProcedure += L", ";
	storedProcedure += playerhp;
	storedProcedure += L", ";
	storedProcedure += monstercnt;

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)storedProcedure.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &user_xpos, 10, &cbuser_name);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &user_xpos, 10, &cbuser_xpos);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &user_ypos, 10, &cbuser_ypos);
		retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &user_hp, 10, &cbuser_hp);
		retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &user_monstercnt, 10, &cbuser_monstercnt);
		for (int i = 0; ; i++) {
			retcode = SQLFetch(hstmt);
			break;
		}
	}
}

void DB::showError()
{
}

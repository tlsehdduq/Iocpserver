#pragma once
#include <string>
#include <locale>
#include <codecvt>

class DB
{
public:
	DB();
	~DB();

public:
	bool isAllowAccess(char* name,int id);
	void saveUserInfo(int id);
	void showError();

	std::wstring charToWstring(const char* charStr) {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(charStr);
	}

private:

	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR user_name;
	SQLINTEGER user_xpos, user_ypos, user_hp, user_monstercnt,user_level;
	SQLLEN cbuser_name = 0, cbuser_xpos = 0, cbuser_ypos = 0, cbuser_hp, cbuser_monstercnt,cbuser_level;

};


#pragma once
#include "sqlite3.h"
#include <string>

class TokenDao
{
public:
	explicit TokenDao(sqlite3* db);
	bool createToken(int userId, const std::string& token);
	bool getUserIdByToken(const std::string& token, int& userId);
	bool deleteToken(const std::string& token);


private:
	sqlite3* db_;
};
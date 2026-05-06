#pragma once
#include "sqlite3.h"
#include <string>

class UserDao
{
public:
	explicit UserDao(sqlite3* db);

	bool isUsernameExists(const std::string& username);
	bool createUser(const std::string& username, const std::string& passwordHash);
	bool getUsername(const std::string& username, int& userId, std::string& passwordHash);

private:
	sqlite3* db_;
};
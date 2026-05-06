#include "dao/UserDao.h"
#include <iostream>

UserDao::UserDao(sqlite3* db) : db_(db)
{
}

bool UserDao::isUsernameExists(const std::string& username)
{
	const char* sql = "SELECT COUNT(*) FROM users WHERE username = ?";
	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare username check SQL: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
	bool exists = false;
	result = sqlite3_step(stmt);
	if (result == SQLITE_ROW)
	{
		int count = sqlite3_column_int(stmt, 0);
		exists = (count > 0);
	}
	sqlite3_finalize(stmt);
	return exists;
}


bool UserDao::createUser(const std::string& username, const std::string& passwordHash)
{
	const char* sql = "INSERT INTO users (username, password_hash, created_at) "
					  "VALUES (?, ?, datetime('now', 'localtime'));";
	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare user creation SQL: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
	result = sqlite3_step(stmt);
	if (result != SQLITE_DONE)
	{
		std::cerr << "Failed to execute user creation SQL: " << sqlite3_errmsg(db_) << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}

bool UserDao::getUsername(const std::string& username, int& userId, std::string& passwordHash)
{
	const char* sql = "SELECT id, password_hash FROM users WHERE username = ?";
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare get user SQL: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
	result = sqlite3_step(stmt);
	if (result == SQLITE_ROW)
	{
		userId = sqlite3_column_int(stmt, 0);
		const unsigned char* text = sqlite3_column_text(stmt, 1);
		if (text)
		{
			passwordHash = reinterpret_cast<const char*>(text);
		}
		else
		{
			passwordHash = "";
		}
		sqlite3_finalize(stmt);
		return true;
	}
	sqlite3_finalize(stmt);
	return false;
}
#include "dao/TokenDao.h"
#include <iostream>

TokenDao::TokenDao(sqlite3* db) : db_(db)
{

}
bool TokenDao::createToken(int userId, const std::string& token)
{
	const char* sql = "INSERT INTO tokens (user_id, token, created_at, expired_at)"
					  "VALUES (?, ?, datetime('now', 'localtime'), datetime('now', '+7 days', 'localtime'));";

	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare token creation SQL: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, userId);
	sqlite3_bind_text(stmt, 2, token.c_str(), -1, SQLITE_TRANSIENT);

	result = sqlite3_step(stmt);
	if (result != SQLITE_DONE)
	{
		std::cerr << "Failed to execute token creation SQL: " << sqlite3_errmsg(db_) << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}
bool TokenDao::getUserIdByToken(const std::string& token, int& userId)
{
	const char* sql = "SELECT user_id FROM tokens WHERE token = ? AND expired_at > datetime('now', 'localtime')";
	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare token query SQL: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}

	sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);

	result = sqlite3_step(stmt);
	if (result == SQLITE_ROW)
	{
		userId = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		return true;
	}
	else
	{
		std::cerr << "Failed to execute token query SQL: " << sqlite3_errmsg(db_) << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}
}

bool TokenDao::deleteToken(const std::string& token)
{
		const char* sql = "DELETE FROM tokens WHERE token = ?";
		sqlite3_stmt* stmt = nullptr;
		int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
		if (result != SQLITE_OK)
		{
			std::cerr << "Failed to prepare token deletion SQL: " << sqlite3_errmsg(db_) << std::endl;
			return false;
		}

		sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
		result = sqlite3_step(stmt);
		if (result != SQLITE_DONE)
		{
			std::cerr << "Failed to execute token deletion SQL: " << sqlite3_errmsg(db_) << std::endl;
			sqlite3_finalize(stmt);
			return false;
		}
		int changedRows = sqlite3_changes(db_);
		sqlite3_finalize(stmt);
		return changedRows > 0;
}
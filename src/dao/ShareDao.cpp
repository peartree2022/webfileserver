#include "dao/ShareDao.h"
#include <iostream>
#include <vector>

ShareDao::ShareDao(sqlite3* db) : db_(db) {}

bool ShareDao::createShare(
	int userId,
	int fileId,
	const std::string& shareToken,
	int& shareId
)
{
	const char* sql =
		"INSERT INTO shares "
		"(user_id, file_id, share_token, created_at, expired_at) "
		"VALUES (?, ?, ?, datetime('now', 'localtime'), datetime('now', 'localtime', '+7 days'));";

	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

	if (result != SQLITE_OK)
	{
		std::cerr << "failed to prepare create share sql: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, userId);
	sqlite3_bind_int(stmt, 2, fileId);
	sqlite3_bind_text(stmt, 3, shareToken.c_str(), -1, SQLITE_TRANSIENT);

	result = sqlite3_step(stmt);
	if (result != SQLITE_DONE)
	{
		std::cerr << "failed to execute create share sql: " << sqlite3_errmsg(db_) << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}

	shareId = static_cast<int>(sqlite3_last_insert_rowid(db_));
	sqlite3_finalize(stmt);
	return true;
}

bool ShareDao::getShareByToken(const std::string& shareToken, ShareInfo& share)
{
	const char* sql =
		"SELECT id, user_id, file_id, share_token, created_at, expired_at "
		"FROM shares "
		"WHERE share_token = ? AND expired_at > datetime('now', 'localtime');";

	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "failed to prepare get share sql: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}

	sqlite3_bind_text(stmt, 1, shareToken.c_str(), -1, SQLITE_TRANSIENT);
	result = sqlite3_step(stmt);

	if (result == SQLITE_ROW)
	{
		share.id = sqlite3_column_int(stmt, 0);
		share.userId = sqlite3_column_int(stmt, 1);
		share.fileId = sqlite3_column_int(stmt, 2);

		const unsigned char* tokenText = sqlite3_column_text(stmt, 3);
		const unsigned char* createdText = sqlite3_column_text(stmt, 4);
		const unsigned char* expiredText = sqlite3_column_text(stmt, 5);

		share.shareToken = tokenText ? reinterpret_cast<const char*>(tokenText) : "";
		share.createdAt = createdText ? reinterpret_cast<const char*>(createdText) : "";
		share.expiredAt = expiredText ? reinterpret_cast<const char*>(expiredText) : "";

		sqlite3_finalize(stmt);
		return true;
	}
	if (result != SQLITE_DONE)
	{
		std::cerr << "failed to execute get share sql: " << sqlite3_errmsg(db_) << std::endl;
	}
	sqlite3_finalize(stmt);
	return false;
}

bool ShareDao::listSharesByUser(int userId, std::vector<ShareListItem>& shares)
{
	const char* sql =
		"SELECT "
		"s.id, s.user_id, s.file_id, f.original_name, "
		"s.share_token, s.created_at, s.expired_at "
		"FROM shares s "
		"LEFT JOIN files f ON s.file_id = f.id "
		"WHERE s.user_id = ? "
		"ORDER BY s.id DESC;";

	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "failed to prepare share listing sql: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}
	sqlite3_bind_int(stmt, 1, userId);
	shares.clear();

	while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		ShareListItem item;

		item.shareId = sqlite3_column_int(stmt, 0);
		item.userId = sqlite3_column_int(stmt, 1);
		item.fileId = sqlite3_column_int(stmt, 2);

		const unsigned char* originalNameText = sqlite3_column_text(stmt, 3);
		const unsigned char* shareTokenText = sqlite3_column_text(stmt, 4);
		const unsigned char* createdAtText = sqlite3_column_text(stmt, 5);
		const unsigned char* expiredAtText = sqlite3_column_text(stmt, 6);

		item.originalName = originalNameText
			? reinterpret_cast<const char*>(originalNameText)
			: "";

		item.shareToken = shareTokenText
			? reinterpret_cast<const char*>(shareTokenText)
			: "";

		item.createdAt = createdAtText
			? reinterpret_cast<const char*>(createdAtText)
			: "";

		item.expiredAt = expiredAtText
			? reinterpret_cast<const char*>(expiredAtText)
			: "";

		shares.push_back(item);
	}

	if (result != SQLITE_DONE)
	{
		std::cerr << "Failed to execute share listing SQL: "
			<< sqlite3_errmsg(db_) << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}
bool ShareDao::getShareById(int shareId, ShareInfo& share)
{
	const char* sql =
		"SELECT id, user_id, file_id, share_token, created_at, expired_at "
		"FROM shares "
		"WHERE id = ?;";

	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare get share SQL: "
			<< sqlite3_errmsg(db_) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, shareId);

	result = sqlite3_step(stmt);

	if (result != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return false;
	}

	share.id = sqlite3_column_int(stmt, 0);
	share.userId = sqlite3_column_int(stmt, 1);
	share.fileId = sqlite3_column_int(stmt, 2);

	const unsigned char* shareTokenText = sqlite3_column_text(stmt, 3);
	const unsigned char* createdAtText = sqlite3_column_text(stmt, 4);
	const unsigned char* expiredAtText = sqlite3_column_text(stmt, 5);

	share.shareToken = shareTokenText
		? reinterpret_cast<const char*>(shareTokenText)
		: "";

	share.createdAt = createdAtText
		? reinterpret_cast<const char*>(createdAtText)
		: "";

	share.expiredAt = expiredAtText
		? reinterpret_cast<const char*>(expiredAtText)
		: "";

	sqlite3_finalize(stmt);
	return true;
}
bool ShareDao::deleteShareByIdAndUserId(int shareId, int userId)
{
	const char* sql =
		"DELETE FROM shares "
		"WHERE id = ? AND user_id = ?;";

	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare delete share SQL: "
			<< sqlite3_errmsg(db_) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, shareId);
	sqlite3_bind_int(stmt, 2, userId);

	result = sqlite3_step(stmt);

	if (result != SQLITE_DONE)
	{
		std::cerr << "Failed to execute delete share SQL: "
			<< sqlite3_errmsg(db_) << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}

	int changedRows = sqlite3_changes(db_);

	sqlite3_finalize(stmt);

	return changedRows > 0;
}
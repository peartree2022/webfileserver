#include "dao/FileDao.h"
#include <iostream>

FileDao::FileDao(sqlite3* db) : db_(db)
{
}

bool FileDao::createFile(
	int userId,
	const std::string& originalName,
	const std::string& storedName,
	const std::string& filePath,
	long long fileSize,
	int& fileId
)
{
	const char* sql =
		"INSERT INTO files "
		"(user_id, original_name, stored_name, file_path, file_size, created_at) "
		"VALUES (?, ?, ?, ?, ?, datetime('now', 'localtime'));";

	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare file creation SQL: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, userId);
	sqlite3_bind_text(stmt, 2, originalName.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, storedName.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 4, filePath.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(stmt, 5, fileSize);

	int stepResult = sqlite3_step(stmt);
	if (stepResult != SQLITE_DONE)
	{
		std::cerr << "Failed to execute file creation SQL: " << sqlite3_errmsg(db_) << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}

	fileId = static_cast<int>(sqlite3_last_insert_rowid(db_));
	sqlite3_finalize(stmt);
	return true;
}

bool FileDao::listFilesByUser(int userId, std::vector<FileInfo>& files)
{
	const char* sql = 
		"SELECT id, user_id, original_name, stored_name, file_path, file_size, created_at "
		"FROM files "
		"WHERE user_id = ? "
		"ORDER BY id DESC;";

	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare file listing SQL: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, userId);
	files.clear();

	while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		FileInfo file;
		file.id = sqlite3_column_int(stmt, 0);
		file.userId = sqlite3_column_int(stmt, 1);
		const unsigned char* originalNameText = sqlite3_column_text(stmt, 2);
		const unsigned char* storedNameText = sqlite3_column_text(stmt, 3);
		const unsigned char* filePathText = sqlite3_column_text(stmt, 4);
		file.fileSize = sqlite3_column_int64(stmt, 5);
		const unsigned char* createdAtText = sqlite3_column_text(stmt, 6);

		file.originalName = originalNameText ? reinterpret_cast<const char*>(originalNameText) : "";
		file.storedName = storedNameText ? reinterpret_cast<const char*>(storedNameText) : "";
		file.filePath = filePathText ? reinterpret_cast<const char*>(filePathText) : "";

		file.createdAt = createdAtText ? reinterpret_cast<const char*>(createdAtText) : "";
		files.push_back(file);
	}

	if (result != SQLITE_DONE)
	{
		std::cerr << "Failed to execute file listing SQL: " << sqlite3_errmsg(db_) << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}

bool FileDao::getFileById(int fileId, FileInfo& file)
{
	const char* sql =
		"SELECT id, user_id, original_name, stored_name, file_path, file_size, created_at "
		"FROM files "
		"WHERE id = ?;";
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to prepare file query SQL: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, fileId);
	result = sqlite3_step(stmt);
	if (result == SQLITE_ROW)
	{
		file.id = sqlite3_column_int(stmt, 0);
		file.userId = sqlite3_column_int(stmt, 1);
		const unsigned char* originalNameText = sqlite3_column_text(stmt, 2);
		const unsigned char* storedNameText = sqlite3_column_text(stmt, 3);
		const unsigned char* filePathText = sqlite3_column_text(stmt, 4);
		const unsigned char* createdAtText = sqlite3_column_text(stmt, 6);

		file.originalName = originalNameText ? reinterpret_cast<const char*>(originalNameText) : "";
		file.storedName = storedNameText ? reinterpret_cast<const char*>(storedNameText) : "";
		file.filePath = filePathText ? reinterpret_cast<const char*>(filePathText) : "";
		file.fileSize = static_cast<long long>(sqlite3_column_int64(stmt, 5));
		file.createdAt = createdAtText ? reinterpret_cast<const char*>(createdAtText) : "";
		sqlite3_finalize(stmt);
		return true;
	}
	if (result != SQLITE_DONE)
	{
		std::cerr << "Failed to execute file query SQL: " << sqlite3_errmsg(db_) << std::endl;
	}
	sqlite3_finalize(stmt);
	return false;
}


bool FileDao::deleteFileById(int fileId)
{
	const char* sql =
		"DELETE FROM files "
		"WHERE id = ?;";

	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "failed to prepare delete file sql: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}
	sqlite3_bind_int(stmt, 1, fileId);
	result = sqlite3_step(stmt);
	if (result != SQLITE_DONE)
	{
		std::cerr << "failed to execute delete file sql: " << sqlite3_errmsg(db_) << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}

	int changedRows = sqlite3_changes(db_);
	sqlite3_finalize(stmt);
	return changedRows > 0;
}
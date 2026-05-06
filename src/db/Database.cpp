#include "db/Database.h"
#include <iostream>
#include <filesystem>

Database::Database(const std::string& dbPath) : dbPath_(dbPath), db_(nullptr)
{
}

Database::~Database()
{
	if (db_)
	{
		sqlite3_close(db_);
		db_ = nullptr;
	}
}

bool Database::open()
{
	std::filesystem::path path(dbPath_);
	std::filesystem::path parentPath = path.parent_path();

	if (!parentPath.empty())
	{
		std::filesystem::create_directories(parentPath);
	}
	int result = sqlite3_open(dbPath_.c_str(), &db_);
	if (result != SQLITE_OK)
	{
		std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << std::endl;
		return false;
	}
	std::cout << "Database opened successfully: " << dbPath_ << std::endl;
	return true;
}

bool Database::execute(const std::string& sql)
{
	char* errMsg = nullptr;
	int result = sqlite3_exec(db_, 
							  sql.c_str(), 
							  nullptr, 
							  nullptr, 
							  &errMsg);
	if (result != SQLITE_OK)
	{
		std::cerr << "SQL error: " << errMsg << std::endl;
		sqlite3_free(errMsg);
		return false;
	}
	return true;
}

bool Database::initDatabase()
{
	const std::string createUsersTableSql = R"(
		CREATE TABLE IF NOT EXISTS users (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			username TEXT NOT NULL UNIQUE,
			password_hash TEXT NOT NULL,
			created_at TEXT NOT NULL
		);
	)";
	if (!execute(createUsersTableSql))
	{
		std::cerr << "Failed to create users table" << std::endl;
		return false;
	}

	const std::string createTokensTableSql = R"(
		CREATE TABLE IF NOT EXISTS tokens (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			user_id INTEGER NOT NULL,
			token TEXT NOT NULL UNIQUE,
			created_at TEXT NOT NULL,
			expired_at TEXT NOT NULL
		);
	)";

	if (!execute(createTokensTableSql))
	{
		std::cerr << "Failed to create tokens table" << std::endl;
		return false;
	}
	

	const std::string createFilesTableSql = R"(
		CREATE TABLE IF NOT EXISTS files (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			user_id INTEGER NOT NULL,
			original_name TEXT NOT NULL,
			stored_name TEXT NOT NULL,
			file_size INTEGER NOT NULL,
			file_path TEXT NOT NULL,
			created_at TEXT NOT NULL
		);
	)";

	if (!execute(createFilesTableSql))
	{
		std::cerr << "Failed to create files table" << std::endl;
		return false;
	}

	

	const std::string createSharesTableSql = R"(
		CREATE TABLE IF NOT EXISTS shares (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			user_id INTEGER NOT NULL,
			file_id INTEGER NOT NULL,
			share_token TEXT NOT NULL UNIQUE,
			created_at TEXT NOT NULL,
			expired_at TEXT NOT NULL
		);
	)";
	if (!execute(createSharesTableSql))
	{
		std::cerr << "failed to create shares table" << std::endl;
		return false;
	}

	std::cout << "Database initialized successfully" << std::endl;
	return true;
}


sqlite3* Database::getConnection() const
{
	return db_;
}
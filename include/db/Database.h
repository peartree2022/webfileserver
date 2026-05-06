#pragma once
#include "sqlite3.h"
#include <string>

class Database
{
public:
	explicit Database(const std::string& dbPath);
	~Database();

	bool open();
	bool initDatabase();
	bool execute(const std::string& sql);

	sqlite3* getConnection() const;

private:
	std::string dbPath_;
	sqlite3* db_;
};
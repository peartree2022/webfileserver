#pragma once
#include "sqlite3.h"
#include <string>
#include <vector>


struct FileInfo
{
	int id;
	int userId;
	std::string originalName;
	std::string storedName;
	std::string filePath;
	long long fileSize;
	std::string createdAt;
};

class FileDao
{
public:
	explicit FileDao(sqlite3* db);

	bool createFile(
		int userId,
		const std::string& originalName,
		const std::string& storedName,
		const std::string& filePath,
		long long fileSize,
		int& fileId
	);

	bool listFilesByUser(int userId, std::vector<FileInfo>& files);
	bool getFileById(int fileId, FileInfo& file);
	bool deleteFileById(int fileId);

private:
	sqlite3* db_;
};
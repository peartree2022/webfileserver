#pragma once
#include "sqlite3.h"
#include <string>
#include <vector>


struct ShareInfo
{
	int id;
	int userId;
	int fileId;
	std::string shareToken;
	std::string createdAt;
	std::string expiredAt;
};

struct ShareListItem
{
	int shareId;
	int userId;
	int fileId;
	std::string originalName;
	std::string shareToken;
	std::string createdAt;
	std::string expiredAt;
};
class ShareDao
{
public:
	explicit ShareDao(sqlite3* db);

	bool createShare(
		int userId,
		int fileId,
		const std::string& shareToken,
		int& shareId
	);

	bool getShareByToken(
		const std::string& token,
		ShareInfo& share
	);

	bool listSharesByUser(int userId, std::vector<ShareListItem>& shares);
	bool getShareById(int shareId, ShareInfo& share);
	bool deleteShareByIdAndUserId(int shareId, int userId);

private:
	sqlite3* db_;
};
#pragma once
#include "httplib.h"
#include "dao/UserDao.h"
#include "dao/TokenDao.h"
#include "dao/FileDao.h"
#include "dao/ShareDao.h"
#include "sqlite3.h"

class Server
{
private:
	void setRoutes();
	bool getUserIdFromRequest(
		const httplib::Request& req,
		httplib::Response& res,
		int& userId
	);
	bool getTokenFromRequest(
		const httplib::Request& req,
		httplib::Response& res,
		std::string& token
	);

private:
	httplib::Server server_;
	UserDao userDao_;
	TokenDao tokenDao_;
	FileDao fileDao_;
	ShareDao shareDao_;

public:
	explicit Server(sqlite3* db);
	void start(int port);
	
};
#pragma once
#include "AppConfig.h"
#include<memory>
#include<mysql_connection.h>

class MySqlDatabase
{
public:
	MySqlDatabase() = default;
	~MySqlDatabase() = default;

	bool connect(const MySqlConfig& config);
	sql::Connection* connection();
	
	bool isConnect() const;

private:
	std::unique_ptr<sql::Connection> conn_;
};
#include "db/MySqlDatabase.h"
#include <iostream>
#include <mysql_driver.h>
#include <cppconn/exception.h>

bool MySqlDatabase::connect(const MySqlConfig& config)
{
	try
	{
		sql::mysql::MySQL_Driver* driver =
			sql::mysql::get_mysql_driver_instance();
		conn_.reset(
			driver->connect(
				config.host,
				config.user,
				config.password
			)
		);
		conn_->setSchema(config.database);

		std::cout << "mysql connected successfully: "
			<< config.host << "/"
			<< config.database
			<< std::endl;

		return true;
	}
	catch (const sql::SQLException& e)
	{
		std::cerr << "mysql connected failed: "
			<< e.what()
			<< ", error code: "
			<< e.getErrorCode()
			<< ", sqlstate: "
			<< std::endl;

		return false;
	}
}
sql::Connection* MySqlDatabase::connection()
{
	return conn_.get();
}

bool MySqlDatabase::isConnect() const
{
	return conn_ != nullptr && !conn_->isClosed();
}
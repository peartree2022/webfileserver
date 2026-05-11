#include "AppConfig.h"
#include "db/MySqlDatabase.h"
#include <iostream>
#include <exception>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>


int main(int argc, char* argv[])
{
	try
	{
		std::string configPath = "D:/visualstudio_project/myproject/WebFileServer/config/app.json";

		if (argc >= 2)
		{
			configPath = argv[1];
		}

		AppConfig config = AppConfig::loadFromFile(configPath);

		MySqlDatabase db;

		if (!db.connect(config.mysql()))
		{
			std::cerr << "failed to connect mysql: " << std::endl;
			return 1;
		}

		std::unique_ptr<sql::Statement> stmt(db.connection()->createStatement());

		std::unique_ptr<sql::ResultSet> result(stmt->executeQuery("SELECT DATABASE() AS db_name"));

		if (result->next())
		{
			std::cout << "current database: "
				<< result->getString("db_name")
				<< std::endl;
		}
		std::cout << "mysqldatabase test success" << std::endl;
		return 0;
	}
	catch (const sql::SQLException& e)
	{
		std::cerr << "sql error: "
			<< e.what()
			<< ", error code: "
			<< e.getErrorCode()
			<< ", sqlstate: "
			<< e.getSQLState()
			<< std::endl;

		return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "test failed: "
			<< e.what()
			<< std::endl;
		return 1;
	}
}
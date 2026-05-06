#include "Server.h"
#include "db/Database.h"
#include <iostream>

int main()
{
	Database database("data/webfiles.db");
	if (!database.open())
	{
		std::cerr << "Failed to open database" << std::endl;
		return 1;
	}
	if (!database.initDatabase())
	{
		std::cerr << "Failed to initialize database" << std::endl;
		return 1;
	}
	Server server(database.getConnection());
	server.start(8080);
	return 0;
}
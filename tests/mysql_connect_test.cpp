#include <iostream>
#include <memory>

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

int main()
{
    try
    {
        sql::mysql::MySQL_Driver* driver =
            sql::mysql::get_mysql_driver_instance();

        std::unique_ptr<sql::Connection> conn(
            driver->connect(
                "tcp://127.0.0.1:3306",
                "webfile_user",
                "WebFile@123456"
            )
        );

        conn->setSchema("webfileserver");

        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> result(
            stmt->executeQuery("SELECT 1 AS ok")
        );

        if (result->next())
        {
            std::cout << "MySQL connected successfully, test value = "
                << result->getInt("ok")
                << std::endl;
        }

        return 0;
    }
    catch (const sql::SQLException& e)
    {
        std::cerr << "MySQL connection failed: "
            << e.what()
            << ", error code: "
            << e.getErrorCode()
            << ", SQLState: "
            << e.getSQLState()
            << std::endl;
        return 1;
    }
}
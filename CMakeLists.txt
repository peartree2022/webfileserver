cmake_minimum_required(VERSION 3.16)
project(WebFileServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===============================
# MySQL Connector/C++
# ===============================
set(MYSQL_CONNECTOR_ROOT "D:/mysql-connector-cpp")
set(MYSQL_CONNECTOR_INCLUDE "${MYSQL_CONNECTOR_ROOT}/include/jdbc")
set(MYSQL_CONNECTOR_LIB "${MYSQL_CONNECTOR_ROOT}/lib64/vs14/mysqlcppconn.lib")
set(MYSQL_CONNECTOR_DLL "${MYSQL_CONNECTOR_ROOT}/lib64/mysqlcppconn-10-vs14.dll")

# ===============================
# Main Server
# ===============================
add_executable(WebFileServer
    main.cpp

    src/AppConfig.cpp
    src/Server.cpp
    src/JsonUtil.cpp

    src/db/Database.cpp
    src/db/MySqlDatabase.cpp

    src/dao/UserDao.cpp
    src/dao/TokenDao.cpp
    src/dao/FileDao.cpp
    src/dao/ShareDao.cpp

    src/util/TokenUtil.cpp
    src/util/RequestUtil.cpp
    src/util/PasswordUtil.cpp

    third_party/sqlite3.c
)

target_include_directories(WebFileServer PRIVATE
    include
    third_party
    "${MYSQL_CONNECTOR_INCLUDE}"
)

target_link_libraries(WebFileServer PRIVATE
    "${MYSQL_CONNECTOR_LIB}"
    bcrypt
)

add_custom_command(TARGET WebFileServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${MYSQL_CONNECTOR_DLL}"
    $<TARGET_FILE_DIR:WebFileServer>
)

# ===============================
# MySQL Connection Test
# ===============================
add_executable(MySqlConnectTest
    tests/mysql_connect_test.cpp
)

target_include_directories(MySqlConnectTest PRIVATE
    "${MYSQL_CONNECTOR_INCLUDE}"
)

target_link_libraries(MySqlConnectTest PRIVATE
    "${MYSQL_CONNECTOR_LIB}"
)

add_custom_command(TARGET MySqlConnectTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${MYSQL_CONNECTOR_DLL}"
    $<TARGET_FILE_DIR:MySqlConnectTest>
)


add_executable(ConfigTest
    tests/config_test
    src/AppConfig.cpp
)

target_include_directories(ConfigTest PRIVATE
    include
    third_party
)

add_executable(MySqlDatabaseTest
    tests/mysql_database_test.cpp
    src/AppConfig.cpp
    src/db/MySqlDatabase.cpp
)

target_include_directories(MySqlDatabaseTest PRIVATE
    include
    third_party
    "${MYSQL_CONNECTOR_INCLUDE}"
)

target_link_libraries(MySqlDatabaseTest PRIVATE
    "${MYSQL_CONNECTOR_LIB}"
)

add_custom_command(TARGET MySqlDatabaseTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${MYSQL_CONNECTOR_DLL}"
    $<TARGET_FILE_DIR:MySqlDatabaseTest>
)
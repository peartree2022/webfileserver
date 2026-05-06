cmake_minimum_required(VERSION 3.16)
project(WebFileServer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(WebFileServer 
			   main.cpp 
			   src/Server.cpp 
			   src/JsonUtil.cpp  
			   src/db/Database.cpp
			   src/dao/UserDao.cpp
			   src/dao/TokenDao.cpp
			   src/dao/FileDao.cpp
			   src/dao/ShareDao.cpp
			   src/util/TokenUtil.cpp
			   src/util/RequestUtil.cpp
			   src/util/PasswordUtil.cpp
			   third_party/sqlite3.c
 "include/AppConfig.h")
target_include_directories(WebFileServer PRIVATE 
						   include 
						   third_party
)

target_link_libraries(WebFileServer PRIVATE bcrypt)
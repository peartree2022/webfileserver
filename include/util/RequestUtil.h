#pragma once
#include "httplib.h"
#include "json.hpp"
#include <string>


namespace RequestUtil
{
	bool parseRequestBody(
		const httplib::Request& req, 
		httplib::Response& res, 
		nlohmann::json& body
	);
	bool getRequiredString(
		const nlohmann::json& body, 
		httplib::Response& res, 
		const std::string& fieldName, 
		std::string& value
	);
	bool getRequiredInt(
		const nlohmann::json& body,
		httplib::Response& res,
		const std::string& fieldName,
		int& value
	);
	bool getPositiveInt(
		const nlohmann::json& body,
		httplib::Response& res,
		const std::string& fieldName,
		int& value
	);
	bool getRequiredQueryString(
		const httplib::Request& req,
		httplib::Response& res,
		const std::string& paramName,
		std::string& value
	);
	bool getRequiredQueryInt(
		const httplib::Request& req,
		httplib::Response& res,
		const std::string& paramName,
		int& value
	);
	bool getPositiveQueryInt(
		const httplib::Request& req,
		httplib::Response& res,
		const std::string& paramName,
		int& value
	);
}
#pragma once
#include "Json.hpp"
#include "httplib.h"
#include <string>

namespace JsonUtil
{
	nlohmann::json makeResponse(int code, const std::string& message);
	nlohmann::json makeResponse(int code, const std::string& message, const nlohmann::json& data);

	void sendResponse(httplib::Response& res, int code, const std::string& message);
	void sendResponse(httplib::Response& res, int code, const std::string& message, const nlohmann::json& data);
}
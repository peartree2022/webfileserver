#include "JsonUtil.h"

nlohmann::json JsonUtil::makeResponse(int code, const std::string& message)
{
	nlohmann::json response;
	response["code"] = code;
	response["message"] = message;
	return response;
}

nlohmann::json JsonUtil::makeResponse(int code, const std::string& message, const nlohmann::json& data)
{
	nlohmann::json response;
	response["code"] = code;
	response["message"] = message;
	response["data"] = data;
	return response;
}
void JsonUtil::sendResponse(httplib::Response& res, int code, const std::string& message)
{
	nlohmann::json response = makeResponse(code, message);
	res.set_content(response.dump(4), "application/json");
}
void JsonUtil::sendResponse(httplib::Response& res, int code, const std::string& message, const nlohmann::json& data)
{
	nlohmann::json response = makeResponse(code, message, data);
	res.set_content(response.dump(4), "application/json");
}
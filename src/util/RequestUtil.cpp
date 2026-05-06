#include "util/RequestUtil.h"
#include "JsonUtil.h"
#include "ErrorCode.h"

namespace RequestUtil
{
	bool parseRequestBody(
		const httplib::Request& req,
		httplib::Response& res,
		nlohmann::json& body
	)
	{
		try
		{
			body = nlohmann::json::parse(req.body);
			return true;
		}
		catch (const std::exception&)
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_JSON, "invalid json");
			
			return false;
		}
	}

	bool getRequiredString(
		const nlohmann::json& body,
		httplib::Response& res,
		const std::string& fieldName,
		std::string& value
	)
	{
		if (!body.contains(fieldName))
		{
			JsonUtil::sendResponse(res, ErrorCode::MISSING_PARAM, fieldName + " is required");
			
			return false;
		}

		if (!body[fieldName].is_string())
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_PARAM, fieldName + " must be string");
			
			return false;
		}

		value = body[fieldName].get<std::string>();

		if (value.empty())
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_PARAM, fieldName + " cannot be empty");
			
			return false;
		}

		return true;
	}

	bool getRequiredInt(
		const nlohmann::json& body,
		httplib::Response& res,
		const std::string& fieldName,
		int& value
	)
	{
		if (!body.contains(fieldName))
		{
			JsonUtil::sendResponse(res, ErrorCode::MISSING_PARAM, fieldName + " is required");
			
			return false;
		}

		if (!body[fieldName].is_number_integer())
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_PARAM, fieldName + " must be integer");
			
			return false;
		}

		value = body[fieldName].get<int>();
		return true;
	}

	bool getPositiveInt(
		const nlohmann::json& body,
		httplib::Response& res,
		const std::string& fieldName,
		int& value
	)
	{
		if (!getRequiredInt(body, res, fieldName, value)) return false;
		if (value <= 0)
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_PARAM, "invalid " + fieldName);
			
			return false;
		}
		return true;
	}
	bool getRequiredQueryString(
		const httplib::Request& req,
		httplib::Response& res,
		const std::string& paramName,
		std::string& value
	)
	{
		if (!req.has_param(paramName))
		{
			JsonUtil::sendResponse(res, ErrorCode::MISSING_PARAM, paramName + " is required");
			
			return false;
		}
		value = req.get_param_value(paramName);

		if (value.empty())
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_PARAM, paramName + " cannot be empty");
			
			return false;
		}
		return true;
	}
	bool getRequiredQueryInt(
		const httplib::Request& req,
		httplib::Response& res,
		const std::string& paramName,
		int& value
	)
	{
		std::string text;
		if (!getRequiredQueryString(req, res, paramName, text)) return false;
		try
		{
			size_t pos = 0;
			int parsedValue = std::stoi(text, &pos);
			if (pos != text.size())
			{
				JsonUtil::sendResponse(res, ErrorCode::INVALID_PARAM,
					paramName + " must be integer");
				
				return false;
			}
			value = parsedValue;
			return true;
		}
		catch (const std::exception&)
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_PARAM,
				paramName + " must be integer");
			
			return false;
		}
	}
	bool getPositiveQueryInt(
		const httplib::Request& req,
		httplib::Response& res,
		const std::string& paramName,
		int& value
	)
	{
		if (!getRequiredQueryInt(req, res, paramName, value)) return false;
		if (value <= 0)
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_PARAM, "invalid " + paramName);
			
			return false;
		}
		return true;
	}
}
#include "Server.h"
#include "json.hpp"
#include "JsonUtil.h"
#include "util/TokenUtil.h"
#include "ErrorCode.h"
#include "util/RequestUtil.h"
#include "util/PasswordUtil.h"
#include "AppConfig.h"
#include <iostream>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

Server::Server(sqlite3* db) : userDao_(db), tokenDao_(db), fileDao_(db), shareDao_(db)
{
	setRoutes();
}

bool Server::getUserIdFromRequest(
	const httplib::Request& req,
	httplib::Response& res,
	int& userId
)
{
	std::string token;
	if (!getTokenFromRequest(req, res, token)) return false;


	if (!tokenDao_.getUserIdByToken(token, userId))
	{
		JsonUtil::sendResponse(res, ErrorCode::INVALID_TOKEN, "invalid token");
		return false;
	}
	return true;
}


bool Server::getTokenFromRequest(
	const httplib::Request& req,
	httplib::Response& res,
	std::string& token
)
{
	if (!req.has_header("Authorization"))
	{
		JsonUtil::sendResponse(res, ErrorCode::MISSING_AUTH_HEADER, "missing authorization header");
		return false;
	}

	std::string authHeader = req.get_header_value("Authorization");
	const std::string prefix = "Bearer ";
	
	if (authHeader.rfind(prefix, 0) != 0)
	{
		JsonUtil::sendResponse(res, ErrorCode::INVALID_AUTH_FORMAT, "invalid authorization format");
		return false;
	}

	token = authHeader.substr(prefix.size());
	if (token.empty())
	{
		JsonUtil::sendResponse(res, ErrorCode::TOKEN_EMPTY, "token cannot be empty");
		return false;
	}
	return true;
}

void Server::setRoutes()
{
	server_.Get("/ping", [](const httplib::Request& req, httplib::Response& res) {
		JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "pong");
		});

	server_.Post("/echo", [](const httplib::Request& req, httplib::Response& res) {
		try
		{
			json requestBody = json::parse(req.body);
			JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "Success", requestBody);
		}
		catch (const std::exception&)
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_JSON, "Invalid JSON");
		}
		});

	
	server_.Post("/api/register", [this](const httplib::Request& req, httplib::Response& res) {
		json requestBody;
		if (!RequestUtil::parseRequestBody(req, res, requestBody)) return;
			

		std::string username;
		std::string password;

		if (!RequestUtil::getRequiredString(requestBody, res, "username", username)) return;
		if (!RequestUtil::getRequiredString(requestBody, res, "password", password)) return;

		if (userDao_.isUsernameExists(username))
		{
			JsonUtil::sendResponse(res, ErrorCode::USERNAME_EXISTS, "Username already exists");
			return;
		}
		std::string passwordHash = PasswordUtil::hashPassword(password);
		if (passwordHash.empty())
		{
			JsonUtil::sendResponse(res, ErrorCode::CREATE_USER_FAILED, "failed to hash password");
			return;
		}

		if (!userDao_.createUser(username, passwordHash))
		{
			JsonUtil::sendResponse(res, ErrorCode::CREATE_USER_FAILED, "Failed to create user");
			return;
		}

		json data;
		data["username"] = username;

		JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "Register success", data);
		
		
		});
	server_.Post("/api/login", [this](const httplib::Request& req, httplib::Response& res) {
		
		json requestBody;
		if (!RequestUtil::parseRequestBody(req, res, requestBody)) return;

		std::string username;
		std::string password;

		if (!RequestUtil::getRequiredString(requestBody, res, "username", username)) return;
		if (!RequestUtil::getRequiredString(requestBody, res, "password", password)) return;


		int userId = 0;
		std::string	passwordHash;
		if (!userDao_.getUsername(username, userId, passwordHash))
		{
			JsonUtil::sendResponse(res, ErrorCode::USER_NOT_FOUND, "user not found");
			return;
		}

		if (!PasswordUtil::verifyPassword(password, passwordHash))
		{
			JsonUtil::sendResponse(res, ErrorCode::INCORRECT_PASSWORD, "Incorrect password");
			return;
		}

		std::string token = TokenUtil::generateToken();
		if (!tokenDao_.createToken(userId, token))
		{
			JsonUtil::sendResponse(res, ErrorCode::CREATE_TOKEN_FAILED, "Failed to create token");
			return;
		}

		json data;
		data["user_id"] = userId;
		data["username"] = username;
		data["token"] = token;
		JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "Login success", data);
		});
	server_.Post("/api/file/upload", [this](const httplib::Request& req, httplib::Response& res) {
		int userId = 0;
		if (!getUserIdFromRequest(req, res, userId)) return;
		

		if (!req.is_multipart_form_data())
		{
			JsonUtil::sendResponse(res, ErrorCode::NOT_MULTIPART_FORM, "request is not multipart form data");
			return;
		}

		if (!req.form.has_file("file"))
		{
			JsonUtil::sendResponse(res, ErrorCode::MISSING_UPLOAD_FILE, "missing upload file");
			
			return;
		}

		const auto file = req.form.get_file("file");
		if (file.content.empty())
		{
			JsonUtil::sendResponse(res, ErrorCode::FILE_EMPTY, "file is empty");
			
			return;
		}
		if (file.content.size() > AppConfig::MAX_UPLOAD_FILE_SIZE)
		{
			JsonUtil::sendResponse(res, ErrorCode::FILE_TOO_LARGE, "file is too large");
			return;
		}

		std::string originalName = std::filesystem::path(file.filename).filename().string();
		if (originalName.empty())
		{
			originalName = "upload.bin";
		}

		std::string storeName = TokenUtil::generateToken(16) + "_" + originalName;
		std::filesystem::path userDir = std::filesystem::path("data") / "files" / std::to_string(userId);
		std::filesystem::create_directories(userDir);
		std::filesystem::path savePath = userDir / storeName;
		std::ofstream ofs(savePath, std::ios::binary);

		if (!ofs.is_open())
		{
			JsonUtil::sendResponse(res, ErrorCode::SAVE_FILE_FAILED, "failed to save file");
			return;
		}

		ofs.write(file.content.data(), static_cast<std::streamsize>(file.content.size()));
		ofs.close();

		int fileId = 0;
		if (!fileDao_.createFile(
			userId,
			originalName,
			storeName,
			savePath.string(),
			file.content.size(),
			fileId
		))
		{
			std::filesystem::remove(savePath);
			JsonUtil::sendResponse(res, ErrorCode::SAVE_FILE_RECORD_FAILED, "failed to save file record");
			return;
		}

		json data;
		data["user_id"] = userId;
		data["file_id"] = fileId;
		data["original_name"] = originalName; 
		data["stored_name"] = storeName;
		data["file_size"] = file.content.size();
		data["file_path"] = savePath.string();

		JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "upload success", data);
		
		});

	server_.Get("/api/me", [this](const httplib::Request& req, httplib::Response& res) {
		int userId = 0;
		if (!getUserIdFromRequest(req, res, userId)) return;

		json data;
		data["user_id"] = userId;

		JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "Token is valid", data);
		
		});

	server_.Get("/api/file/list", [this](const httplib::Request& req, httplib::Response& res) {
		int userId = 0;
		if (!getUserIdFromRequest(req, res, userId)) return;

		std::vector<FileInfo> files;
		if (!fileDao_.listFilesByUser(userId, files))
		{
			JsonUtil::sendResponse(res, ErrorCode::LIST_FILE_FAILED, "failed to get files");
			
			return;
		}

		json data = json::array();
		for (const auto& file : files)
		{
			json item;
			item["file_id"] = file.id;
			item["original_name"] = file.originalName;
			item["file_size"] = file.fileSize;
			item["created_at"] = file.createdAt;
			data.push_back(item);
		}
		JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "file list", data);
		
		});
	server_.Get("/api/file/download", [this](const httplib::Request& req, httplib::Response& res) {
		int userId = 0;
		if (!getUserIdFromRequest(req, res, userId)) return;
		int fileId = 0;
		if (!RequestUtil::getPositiveQueryInt(req, res, "file_id", fileId)) return;
		FileInfo file;
		if (!fileDao_.getFileById(fileId, file))
		{
			JsonUtil::sendResponse(res, ErrorCode::FILE_NOT_FOUND, "file not found");
			
			return;
		}
		if (file.userId != userId)
		{
			JsonUtil::sendResponse(res, ErrorCode::PERMISSION_DENIED, "access denied");
			
			return;
		}
		if (!std::filesystem::exists(file.filePath))
		{
			JsonUtil::sendResponse(res, ErrorCode::FILE_NOT_FOUND, "file not found on server");
			
			return;
		}
		res.set_header(
			"Content-Disposition",
			"attachment; filename=\"" + file.originalName + "\""
		);
		res.set_file_content(file.filePath, "application/octet-stream");
	});
	server_.Post("/api/file/delete", [this](const httplib::Request& req, httplib::Response& res) {
		int userId = 0;
		if (!getUserIdFromRequest(req, res, userId)) return;
		
		json requestBody;
		if (!RequestUtil::parseRequestBody(req, res, requestBody)) return;

		int fileId = 0;
		if (!RequestUtil::getPositiveInt(requestBody, res, "file_id", fileId)) return;


		FileInfo file;
		if (!fileDao_.getFileById(fileId, file))
		{
			JsonUtil::sendResponse(res, ErrorCode::FILE_NOT_FOUND, "file not found");
			return;
		}
		if (file.userId != userId)
		{
			JsonUtil::sendResponse(res, ErrorCode::PERMISSION_DENIED, "permission denied");
			
			return;
		}

		if (std::filesystem::exists(file.filePath))
		{
			std::error_code ec;
			bool removed = std::filesystem::remove(file.filePath, ec);
			if (!removed || ec)
			{
				JsonUtil::sendResponse(res, ErrorCode::DELETE_FILE_FAILED, "failed to delete file from disk");
				
				return;
			}
		}

		if (!fileDao_.deleteFileById(fileId))
		{
			JsonUtil::sendResponse(res, ErrorCode::DELETE_FILE_RECORD_FAILED, "failed to delete file record");
			
			return;
		}

		json data;
		data["file_id"] = fileId;
		data["original_name"] = file.originalName;
		JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "delete success", data);
		
	});
	server_.Post("/api/file/share", [this](const httplib::Request& req, httplib::Response& res) {
		int userId = 0;
		if (!getUserIdFromRequest(req, res, userId)) return;

		json requestBody;
		if (!RequestUtil::parseRequestBody(req, res, requestBody)) return;

		int fileId = 0;
		if (!RequestUtil::getPositiveInt(requestBody, res, "file_id", fileId)) return;

		FileInfo file;
		if (!fileDao_.getFileById(fileId, file))
		{
			JsonUtil::sendResponse(res, ErrorCode::FILE_NOT_FOUND, "file not found");
			
			return;
		}

		if (file.userId != userId)
		{
			JsonUtil::sendResponse(res, ErrorCode::PERMISSION_DENIED, "permission denied");
			
			return;
		}

		std::string shareToken = TokenUtil::generateToken(32);
		int shareId = 0;

		if (!shareDao_.createShare(userId, fileId, shareToken, shareId))
		{
			JsonUtil::sendResponse(res, ErrorCode::CREATE_SHARE_FAILED, "failed to create share");
			
			return;
		}

		std::string shareUrl = "http://127.0.0.1:8080/api/share/download?share_token=" + shareToken;

		json data;
		data["share_id"] = shareId;
		data["file_id"] = fileId;
		data["original_name"] = file.originalName;
		data["share_token"] = shareToken;
		data["share_url"] = shareUrl;

		JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "share success", data);
		
	});

	server_.Get("/api/share/list", [this](const httplib::Request& req, httplib::Response& res) {
		int userId = 0;

		if (!getUserIdFromRequest(req, res, userId))
		{
			return;
		}

		std::vector<ShareListItem> shares;

		if (!shareDao_.listSharesByUser(userId, shares))
		{
			JsonUtil::sendResponse(
				res,
				ErrorCode::LIST_SHARE_FAILED,
				"failed to get shares"
			);
			return;
		}

		json data = json::array();

		for (const auto& share : shares)
		{
			std::string shareUrl =
				"http://127.0.0.1:8080/api/share/download?share_token="
				+ share.shareToken;

			json item;
			item["share_id"] = share.shareId;
			item["file_id"] = share.fileId;
			item["original_name"] = share.originalName;
			item["share_token"] = share.shareToken;
			item["share_url"] = shareUrl;
			item["created_at"] = share.createdAt;
			item["expired_at"] = share.expiredAt;

			data.push_back(item);
		}

		JsonUtil::sendResponse(
			res,
			ErrorCode::SUCCESS,
			"share list",
			data
		);
		});

	server_.Get("/api/share/download", [this](const httplib::Request& req, httplib::Response& res) {

		std::string shareToken;
		if (!RequestUtil::getRequiredQueryString(req, res, "share_token", shareToken)) return;

		ShareInfo share;
		if (!shareDao_.getShareByToken(shareToken, share))
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_SHARE_TOKEN, "invalid or expired share token");
			
			return;
		}

		FileInfo file;
		if (!fileDao_.getFileById(share.fileId, file))
		{
			JsonUtil::sendResponse(res, ErrorCode::FILE_NOT_FOUND, "file not found");
			
			return;
		}

		if (!std::filesystem::exists(file.filePath))
		{
			JsonUtil::sendResponse(res, ErrorCode::FILE_NOT_FOUND, "file not found on server");
			
			return;
		}

		res.set_header(
			"Content-Disposition",
			"attachment; filename=\"" + file.originalName + "\""
		);
		res.set_file_content(file.filePath, "application/octet-stream");
	});

	server_.Post("/api/share/cancel", [this](const httplib::Request& req, httplib::Response& res) {
		int userId = 0;

		if (!getUserIdFromRequest(req, res, userId))
		{
			return;
		}

		json requestBody;

		if (!RequestUtil::parseRequestBody(req, res, requestBody))
		{
			return;
		}

		int shareId = 0;

		if (!RequestUtil::getPositiveInt(requestBody, res, "share_id", shareId))
		{
			return;
		}

		ShareInfo share;

		if (!shareDao_.getShareById(shareId, share))
		{
			JsonUtil::sendResponse(
				res,
				ErrorCode::SHARE_NOT_FOUND,
				"share not found"
			);
			return;
		}

		if (share.userId != userId)
		{
			JsonUtil::sendResponse(
				res,
				ErrorCode::PERMISSION_DENIED,
				"permission denied"
			);
			return;
		}

		if (!shareDao_.deleteShareByIdAndUserId(shareId, userId))
		{
			JsonUtil::sendResponse(
				res,
				ErrorCode::CANCEL_SHARE_FAILED,
				"failed to cancel share"
			);
			return;
		}

		json data;
		data["share_id"] = shareId;

		JsonUtil::sendResponse(
			res,
			ErrorCode::SUCCESS,
			"cancel share success",
			data
		);
		});

	server_.Post("/api/logout", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token;
		if (!getTokenFromRequest(req, res, token)) return;

		int userId = 0;
		if (!tokenDao_.getUserIdByToken(token, userId))
		{
			JsonUtil::sendResponse(res, ErrorCode::INVALID_TOKEN, "invalid token");
			return;
		}
		if (!tokenDao_.deleteToken(token))
		{
			JsonUtil::sendResponse(res, ErrorCode::LOGOUT_FAILED, "logout failed");
			return;
		}
		JsonUtil::sendResponse(res, ErrorCode::SUCCESS, "logout success");
	});

	server_.Get("/", [](const httplib::Request& req, httplib::Response& res) {
		if (std::filesystem::exists("./web/index.html"))
		{
			res.set_file_content("./web/index.html", "text/html; charset=utf-8");
		}
		else
		{
			JsonUtil::sendResponse(res, ErrorCode::FILE_NOT_FOUND, "frontend page not found");
		}
		});
	if (!server_.set_mount_point("/static", "./web"))
	{
		std::cerr << "failed to mount static files: ./web" << std::endl;
	}
}

void Server::start(int port)
{
	std::cout << "Server is running at http://127.0.0.1:" << port << std::endl;
	if (!server_.listen("0.0.0.0", port))
	{
		std::cerr << "Failed to start server" << std::endl;
	}
}
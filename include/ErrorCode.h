#pragma once

namespace ErrorCode
{
	constexpr int SUCCESS = 0;

	constexpr int INVALID_JSON = 1001;
	constexpr int MISSING_PARAM = 1002;
	constexpr int INVALID_PARAM = 1003;

	constexpr int MISSING_AUTH_HEADER = 2001;
	constexpr int INVALID_AUTH_FORMAT = 2002;
	constexpr int TOKEN_EMPTY = 2003;
	constexpr int INVALID_TOKEN = 2004;
	constexpr int PERMISSION_DENIED = 2005;
	constexpr int LOGOUT_FAILED = 2006;

	constexpr int USERNAME_EXISTS = 3001;
	constexpr int USER_NOT_FOUND = 3002;
	constexpr int INCORRECT_PASSWORD = 3003;
	constexpr int CREATE_USER_FAILED = 3004;
	constexpr int CREATE_TOKEN_FAILED = 3005;

	constexpr int FILE_NOT_FOUND = 4001;
	constexpr int FILE_EMPTY = 4002;
	constexpr int MISSING_UPLOAD_FILE = 4003;
	constexpr int SAVE_FILE_FAILED = 4004;
	constexpr int SAVE_FILE_RECORD_FAILED = 4005;
	constexpr int DELETE_FILE_FAILED = 4006;
	constexpr int DELETE_FILE_RECORD_FAILED = 4007;
	constexpr int LIST_FILE_FAILED = 4008;
	constexpr int NOT_MULTIPART_FORM = 4009;
	constexpr int FILE_TOO_LARGE = 4010;

	constexpr int CREATE_SHARE_FAILED = 5001;
	constexpr int INVALID_SHARE_TOKEN = 5002;
	constexpr int SHARE_NOT_FOUND = 5003;
	constexpr int LIST_SHARE_FAILED = 5004;
	constexpr int CANCEL_SHARE_FAILED = 5005;
}
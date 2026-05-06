#include "util/TokenUtil.h"
#include <random>

std::string TokenUtil::generateToken(int length)
{
	static const std::string chars = "0123456789"
									 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
									 "abcdefghijklmnopqrstuvwxyz";
	static std::random_device rd;
	static std::mt19937 generator(rd());
	static std::uniform_int_distribution<> distribution(0, static_cast<int>(chars.size() - 1));

	std::string token;
	token.reserve(length);
	
	for (int i = 0; i < length; ++i)
	{
		token.push_back(chars[distribution(generator)]);
	}
	return token;
}
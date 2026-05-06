#include "util/PasswordUtil.h"

#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace
{
	constexpr int PBKDF2_ITERATIONS = 600000;
	constexpr size_t SALT_SIZE = 16;
	constexpr size_t HASH_SIZE = 32;

	bool isSuccess(NTSTATUS status)
	{
		return status >= 0;
	}

	std::vector<unsigned char> generateRandomBytes(size_t size)
	{
		std::vector<unsigned char> bytes(size);
		NTSTATUS status = BCryptGenRandom(
			nullptr,
			bytes.data(),
			static_cast<ULONG>(bytes.size()),
			BCRYPT_USE_SYSTEM_PREFERRED_RNG
		);

		if (!isSuccess(status)) return{};
		return bytes;
	}

	std::string bytesToHex(const std::vector<unsigned char>& bytes)
	{
		std::ostringstream oss;
		for (unsigned char b : bytes)
		{
			oss << std::hex
				<< std::setw(2)
				<< std::setfill('0')
				<< static_cast<int>(b);
		}
		return oss.str();
	}

	bool hexToBytes(
		const std::string& hex,
		std::vector<unsigned char>& bytes
	)
	{
		if (hex.size() % 2 != 0) return false;
		bytes.clear();
		bytes.reserve(hex.size() / 2);
		
		for (size_t i = 0; i < hex.size(); i += 2)
		{
			std::string byteString = hex.substr(i, 2);
			try
			{
				unsigned long value = std::stoul(byteString, nullptr, 16);
				bytes.push_back(static_cast<unsigned char>(value));
			}
			catch (const std::exception&)
			{
				return false;
			}
		}
		return true;
	}

	bool deriveKeyPBKDF2(
		const std::string& password,
		const std::vector<unsigned char>& salt,
		int iterations,
		std::vector<unsigned char>& output
	)
	{
		BCRYPT_ALG_HANDLE hAlg = nullptr;

		NTSTATUS status = BCryptOpenAlgorithmProvider(
			&hAlg,
			BCRYPT_SHA256_ALGORITHM,
			nullptr,
			BCRYPT_ALG_HANDLE_HMAC_FLAG
		);

		if (!isSuccess(status)) return false;

		output.assign(HASH_SIZE, 0);

		status = BCryptDeriveKeyPBKDF2(
			hAlg,
			reinterpret_cast<PUCHAR>(const_cast<char*>(password.data())),
			static_cast<ULONG>(password.size()),
			const_cast<PUCHAR>(salt.data()),
			static_cast<ULONG>(salt.size()),
			static_cast<ULONGLONG>(iterations),
			output.data(),
			static_cast<ULONG>(output.size()),
			0
		);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return isSuccess(status);
	}

	std::vector<std::string> split(
		const std::string& text,
		char delimiter
	)
	{
		std::vector<std::string> parts;
		std::stringstream ss(text);
		std::string item;
		while (std::getline(ss, item, delimiter))
		{
			parts.push_back(item);
		}
		return parts;
	}

	bool constantTimeEqual(
		const std::vector<unsigned char>& a,
		const std::vector<unsigned char>& b
	)
	{
		if (a.size() != b.size()) return false;

		unsigned char diff = 0;
		for (size_t i = 0; i < a.size(); ++i)
		{
			diff |= a[i] ^ b[i];
		}
		return diff == 0;
	}
}

namespace PasswordUtil
{
	std::string hashPassword(const std::string& password)
	{
		std::vector<unsigned char> salt = generateRandomBytes(SALT_SIZE);
		if (salt.empty()) return "";

		std::vector<unsigned char> hash;

		if (!deriveKeyPBKDF2(password, salt, PBKDF2_ITERATIONS, hash)) return "";

		std::string saltHex = bytesToHex(salt);
		std::string hashHex = bytesToHex(hash);

		return "pbkdf2_sha256$" +
			std::to_string(PBKDF2_ITERATIONS) +
			"$" +
			saltHex +
			"$" +
			hashHex;
	}

	bool verifyPassword(
		const std::string& password,
		const std::string& storedHash
	)
	{
		std::vector<std::string> parts = split(storedHash, '$');

		if (parts.size() != 4) return false;
		if (parts[0] != "pbkdf2_sha256") return false;

		int iterations = 0;

		try
		{
			iterations = std::stoi(parts[1]);
		}
		catch (const std::exception&)
		{
			return false;
		}

		if (iterations <= 0) return false;

		std::vector<unsigned char> salt;
		std::vector<unsigned char> expectedHash;

		if (!hexToBytes(parts[2], salt)) return false;
		if (!hexToBytes(parts[3], expectedHash)) return false;

		std::vector<unsigned char> actualHash;
		if (!deriveKeyPBKDF2(password, salt, iterations, actualHash)) return false;
		return constantTimeEqual(actualHash, expectedHash);
	}
}
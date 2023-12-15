#ifndef BINARY_PARSER_HPP
#define BINARY_PARSER_HPP

#include <openssl/sha.h>

#include <fstream>
#include <string>
#include <vector>

#include "ini.h"

class BinaryParser
{
   public:
	enum class Type : uint8_t
	{
		NONE = 0,
		MD80 = 1,
		CANDLE = 2,
		BOOT = 3,
	};

	enum class Status : uint8_t
	{
		OK = 0,
		ERROR_FILE = 1,
		ERROR_TAG = 2,
		ERROR_CHECKSUM = 3,
	};

   public:
	static Status processFile(std::string filePath)
	{
		mINI::INIFile file(filePath);
		mINI::INIStructure ini;

		if (!file.read(ini))
			return Status::ERROR_FILE;

		firmwareEntry1 = parseFirmwareEntry(ini, std::string("header1"));

		if (firmwareEntry1.tag == "md80")
			fileType = Type::MD80;
		else if (firmwareEntry1.tag == "candle")
			fileType = Type::CANDLE;
		else
			return Status::ERROR_TAG;

		if (ini.has("header2"))
		{
			firmwareEntry2 = parseFirmwareEntry(ini, std::string("header2"));
			if (firmwareEntry1.tag == "boot_primary" && firmwareEntry2.tag == "boot_secondary")
				fileType = Type::BOOT;
			else
				return Status::ERROR_TAG;
		}

		if (firmwareEntry1.status == Status::ERROR_CHECKSUM || firmwareEntry2.status == Status::ERROR_CHECKSUM)
			return Status::ERROR_CHECKSUM;

		return Status::OK;
	}

	static std::vector<uint8_t> getPrimaryFirmwareFile()
	{
		return firmwareEntry1.binary;
	}

	static std::vector<uint8_t> getSecondaryFirmwareFile()
	{
		return firmwareEntry2.binary;
	}

	static BinaryParser::Type getFirmwareFileType()
	{
		return fileType;
	}

   private:
	struct FirmwareEntry
	{
		std::string tag;
		size_t size;
		std::string checksum;
		std::vector<uint8_t> binary;
		Status status = Status::OK;
		FirmwareEntry() {} /* this is to fix GCC bug with default initializers */
	};

	inline static FirmwareEntry firmwareEntry1;
	inline static FirmwareEntry firmwareEntry2;
	inline static Type fileType = Type::MD80;

   private:
	static FirmwareEntry parseFirmwareEntry(mINI::INIStructure& ini, std::string&& header)
	{
		FirmwareEntry temp{};
		temp.tag = ini.get(header).get("tag");
		temp.size = stoi(ini.get(header).get("size"));
		temp.checksum = ini.get(header).get("checksum");

		temp.binary = hexStringToBytes(ini.get(header).get("binary"));

		if (!validateChecksum(temp.binary, temp.checksum))
			temp.status = Status::ERROR_CHECKSUM;

		return temp;
	}

	static bool validateChecksum(std::vector<uint8_t>& data, std::string& expectedChecksum)
	{
		SHA256_CTX sha256Context;
		SHA256_Init(&sha256Context);
		SHA256_Update(&sha256Context, data.data(), data.size());
		unsigned char calculateChecksum[SHA256_DIGEST_LENGTH];
		SHA256_Final(calculateChecksum, &sha256Context);

		auto expectedChecksumBytes = hexStringToBytes(expectedChecksum);

		for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++)
		{
			if (expectedChecksumBytes[i] != calculateChecksum[i])
				return false;
		}

		return true;
	}

	static std::vector<uint8_t> hexStringToBytes(std::string str)
	{
		std::vector<uint8_t> result;

		for (size_t i = 0; i < str.length(); i += 2)
		{
			std::string byteString = str.substr(i, 2);
			result.push_back(std::stoi(byteString, nullptr, 16));
		}

		return result;
	}
};

#endif
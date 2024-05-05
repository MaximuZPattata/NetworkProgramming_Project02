#pragma once
#include "pch.h"

// This class is created to hash the raw data provided
class cHash
{
public:
	std::string HashPasswordWithSalt(const std::string& rawPassword, const std::string& rawSalt);
	std::string GenerateRandomString(int length);

private:
	std::string HashRawData(const std::string& rawData);
};


#include "pch.h"
#include "cHash.h"

#include <random>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

// Function to hash the data being sent
// [param_1]: String value is sent as the parameter
// [return_value]: The function also converts the hash data into a string format and returns it
std::string cHash::HashRawData(const std::string& rawData)
{
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, rawData.c_str(), rawData.length());

	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_Final(hash, &sha256);

	//-----Converting the raw binary output of SHA-256 hash to hexadecimal representation--------------
	
	std::stringstream strStream;

	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		strStream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
	}

	return strStream.str();
}
	
// Function to generate random string with each character being either A-Z or a-z or 0-9
// [param_1]: Int value is passed as a parameter
// [return_value]: The function returns a string value
std::string cHash::GenerateRandomString(int length)
{
	//-------Set up the random number generator-------------------

	std::random_device randomizerDev;
	std::mt19937 gen(randomizerDev());

	//----------Defining the character set------------------------

	const std::string charSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

	//-----Create a distribution to get random indices------------

	std::uniform_int_distribution<> dis(0, charSet.size() - 1);

	//----------Generate the random string------------------------

	std::string randomString;

	for (int i = 0; i < length; ++i)
		randomString += charSet[dis(gen)];

	return randomString;
}

// Function to generate the hashed password along with salt
// [param_1]: Raw password is passed as string in a parameter
// [param_2]: Raw salt is passed as string in a parameter
// [return_value]: The function returns the hashed password as a string value
std::string cHash::HashPasswordWithSalt(const std::string& rawPassword, const std::string& rawSalt)
{
	std::string hashedSalt = HashRawData(rawSalt);

	std::string saltedPassword = hashedSalt + rawPassword;

	std::string hashedPassword = HashRawData(saltedPassword);

	return hashedPassword;
}

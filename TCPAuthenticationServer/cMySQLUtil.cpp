#include "pch.h"
#include "cMySQLUtil.h"
#include <iostream>
#include <chrono>
#include <ctime>

#define CATCH_SQL_ERROR() catch (sql::SQLException &e) \
{\
		std::cout << "# ERR: SQLException in " << __FILE__; \
		std::cout << "(" << __FUNCTION__ << ") on line" << __LINE__ << std::endl; \
		std::cout << "#ERR: " << e.what(); \
		std::cout << "(MySQL error code: " << e.getErrorCode(); \
		std::cout << ",SQLState: " << e.getSQLState() << ")" << std::endl; \
\
		printf("SQL CONNECTION ---> FAILED \n"); \
} \

cMySQLUtil::cMySQLUtil()
	: mConnectedToDB(false), mConnection(nullptr), mDriver(nullptr),
	mResultSet(nullptr), mStatement(nullptr)
{

}

cMySQLUtil::~cMySQLUtil()
{
	if (mConnection != nullptr)
		delete mConnection;

	if (mResultSet != nullptr)
		delete mResultSet;

	if (mStatement != nullptr)
		delete mStatement;
}

void cMySQLUtil::ConnectToDatabase(const char* host, const char* userName,
	const char* password, const char* schema)
{
	if (mConnectedToDB)
		return;

	try
	{
		mDriver = sql::mysql::get_mysql_driver_instance();
		mConnection = mDriver->connect(host, userName, password);
		mStatement = mConnection->createStatement();
		mConnection->setSchema(schema);
	}

	CATCH_SQL_ERROR();

	printf("SQL CONNECTION ---> SUCCESS \n");

	mConnectedToDB = true;
}

void cMySQLUtil::Disconnect()
{
	if (!mConnectedToDB)
		return;

	mConnection->close();

	mConnectedToDB = false;
}

sql::PreparedStatement* cMySQLUtil::PrepareStatement(const char* query)
{
	if (!mConnectedToDB)
		return nullptr;

	return mConnection->prepareStatement(query);
}

sql::ResultSet* cMySQLUtil::Select(const char* query)
{
	try
	{
		mResultSet = mStatement->executeQuery(query);
	}
	
	CATCH_SQL_ERROR();

	return mResultSet;
}

int cMySQLUtil::AddUserAccount(const char* Email, const char* Salt, const char* Hashed_Password, int UserId)
{
	int count = 0;

	sql::PreparedStatement* pStatement = PrepareStatement("INSERT INTO web_auth (Email, Salt, Hashed_Password, UserId) VALUES (?, ?, ?, ?);");;

	pStatement->setString(1, Email);
	pStatement->setString(2, Salt);
	pStatement->setString(3, Hashed_Password);
	pStatement->setInt(4, UserId);
	
	try
	{
		count = pStatement->executeUpdate();
	}

	CATCH_SQL_ERROR();

	delete pStatement;

	return count;
}

int cMySQLUtil::AddNewUser()
{
	int count = 0;

	sql::PreparedStatement* pStatement = PrepareStatement("INSERT INTO user (Last_Login, Creation_Date) VALUES (?, ?);");;

	auto currentTimePoint = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(currentTimePoint);
	struct std::tm* timeInfo = std::localtime(&currentTime);

	// Convert to MySQL timestamp and date string 
	char timestampString[20];  // "YYYY-MM-DD HH:MM:SS\0"
	char dateTimeString[20];  // "YYYY-MM-DD HH:MM:SS\0"

	std::strftime(timestampString, sizeof(timestampString), "%F %T", timeInfo);
	std::strftime(dateTimeString, sizeof(dateTimeString), "%F %T", timeInfo);

	pStatement->setString(1, timestampString);
	pStatement->setDateTime(2, dateTimeString);

	try
	{
		count = pStatement->executeUpdate();
	}

	CATCH_SQL_ERROR();

	delete pStatement;

	return count;
}

int cMySQLUtil::UpdateUser(int Id)
{
	int count = 0;
	
	sql::PreparedStatement* pStatement = PrepareStatement("UPDATE user SET Last_Login = ? WHERE Id = ?");

	auto currentTimePoint = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(currentTimePoint);
	struct std::tm* timeInfo = std::localtime(&currentTime);

	// Convert to MySQL timestamp and date string 
	char timestampString[20];  // "YYYY-MM-DD HH:MM:SS\0"

	std::strftime(timestampString, sizeof(timestampString), "%F %T", timeInfo);

	pStatement->setString(1, timestampString);
	pStatement->setInt(2, Id);

	try
	{
		count = pStatement->executeUpdate();
	}

	CATCH_SQL_ERROR();

	return count;
}


#pragma once

#include <mysql/jdbc.h>
#include <map>

class cMySQLUtil
{
public:
	cMySQLUtil();
	~cMySQLUtil();

	void ConnectToDatabase(const char* host, const char* userName, const char* password, const char* schema);
	void Disconnect();
	bool IsConnected() const { return mConnectedToDB; }

	int AddUserAccount(const char* Email, const char* Salt, const char* Hashed_Password, int UserId);
	int AddNewUser();
	int UpdateUser(int Id);

	sql::ResultSet* Select(const char* query);
	sql::PreparedStatement* PrepareStatement(const char* query);

private:

	sql::mysql::MySQL_Driver* mDriver;
	sql::Connection* mConnection;
	sql::Statement* mStatement;
	sql::ResultSet* mResultSet;

	bool mConnectedToDB;
};


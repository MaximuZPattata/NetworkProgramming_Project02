#include "pch.h"
#include "cNetworkManager.h"

#include <regex>
#include <AuthProtocolMessages.pb.h>

//Constructor
cNetworkManager::cNetworkManager()
{ }

//Destructor
cNetworkManager::~cNetworkManager()
{ }

// Function to send a response message to the server.
// [param_1]: Reference to the sChatMessage for message details.
// [param_2]: Reference to the cBuffer instance for message storage.
// [param_3]: Message to send.
// [param_4]: Reference to the Server socket instance
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::WriteResponseToChatServer(std::string serializedMessage)
{
	int sendResult = send(chatServerSocket, serializedMessage.c_str(), serializedMessage.size(), 0);

	if (sendResult == SOCKET_ERROR)
	{
		printf("Message sending ---> FAILED | ERROR : %d\n", WSAGetLastError());
		CleanSocket(chatServerSocket, info);
		return sendResult;
	}

	return 0;
}

// Function to close and clean up the socket and Windows Sockets API.
// [param_1]: Reference to the Server socket instance.
// [param_2]: Address information.
void cNetworkManager::CleanSocket(SOCKET& serverSocket, PADDRINFOA info)
{
	closesocket(serverSocket);
	freeaddrinfo(info);
	WSACleanup();
}

// Function to initialize the Windows Sockets API (WSA).
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::InitializeWSA()
{
	WSADATA wsaData;
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0)
	{
		printf("WSA Startup ---> FAILED | ERROR : %d\n", result);
		return 1;
	}

	printf("WSA Startup ---> SUCCESS \n");
	return 0;
}

// Function to initialize address information.
// [param_1]: IP address as a string 
// [param_2]: Port number as a string
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::InitializeAddrInfo(const char* ipaddress, const char* port)
{
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int result = getaddrinfo(ipaddress, port, &hints, &info);

	if (result != 0)
	{
		printf("getaddrinfo ---> FAILED | ERROR : %d\n", result);
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}
	printf("getaddrinfo ---> SUCCESS \n");
	return 0;
}

// Function to create a socket.
// [param_1]: Reference to the server socket instance.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::CreateSocket(SOCKET& serverSocket)
{
	serverSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

	if (serverSocket == INVALID_SOCKET)
	{
		printf("Socket Creation ---> FAILED | ERROR : %d\n", WSAGetLastError());
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}
	printf("Socket Creation ---> SUCCESS \n");
	return 0;

	/*unsigned long NonBlock = 1;
	result = ioctlsocket(serverSocket, FIONBIO, &NonBlock);*/
}

// Function to bind the socket to an address.
// [param_1]: Reference to the server socket instance.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success)
int cNetworkManager::Bind(SOCKET& serverSocket)
{
	int result = bind(serverSocket, info->ai_addr, (int)info->ai_addrlen);

	if (result == SOCKET_ERROR)
	{
		printf("Binding ---> FAILED | ERROR : %d\n", WSAGetLastError());
		CleanSocket(serverSocket, info);
		return 1;
	}
	printf("Binding ---> SUCCESS \n");
	return 0;
}

// Function to listen for incoming connections.
// [param_1]: Reference to the server socket instance.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::Listen(SOCKET& serverSocket)
{
	int result = listen(serverSocket, SOMAXCONN);

	if (result == SOCKET_ERROR)
	{
		printf("Listening ---> FAILED | ERROR : %d\n", WSAGetLastError());
		CleanSocket(serverSocket, info);
		return 1;
	}
	printf("Listening ---> SUCCESS \n");
	printf("--------------------------------------------------------------------------------------------------\n");
	return 0;
}

// Function to if the email provided passes the valid email contraints
// [param_1]: Email to be checked is passed as a string value in the parameter
// [return_value]: Either true or false is returned
bool isValidEmail(const std::string& email) 
{
	std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");

	return std::regex_match(email, emailRegex);
}

// Function to listen for incoming messages from ChatServer
// [param_1]: Instance of the class cMySQLUtil is passed as a paramter
// [return_value]: The error indication is returned as an integer value (1 - error, 0 - success).
int cNetworkManager::ListenToChatServer(cMySQLUtil& connectSQLDB)
{		
	char bufferString[2056];

	accountAuthProtocol::MessageWithPrefix receiveMessageWithPrefix;
	accountAuthProtocol::AccountCreation registerAccount;
	accountAuthProtocol::AuthenticateAccount loginAccount;

	// Receive a message from the client.
	int bytesReceived = recv(chatServerSocket, bufferString, sizeof(bufferString), 0);

	if (bytesReceived == SOCKET_ERROR)
	{
		printf("RECEIVE ---> FAILED | ERROR : %d\n", WSAGetLastError());
		CleanSocket(chatServerSocket, info);
		return 1;
	}

	//--------------Parse the received message with prefix--------------------------------

	bool success = receiveMessageWithPrefix.ParseFromArray(bufferString, bytesReceived);

	if (!success)
	{
		printf("PARSING ---> FAILED");
		CleanSocket(chatServerSocket, info);
		return 1;
	}

	//-------------------Parse the actual message----------------------------------------

	success = registerAccount.ParseFromString(receiveMessageWithPrefix.serializedmessage());

	if (!success)
	{
		printf("PARSING ---> FAILED");
		CleanSocket(chatServerSocket, info);
		return 1;
	}	

	//-------Checking what type of message has been received-----------------

	accountAuthProtocol::MessageType receiveType;
	accountAuthProtocol::MessageWithPrefix messageWithLengthPrefix;
	accountAuthProtocol::AccountCreationFailure registrationFail;
	accountAuthProtocol::AccountCreationSuccess registrationSuccess;
	accountAuthProtocol::AuthenticationSuccess loginSuccess;
	accountAuthProtocol::AuthenticationFailure loginFail;

	receiveType = registerAccount.type();
	
	int insertCount = 0;
	int rowCount = 0;
	int updateCount = 0;
	int rowId = 0;
	bool accountSignInFailed = false;
	std::string salt;
	std::string hashedPassword;
	std::string creationDate;
	std::string selectString;
	std::string serializedString;
	std::string serializedStringWithPrefix;
	std::string emailId;
	sql::SQLString passwordFromDB;
	sql::ResultSet* result;

		switch (receiveType) 
		{
		case accountAuthProtocol::MessageType::REGISTER:
			
			//--------------------------Checking if email already exists------------------------------------------
			if (isValidEmail(registerAccount.email()))
			{
				selectString = "SELECT * FROM `web_auth` WHERE Email = '" + registerAccount.email() + "';";

				result = connectSQLDB.Select(selectString.c_str());

				if (result->next())
					rowCount = result->getInt(1);

				if (rowCount == 0)
				{
					if (registerAccount.plaintextpassword().size() > 8) // Checking password constraint( more than 8 characters )
					{
						//---------------------Inserting into user table and fetching the user Id--------------------------

						insertCount = connectSQLDB.AddNewUser();
						printf("MYSQL_INSERT : %d rows were inserted\n", insertCount);

						selectString = "SELECT Id FROM `user` ORDER BY ID DESC LIMIT 1";

						result = connectSQLDB.Select(selectString.c_str());

						if (result->next())
							rowId = result->getInt("Id");

						if (result != NULL)
						{
							//-----------------------Hashing password--------------------------------------------

							salt = hashData.GenerateRandomString(64);
							hashedPassword = hashData.HashPasswordWithSalt(registerAccount.plaintextpassword(), salt);

							//-----------------Inserting into web_auth table-------------------------------------

							insertCount = connectSQLDB.AddUserAccount(registerAccount.email().c_str(), salt.c_str(), hashedPassword.c_str(), rowId);
							printf("MYSQL_INSERT : %d rows were inserted\n", insertCount);

							//------------------Setting up success message---------------------------------------

							registrationSuccess.set_type(accountAuthProtocol::MessageType::REGISTER_SUCCESS);
							registrationSuccess.set_requestid(registerAccount.requestid());
							registrationSuccess.set_userid(result->getInt("Id"));
							registrationSuccess.SerializeToString(&serializedString);

							//-----------Adding length prefix before sending----------------------------

							messageWithLengthPrefix.set_length(serializedString.size());
							messageWithLengthPrefix.set_serializedmessage(serializedString);
							messageWithLengthPrefix.SerializeToString(&serializedStringWithPrefix);

							WriteResponseToChatServer(serializedStringWithPrefix);
						}

						else // Error : Internal server error
						{
							accountSignInFailed = true;

							registrationFail.set_reason(accountAuthProtocol::AccountCreationFailure::FailReason::AccountCreationFailure_FailReason_INTERNAL_SERVER_ERROR);
						}
					}
					else // Error : Invalid password
					{
						accountSignInFailed = true;

						registrationFail.set_reason(accountAuthProtocol::AccountCreationFailure::FailReason::AccountCreationFailure_FailReason_INVALID_PASSWORD);
					}
				}
				else // Error : Account already exists
				{
					accountSignInFailed = true;

					registrationFail.set_reason(accountAuthProtocol::AccountCreationFailure::FailReason::AccountCreationFailure_FailReason_ACCOUNT_ALREADY_EXISTS);
				}
			}
			else // Error : Invalid Email
			{
				accountSignInFailed = true;

				registrationFail.set_reason(accountAuthProtocol::AccountCreationFailure::FailReason::AccountCreationFailure_FailReason_INVALID_EMAIL);
			}

			//-------------------Setting up failure message------------------------------

			if (accountSignInFailed)
			{
				registrationFail.set_requestid(registerAccount.requestid());
				registrationFail.set_type(accountAuthProtocol::MessageType::REGISTER_FAIL);
				registrationFail.SerializeToString(&serializedString);

				//-----------Adding length prefix before sending----------------------------

				messageWithLengthPrefix.set_length(serializedString.size());
				messageWithLengthPrefix.set_serializedmessage(serializedString);
				messageWithLengthPrefix.SerializeToString(&serializedStringWithPrefix);

				WriteResponseToChatServer(serializedStringWithPrefix);
			}
			break;

		case accountAuthProtocol::MessageType::AUTHENTICATE:

			success = loginAccount.ParseFromString(receiveMessageWithPrefix.serializedmessage());

			if (!success)
			{
				printf("Parsing ---> FAILED");
				CleanSocket(chatServerSocket, info);
				return 1;
			}
			
			//--------Comparing with the email id and password in database---------------

			emailId = loginAccount.email();

			selectString = "SELECT * FROM `web_auth` WHERE Email = '" + emailId + "';";

			result = connectSQLDB.Select(selectString.c_str());

			if (result->next())
			{
				rowCount = result->getInt(1);

				if (rowCount > 0)
				{
					//---------------Get password and salt from DB-----------------------

					passwordFromDB = result->getString("Hashed_Password");
					salt = result->getString("Salt");
					
					//--------------------Hash the new password---------------------------

					hashedPassword = hashData.HashPasswordWithSalt(loginAccount.plaintextpassword(), salt);

					//--------------------Compare the passwords---------------------------

					if (hashedPassword == passwordFromDB)
					{
						rowId = result->getInt("UserId");

						updateCount = connectSQLDB.UpdateUser(rowId);
						printf("MYSQL_UPDATE : %d rows were updated\n", updateCount);

						if (updateCount != 0)
						{
							selectString = "SELECT Creation_Date FROM `user` WHERE Id = " + std::to_string(rowId) + ";";

							result = connectSQLDB.Select(selectString.c_str());

							if (result->next())
							{
								rowCount = result->getInt(1);

								if (rowCount > 0)
								{
									creationDate = result->getString("Creation_Date");

									loginSuccess.set_type(accountAuthProtocol::MessageType::AUTHENTICATE_SUCCESS);
									loginSuccess.set_userid(rowId);
									loginSuccess.set_requestid(loginAccount.requestid());
									loginSuccess.set_creationdate(creationDate);
									loginSuccess.SerializePartialToString(&serializedString);

									//-----------Adding length prefix before sending----------------------------

									messageWithLengthPrefix.set_length(serializedString.size());
									messageWithLengthPrefix.set_serializedmessage(serializedString);
									messageWithLengthPrefix.SerializeToString(&serializedStringWithPrefix);

									WriteResponseToChatServer(serializedStringWithPrefix);
								}
								else
								{
									accountSignInFailed = true;
									loginFail.set_reason(accountAuthProtocol::AuthenticationFailure::INTERNAL_SERVER_ERROR);
								}
							}
							else
							{
								accountSignInFailed = true;
								loginFail.set_reason(accountAuthProtocol::AuthenticationFailure::INTERNAL_SERVER_ERROR);
							}
						}
						else
						{
							accountSignInFailed = true;
							loginFail.set_reason(accountAuthProtocol::AuthenticationFailure::INTERNAL_SERVER_ERROR);
						}
					}
					else
					{
						accountSignInFailed = true;
						loginFail.set_reason(accountAuthProtocol::AuthenticationFailure::INVALID_CREDENTIALS);
					}
				}
				else
				{
					accountSignInFailed = true;
					loginFail.set_reason(accountAuthProtocol::AuthenticationFailure::INVALID_CREDENTIALS);
				}
			}
			else
			{
				accountSignInFailed = true;
				loginFail.set_reason(accountAuthProtocol::AuthenticationFailure::INVALID_CREDENTIALS);
			}

			if (accountSignInFailed)
			{
				loginFail.set_requestid(loginAccount.requestid());
				loginFail.set_type(accountAuthProtocol::MessageType::AUTHENTICATE_FAIL);
				loginFail.SerializeToString(&serializedString);

				//-----------Adding length prefix before sending----------------------------

				messageWithLengthPrefix.set_length(serializedString.size());
				messageWithLengthPrefix.set_serializedmessage(serializedString);
				messageWithLengthPrefix.SerializeToString(&serializedStringWithPrefix);

				WriteResponseToChatServer(serializedStringWithPrefix);
			}			

			break;
		}

	return 0;
}
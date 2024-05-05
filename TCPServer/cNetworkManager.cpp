#include "pch.h"
#include "cNetworkManager.h"

// Function to create and send a message to a buffer based on the given input.
// [param_1]: Reference to the sChatMessage instance.
// [param_2]: Reference to the cBuffer instance to store the message.
// [param_3]: Input message as a string to be sent.
void cNetworkManager::SendMessageToBuffer(sChatMessage& message, cBuffer& buffer, std::string input)
{

	message.message = input;
	message.messageLength = message.message.length();
	message.header.messageType = 1;
	message.header.packetSize =
		message.message.length()
		+ sizeof(message.messageLength)
		+ sizeof(message.header.messageType)
		+ sizeof(message.header.packetSize);

	buffer.WriteUInt32BE(message.header.packetSize);
	buffer.WriteUInt16BE(message.header.messageType);
	buffer.WriteUInt16BE(message.messageLength);
	buffer.WriteString(message.message);
}

// Function to clean up the socket.
// [param_1]: Reference to the server socket instance.
// [param_2]: PADDRINFOA information passed.
void cNetworkManager::CleanSocket(SOCKET& serverSocket, PADDRINFOA info)
{
	freeaddrinfo(info);
	closesocket(serverSocket);
	WSACleanup();
}

// Function to initialize the Windows Sockets API.
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
// [param_1]: IP address as string.
// [param_2]: Port passed as string.
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

// Function to initialize address information.
// [param_1]: IP address as string.
// [param_2]: Port passed as string.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::InitializeAddrInfoforAuthServer(const char* ipaddress, const char* port)
{
	ZeroMemory(&hints_authServer, sizeof(hints_authServer));

	hints_authServer.ai_family = AF_INET;
	hints_authServer.ai_socktype = SOCK_STREAM;
	hints_authServer.ai_protocol = IPPROTO_TCP;
	hints_authServer.ai_flags = AI_PASSIVE;

	int result = getaddrinfo(ipaddress, port, &hints_authServer, &info_authServer);

	if (result != 0)
	{
		printf("getaddrinfo ---> FAILED | ERROR : %d\n", result);
		freeaddrinfo(info_authServer);
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

// Function to create a socket.
// [param_1]: Reference to the auth server socket instance.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::CreateSocketForAuthServer(SOCKET& authServerSocket)
{
	authServerSocket = socket(info_authServer->ai_family, info_authServer->ai_socktype, info_authServer->ai_protocol);

	if (authServerSocket == INVALID_SOCKET)
	{
		printf("Socket Creation ---> FAILED | ERROR : %d\n", WSAGetLastError());
		freeaddrinfo(info_authServer);
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

// Function to connect the socket to the server.
// [param_1]: Reference to auth server socket instance.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::ConnectSocket(SOCKET& authServerSocket)
{
	connect(authServerSocket, info_authServer->ai_addr, (int)info_authServer->ai_addrlen);

	if (authServerSocket == INVALID_SOCKET)
	{
		printf("Connection ---> FAILED | ERROR : %d\n", WSAGetLastError());
		CleanSocket(authServerSocket, info_authServer);
		return 1;
	}

	printf("Connection ---> SUCCESS \n");
	printf("--------------------------------------------------------------------------------------------------\n");
	return 0;
}

// Function to send the message to the client.
// [param_1]: Reference to the sChatMessage instance.
// [param_2]: cBuffer instance to store the message.
// [param_3]: Full message to be sent.
// [param_4]: Socket of the client to receive the message.
// [return_value]: The error indication is returned as an integer value (1 - error, 0 - success).
int cNetworkManager::SendMessageToClient(sChatMessage& message, cBuffer& buffer, std::string fullMessage, SOCKET clientSocket)
{
	// Send the message to the client.
	SendMessageToBuffer(message, buffer, fullMessage);

	int result = send(clientSocket, (const char*)(&buffer.mBufferData[0]), message.header.packetSize, 0);

	if (result == SOCKET_ERROR)
	{
		printf("\nMESSAGE SENDING ---> FAILED | ERROR : %d\n", WSAGetLastError());
		CleanSocket(clientSocket, info);
		return 1;
	}

	printf("\nMESSAGE SENT TO CLIENT | SOCKET : %lu\n", clientSocket);

	return 0;
}

// Function to send the message to the auth server.
// [param_1]: Message to be sent is passed as a string value in the parameter
// [return_value]: The error indication is returned as an integer value (1 - error, 0 - success).
int cNetworkManager::SendMessageToAuthServer(std::string messageToBeSent)
{
	int result = send(authServerSocketNumber, messageToBeSent.c_str(), messageToBeSent.size(), 0);

	if (result == SOCKET_ERROR)
	{
		printf("\nMESSAGE SENDING ---> FAILED | ERROR : %d\n", WSAGetLastError());
		CleanSocket(authServerSocketNumber, info_authServer);
		return 1;
	}

	printf("\nMESSAGE SENT TO AUTH SERVER | SOCKET : %lu\n", authServerSocketNumber);

	return 0;
}

// Function to receive a message from a client.
// [param_1]: Socket of the client to receive the message.
// [param_2]: cBuffer instance to store the received message.
// [param_3]: Buffer size.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::ReceiveMessageFromClient(SOCKET clientSocket, cBuffer& buffer, int bufSize)
{
	// Receive a message from the client.
	int result = recv(clientSocket, (char*)(&buffer.mBufferData[0]), bufSize, 0);

	if (result == SOCKET_ERROR)
	{
		printf("\nMESSAGE RECEIVING---> FAILED | ERROR : %d\n", WSAGetLastError());
		CleanSocket(clientSocket, info);
		return 1;
	}

	printf("\nMESSAGE RECEIVED FROM CLIENT | SOCKET : %lu\n", clientSocket);

	return 0;
}

// Function to receive a message from the auth server.
// [param_1]: Reference to the sChatMessage instance.
// [param_2]: cBuffer instance to store the received message.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::ReceiveMessageFromAuthServer(sChatMessage& message, cBuffer& buffer)
{
	char bufferString[2056];
	
	accountAuthProtocol::MessageWithPrefix receiveMessageWithPrefix;
	accountAuthProtocol::AccountCreationSuccess registerSuccess;
	accountAuthProtocol::AccountCreationFailure registerFail;
	accountAuthProtocol::AuthenticationSuccess loginSuccess;
	accountAuthProtocol::AuthenticationFailure loginFail;

	int bytesReceived = recv(authServerSocketNumber, bufferString, sizeof(bufferString), 0);

	if (bytesReceived == SOCKET_ERROR)
	{
		printf("\nMESSAGE RECEIVING ---> FAILED | ERROR : %d\n", WSAGetLastError());
		CleanSocket(authServerSocketNumber, info_authServer);
		return 1;
	}

	printf("\nMESSAGE RECEIVED FROM AUTH SERVER | SOCKET : %lu\n", authServerSocketNumber);

	//--------------Parse the received message with prefix--------------------------------

	bool success = receiveMessageWithPrefix.ParseFromArray(bufferString, bytesReceived);

	if (!success)
	{
		printf("\nPARSING ---> FAILED\n");
		CleanSocket(authServerSocketNumber, info_authServer);
		return 1;
	}

	//-------------------Parse the actual message----------------------------------------

	success = registerSuccess.ParseFromString(receiveMessageWithPrefix.serializedmessage());

	if (!success)
	{
		printf("\nPARSING ---> FAILED\n");
		CleanSocket(authServerSocketNumber, info_authServer);
		return 1;
	}

	std::string fullMessage = "";
	SOCKET clientSocket = NULL;

	//-------Checking what type of message has been received-----------------

	accountAuthProtocol::MessageType receiveType;

	receiveType = registerSuccess.type();

	switch (receiveType)
	{
	case accountAuthProtocol::MessageType::REGISTER_SUCCESS: // If registration succeeds
		
		//-------------Checking from which client socket the request id came from---------------------

		clientSocket = mClientRequestIdList[registerSuccess.requestid()];
		
		mClientUserIdList[clientSocket] = registerSuccess.userid();

		//---------------------Sending relevant message back to client--------------------------------

		fullMessage = "$RPASS$**ACCOUNT HAS BEEN REGISTERED SUCCESSFULLY!**";

		SendMessageToClient(message, buffer, fullMessage, clientSocket);

		break;
	
	case accountAuthProtocol::MessageType::REGISTER_FAIL: // If registration fails

		success = registerFail.ParseFromString(receiveMessageWithPrefix.serializedmessage());

		if (!success)
		{
			printf("\nPARSING ---> FAILED\n");
			CleanSocket(authServerSocketNumber, info_authServer);
			return 1;
		}

		//-------------Checking from which client socket the request id came from---------------------

		clientSocket = mClientRequestIdList[registerFail.requestid()];
		
		//---------------------Sending relevant message back to client--------------------------------

		if(registerFail.reason() == accountAuthProtocol::AccountCreationFailure::ACCOUNT_ALREADY_EXISTS)
			fullMessage = "$RFAIL$**ACCOUNT ALREADY EXISTS ! TRY A DIFFERENT EMAIL ID**";

		else if (registerFail.reason() == accountAuthProtocol::AccountCreationFailure::INVALID_EMAIL)
			fullMessage = "$RFAIL$**EMAIL IS INVALID ! WRITE YOUR EMAIL IN THE CORRECT FORMAT**";

		else if(registerFail.reason() == accountAuthProtocol::AccountCreationFailure::INVALID_PASSWORD)
			fullMessage = "$RFAIL$**PASSWORD IS INVALID ! PASSWORD SHOULD BE MORE THAN 8 CHARACTERS LONG**";

		else if (registerFail.reason() == accountAuthProtocol::AccountCreationFailure::INTERNAL_SERVER_ERROR)
			fullMessage = "$RFAIL$**AUTHENTICATION SERVER ERROR !**";

		SendMessageToClient(message, buffer, fullMessage, clientSocket);

		break;
	
	case accountAuthProtocol::MessageType::AUTHENTICATE_SUCCESS: // If authentication succeeds

		success = loginSuccess.ParseFromString(receiveMessageWithPrefix.serializedmessage());

		if (!success)
		{
			printf("\nPARSING ---> FAILED\n");
			CleanSocket(authServerSocketNumber, info_authServer);
			return 1;
		}
		
		//-------------Checking from which client socket the request id came from---------------------
		
		clientSocket = mClientRequestIdList[loginSuccess.requestid()];

		//---------------------Sending relevant message back to client--------------------------------

		fullMessage = "$LPASS$**LOGGED IN SUCCESSFULLY [ACCOUNT CREATION DATE : " + loginSuccess.creationdate() + "]**";

		SendMessageToClient(message, buffer, fullMessage, clientSocket);

		break;
	
	case accountAuthProtocol::MessageType::AUTHENTICATE_FAIL: // If authentication fails

		success = loginFail.ParseFromString(receiveMessageWithPrefix.serializedmessage());

		if (!success)
		{
			printf("\nPARSING ---> FAILED\n");
			CleanSocket(authServerSocketNumber, info_authServer);
			return 1;
		}

		//-------------Checking from which client socket the request id came from---------------------

		clientSocket = mClientRequestIdList[loginFail.requestid()];

		//---------------------Sending relevant message back to client--------------------------------

		if (loginFail.reason() == accountAuthProtocol::AuthenticationFailure::INVALID_CREDENTIALS)
			fullMessage = "$LFAIL$**INVALID CREDENTIALS ! TRY AGAIN !**";

		else if (loginFail.reason() == accountAuthProtocol::AuthenticationFailure::INTERNAL_SERVER_ERROR)
			fullMessage = "$LFAIL$**AUTHENTICATION SERVER ERROR !**";

		SendMessageToClient(message, buffer, fullMessage, clientSocket);

		break;
	}

	buffer.ClearBuffer();

	return 0;
}

// Function to add all clients to the FDSET for reading.
// [param_1]: Reference to the FD_SET.
void cNetworkManager::AddAllClientsToFDSET(FD_SET& socketsReadyForReading)
{
	// Add all clients to the FD_SET for reading.
	for (int i = 0; i < mClientConnectionList.size(); i++)
	{
		FD_SET(mClientConnectionList[i], &socketsReadyForReading);
	}
}

// Function to add a new client socket to the list and handle related actions.
// [param_1]: Reference to the client socket being added.
// [param_2]: Reference to the FD_SET of sockets ready for reading.
// [param_3]: Reference to the active sockets FD_SET.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::AddNewClientToList(SOCKET& clientSocket, FD_SET& socketsReadyForReading, FD_SET& activeSockets)
{
	// Accept a new client connection.
	SOCKET newConnection = accept(clientSocket, NULL, NULL);
	if (newConnection == INVALID_SOCKET)
	{
		printf("\nACCEPTING NEW CLIENT ---> FAILED | ERROR : %d\n", WSAGetLastError());
		return 1;
	}

	// Add the new client socket to the list, active sockets FD_SET, and remove from ready sockets FD_SET.
	mClientConnectionList.push_back(newConnection);
	FD_SET(newConnection, &activeSockets);
	FD_CLR(clientSocket, &socketsReadyForReading);

	printf("\n--------------------------------------------------------------------------------------------------\n");
	printf("\nACCEPTING NEW CLIENT ---> SUCCESS | SOCKET : %d\n", (int)newConnection);
	printf("\n--------------------------------------------------------------------------------------------------\n");
	return 0;
}

// Function to remove a client socket from the list.
// [param_1]: Socket of the client to be removed.
void cNetworkManager::RemoveClientFromList(SOCKET& clientSocket)
{
	// Remove the client socket from the list.
	mClientConnectionList.erase(std::remove(mClientConnectionList.begin(), mClientConnectionList.end(), clientSocket), mClientConnectionList.end());
}

// Function to loop through the client socket list and handle messages.
// [param_1]: Reference to the FD_SET of sockets ready for reading.
// [param_2]: Reference to the active sockets FD_SET.
// [param_3]: Reference to the sChatMessage to handle messages.
// [param_4]: Reference to the cBuffer instance for message handling.
// [param_5]: Buffer size for reading messages.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::LoopThroughClientList(FD_SET& socketsReadyForReading, FD_SET& activeSockets, sChatMessage& message, cBuffer& buffer, int bufSize)
{
	int result = 0;
	std::string roomName;
	std::string fullMessage;

	for (int i = 0; i < mClientConnectionList.size(); i++)
	{
		SOCKET socket = mClientConnectionList[i];

		if (FD_ISSET(socket, &socketsReadyForReading))
		{
			// Receive a message from the client.
			result = ReceiveMessageFromClient(socket, buffer, bufSize);

			if (result != 0)
			{
				// Handle client disconnection by removing them from the list and FD_SETs.
				RemoveClientFromList(socket);
				FD_CLR(socket, &activeSockets);
				FD_CLR(socket, &socketsReadyForReading);
				CleanSocket(socket, info);
				return 1;
			}

			// Parse and handle the received message.
			uint32_t packetSize = buffer.ReadUInt32BE();
			uint32_t messageType = buffer.ReadUInt16BE();

			if (buffer.mBufferData.size() >= packetSize)
			{
				buffer.mBufferData.resize(bufSize * 2);
			}

			if (messageType == 1)
			{
				uint32_t messageLength = buffer.ReadUInt16BE();
				std::string msg = buffer.ReadString(messageLength);
				std::string tempString;

				tempString = msg.substr(0, 6);

				if (tempString == "$NAME$")
				{
					printf("\n[%s] IS CONNECTED TO THE SERVER\n", msg.substr(6).c_str());

					// Store the client's name.
					clientName[socket] = msg.substr(6);
				}

				else if (tempString == "$JOIN$")
				{
					roomName = msg.substr(6);

					// Check if the room exists, if not, create it
					if (rooms.find(roomName) == rooms.end())
					{
						sRoom room;
						rooms[roomName] = room;

						printf("\nNEW ROOM CREATED | ROOM NAME : %s\n", roomName.c_str());
					}

					// Add the client to the room
					rooms[roomName].clients.push_back(socket);
					clientRooms[socket] = roomName;

					printf("\n[%s] HAS JOINED THE ROOM (%s)\n", clientName[socket].c_str(), roomName.c_str());

					fullMessage = "$READY$[" + clientName[socket] + "] IS ADDED TO THE ROOM (" + roomName + ")";

					// Send a confirmation message to the client.
					result = SendMessageToClient(message, buffer, fullMessage, socket);

					if (result != 0)
						return 1;
					else
					{
						buffer.ClearBuffer();

						// Broadcast the new client's arrival to all clients in the room.
						result = BroadcastToAll(socket, true, buffer, message, roomName);

						if (result != 0)
							return 1;
					}
				}

				else if (tempString == "$CHAT$")
				{

					std::string username = clientName[socket];
					std::string userRoom = clientRooms[socket];

					sRoom& room = rooms[userRoom];

					for (SOCKET clientSocket : room.clients)
					{
						if (clientSocket != socket)
						{
							fullMessage = "$REPLY$" + username + " : " + msg.substr(6);
							result = SendMessageToClient(message, buffer, fullMessage, clientSocket);

							if (result != 0)
								return 1;

							buffer.ClearBuffer();
						}
						else
						{
							fullMessage = "YOU : " + msg.substr(6);
							result = SendMessageToClient(message, buffer, fullMessage, clientSocket);

							if (result != 0)
								return 1;

							buffer.ClearBuffer();
						}
					}
				}

				else if (tempString == "$EXIT$")
				{
					// Handle client disconnection from the room
					roomName = clientRooms[socket];
					rooms[roomName].clients.erase(std::remove(rooms[roomName].clients.begin(), rooms[roomName].clients.end(), socket), rooms[roomName].clients.end());
					clientRooms.erase(socket);

					fullMessage = "$LEFT$";

					// Notify other clients in the room about the client's departure.
					result = SendMessageToClient(message, buffer, fullMessage, socket);

					if (result != 0)
						return 1;
					else
						printf("\n[%s] HAS LEFT THE ROOM (%s)\n", clientName[socket].c_str(), roomName.c_str());

					buffer.ClearBuffer();

					// Broadcast the client's departure to all remaining clients in the room.
					result = BroadcastToAll(socket, false, buffer, message, roomName);

					if (result != 0)
						return 1;
				}

				else if (tempString == "$REML$")
				{
					mClientEmailList[socket] = msg.substr(6);
				}
				else if (tempString == "$RPWD$")
				{
					std::string serializedString;
					std::string serializedStringWithPrefix;

					accountAuthProtocol::MessageWithPrefix messageWithLengthPrefix;
					accountAuthProtocol::AccountCreation* registerAccount = new accountAuthProtocol::AccountCreation;

					mClientRequestIdList[++requestId] = socket;

					registerAccount->set_type(accountAuthProtocol::MessageType::REGISTER);
					registerAccount->set_requestid(requestId);
					registerAccount->set_email(mClientEmailList[socket]);
					registerAccount->set_plaintextpassword(msg.substr(6));

					registerAccount->SerializeToString(&serializedString);

					//-----------Adding length prefix before sending----------------------------

					messageWithLengthPrefix.set_length(serializedString.size());
					messageWithLengthPrefix.set_serializedmessage(serializedString);
					messageWithLengthPrefix.SerializeToString(&serializedStringWithPrefix);

					result = SendMessageToAuthServer(serializedStringWithPrefix);

					mClientEmailList.erase(socket);

					delete registerAccount;

					if (result != 0)
						return 1;
				}
				else if (tempString == "$AEML$")
				{
					mClientEmailList[socket] = msg.substr(6);
				}
				else if (tempString == "$APWD$")
				{
					std::string serializedString;
					std::string serializedStringWithPrefix;

					accountAuthProtocol::MessageWithPrefix messageWithLengthPrefix;
					accountAuthProtocol::AuthenticateAccount* loginAccount = new accountAuthProtocol::AuthenticateAccount;

					mClientRequestIdList[++requestId] = socket;

					loginAccount->set_type(accountAuthProtocol::MessageType::AUTHENTICATE);
					loginAccount->set_requestid(requestId);
					loginAccount->set_email(mClientEmailList[socket]);
					loginAccount->set_plaintextpassword(msg.substr(6));

					loginAccount->SerializeToString(&serializedString);

					//-----------Adding length prefix before sending----------------------------

					messageWithLengthPrefix.set_length(serializedString.size());
					messageWithLengthPrefix.set_serializedmessage(serializedString);
					messageWithLengthPrefix.SerializeToString(&serializedStringWithPrefix);

					result = SendMessageToAuthServer(serializedStringWithPrefix);

					mClientEmailList.erase(socket);

					delete loginAccount;

					if (result != 0)
						return 1;
				}

			}

			buffer.ClearBuffer();

			FD_CLR(socket, &socketsReadyForReading);
			readCount--;
		}
	}

	return 0;
}

// Function to broadcast a message to all clients in a room.
// [param_1]: Socket of the client sending the broadcast.
// [param_2]: Indicates if a new client has joined the room.
// [param_3]: Reference to the cBuffer instance for sending messages.
// [param_4]: Reference to the sChatMessage for handling messages.
// [param_5]: Name of the room to broadcast the message to.
// [return_value]: The error indication is passed as an integer value (1 - error, 0 - success).
int cNetworkManager::BroadcastToAll(SOCKET& clientSocket, bool newClientJoined, cBuffer& buffer, sChatMessage message, std::string userRoom)
{
	int result = 0;
	std::string fullMessage;

	if (newClientJoined)
	{
		// Get the username of the client that has joined.
		std::string username = clientName[clientSocket];
		// Retrieve the room the client joined.
		sRoom& room = rooms[userRoom];

		for (SOCKET otherClientSocket : room.clients)
		{
			if (otherClientSocket != clientSocket)
			{
				// Create a message to inform other clients about the new client joining.
				fullMessage = "$REPLY$**[" + username + "] HAS JOINED THE ROOM**";
				result = SendMessageToClient(message, buffer, fullMessage, otherClientSocket);

				if (result != 0)
					return 1;

				buffer.ClearBuffer();
			}
		}
	}
	else
	{
		// Get the username of the client that has left.
		std::string username = clientName[clientSocket];
		// Retrieve the room the client left.
		sRoom& room = rooms[userRoom];

		for (SOCKET otherClientSocket : room.clients)
		{
			// Create a message to inform other clients about the client leaving the room.
			fullMessage = "$REPLY$**[" + username + "] HAS LEFT THE ROOM**";
			result = SendMessageToClient(message, buffer, fullMessage, otherClientSocket);

			if (result != 0)
				return 1;

			buffer.ClearBuffer();
		}
	}

	return 0;
}

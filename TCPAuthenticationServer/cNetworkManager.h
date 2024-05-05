#pragma once
#include "pch.h"
#include "cMySQLUtil.h"
#include "cHash.h"

// Structure to represent the packet header
struct sPacketHeader
{
	uint32_t packetSize;
	uint32_t messageType;
};

// Structure to represent message details
struct sChatMessage
{
	sPacketHeader header;
	uint32_t messageLength;
	std::string message;
};

// This class is created to manage the network related functionalities
class cNetworkManager
{
private:

	bool mFDCleared = false;
	cHash hashData;

public:

	// Pointers and structures for managing network connections
	struct addrinfo* info = nullptr;
	struct addrinfo hints;

	int readCount = 0;

	SOCKET chatServerSocket = NULL;
	SOCKET authServerSocket = NULL;

	cNetworkManager();
	~cNetworkManager();

	int WriteResponseToChatServer(std::string serializedMessage);
	void CleanSocket(SOCKET& serverSocket, PADDRINFOA info);

	int InitializeWSA();
	int InitializeAddrInfo(const char* ipaddress, const char* port);

	int CreateSocket(SOCKET& serverSocket);
	int Bind(SOCKET& serverSocket);
	int Listen(SOCKET& serverSocket);

	int ListenToChatServer(cMySQLUtil& connectSQLDB);
};


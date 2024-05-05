#pragma once 

#include "pch.h"
#include "cNetworkManager.h"
#include "cMySQLUtil.h"

#define DEFAULT_PORT "8401"

int main(int arg, char** argv)
{
	int result = 0;
	const int bufSize = 512;

	printf("<<----------------------AUTHENTICATION SERVER---------------------------->>\n\n");

	//---------------------Initialize class instances-------------------------------------

	cMySQLUtil connectSQLDB;
	cNetworkManager networkManager;
	sChatMessage message;

	//-------------------Initialize Database Connection-----------------------------------

	connectSQLDB.ConnectToDatabase("127.0.0.1:3306", "root", "root", "webaccount");
	
	if (!connectSQLDB.IsConnected())
		return -1;

	//--------------Initialize the Windows Sockets API (WSA)------------------------------

	result = networkManager.InitializeWSA();

	if (result != 0)
		return 1;

	// Initialize address information
	result = networkManager.InitializeAddrInfo(NULL, DEFAULT_PORT);

	if (result != 0)
		return 1;

	SOCKET serverSocket;

	result = networkManager.CreateSocket(serverSocket);

	if (result != 0)
		return 1;

	// Bind the server socket.
	result = networkManager.Bind(serverSocket);

	if (result != 0)
		return 1;

	// Listening for incoming connections on the server socket.
	result = networkManager.Listen(serverSocket);

	if (result != 0)
		return 1;

	networkManager.authServerSocket = serverSocket;

	//FD_SET for socket operations				
	FD_SET activeSockets;
	FD_SET socketsReadyForReading;

	FD_ZERO(&activeSockets);

	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	while (true)
	{
		FD_ZERO(&socketsReadyForReading);

		if (networkManager.chatServerSocket != NULL && serverSocket != networkManager.chatServerSocket)
			serverSocket = networkManager.chatServerSocket;

		FD_SET(serverSocket, &socketsReadyForReading);

		networkManager.readCount = select(0, &socketsReadyForReading, NULL, NULL, &tv); // Use the `select` function to check for sockets ready for reading.

		if (networkManager.readCount == 0)
		{
			continue;
		}

		if (networkManager.readCount == SOCKET_ERROR)
		{
			printf("Select() ---> FAILED | ERROR : %d\n", WSAGetLastError());
			closesocket(serverSocket);
			return 1;
		}

		if (networkManager.readCount > 0)
		{
			if (FD_ISSET(serverSocket, &socketsReadyForReading))
			{
				if (networkManager.chatServerSocket == NULL)
				{
					networkManager.chatServerSocket = accept(serverSocket, NULL, NULL);

					if (networkManager.chatServerSocket == INVALID_SOCKET)
					{
						printf("Chat Server Connection ---> FAILED | ERROR : %d\n", WSAGetLastError());
						break;
					}

					FD_SET(networkManager.chatServerSocket, &activeSockets);
					FD_CLR(serverSocket, &socketsReadyForReading);

					printf("Chat Server Connection ---> SUCCESS | SOCKET : %d\n", (int)networkManager.chatServerSocket);
					printf("--------------------------------------------------------------------------------------------------\n");
				}
				else
				{
					result = networkManager.ListenToChatServer(connectSQLDB);

					if (result != 0)
					{
						FD_CLR(networkManager.chatServerSocket, &activeSockets);
						FD_CLR(networkManager.chatServerSocket, &socketsReadyForReading);
						networkManager.CleanSocket(networkManager.chatServerSocket, networkManager.info);
						break;
					}
				}
			}
		}
	}

	// Clean up and close the server socket.
	networkManager.CleanSocket(serverSocket, networkManager.info);
	return 0;
}


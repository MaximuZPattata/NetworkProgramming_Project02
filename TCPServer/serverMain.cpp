#pragma once

#include "pch.h"
#include "cBuffer.h"
#include "cNetworkManager.h"

#define DEFAULT_PORT "8400"
#define NEW_PORT "8401"

int main(int arg, char** argv)
{
	const int bufSize = 512;
	int result = 0;

	printf("<<--------------------------CHAT SERVER----------------------------------->>\n\n");

	cNetworkManager networkManager;
	sChatMessage message;
	cBuffer buffer(bufSize);

	// Initialize the Windows Sockets API (WSA) for network communication.
	result = networkManager.InitializeWSA();

	if (result != 0)
		return 1;

	// Initialize address information for server socket.
	result = networkManager.InitializeAddrInfo(NULL, DEFAULT_PORT);

	if (result != 0)
		return 1;

	// Initialize address information for authentication server socket.
	result = networkManager.InitializeAddrInfoforAuthServer("127.0.0.1", NEW_PORT);

	if (result != 0)
		return 1;

	//Socket creation.
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

	//----------------Auth Server connect-----------------------------

	SOCKET authServerSocket;

	result = networkManager.CreateSocketForAuthServer(authServerSocket);

	if (result != 0)
		return 1;

	// Connect to the server
	result = networkManager.ConnectSocket(authServerSocket);

	if (result != 0)
		return 1;

	networkManager.authServerSocketNumber = authServerSocket;

	//------------------Initialize sets-------------------------------

	FD_SET activeSockets;
	FD_SET socketsReadyForReading;

	FD_ZERO(&activeSockets);

	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	//--------------------Loop starts---------------------------------

	while (true)
	{
		//-----------------------------To listen to messages from Client Servers-----------------------------------------------------------

		FD_ZERO(&socketsReadyForReading);

		FD_SET(serverSocket, &socketsReadyForReading);

		// Add all connected clients to the sets.
		networkManager.AddAllClientsToFDSET(socketsReadyForReading);

		networkManager.readCount = select(0, &socketsReadyForReading, NULL, NULL, &tv); // Using the `select` function to check for sockets ready for reading.

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

		// Loop through the client list to process for messages.
		result = networkManager.LoopThroughClientList(socketsReadyForReading, activeSockets, message, buffer, bufSize);

		if (result != 0)
			break;

		if (networkManager.readCount > 0)
		{	
			if (FD_ISSET(serverSocket, &socketsReadyForReading))
			{	
				result = networkManager.AddNewClientToList(serverSocket, socketsReadyForReading, activeSockets); // Adding new client to the list.

				if (result != 0)
					break;
			}
		}

		//-----------------------------To listen to messages from Authentication Server----------------------------------------------------

		FD_ZERO(&socketsReadyForReading);

		FD_SET(networkManager.authServerSocketNumber, &socketsReadyForReading);

		networkManager.readCount = select(0, &socketsReadyForReading, NULL, NULL, &tv); // Using the `select` function to check for sockets ready for reading.

		if (networkManager.readCount == 0)
		{
			continue;
		}

		if (networkManager.readCount == SOCKET_ERROR)
		{
			printf("Select() ---> FAILED | ERROR : %d\n", WSAGetLastError());
			closesocket(networkManager.authServerSocketNumber);
			return 1;
		}

		if (networkManager.readCount > 0)
		{
			if (FD_ISSET(networkManager.authServerSocketNumber, &socketsReadyForReading))
			{
				// Receive a message from the authServer
				result = networkManager.ReceiveMessageFromAuthServer(message, buffer);

				if (result != 0)
				{
					FD_CLR(networkManager.authServerSocketNumber, &activeSockets);
					FD_CLR(networkManager.authServerSocketNumber, &socketsReadyForReading);
					networkManager.CleanSocket(networkManager.authServerSocketNumber, networkManager.info_authServer);
					return 1;
				}
			}
		}
	}

	// Clean up and close the server socket.
	networkManager.CleanSocket(serverSocket, networkManager.info);
	networkManager.CleanSocket(networkManager.authServerSocketNumber, networkManager.info_authServer);

	return 0;
}

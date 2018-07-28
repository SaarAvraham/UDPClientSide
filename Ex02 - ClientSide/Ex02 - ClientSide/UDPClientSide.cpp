#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
// Don't forget to include "ws2_32.lib" in the library list.
#include <winsock2.h> 
#include <string.h>

#define TIME_PORT	27015

bool connectToServer(string i_IPAddress, SOCKET& io_ConnSocket, sockaddr_in& io_Server)
{
	// Initialize Winsock (Windows Sockets).

	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Client: Error at WSAStartup()\n";

		return false;
	}

	// Client side:
	// Create a socket and connect to an internet address.

	io_ConnSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == io_ConnSocket)
	{
		cout << "Time Client: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();

		return false;
	}

	// For a client to communicate on a network, it must connect to a server.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called server. 

	io_Server.sin_family = AF_INET;
	io_Server.sin_addr.s_addr = inet_addr("127.0.0.1");
	io_Server.sin_port = htons(TIME_PORT);

	return true;
}

bool sendMessageToServer(const char* i_Message, SOCKET& io_ConnSocket, sockaddr_in& io_Server)
{
	int bytesSent = 0;
	int bytesRecv = 0;

	// Asks the server what's the currnet time.
	// The send function sends data on a connected socket.
	// The buffer to be sent and its size are needed.
	// The fourth argument is an idicator specifying the way in which the call is made (0 for default).
	// The two last arguments hold the details of the server to communicate with. 
	// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
	bytesSent = sendto(io_ConnSocket, i_Message, (int)strlen(i_Message), 0, (const sockaddr *)&io_Server, sizeof(io_Server));
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Client: Error at sendto(): " << WSAGetLastError() << endl;
		closesocket(io_ConnSocket);
		WSACleanup();

		return false;
	}
	cout << "Time Client: Sent: " << bytesSent << "/" << strlen(i_Message) << " bytes of \"" << i_Message << "\" message.\n";

	return true;
}

bool recieveMessageFromServer(char* i_ReciveBuffer, SOCKET& io_ConnSocket, sockaddr_in& io_Server)
{
	int bytesRecv = 0;

	// Gets the server's answer using simple recieve (no need to hold the server's address).
	bytesRecv = recv(io_ConnSocket, i_ReciveBuffer, 255, 0);
	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(io_ConnSocket);
		WSACleanup();

		return false;
	}

	i_ReciveBuffer[bytesRecv] = '\0'; //add the null-terminating to make it a string
	cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << i_ReciveBuffer << "\" message.\n";

	return true;
}

void main()
{
	SOCKET connSocket;
	sockaddr_in server;
	char sendBuff[255] = "What's the time?";
	char recvBuff[255];

	if (!connectToServer("127.0.0.1", connSocket, server))
	{
		return;
	}

	if (!sendMessageToServer(sendBuff, connSocket, server))
	{
		return;
	}

	if (!recieveMessageFromServer(recvBuff, connSocket, server))
	{
		return;
	}

	// Closing connections and Winsock.
	cout << "Time Client: Closing Connection.\n";
	closesocket(connSocket);
}
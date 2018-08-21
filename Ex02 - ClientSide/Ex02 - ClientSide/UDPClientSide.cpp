#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
// Don't forget to include "ws2_32.lib" in the library list.
#include <winsock2.h> 
#include <string.h>
#include <sstream>
#include <Windows.h>

#define TIME_PORT	27015
#define QUIT_CHOICE 0
#define DELAY_ESTIMATION 4
#define DELAY_ESTIMATION_REQUESTS 100
#define MEASURE_RTT 5
#define MEASURE_RTT_REQUESTS 100
#define FIRST_CHOICE 1
#define LAST_CHOICE 11
#define BUFFER_SIZE 255

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
	bytesRecv = recv(io_ConnSocket, i_ReciveBuffer, BUFFER_SIZE, 0);
	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(io_ConnSocket);
		WSACleanup();

		return false;
	}

	i_ReciveBuffer[bytesRecv] = '\0'; //add the null-terminating to make it a string
	cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << i_ReciveBuffer << "\" message.\n\n\n";

	return true;
}

void displayOptionsToUser()
{
	cout << "Please enter a number: \n";
	cout << "0 - Exit! \n";
	cout << "1 - GetTime \n";
	cout << "2 - GetTimeWithoutDate \n";
	cout << "3 - GetTimeSinceEpoch \n";
	cout << "4 - GetClientToServerDelayEstimation \n";
	cout << "5 - MeasureRTT \n";
	cout << "6 - GetTimeWithoutDateOrSeconds \n";
	cout << "7 - GetYear \n";
	cout << "8 - GetMonthAndDay \n";
	cout << "9 - GetSecondsSinceBeginingOfMonth \n";
	cout << "10 - GetDayOfYear \n";
	cout << "11 - GetDaylightSavings \n\n";
}

bool isLegitChoice(const char* i_UserInput, int &o_UserInputNumber)
{
	int i = 0, mult = 1, len = strlen(i_UserInput);
	o_UserInputNumber = 0;
	bool isNumber = true;
	bool isInRange = true;

	for (int i = 0; i < len && isNumber; i++)
	{
		if (i_UserInput[i] < '0' || i_UserInput[i] > '9')
		{
			isNumber = false;
		}
		else // Current char is a number.
		{
			o_UserInputNumber = o_UserInputNumber*mult + i_UserInput[i] - '0';
			mult *= 10;
		}
	}

	if (o_UserInputNumber < QUIT_CHOICE || o_UserInputNumber > LAST_CHOICE)
	{
		isInRange = false;
	}

	return isInRange && isNumber;
}

DWORD calculateDelay(DWORD i_Responses[], int i_Length)
{
	DWORD average, sum = 0;

	for (int i = 0; i < i_Length - 1; i++)
	{
		sum += i_Responses[i + 1] - i_Responses[i];
	}

	average = sum / (i_Length - 1);

	return average;
}

bool getClientToServerDelayEstimation(char* io_SendBuff, char* io_RecvBuff, SOCKET &io_ConnSocket, sockaddr_in &io_Server)
{
	DWORD responsesFromServer[DELAY_ESTIMATION_REQUESTS];


	for (int i = 0; i < DELAY_ESTIMATION_REQUESTS; i++)
	{
		if (!sendMessageToServer(io_SendBuff, io_ConnSocket, io_Server))
		{
			return false;
		}
	}

	for (int i = 0; i < DELAY_ESTIMATION_REQUESTS; i++)
	{
		if (!recieveMessageFromServer(io_RecvBuff, io_ConnSocket, io_Server))
		{
			return false;
		}

		responsesFromServer[i] = atoi(io_RecvBuff);
	}

	cout << "\nClient to server delay estimation: " << calculateDelay(responsesFromServer, DELAY_ESTIMATION_REQUESTS) << " milliseconds\n" << endl;

	return true;
}

bool measureRTT(char* io_SendBuff, char* io_recvBuff, SOCKET &io_ConnSocket, sockaddr_in &io_Server)
{
	DWORD startTime, endTime, deltaTime, delayInMilliseconds = 0;

	for (int i = 0; i < DELAY_ESTIMATION_REQUESTS; i++)
	{
		startTime = GetTickCount();
		if (!sendMessageToServer(io_SendBuff, io_ConnSocket, io_Server))
		{
			return false;
		}
		if (!recieveMessageFromServer(io_recvBuff, io_ConnSocket, io_Server))
		{
			return false;
		}
		endTime = GetTickCount();

		deltaTime = endTime - startTime;
		delayInMilliseconds += deltaTime;
	}

	delayInMilliseconds /= MEASURE_RTT_REQUESTS;
	cout << "RTT: " << delayInMilliseconds << " milliseconds\n" << endl;

	return true;
}

void answerUserRequests()
{
	SOCKET connSocket;
	sockaddr_in server;
	int userChoice;
	char sendBuff[BUFFER_SIZE];
	char recvBuff[BUFFER_SIZE];
	bool doesUserWantToRequest = true;

	if (!connectToServer("127.0.0.1", connSocket, server))
	{
		return;
	}

	while(doesUserWantToRequest)
	{

		displayOptionsToUser();
		cin >> sendBuff;
		if(!isLegitChoice(sendBuff, userChoice))
		{
			cout << "\nYou didn't enter a number in the given range!\n";
		}
		else
		{
			if (userChoice == QUIT_CHOICE)
			{
				doesUserWantToRequest = false;
			}
			else if (userChoice == DELAY_ESTIMATION)
			{
				if (!getClientToServerDelayEstimation(sendBuff, recvBuff, connSocket, server))
				{
					return;
				}
			}
			else if (userChoice == MEASURE_RTT)
			{
				if (!measureRTT(sendBuff, recvBuff, connSocket, server))
				{
					return;
				}
			}
			else if (userChoice >= FIRST_CHOICE && userChoice <= LAST_CHOICE)
			{
				if (!sendMessageToServer(sendBuff, connSocket, server))
				{
					return;
				}

				if (!recieveMessageFromServer(recvBuff, connSocket, server))
				{
					return;
				}
			}
		}
	}

	// Closing connections and Winsock.
	cout << "Time Client: Closing Connection.\n\n\n";
	closesocket(connSocket);
}

void main()
{
	answerUserRequests();
}
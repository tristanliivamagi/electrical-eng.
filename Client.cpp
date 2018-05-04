#include "stdafx.h"
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <conio.h>
#include "opencv.hpp"
#pragma comment(lib,".\\opencv\\lib\\opencv_world310d.lib")
#include "Winsock2.h"
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
using namespace std;

#define PORT 4618			//The port over which communication will take place, for both the client and server
#define CLIENTBUFFER 65535	//Size of the client's receive buffer in bytes

SOCKET clientSocket = 0;			//The socket on the client used to communicate with the server
									//Destructor of Client needs it so it must be a global variable

//Contains functions to (1) get and display the image from the server, and (2) read commands from the keyboard and send them to the server
//to control the motors, or change the mode (automatic or manual).
//Sample usage:
//Client client;
//while (!client.GetClientError())
//{
//	client.GetImage();
//	client.Command();
//}
class Client
{
public:
	Client();
	~Client();
	void GetImage(); //Send the "im" command to the server, receive the image, and display it. This should run in an endless loop in a separate thread. 
	void Client::Command(); //In an endless loop, receive user input and send the commands (other than im) to the server.
	bool GetClientError() { return clientError; };
private:
	WSADATA wsData;						//Required input to WSAStartup()
	int returnValue;					//Used to check return values of functions
	bool clientError = false;			//Flag that is set if an error occurs
	SOCKET clientSocket = 0;			//The socket on the client used to communicate with the server
	u_long nonblockingMode = 1;			//Required input to ioctlsocket()
	//string serverAddress = "192.168.1.24";	//The address of the server to connect to
	string serverAddress = "192.168.1.6";	//The address of the server to connect to
	sockaddr_in ipaddr;					//Structure containing the address of the server to connect to
	int errorCode;						//Used to store the return value of WSAGetLastError() to determine the error code
	int startTime;						//Used to time operations and timeout if they take too long
	int elapsedTime;					//Used to time operations and timeout if they take too long
	string command;						//The command to send to the server
	char receiveBuffer[CLIENTBUFFER];	//The receive buffer
	string str;							//The string received from the server in response to the "cmd" command
	cv::Mat receivedImage;				//The decoded image received from the server
	int imageSize;						//The size of the received image in bytes. The server sends the size of the image before sending the image.
	int bytesRead;						//The number of bytes of image data read
	int input = 0;						//First key code of key pressed
	int input2 = 0;						//Second key code of key pressed (if any)
	int keyState = 0;					//keyState >= 0: key not pressed //keyState < 0: key pressed
	bool keyPressed = false;			//Flag to indicate if a key was pressed
	int MapToVK(int input2);			//Convert key input code to virtual key code (required input format for GetKeyState())
};

//Constructor: set up sockets and connect to the server
Client::Client()
{
	//Start sockets API
	returnValue = WSAStartup(0x0101, &wsData);
	if (returnValue != 0)
	{
		clientError = true;
		cout << "Client error: failed to initialize sockets API\n";
		_getch();
	}

	//If successful, create a socket
	if (clientError == false)
	{
		clientSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (clientSocket == SOCKET_ERROR)
		{
			clientError = true;
			cout << "Client error: failed to create socket\n";
			_getch();
		}
	}

	//If successful, set the socket to nonblocking mode
	if (clientError == false)
	{
		returnValue = ioctlsocket(clientSocket, FIONBIO, &nonblockingMode);
		if (returnValue == SOCKET_ERROR)
		{
			clientError = true;
			cout << "Client error: failed to set socket to nonblocking mode\n";
			_getch();
		}
	}
	printf("Socket initialized\n");

	//If successful, attempt to connect to the server
	if (clientError == false)
	{
		ipaddr.sin_family = AF_INET;	//IPV4
		ipaddr.sin_port = htons(PORT);	//Port 4618
		ipaddr.sin_addr.s_addr = inet_addr(serverAddress.c_str());

		//Loop until successful connection has been verified, timeout after 2 seconds
		errorCode = NULL;
		double startTime = cv::getTickCount();
		double elapsedTime = 0;
		while (errorCode != WSAEISCONN && elapsedTime < 2.0)
		{
			returnValue = connect(clientSocket, (struct sockaddr *)&ipaddr, sizeof(ipaddr));
			//A nonblocking socket connection is always "unsuccessful" the first time because it does not wait to see if the connection was successful.
			errorCode = WSAGetLastError();
			elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
			Sleep(10);
		}
		if (errorCode != WSAEISCONN) //If timed out
		{
			clientError = true;
			cout << "Client error: failed to connect to server\n";
			_getch();
		}
		if (clientError == false)
			printf("Connected to server\nEnter command: \n");

	}
}

Client::~Client()
{
	closesocket(clientSocket);
	WSACleanup();
}

void Client::GetImage()
{
	//Send command to server
	command = "im";
	returnValue = send(clientSocket, command.c_str(), command.length(), 0);
	if (returnValue == SOCKET_ERROR)
	{
		clientError = true;
		cout << "Client error: failed to send command\n";
	}

	//If successful, receive image size
	if (clientError == false)
	{
		//Loop until data has been received, timeout after 2 seconds
		returnValue = SOCKET_ERROR;
		double startTime = cv::getTickCount();
		double elapsedTime = 0;
		while (returnValue == SOCKET_ERROR && elapsedTime < 2.0)
		{
			returnValue = recv(clientSocket, (char *)&imageSize, sizeof(imageSize), 0);
			elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
		}
	}
	if (returnValue == SOCKET_ERROR) //If timed out
	{
		//clientError = true;
		cout << "Client error: failed to receive image size from server\n";
	}
	else if (returnValue == 0) //If connection was gracefully closed
	{
		clientError = true; //This isn't really an error, this is just to get the loop to exit
		cout << "Connection was closed. Exiting.\n";
	}

	//Allow some time for the image to be transmitted
	Sleep(200);

	//If successful, receive image
	if (clientError == false)
	{
		//Loop until all bytes of the image have been received, timeout after 2 seconds
		bytesRead = 0;
		double startTime = cv::getTickCount();
		double elapsedTime = 0;
		while (bytesRead < imageSize && elapsedTime < 2.0)
		{
			returnValue = recv(clientSocket, &receiveBuffer[bytesRead], CLIENTBUFFER, 0);
			//If data received then offset for next potential read
			if (returnValue > 0)
			{
				bytesRead += returnValue;
			}
			elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
		}
		if (bytesRead != imageSize)
		{
			//clientError = true;
			cout << "Client error: failed to receive all image bytes from server\n";
		}

		//If successful, decode and display the image
		if (clientError == false && imageSize > 0)
		{
			receivedImage = imdecode(cv::Mat(imageSize, 1, CV_8UC3, receiveBuffer), 1);
			if (receivedImage.empty() == false)
			{
				cv::imshow("rx", receivedImage);
				cv::waitKey(100);
			}
		}
	}
}

void Client::Command()
{
	if (keyPressed == true)
	{
		keyState = GetKeyState(MapToVK(input2));

		if (keyState >= 0)
		{
			//Send command to server
			command = "s";
			returnValue = send(clientSocket, command.c_str(), command.length(), 0);
			if (returnValue == SOCKET_ERROR)
			{
				clientError = true;
				cout << "Client error: failed to send command\n";
			}

			printf("STOP\n");
			keyPressed = false;
			input = 0;
			input2 = 0;
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		}
	}
	else
	{
		if (_kbhit())
		{
			input = _getch();
			if (input == 224)
			{
				input2 = _getch();

				if (input2 == 72/*up arrow*/)
				{
					//Send command to server
					command = "f";
					returnValue = send(clientSocket, command.c_str(), command.length(), 0);
					if (returnValue == SOCKET_ERROR)
					{
						clientError = true;
						cout << "Client error: failed to send command\n";
					}

					printf("FWD\n");
					keyPressed = true;
				}
				else if (input2 == 80/*down arrow*/)
				{
					//Send command to server
					command = "b";
					returnValue = send(clientSocket, command.c_str(), command.length(), 0);
					if (returnValue == SOCKET_ERROR)
					{
						clientError = true;
						cout << "Client error: failed to send command\n";
					}

					printf("BACK\n");
					keyPressed = true;
				}
				else if (input2 == 75/*left arrow*/)
				{
					//Send command to server
					command = "l";
					returnValue = send(clientSocket, command.c_str(), command.length(), 0);
					if (returnValue == SOCKET_ERROR)
					{
						clientError = true;
						cout << "Client error: failed to send command\n";
					}

					printf("LEFT\n");
					keyPressed = true;
				}
				else if (input2 == 77/*right arrow*/)
				{
					//Send command to server
					command = "r";
					returnValue = send(clientSocket, command.c_str(), command.length(), 0);
					if (returnValue == SOCKET_ERROR)
					{
						clientError = true;
						cout << "Client error: failed to send command\n";
					}

					printf("RIGHT\n");
					keyPressed = true;
				}
			}
			else if (input == 'a')
			{
				//Send command to server
				command = "a";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("AUTO\n");
			}
			else if (input == 'm')
			{
				//Send command to server
				command = "m";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("MANUAL\n");
			}
			else if (input == 'x')
				clientError = true; //Exit
			else
				printf("Invalid command\n");
		}
	}
}

int Client::MapToVK(int input2)
{
	if (input2 == 72)
		return VK_UP;
	else if (input2 == 80)
		return VK_DOWN;
	else if (input2 == 75)
		return VK_LEFT;
	else if (input2 == 77)
		return VK_RIGHT;
	//Don't press a function key!
}

int main()
{
	Client client;
	while (!client.GetClientError())
	{

		client.GetImage();
		client.Command();
	}
	return 0;
}//end main
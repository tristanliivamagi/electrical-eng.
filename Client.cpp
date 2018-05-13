#include "stdafx.h"
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <conio.h>
#include <sstream>
#include "opencv.hpp"
#pragma comment(lib,".\\opencv\\lib\\opencv_world310d.lib")
#include "Winsock2.h"
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
using namespace std;

#define PORT 4618			//The port over which communication will take place, for both the client and server
#define CLIENTBUFFER 1000000	//Size of the client's receive buffer in bytes
#define IMAGEDELAY 10 //Time in ms to wait for the image to transmit
#define DELAYTIME 100 //Delay time in ms for disabling the image transmission when a priority command is sent

//Status flags returned by GetCommand()
#define PRIORITYCOMMANDSENT 1
#define STOPCOMMANDSENT 2
#define NOCOMMANDSENT 3

SOCKET clientSocket = 0;			//The socket on the client used to communicate with the server
									//Destructor of Client needs it so it must be a global variable

class Client
{
public:
	Client();
	~Client();
	void GetImage(); //Send the "im" command to the server, receive the image, and display it.
	bool Client::Command(); //Receive user input and send the commands (other than im) to the server.
	bool GetClientError() { return clientError; };
	bool GetManual() { return manual; }
private:
	WSADATA wsData;						//Required input to WSAStartup()
	int returnValue;					//Used to check return values of functions
	bool clientError = false;			//Flag that is set if an error occurs
	SOCKET clientSocket = 0;			//The socket on the client used to communicate with the server
	u_long nonblockingMode = 1;			//Required input to ioctlsocket()
	//string serverAddress = "192.168.1.22";	//The address of the server to connect to
	string serverAddress = "192.168.1.10";	//The address of the server to connect to
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
	int keyState = 0;					//keyState >= 0: key not pressed //keyState < 0: key pressed
	bool keyPressed = false;			//Flag to indicate if a key was pressed
	int MapToVK(int input2);			//Convert key input code to virtual key code (required input format for GetKeyState())
	bool commandPriority = false;		//Flag that is used to briefly disable getting the image when a command is sent, in order to increase the speed of
										//commands that occur in rapid sequence
	bool manual = true;					//flag to indicate that the robot is in manual mode, meaning the client is able to send commands to it
	int ultrasonicReading;
	string ultrasonicDistanceStr;
	ostringstream convert;
	bool ultrasonicStatus;
};
//=============================================================================================================
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
//=============================================================================================================
Client::~Client()
{
	closesocket(clientSocket);
	WSACleanup();
}
//==============================================================================================================
void Client::GetImage()
{
	//Send command to server
	command = "i";
	returnValue = send(clientSocket, command.c_str(), command.length(), 0);
	if (returnValue == SOCKET_ERROR)
	{
		clientError = true;
		cout << "Client error: failed to send command\n";
	}

	//If successful, receive ultrasonic reading
	if (clientError == false)
	{
		//Loop until data has been received, timeout after 2 seconds
		returnValue = SOCKET_ERROR;
		double startTime = cv::getTickCount();
		double elapsedTime = 0;
		while (returnValue == SOCKET_ERROR && elapsedTime < 2.0)
		{
			returnValue = recv(clientSocket, (char *)&ultrasonicReading, sizeof(ultrasonicReading), 0);
			elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
		}
	}
	if (returnValue == SOCKET_ERROR) //If timed out
	{
		//clientError = true;
		cout << "Client error: failed to receive ultrasonic reading from server\n";
	}
	else if (returnValue == 0) //If connection was gracefully closed
	{
		clientError = true; //This isn't really an error, this is just to get the loop to exit
		cout << "Connection was closed. Exiting.\n";
	}

	//If successful, receive ultrasonic status
	if (clientError == false)
	{
		//Loop until data has been received, timeout after 2 seconds
		returnValue = SOCKET_ERROR;
		double startTime = cv::getTickCount();
		double elapsedTime = 0;
		while (returnValue == SOCKET_ERROR && elapsedTime < 2.0)
		{
			returnValue = recv(clientSocket, (char *)&ultrasonicStatus, sizeof(ultrasonicStatus), 0);
			elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
		}
	}
	if (returnValue == SOCKET_ERROR) //If timed out
	{
		//clientError = true;
		cout << "Client error: failed to receive ultrasonic reading from server\n";
	}
	else if (returnValue == 0) //If connection was gracefully closed
	{
		clientError = true; //This isn't really an error, this is just to get the loop to exit
		cout << "Connection was closed. Exiting.\n";
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
				convert.str(string());//clear the stream
				convert << ultrasonicReading;
				ultrasonicDistanceStr = convert.str();
				cv::Point textLowerLeft = cv::Point(200, 90);
				cv::putText(receivedImage, ultrasonicDistanceStr, textLowerLeft, CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1, 8, false);
				cv::Point circlePosition = cv::Point(205, 100);
				if(ultrasonicStatus == false)
					cv::circle(receivedImage, circlePosition, 4.0, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
				else
					cv::circle(receivedImage, circlePosition, 4.0, cv::Scalar(0, 255, 0), -1, cv::LINE_AA);
				cv::imshow("rx", receivedImage);
				cv::waitKey(100);
			}
		}
	}
}
//===========================================================================================================
bool Client::Command()
{
	if (keyPressed == true)
	{
		keyState = GetKeyState(MapToVK(input));

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
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			commandPriority = false;
			Sleep(100);
		}
	}
	else
	{
		if (_kbhit())
		{
			input = _getch();

			if (input == 56)//8, forward
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
				commandPriority = true;
			}
			else if (input == 53)//5, backward
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
				commandPriority = true;
			}
			else if (input == 52)//4, turn left on the spot
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
				commandPriority = true;
			}
			else if (input == 54)//6, turn right on the spot
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
				commandPriority = true;
			}
			else if (input == 47)///, pulse forward
			{
				//Send command to server
				command = "/";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("PULSE FWD\n");
			}
			else if (input == 42)//*, smart pulse forward
			{
				//Send command to server
				command = "*";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("SMART PULSE FWD\n");
			}
			else if (input == 50)//2, pulse backward
			{
				//Send command to server
				command = "2";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("PULSE BACKWARD\n");
			}
			else if (input == 55)//7, gradual left
			{
				//Send command to server
				command = "7";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("GRADUAL LEFT\n");
				keyPressed = true;
				commandPriority = true;
			}
			else if (input == 57)//9, gradual right
			{
				//Send command to server
				command = "9";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("GRADUAL RIGHT\n");
				keyPressed = true;
				commandPriority = true;
			}
			else if (input == 'a')//a, automatic
			{
				//Send command to server
				command = "a";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				manual = false;
				printf("AUTO\n");
			}
			else if (input == 'R')
			{
				//Send command to server
				command = "R";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("RED\n");
			}
			else if (input == 'G')
			{
				//Send command to server
				command = "G";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("GREEN\n");
			}
			else if (input == 'B')
			{
				//Send command to server
				command = "B";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("BLUE\n");
			}
			else if (input == 'P')
			{
				//Send command to server
				command = "P";
				returnValue = send(clientSocket, command.c_str(), command.length(), 0);
				if (returnValue == SOCKET_ERROR)
				{
					clientError = true;
					cout << "Client error: failed to send command\n";
				}

				printf("PINK\n");
			}
			else if (input == 'x')
				clientError = true; //Exit
			else
				printf("Invalid command\n");
		}
	}
	return commandPriority;
}

int Client::MapToVK(int input)
{
	if (input == 56)
		return VK_NUMPAD8;
	else if (input == 53)
		return VK_NUMPAD5;
	else if (input == 52)
		return VK_NUMPAD4;
	else if (input == 54)
		return VK_NUMPAD6;
	else if (input == 47)
		return 47;
	else if (input == 50)
		return VK_NUMPAD2;
	else if (input == 55)
		return VK_NUMPAD7;
	else if (input == 57)
		return VK_NUMPAD9;
	else
		return 1;//left mouse button... just something random
}

int main()
{
	bool commandPriority = false;
	bool previousCommandPriority = false;
	bool disableImage = false;
	double commandPriorityStartTime;
	double commandPriorityElapsedTime;

	Client client;
	Sleep(1000);
	while (!client.GetClientError() && client.GetManual() == true)//Run as long as there is no error and robot is in manual mode
	{
		if (disableImage == false)
		{
			client.GetImage();
		}
		commandPriority = client.Command();

		//----------------------------------------------------------------------------------------------------------------------------------------
		//Enable image transmission again after a certain amount of time, to allow the image to continue to be transmitted when a key is held down
		if (commandPriority == true)
		{
			if (previousCommandPriority == false)//was a priority command just sent?
			{
				disableImage = true;
				commandPriorityStartTime = cv::getTickCount();
				previousCommandPriority = true;
			}
			else
			{
				commandPriorityElapsedTime = (double)(cv::getTickCount() - commandPriorityStartTime) / (double)cv::getTickFrequency();
				if (commandPriorityElapsedTime > ((double)DELAYTIME / 1000.0))
					disableImage = false;
			}
		}
		else
		{
			disableImage = false;
			previousCommandPriority = false;
		}
	}
	if (!client.GetClientError())
	{
		printf("Robot set to automatic mode. Press any key to exit.\n");
		_getch();
	}
	return 0;
}//end main
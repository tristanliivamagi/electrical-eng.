#include <iostream>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <pigpio.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include "raspicam_cv.h"

#include "Color.h"
using namespace std;

#define PORT 4618			//The port over which communication will take place, for both the client and server
#define BACKLOG 5			//The maximum length of the queue of pending connections to the server. A required input to listen()
#define SERVERBUFFER 1		//Size of the server's receive buffer in bytes
#define FRAME_WIDTH 640		//Width of the image from the camera in pixels
#define FRAME_HEIGHT 480	//Height of the image from the camera in pixels

//GPIO numbers
#define TRIGGER 2
#define ECHO 4
#define AIN1 17
#define AIN2 27
#define PWMA 22
#define BIN1 10
#define BIN2 9
#define PWMB 11

//Movement modes
#define STOP 0
#define FORWARD 1
#define BACKWARD -1
#define TURN 2
//------------
#define LEFT 3
#define RIGHT 4

//Relative motor speeds
#define MAXSPEEDLEFT 100
#define MAXSPEEDRIGHT 100

//Hysteresis value for position setpoint when turning
//System will be satisfied when position is within +/- TURNINGHYSTERESIS of the setpoint, which is 0, the centre of the screen
#define TURNINGHYSTERESIS 50

#define OBJECTLOSTMINSPEED 25 	//The minimum speed to slow down to when the object is lost
#define OBJECTLOSTTURNADJUST 1 	//Amount to adjust the turning variable by when the object is lost
#define LONGWAY 1 //Comment out this line to go the short way
#define MANUALTURNSPEED 50
#define MANUALFORWARDSPEED 100
#define MANUALBACKWARDSPEED 100

//Image cropping settings
//These settings have been carefully calibrated
#define RESIZEDWIDTH 680
#define RESIZEDHEIGHT 480
#define LEFTEDGE 75
#define TOPEDGE 130
#define WIDTH 480
#define HEIGHT 350

raspicam::RaspiCam_Cv Camera;
cv::Mat rawCameraImage;
cv::Mat cameraImage;
cv::Mat resizedCameraImage;
//cv::VideoCapture vid;

//============================================================================================================================
//This class controls the motors.
//To use it, call the member functions Stop(), Forward(), etc.
class Motor
{
public:
	Motor();
	void Stop();
	void Forward(int speed);
	void Backward(int speed);
	void Turn(int speed, int direction); 	//"on the spot" turn
	void Steer(); 							//ADJUSTS the motor speeds based on the turning variable to turn the robot.
	void SlowDown(int amount);				//ADJUSTS the motor speeds equally to slow them down by the amount passed to it.
	void SpeedUp(int amount);				//ADJUSTS the motor speeds equally to speed them up by the amount passed to it.
	int GetAverageSpeed() {return (leftSpeed + rightSpeed)/2;}
	int turning = 0;	//The amount of turning. Positive is to the right and negative is to the left.
private:
	int mode;
	int leftSpeed;
	int rightSpeed;
	void SetSpeed(int speed); //Used internally to SET the speed when both motors are to turn at the same speed.
};

Motor::Motor()
{
	//Assuming the gpio has been initialized elsewhere
	gpioSetMode(AIN1, PI_OUTPUT);
	gpioSetMode(AIN2, PI_OUTPUT);
	gpioSetMode(PWMA, PI_OUTPUT);
	gpioSetMode(BIN1, PI_OUTPUT);
	gpioSetMode(BIN2, PI_OUTPUT);
	gpioSetMode(PWMB, PI_OUTPUT);
	gpioSetPWMrange(PWMA, 100);
	gpioSetPWMrange(PWMB, 100);
	Stop();
}

void Motor::Stop()
{
	gpioWrite(AIN1, 0);
	gpioWrite(AIN2, 0);
	gpioWrite(BIN1, 0);
	gpioWrite(BIN2, 0);
	mode = STOP;
	cv::waitKey(10);
}

void Motor::Forward(int speed)
{
	if (mode == FORWARD)
		SetSpeed(speed);
	else
	{
		Stop();
		SetSpeed(speed);
		gpioWrite(AIN2, 0);
		gpioWrite(AIN1, 1);
		gpioWrite(BIN2, 0);
		gpioWrite(BIN1, 1);
		mode = FORWARD;
	}
}

void Motor::Backward(int speed)
{
	if (mode == BACKWARD)
		SetSpeed(speed);
	else
	{
		Stop();
		SetSpeed(speed);
		gpioWrite(AIN1, 0);
		gpioWrite(AIN2, 1);
		gpioWrite(BIN1, 0);
		gpioWrite(BIN2, 1);
		mode = BACKWARD;
	}
}

void Motor::Turn(int speed, int direction)
{
	Stop();
	SetSpeed(speed);
	if (direction == RIGHT)
	{
		gpioWrite(AIN1, 0);
		gpioWrite(AIN2, 1);
		gpioWrite(BIN2, 0);
		gpioWrite(BIN1, 1);
	}
	else
	{
		gpioWrite(AIN2, 0);
		gpioWrite(AIN1, 1);
		gpioWrite(BIN1, 0);
		gpioWrite(BIN2, 1);
	}
	mode = TURN;
}

void Motor::Steer()
{
	leftSpeed += turning;
	rightSpeed -= turning;
	gpioPWM(PWMA, leftSpeed);
	gpioPWM(PWMB, rightSpeed);
}

void Motor::SlowDown(int amount)
{
	leftSpeed -= amount;
	rightSpeed -= amount;
}

void Motor::SpeedUp(int amount)
{
	leftSpeed += amount;
	rightSpeed += amount;
}

void Motor::SetSpeed(int speed)
{
	leftSpeed = (int)(((float)speed) / 100.0 * ((float)MAXSPEEDLEFT));
	rightSpeed = (int)(((float)speed) / 100.0 * ((float)MAXSPEEDRIGHT));
	gpioPWM(PWMA, leftSpeed);
	gpioPWM(PWMB, rightSpeed);
}
//=========================================================================================================================================

class Server
{
public:
	void Run();									//The main program
	Server();									//Constructor: gets connected to the client
	~Server();									//Destructor: cleans up
	bool GetServerError() 						{return serverError;};
private:
	Motor motor;
	bool serverError = false;					//Flag that is set when there is an error
	int addressSize;							//The size of the sockaddr_in structure, which may be different on the Pi
	int returnValue;							//Used to check return values of functions
	struct sockaddr_in serverAddress;			//Holds settings for serverSocket such as port and IP addresses to listen to. Used in bind()
	struct sockaddr_in clientAddress;			//Required input to connect()
	SOCKET serverSocket = 0;					//The socket used to listen for incoming connections
	SOCKET clientSocket = 0;					//The socket (also on the server) used to communicate with the client
	double startTime;
	double elapsedTime;
	bool connected = false;						//Flag to indicate whether a connection with the client has been made
	char receiveBuffer[SERVERBUFFER];			//The receive buffer
	char commandBuffer[SERVERBUFFER+1];			//Used in converting data received to string
	string command;								//The command from the client
	int size;									//Size of the image in bytes
	vector<unsigned char> compressedImage;		//Holds the compressed jpeg image
	vector<int> compressionSettings;			//Holds the image compression settings
	bool received = false;
	bool manual = true;
	int stage = 1;
	int centrePosition = 0;						//Holds the centre position of the object obtain from vision functions.
	cv::Rect cropArea;
	
	
	
	CColor red;

	CColor green;
	
};

void Server::Run()
{
	Camera.grab();
	Camera.retrieve(rawCameraImage);
	//cv::imshow("raw", rawCameraImage);
	//cv::waitKey(100);
	resize(rawCameraImage, resizedCameraImage, resizedCameraImage.size(), 0, 0, CV_INTER_LINEAR);
	//cv::imshow("resized", resizedCameraImage);
	//cv::waitKey(100);
    cropArea.x = LEFTEDGE;
    cropArea.y = TOPEDGE;
    cropArea.width = WIDTH;
    cropArea.height = HEIGHT;
    cameraImage = resizedCameraImage(cropArea);
    //cv::imshow("cameraImage", cameraImage);
    //cv::waitKey(100);

	
	//if (vid.isOpened() == true)
	//{
	//	vid >> cameraImage;
	//}
	//else
	//	printf("Error: could not get image from camera\n");

	if(manual == true) //Only respond to commands in manual mode
	{
		//Receive command from client
		//Loop until data has been received, timeout after 2 seconds
		returnValue = SOCKET_ERROR;
		startTime = cv::getTickCount();
		elapsedTime = 0;
		received = false;
		while (returnValue == SOCKET_ERROR && elapsedTime < 10.0)
		{
			returnValue = recv(clientSocket, receiveBuffer, SERVERBUFFER, 0);
			elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
		}
		if (returnValue == SOCKET_ERROR) //If nothing received
		{
			serverError = true;
			cout << "Server error: failed to receive data from client";
		}
		else if (returnValue == 0) //If connection gracefully closed
		{
			serverError = true;
			cout << "Connection was closed. Exiting.";
		}
		else
			received = true;
			
		//If data received, respond to the command
		if (serverError == false && received == true)
		{
			//Convert received data to string
			commandBuffer[0] = receiveBuffer[0];
			commandBuffer[returnValue] = 0; //Add NULL terminator
			command = commandBuffer;
			
			//If the client sent the "s" command, stop
			if (command == "s")
			{
				motor.Stop();
				printf("Stopping\n");
			}
			//If the client sent the "f" command, move forward
			else if (command == "f")
			{
				motor.Forward(MANUALFORWARDSPEED);
				printf("Moving forward\n");
			}
			//If the client sent the "b" command, move backward
			else if (command == "b")
			{
				motor.Backward(MANUALBACKWARDSPEED);
				printf("Moving backward\n");
			}
			//If the client sent the "l" command, turn left on the spot
			else if (command == "l")
			{
				motor.Turn(MANUALTURNSPEED, LEFT);
				printf("Turning left on the spot\n");
			}
			//If the client sent the "r" command, turn right on the spot
			else if (command == "r")
			{
				motor.Turn(MANUALTURNSPEED, RIGHT);
				printf("Turning right on the spot\n");
			}
			//If the client sent the "a" command, switch into automatic mode
			else if (command == "a")
			{
				manual = false;
				printf("Automatic mode\n");
			}
			//If the client sent the "R" command, switch into calibration mode for red
			else if (command == "R")
			{
				printf("Calibrate red\n");
			}
			//If the client sent the "G" command, switch into calibration mode for green
			else if (command == "G")
			{
				printf("Calibrate green\n");
			}
			//If the client sent the "B" command, switch into calibration mode for blue
			else if (command == "B")
			{
				printf("Calibrate blue\n");
			}
			//If the client sent the "P" command, switch into calibration mode for pink
			else if (command == "P")
			{
				printf("Calibrate pink\n");
			}
							
			//If the client sent the "im" command, send it the image size and an image from the camera
			else if (command == "i")
			{
				//Compress image to jpeg format
				compressedImage.clear();
				if (cameraImage.empty() == false)
				{
					compressionSettings.push_back(cv::IMWRITE_JPEG_QUALITY); //Set jpeg quality...
					compressionSettings.push_back(30);						 //...to 30 (out of 100)
					cv::imencode("image.jpg", cameraImage, compressedImage, compressionSettings);
				}
	
				//Send image size
				size = compressedImage.size();
				send(clientSocket, (char *)&size, sizeof(size), 0);
				if (returnValue == SOCKET_ERROR)
				{
					serverError = true;
					cout << "Server error: failed to send image size";
				}
	
				//If successful, send image
				if (serverError == false)
				{
					send(clientSocket, reinterpret_cast<char*>(&compressedImage[0]), compressedImage.size(), 0);
					if (returnValue == SOCKET_ERROR)
					{
						serverError = true;
						cout << "Server error: failed to send image";
					}
				}
			}
			//Otherwise, print the string received
			else 
				std :: cout <<"Invalid command "<< command << "\n";
		}//end "if data received, respond to command"
	}//end "if in manual mode"
	
	else //Automatic mode//Automatic mode//Automatic mode//Automatic mode//Automatic mode//Automatic mode
	{
		//int greenpos;
		//int redpos;
		//greenpos = green.vision_go(cameraImage);
		float gain = 0.1;
		int visiondistance;
		bool nextisleft;
		bool letsturn = false;
		
		if(LONGWAY == 1)
		{
			if(stage >= 1 && stage <= 11)
			{
				//Align
				do
				{
					//---Get centre position of object---
					//green object
					if(stage==1 || stage==2 || stage==5 || stage==6 || stage==7 || stage==10 || stage==11)
					{
						centrePosition = green.vision_go(cameraImage);
						visiondistance = green.distance;
						nextisleft=false;
						
					}
					else//red object
					{
						centrePosition = red.vision_go(cameraImage);
						visiondistance = red.distance;
						nextisleft=true;
					}
					
					if(centrePosition == -1000)//no object found
					{
						//Delay
						startTime = cv::getTickCount();
						elapsedTime = 0;
						received = false;
						while (returnValue == SOCKET_ERROR && elapsedTime < 10.0)
						{
							returnValue = recv(clientSocket, receiveBuffer, SERVERBUFFER, 0);
							elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
						}

						//Slow down but don't stop
						if(motor.GetAverageSpeed() > OBJECTLOSTMINSPEED)
							motor.SlowDown(1);
						//Incrementally turn the other way
						if(motor.turning != 0)
							motor.turning += -motor.turning/motor.turning*OBJECTLOSTTURNADJUST;
					}
					else if(centrePosition == 1000)//greater than 50 contours found (too much noise in the signal)
					{
						//Slow down but don't stop
						if(motor.GetAverageSpeed() > OBJECTLOSTMINSPEED)
							motor.SlowDown(1);
						//Incrementally turn the other way
						if(motor.turning != 0)
							motor.turning += -motor.turning/motor.turning*OBJECTLOSTTURNADJUST;
					}
					
					//Calculate proportional response
					//Calculate integral response
					//Calculate total response
					//Set turning variable based on total response
					//Turn based on turning variable
					
						
					//Move
					motor.turning= ( centrePosition * gain * visiondistance) + motor.turning;
					//Go to next stage
				
					if(visiondistance<50)
					{
						letsturn=true;
						stage++;
						if(nextisleft==true)
						{
							//turnleft
						}
						else
						{
							//turnright
						}
					}
				
				}
				while(centrePosition < -TURNINGHYSTERESIS || centrePosition > TURNINGHYSTERESIS);
			
			}
		}
		else
		{
		}
	}
}//end Automatic mode//end Automatic mode//end Automatic mode//end Automatic mode//end Automatic mode//end Automatic mode



Server::Server()
{
	
	green.colorid('g');//load color id with default hsv values
	red.colorid('r');
	
	//Initialize resizedCameraImage
	resizedCameraImage = cv::Mat::zeros(cv::Size(RESIZEDWIDTH, RESIZEDHEIGHT), CV_8UC3);

	//Get size of sockaddr_in structure
	addressSize = sizeof(serverAddress);

	//If successful, create serverSocket
	if (serverError == false)
	{
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocket == SOCKET_ERROR)
		{
			serverError = true;
			cout << "Server error: failed to create socket";
		}
	}

	//If successful, set serverSocket to nonblocking mode
	if (serverError == false)
	{
		int flags = fcntl(serverSocket, F_GETFL, 0);
		if (flags < 0)
		{
			serverError = true;
			cout << "Server error: fcntl() could not get flags for serverSocket";
		}
		else //no problems, carry on
		{
			flags = flags | O_NONBLOCK;
			returnValue = fcntl(serverSocket, F_SETFL, flags);
			if (returnValue != 0)
			{
				serverError = true;
				cout << "Server error: failed to set serverSocket to nonblocking mode";
			}
		}
	}

	//If successful, bind serverSocket to a port
	if (serverError == false)
	{
		serverAddress.sin_family = AF_INET;   //IPV4
		serverAddress.sin_port = htons(PORT); //Port 4618
		serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); //Listen to any IP address
		returnValue = ::bind(serverSocket, (sockaddr *)&serverAddress, sizeof(serverAddress));
		if (returnValue == SOCKET_ERROR)
		{
			serverError = true;
			cout << "Server error: failed to bind serverSocket to a port";
		}
	}

	//If successful, set serverSocket to listen
	if (serverError == false)
	{
		returnValue = listen(serverSocket, BACKLOG);
		if (returnValue == SOCKET_ERROR)
		{
			serverError = true;
			cout << "Server error: failed to set serverSocket to listen";
		}
	}
	
	cout << "Waiting for connection...\n";
	//If successful, keep checking for an incoming connection from the client. When there is one, set the socket for communicating with the client to nonblocking mode.
	while (connected == false && serverError == false)
	{
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&addressSize);
		if (clientSocket != INVALID_SOCKET) //If there was an incoming connection
		{
			//Set clientSocket to nonblocking mode
			int flags = fcntl(clientSocket, F_GETFL, 0);
			if (flags < 0)
			{
				serverError = true;
				cout << "Server error: fcntl() could not get flags for socket";
			}
			else //no problems, carry on
			{
				flags = flags | O_NONBLOCK;
				returnValue = fcntl(clientSocket, F_SETFL, flags);
				if (returnValue != 0)
				{
					serverError = true;
					cout << "Server error: failed to set serverSocket to nonblocking mode";
				}
			}
			printf("Connected to client\n");	
			connected = true;
		}	
	}
}

Server::~Server()
{
	close(clientSocket);
	close(serverSocket);
}
//=======================================================================================================================================
int main()
{
	//Initialize GPIO
	if (gpioInitialise() < 0)
	{
		printf("Could not initialize GPIO!\n");
		int i; std::cin >> i;
	}
	
	//Initialize camera
	//vid.open(0);
	//if (vid.isOpened() == false)
	//	printf("Error: could not open camera\n");
		
	Camera.open();

	Server server;

	while (!server.GetServerError())
	{	
		server.Run();		
	}
		
	gpioTerminate();
	Camera.release();
}//end main

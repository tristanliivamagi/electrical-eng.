#include <iostream>
//#include <ncurses.h>
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

//Hysteresis value for position setpoint when turning
//System will be satisfied when position is within +/- TURNINGHYSTERESIS of the setpoint, which is 0, the centre of the screen
#define TURNINGHYSTERESIS 50

//Relative motor speeds
#define MAXSPEEDLEFT 90
#define MAXSPEEDRIGHT 100

#define TIGHTNESS 50 //Gradual turn tightness
#define PULSETIME 40 //forward and backward pulse time in ms
#define IDEALDISTANCE 175 //ideal distance to begin a gradual turn in mm
#define TIMEPERDISTANCE 1600 //number of us required to drive 1 mm

#define scaledist 6350
#define turndist 175 //mm
#define slowdist 300 //mm
#define decel 5
#define accel 5
#define turntime 4000 //blind turn durration in milliseconds
#define gain  100 //divided by 1000
#define minturn  -69 //max and min turn stops
#define maxturn  69
#define Kp 50 //divided by 1000
#define Td 0  //divided by 1000
#define startspeed 20//speed at the start
#define MaxSpeedAuto  25
#define speedinturn 50
#define turnrad 10
#define OBJECTLOSTMINSPEED 18	//The minimum speed to slow down to when the object is lost
#define OBJECTLOSTTURNADJUST 5 	//Amount to adjust the turning variable by when the object is lost
#define LONGWAY 1 //Comment out this line to go the short way


//manual
#define MANUALTURNSPEED 20
#define MANUALFORWARDSPEED 100
#define MANUALBACKWARDSPEED 100
#define MANUALGRADUALTURNSPEED 80
#define MANUALGRADUALTURNTIMERIGHT 700//550
#define MANUALGRADUALTURNTIMELEFT 600//700

//Image cropping settings
//These settings have been carefully calibrated
#define RESIZEDWIDTH 680
#define RESIZEDHEIGHT 480
#define LEFTEDGE 75
#define TOPEDGE 80
#define WIDTH 480
#define HEIGHT 200
//Manual mode image cropping setings
#define LEFTEDGEMANUAL 75
#define TOPEDGEMANUAL 120//80
#define WIDTHMANUAL 480
#define HEIGHTMANUAL 150//200

//automatic settings

raspicam::RaspiCam_Cv Camera;
cv::Mat rawCameraImage;
cv::Mat cameraImage;
cv::Mat resizedCameraImage;
//cv::VideoCapture vid;

class Ultrasonic
{
public:
	double GetDistance();
	double GetFilteredDistance();
	bool GetStatus() {return reliableReading;}
	Ultrasonic();
private:
	double triggerStartTime;
	double triggerElapsedTime;
	int echoPulse = 0;
	double echoPulseTime;
	bool pulseOccurring = false;
	double distance = 0.0;
	bool pulseFinished = false;
	double filteredDistance = 0.0;
	double filterStartTime;			//Used to ensure at least 60 ms between calls to GetDistance(), to prevent one echo from interfering with the next.
	double filterElapsedTime;		//Ditto
	double tempDistance;
	int initCounter = 0;			//Used to initialize the filter array. It is set to 0 if the sensor reading is lost and the array needs to be reinitialized.
	double difference;
	int rejectionCounter = 0;
	double calDistance;
	bool reliableReading = false;
	double rejectionThreshold = 20.0;	//Reject a reading if it is this far from the reference value
	int rejectionNumber = 2;			//Recalibrate if this many values in a row rejected
	int calNumber = 2;					//During calibration, the number of similar values to read before proceeding with normal operation
};

Ultrasonic::Ultrasonic()
{
	gpioSetMode(2/*GPIO2*/, PI_OUTPUT);
	gpioSetMode(4/*GPIO4*/, PI_INPUT);
	gpioWrite(2/*GPIO2*/, 1/*Pi output high, sensor trigger input low*/); //Turn trigger output off	
	filterStartTime = cv::getTickCount();
}

double Ultrasonic::GetDistance()
{
	//Send trigger pulse
	gpioWrite(2/*GPIO2*/, 0/*Pi output low, sensor trigger input high*/);
	triggerElapsedTime = 0;
	triggerStartTime = cv::getTickCount();//Start timer for trigger output
	while(triggerElapsedTime < 0.000015)
	{
		triggerElapsedTime = (double)(cv::getTickCount() - triggerStartTime) / cv::getTickFrequency();
	}
	gpioWrite(2/*GPIO2*/, 1/*Pi output high, sensor trigger input low*/);
		
	//Read echo pulse
	pulseFinished = false;
	while(pulseFinished == false)
	{
		echoPulse = gpioRead(4);//GPIO4
		if(echoPulse == 1)
		{
			if(pulseOccurring == false)
			{
				pulseOccurring = true;
				echoPulseTime = cv::getTickCount();
			}
		}
		else
		{
			if(pulseOccurring == true)
			{
				pulseOccurring = false;
				distance = ((double)(cv::getTickCount() - echoPulseTime) / cv::getTickFrequency()) * 171500.0 - 0.5;
				pulseFinished = true;
			}
		}
	}
	return distance;
}

double Ultrasonic::GetFilteredDistance()
{
	filterElapsedTime = ((double)(cv::getTickCount() - filterStartTime) / cv::getTickFrequency());
	if(filterElapsedTime > 0.000100)
	{
		//If the sensor reading was lost, find it (do not proceed to normal operation until a certain number of similar values have been read)
		if(initCounter < calNumber)
		{	
			//If the sensor reading was just lost, get a calibration value to compare against
			if(initCounter == 0)
			{
				calDistance = GetDistance();
				//Accept the calibration value if it is reasonable
				if(calDistance < 1500.0)
				{
					initCounter++;
				}
			}
			
			//Otherwise, get a reading, and decide whether or not to accept it
			else
			{
				//Get a reading
				tempDistance = GetDistance();
				
				//Find the magnitude of the difference between the new value and the calibration value
				difference = tempDistance - calDistance;
				if(difference < 0.0)
					difference = -difference;
			
				//Accept it and set it as the new calibration value if it is reasonable
				if(difference < rejectionThreshold)
				{
					initCounter++;
					calDistance = tempDistance;
					
					//If sufficient similar values have been read, set filtered value to calibration value and clear rejection counter
					if(initCounter == calNumber)
					{
						filteredDistance = calDistance;
						rejectionCounter = 0;
					}
				}
				
				//Otherwise reject
				else
				{
					rejectionCounter++;
					//If too many readings in a row rejected, reinitialize
					if(rejectionCounter > rejectionNumber)
					{
						initCounter = 0; //signal to reinitialize
						rejectionCounter = 0;
						reliableReading = false;
					}
				}
			}
		}
		
		//Normal operation: read in a new value; accept if reasonable.
		//If too many values in a row are rejected, reinitialize.
		else
		{
			//Signal that the reading is reliable now
			reliableReading = true;

			//Get a reading
			tempDistance = GetDistance();
		
			//Find the magnitude of the difference between the new value and the calibration value
			difference = tempDistance - filteredDistance;
			if(difference < 0.0)
				difference = -difference;
			
			//Accept, set as new reference value, and set as filtered value if close to calibration value
			if(difference < rejectionThreshold)
			{
				rejectionCounter = 0;
				calDistance = tempDistance;
				filteredDistance = tempDistance;
			}
			
			//Otherwise reject
			else
			{
				rejectionCounter++;
				//If too many readings in a row rejected, reinitialize
				if(rejectionCounter > rejectionNumber)
				{
					initCounter = 0; //signal to reinitialize
					rejectionCounter = 0;
					reliableReading = false;
				}
			}
		}
		//Reset timer
		filterStartTime = cv::getTickCount();
	}
	return filteredDistance;
}
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
	void PulseForward();
	void PulseBackward();
	void GradualTurn(int speed, int direction, int tightness, int delayTime);
	void SmartPulseForward(int ultrasonicDistanceInt);
	void SetSpeed(int speed); //Used to SET the speed when both motors are to turn at the same speed.
private:
	int mode;
	int leftSpeed;
	int rightSpeed;
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
	if (direction == LEFT)
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

void Motor::PulseForward()
{	
	if (mode != FORWARD)
	{
		Stop();
		SetSpeed(100);
		gpioWrite(AIN2, 0);
		gpioWrite(AIN1, 1);
		gpioWrite(BIN2, 0);
		gpioWrite(BIN1, 1);
		
		//Delay
		double startTime = cv::getTickCount();
		double elapsedTime = 0.0;
		while(elapsedTime < ((double)PULSETIME / 1000.0))
		{
			elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
		}
		Stop();
	}
}

void Motor::SmartPulseForward(int ultrasonicDistanceInt)
{
	int delayTime; //Pulse time in ms
	
	if (mode != FORWARD)
	{
		Stop();
		
		//Calculate Delay
		if(ultrasonicDistanceInt == 0) //pass 0 to the function to just use PULSETIME
			delayTime = PULSETIME;
		else
		{
			int difference = ultrasonicDistanceInt - IDEALDISTANCE;
			if(difference < 0)
				delayTime = 0; //if too close don't move
			else
				delayTime = TIMEPERDISTANCE * difference; //time[us] ~= 1500 us/mm * difference[mm]
		}
		
		if(delayTime > 1000) //minimum pulse 1 ms
		{
		SetSpeed(100);
		gpioWrite(AIN2, 0);
		gpioWrite(AIN1, 1);
		gpioWrite(BIN2, 0);
		gpioWrite(BIN1, 1);
		//Delay
		double startTime = cv::getTickCount();
		double elapsedTime = 0.0;
		//while(elapsedTime < ((double)PULSETIME / 1000.0))
		while(elapsedTime < ((double)delayTime / 1000000.0))
		{
			elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
		}
		Stop();
		}
	}
}

void Motor::PulseBackward()
{
	if (mode != BACKWARD)
	{
		Stop();
		SetSpeed(50);
		gpioWrite(AIN1, 0);
		gpioWrite(AIN2, 1);
		gpioWrite(BIN1, 0);
		gpioWrite(BIN2, 1);
		
		//Delay
		double startTime = cv::getTickCount();
		double elapsedTime = 0.0;
		while(elapsedTime < ((double)PULSETIME / 1000.0))
		{
			elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
		}
		
		Stop();
	}
}

void Motor::GradualTurn(int speed, int direction, int tightness, int delayTime) //"speed" is the speed of the motor that is to turn fastest
{
	Stop();

	if (direction == RIGHT)
	{
		//left motor turns the fastest
		leftSpeed = (int)(((float)speed) / 100.0 * ((float)MAXSPEEDLEFT));
		
		//right motor turns slower depending on "tightness". 
		//Tightness = 0: right motor speed = same as left motor speed. Tightness = 100: right motor speed = 0.
		tightness = 100 - tightness;
		rightSpeed = (int)(((float)(speed * tightness)) / 10000.0 * ((float)MAXSPEEDRIGHT));
		
		gpioPWM(PWMA, leftSpeed);
		gpioPWM(PWMB, rightSpeed);
	
		gpioWrite(AIN2, 0);
		gpioWrite(AIN1, 1);
		gpioWrite(BIN2, 0);
		gpioWrite(BIN1, 1);
	}
	else
	{
		//right motor turns the fastest
		rightSpeed = (int)(((float)speed) / 100.0 * ((float)MAXSPEEDRIGHT));
		
		//left motor turns slower depending on "tightness". 
		//Tightness = 0: left motor speed = same as right motor speed. Tightness = 100: left motor speed = 0.
		tightness = 100 - tightness;
		leftSpeed = (int)(((float)(speed * tightness)) / 10000.0 * ((float)MAXSPEEDLEFT));

		gpioPWM(PWMA, leftSpeed);
		gpioPWM(PWMB, rightSpeed);

		gpioWrite(AIN2, 0);
		gpioWrite(AIN1, 1);
		gpioWrite(BIN2, 0);
		gpioWrite(BIN1, 1);
	}
	mode = TURN;
	//Delay
	double startTime = cv::getTickCount();
	double elapsedTime = 0.0;
	while(elapsedTime < ((double)delayTime / 1000.0))
	{
		elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
	}
	
	Stop();
}

void Motor::Steer()
{
	leftSpeed += turning;
	rightSpeed -= turning;
	if(leftSpeed > 100)
		leftSpeed = 100;
	else if(leftSpeed < 0)
		leftSpeed = 0;
	if(rightSpeed > 100)
		rightSpeed = 100;
	else if(rightSpeed < 0)
		rightSpeed = 0;
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
	Ultrasonic ultrasonic;
	double ultrasonicDistance;
	bool ultrasonicStatus;
	int ultrasonicDistanceInt;
	string ultrasonicDistanceStr;
	ostringstream convert;
	
	bool startauto;
	CColor red;

	CColor green;
};

void Server::Run()
{
	Camera.grab();
	Camera.retrieve(rawCameraImage);
	
	if(manual == true)
	{
		//cv::imshow("raw", rawCameraImage);
		//cv::waitKey(100);
		resize(rawCameraImage, resizedCameraImage, resizedCameraImage.size(), 0, 0, CV_INTER_LINEAR);
		//cv::imshow("resized", resizedCameraImage);
		//cv::waitKey(100);
		cropArea.x = LEFTEDGEMANUAL;
		cropArea.y = TOPEDGEMANUAL;
		cropArea.width = WIDTHMANUAL;
		cropArea.height = HEIGHTMANUAL;
		cameraImage = resizedCameraImage(cropArea);
	}
	else
	{
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
	}
    
    cv :: flip(cameraImage,cameraImage,-1);
    //cv::imshow("cameraImage", cameraImage);
    //cv::waitKey(100);

	//ultrasonicDistance = ultrasonic.GetFilteredDistance();
	ultrasonicDistance = ultrasonic.GetDistance();
	ultrasonicDistanceInt = (int)ultrasonicDistance;
	ultrasonicStatus = ultrasonic.GetStatus();
	
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
			//If the client sent the "/" command, pulse forward
			else if (command == "/")
			{
				motor.PulseForward();
				printf("Pulse forward\n");
			}
			//If the client sent the "*" command, smart pulse forward
			else if (command == "*")
			{
				motor.SmartPulseForward(ultrasonicDistanceInt);
				printf("Smart pulse forward\n");
			}
			//If the client sent the "2" command, pulse backward
			else if (command == "2")
			{
				motor.PulseBackward();
				printf("Pulse backward\n");
			}
			//If the client sent the "9" command, turn right gradually
			else if (command == "9")
			{
				motor.GradualTurn(MANUALGRADUALTURNSPEED, RIGHT, TIGHTNESS, MANUALGRADUALTURNTIMERIGHT);
				printf("Turning right gradually\n");
			}
			//If the client sent the "7" command, turn left gradually
			else if (command == "7")
			{
				motor.GradualTurn(MANUALGRADUALTURNSPEED, LEFT, TIGHTNESS, MANUALGRADUALTURNTIMELEFT);
				printf("Turning left gradually\n");
			}
			//If the client sent the "a" command, switch into automatic mode
			else if (command == "a")
			{
				manual = false;
				startauto=true;
				printf("Automatic mode\n");
			}
			//If the client sent the "R" command, switch into calibration mode for red
					else if (command == "R")
			{
				printf("Calibrate red\n");
				red.vision_cal(rawCameraImage);
				
				int pos = red.vision_go(cameraImage);
				cv::imshow("red calibrate HSV", rawCameraImage);
				cv::imshow("red calibrate tresh", cameraImage);
				std :: cout <<pos;
				cv::waitKey (10);
			}
			//If the client sent the "G" command, switch into calibration mode for green
			else if (command == "G")
			{
				printf("Calibrate green\n");
				green.vision_cal(rawCameraImage);
				
				int pos = green.vision_go(cameraImage);
				cv::imshow("green calibrate HSV", rawCameraImage);
				cv::imshow("green calibrate tresh",cameraImage);
				std :: cout <<pos;
				
				cv::waitKey (10);
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
					compressionSettings.push_back(5);						 //...to 30 (out of 100)
					cv::imencode("image.jpg", cameraImage, compressedImage, compressionSettings);
				}
	
				//Send ultrasonic reading
				send(clientSocket, (char *)&ultrasonicDistanceInt, sizeof(int), 0);
				if (returnValue == SOCKET_ERROR)
				{
					serverError = true;
					cout << "Server error: failed to send ultrasonic reading";
				}
				
				//Send ultrasonic status
				send(clientSocket, (char *)&ultrasonicStatus, sizeof(bool), 0);
				if (returnValue == SOCKET_ERROR)
				{
					serverError = true;
					cout << "Server error: failed to send ultrasonic status";
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
	
	double delta_T;
	bool letsturn = false;
	while(manual == false) //Automatic mode//Automatic mode//Automatic mode//Automatic mode//Automatic mode//Automatic mode
	{
		double start_tic, freq;
		freq = cv::getTickFrequency(); // Get tick frequency
		start_tic = cv::getTickCount(); // Get number of ticks since event (such as computer on)

		float visionDistance;

		int previousPosition = centrePosition;//starts as 0 
		
		int speed;
		bool nextisleft;
		bool startturn;
		int turntimevar;
		bool longway=1;
		
		if(startauto==true)
		{
			speed = startspeed;
			motor.Forward(startspeed);
			delta_T=0.1;
			centrePosition=0;
			startauto=false;
			
		}
	
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
    
		cv :: flip(cameraImage,cameraImage,-1);
		//cv::imshow("cameraImage", cameraImage);
		//int greenpos;
		//int redpos;
		//greenpos = green.vision_go(cameraImage);

		
		if(longway == 1)
		{
			
			if(stage >= 1 && stage <= 11)
			{
				
				//---Get centre position of object---
				//green object
				if(stage==1 || stage==2 || stage==5 || stage==6 || stage==7 || stage==10 || stage==11)
				{
					centrePosition = green.vision_go(cameraImage);
					visionDistance = scaledist / green.objrad;
					nextisleft=false;
						
					std::cout<<"\nhere in green centrePosition:  "<<centrePosition;
					std::cout<<"\nhere in green visiondistance: "<<visionDistance;
					std::cout<<"\nhere in green objrad: "<<green.objrad;
				}
				else//red object
				{
					centrePosition = red.vision_go(cameraImage);
					visionDistance = scaledist / green.objrad;
					nextisleft=true;
						
					std::cout<<"\nhere in red centrePosition:  "<<centrePosition;
					std::cout<<"\nhere in red visiondistance: "<<visionDistance;
					std::cout<<"\nhere in red objrad: "<<red.objrad;
				}
					
				if(centrePosition == -1000)//no object found
				{
					//Delay
					std::cout<<"\nhere in no object found";
					startTime = cv::getTickCount();
					elapsedTime = 0;
					received = false;
					while (returnValue == SOCKET_ERROR && elapsedTime < 10.0)
					{
						returnValue = recv(clientSocket, receiveBuffer, SERVERBUFFER, 0);
						elapsedTime = (double)(cv::getTickCount() - startTime) / (double)cv::getTickFrequency();
					}
					//motor.turning = 0;
					//motor.Steer();
					if(speed > OBJECTLOSTMINSPEED)
					{
						speed = speed - decel;
					}
					else if(speed < OBJECTLOSTMINSPEED)
					{
						speed = speed +accel;
					}
					motor.Forward(speed);
						//Slow down but don't stop
						//if(motor.GetAverageSpeed() > OBJECTLOSTMINSPEED)
						//{
						//	motor.SlowDown(1);
							
						//}
						//Incrementally turn the other way
						//if(motor.turning != 0)
							//motor.turning += -motor.turning/motor.turning*OBJECTLOSTTURNADJUST;
					}
					else if(centrePosition == 1000)//greater than 50 contours found (too much noise in the signal)
					{
						std::cout<<"\nhere in to many objects found";
						
						if(speed > OBJECTLOSTMINSPEED)
						{
							speed = speed - decel;
						}
						else if(speed < OBJECTLOSTMINSPEED)
						{
							speed = speed +accel;
						}
						motor.Forward(speed);
						//Slow down but don't stop
						//if(motor.GetAverageSpeed() > OBJECTLOSTMINSPEED)
						//{
						//	motor.SlowDown(1);
						//	motor.turning = 0;
						//	motor.Steer();
						//}
						//Incrementally turn the other way
						//if(motor.turning != 0)
							//motor.turning += -motor.turning/motor.turning*OBJECTLOSTTURNADJUST;
					}
					else//NORMAL OPERATION ------------------------------------------------------
					{
					
						//Calculate proportional response
						float prop= ( (float)centrePosition   *  ((float)Kp/1000.0)  ); 
						//Calculate deriative response
						float deriv = (centrePosition-previousPosition)*((float)Kp/1000.0) * (((float)Td/1000.0)/delta_T);
					
						if(letsturn==false)
						{	
							//Calculate total response
							motor.turning= ( prop + deriv );
							std :: cout <<"\nturning input:  "<<motor.turning;
							//Limit total response
							if(motor.turning>maxturn)
							{
								motor.turning=maxturn;
							}
						
							if(motor.turning < minturn)
							{
								motor.turning=minturn;
							
							}
						
							//Set the amount of turning
							motor.SetSpeed(motor.GetAverageSpeed());
							motor.Steer();
						
							std :: cout <<"\nturning:  "<<motor.turning;
					
							if(visionDistance < turndist && visionDistance>0)
							{
								letsturn=true;	
								startturn=true;
								turntimevar=0;				
							
							}
							else if( visionDistance< slowdist && visionDistance>0)
							{
								if(motor.GetAverageSpeed()<speedinturn)
								{
									motor.SpeedUp(accel);
									speed = motor.GetAverageSpeed();
								}
								else
								{
									motor.SlowDown(decel);
									speed = motor.GetAverageSpeed();
								}
							}
							else if(motor.GetAverageSpeed()<MaxSpeedAuto)
							{
						
								motor.SpeedUp(accel);
								speed = motor.GetAverageSpeed();
								std::cout<<"\nhere in speedup motorspeed: "<<motor.GetAverageSpeed();
							}
							else if(motor.GetAverageSpeed()>MaxSpeedAuto)
							{
							
								motor.SlowDown(decel);
								speed = motor.GetAverageSpeed();
								std::cout<<"\nhere in slow motorspeed: "<<motor.GetAverageSpeed();
							}
					
						}
					}
					
					if(letsturn==true)
					{
						if(startturn == true)
						{
							//speed=speedinturn;
							//motor.Forward(speedinturn);
							if(nextisleft==true)
							{
								motor.GradualTurn(speedinturn, LEFT, TIGHTNESS, 7 * speedinturn);

								//motor.turning=-turnrad;
								//motor.Steer();
								//turnleft
								std::cout<<"turningleft\n";
							}
							else
							{
								motor.GradualTurn(speedinturn, RIGHT, TIGHTNESS, 7 * speedinturn);

								//motor.turning=turnrad;
								//motor.Steer();
								//turnright
								std::cout <<"turning right\n";
							}
							
							stage ++;
							startturn=false;
						}
						/*
						if(turntimevar<turntime)
						{
							turntimevar=turntimevar+(int)delta_T;
							
						}
						else
						{
							speed=speedinturn;
							motor.Forward(speedinturn);
							letsturn=false;
						}
						*/
						letsturn=false; //added this here - WM

					
					}
					
			
			}
		}
		else
		{
		}
		
		std :: cout <<"\nstage number: " << stage;
		cv::imshow("automatic", cameraImage);
		cv::waitKey (10);
		delta_T = (cv::getTickCount() - start_tic) / freq;
		std::cout << "\nelapsed time: " << delta_T; 
		
		if (getch())
		{
			manual = true;
		}
	}//end Automatic mode//end Automatic mode//end Automatic mode//end Automatic mode//end Automatic mode//end Automatic mode


}

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
	//Initialize ncurses
	//initscr();
    //cbreak();
    //noecho();
    //scrollok(stdscr, TRUE);
    //nodelay(stdscr, TRUE);
    
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

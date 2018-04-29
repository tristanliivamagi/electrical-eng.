CC=g++
CFLAGS=-Wall `pkg-config --cflags opencv` -std=c++11 -I/usr/local/include/ -c
LDFLAGS=-Wall `pkg-config --libs opencv` -std=c++11 -lpigpio -lrt -lpthread -lraspicam -lraspicam_cv -lopencv_core -lopencv_highgui

 

all: main

main: main.o 
	$(CC) $(LDFLAGS) main.o -o main

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm *o main

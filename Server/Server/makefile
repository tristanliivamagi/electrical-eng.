CC=g++
CFLAGS=-Wall `pkg-config --cflags opencv` -std=c++11 -I/usr/local/include/ -c
LDFLAGS=-Wall `pkg-config --libs opencv` -std=c++11 -lpigpio -lrt -lpthread -lraspicam -lraspicam_cv -lopencv_core -lopencv_highgui

 

all: hello

hello: Server.o Color.o
	$(CC) $(LDFLAGS) Server.o Color.o -o hello


Server.o: Server.cpp
	$(CC) $(CFLAGS) Server.cpp


Color.o: Color.cpp
	$(CC) $(CFLAGS) Color.cpp

clean:
	rm *o hello

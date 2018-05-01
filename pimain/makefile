CC=g++
CFLAGS=-Wall `pkg-config --cflags opencv` -std=c++11 -I/usr/local/include/ -c
LDFLAGS=-Wall `pkg-config --libs opencv` -std=c++11 -lpigpio -lrt -lpthread -lraspicam -lraspicam_cv -lopencv_core -lopencv_highgui

 

all: hello

hello: pimain.o Pi.o 
	$(CC) $(LDFLAGS) pimain.o Pi.o -o hello

pimain.o: pimain.cpp
	$(CC) $(CFLAGS) pimain.cpp


Pi.o: Pi.cpp
	$(CC) $(CFLAGS) Pi.cpp

clean:
	rm *o hello

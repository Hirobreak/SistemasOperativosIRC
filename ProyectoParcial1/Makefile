CC=g++
CFLAGS=-c

all: server

server: server.o
	$(CC) server.o -o server

server.o: server.cpp
	$(CC) $(CFLAGS) server.cpp

clean:
	rm *o server
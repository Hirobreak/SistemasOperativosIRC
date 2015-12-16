CC=g++
CFLAGS=-c

all: espolirc

espolirc: server.o
	$(CC) server.o -lpthread -o espolirc

server.o: server.cpp
	$(CC) $(CFLAGS) server.cpp

clean:
	rm *o server
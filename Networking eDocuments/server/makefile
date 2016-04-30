CC=gcc -std=c99

CFLAGS=-g -lpthread

all: server

reader: server.c
	$(CC) server.c $(CFLAGS) -o server
clean:
	rm -f server *.o *~

CC=gcc

CFLAGS=-W -g

all: server

server: server.c message_handler.c server_core_data.c s2s_handler.c
	$(CC) server.c message_handler.c server_core_data.c s2s_handler.c $(CFLAGS) -o server
clean:
	rm -f server *.o *~

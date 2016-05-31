CC=gcc

CFLAGS=-W -g

all: client

client: client.c channel.c command_parse.c command_handler.c common.c message_handler.c raw.c
	$(CC) client.c channel.c command_parse.c command_handler.c common.c message_handler.c raw.c $(CFLAGS) -o client
clean:
	rm -f client *.o *~

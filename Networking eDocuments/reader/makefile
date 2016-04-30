CC=gcc -std=c99

CFLAGS=-g -lpthread

all: reader

reader: reader.c
	$(CC) reader.c $(CFLAGS) -o reader
clean:
	rm -f reader *.o *~

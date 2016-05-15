CC=gcc -std=gnu99

CFLAGS=-g -lpthread -lcrypto




tserver: tserver.c
	$(CC) tserver.c $(CFLAGS) -o tserver

tclient: tclient.c
	$(CC) tclient.c $(CFLAGS) -o tclient
	
clean:
	rm -f server *.o *~

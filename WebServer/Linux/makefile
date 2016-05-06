web:webServer.o handle.o
	gcc -o web webServer.o handle.o
webServer.o:webServer.c handle.h
	gcc -c  webServer.c
handle.o:handle.c handle.h
	gcc -c handle.c

clean:
	rm handle.o webServer.o


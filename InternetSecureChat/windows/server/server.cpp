/*
 * server.cpp
 * author: Hancheng Wang
 *   date: 2016.5.7
 */

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <winsock2.h>
#include <fstream>
using namespace std;

#define PORT 65530
#define IP_ADDRESS "127.0.0.1"
#define MAXLEN 1024

#pragma comment(lib, "ws2_32.lib")

char stdio_str[1024] = {0};
char socket_str[1024] = {0};
int stdio_flag = 0;
int socket_flag = 0;
int bye_flag = 0;

void encrypt_message(char *message){
	int len = strlen(message);
	for(int i=0; i<len; i++){
		message[i] = message[i] + 3;
	}
}

void decrypt_message(char *message){
	int len = strlen(message);
	for(int i=0; i<len; i++){
		message[i] = message[i] - 3;
	}
}

DWORD WINAPI read_from_stdio(LPVOID lpParameter)
{
	while (1){
		while(stdio_flag == 1)
			;
		memset(stdio_str,0,1024);
		cin.getline(stdio_str,1024,'\n');
		if(strcmp(stdio_str,"Bye") == 0)
				bye_flag = 1;
		encrypt_message(stdio_str);
		stdio_flag = 1;
	}
}

DWORD WINAPI read_from_socket(LPVOID lpParameter)
{
    SOCKET acceptfd = (SOCKET)lpParameter;
	while(1){
		while(socket_flag == 1)
			;
		memset(socket_str,0,1024);
		recv(acceptfd, socket_str, 1024, 0);
		socket_flag = 1;
	}
}

int main(){
    WSADATA  Ws;
    SOCKET ServerSocket, ClientSocket;
    struct sockaddr_in LocalAddr, ClientAddr;
    int Ret = 0;
    int AddrLen = 0;
    HANDLE hThread1 = NULL,hThread2 = NULL;

    //Init Windows Socket
    if ( WSAStartup(MAKEWORD(2,2), &Ws) != 0 )
    {
        cout << "Init Windows Socket Failed::" << GetLastError() << endl;
        return -1;
    }

    //Create Socket
    ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( ServerSocket == INVALID_SOCKET )
    {
        cout << "Create Socket Failed::" << GetLastError() << endl;
        return -1;
    }

    LocalAddr.sin_family = AF_INET;
    LocalAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    LocalAddr.sin_port = htons(PORT);
    memset(LocalAddr.sin_zero, 0x00, 8);

    //Bind Socket
    Ret = bind(ServerSocket, (struct sockaddr*)&LocalAddr, sizeof(LocalAddr));
    if ( Ret != 0 )
    {
        cout << "Bind Socket Failed::" << GetLastError() << endl;
        return -1;
    }
    //listen
    Ret = listen(ServerSocket, 10);
    if ( Ret != 0 )
    {
        cout << "listen Socket Failed::" << GetLastError() << endl;
        return -1;
    }

    cout << "Server Start:" << endl;


    AddrLen = sizeof(ClientAddr);
    ClientSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddr, &AddrLen);
    if ( ClientSocket == INVALID_SOCKET )
    {
        cout << "Accept Failed::" << GetLastError() << endl;
        return -1;
    }
	char username[1024] = {0};
	recv(ClientSocket, username, 1024, 0);
	cout << "Received User's Name:" << username <<", let's start to exchange the encrypted message."<< endl;
	send(ClientSocket,"ACK",4,0);
	
	ofstream out("log.txt");

	hThread1 = CreateThread(NULL, 0, read_from_stdio, (LPVOID)ClientSocket, 0, NULL);
    if ( hThread1 == NULL )
    {
        cout << "Create Thread1 Failed!" << endl;
        return -1;
    }
	
	hThread2 = CreateThread(NULL, 0, read_from_socket, (LPVOID)ClientSocket, 0, NULL);
    if ( hThread2 == NULL )
    {
        cout << "Create Thread2 Failed!" << endl;
        return -1;
    }
	WaitForSingleObject(hThread1, 0);  
	WaitForSingleObject(hThread2, 0);  
	
    while(1){
		if(stdio_flag){
			out << stdio_str << endl;
			send(ClientSocket,stdio_str,1024,0);
			stdio_flag = 0;
		}
		if(socket_flag){
			out << socket_str << endl;
			decrypt_message(socket_str);
			if(strcmp(socket_str,"Bye") == 0)
				bye_flag = 1;
			cout << username <<": " << socket_str << endl;
			socket_flag = 0;
		}	
		if(bye_flag && (!stdio_flag) && (!socket_flag)){
			out.close();
			Sleep(2);
			system("pause");
			exit(0);
		}
	}

    closesocket(ServerSocket);
    WSACleanup();

    return 0;

}

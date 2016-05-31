/*
 * client.cpp
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
    SOCKET ClientSocket;
    int Ret = 0;
    HANDLE hThread1 = NULL,hThread2 = NULL;

    //Init Windows Socket
    if ( WSAStartup(MAKEWORD(2,2), &Ws) != 0 )
    {
        cout << "Init Windows Socket Failed::" << GetLastError() << endl;
        return -1;
    }

    //Create Socket
    ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( ClientSocket == INVALID_SOCKET )
    {
        cout << "Create Socket Failed::" << GetLastError() << endl;
        return -1;
    }

    SOCKADDR_IN addrSrv;  
    addrSrv.sin_addr.S_un.S_addr = inet_addr(IP_ADDRESS);      // 本地回路地址是127.0.0.1;   
    addrSrv.sin_family = AF_INET;  
    addrSrv.sin_port = htons(PORT);  
    connect(ClientSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));  

    cout << "Client Start:" << endl;
	char username[1024] = {0};
	cout << "Enter your username: ";
	cin.getline(username,1024,'\n');
	send(ClientSocket, username, 1024, 0);
	char buff[1024] = {0};
	recv(ClientSocket, buff, 1024, 0);

	cout << "Received "<<buff<<", let's start to exchange the encrypted message."<< endl;
	
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
			cout << "server: " << socket_str << endl;
			socket_flag = 0;
		}	
		if(bye_flag && (!stdio_flag) && (!socket_flag)){
			out.close();
			Sleep(2);
			system("pause");
			exit(0);
		}
	}

    closesocket(ClientSocket);
    WSACleanup();

    return 0;

}

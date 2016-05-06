#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <winsock2.h>
using namespace std;

#define PORT 8080
#define IP_ADDRESS "127.0.0.1"
#define MAXLEN 1024

#pragma comment(lib, "ws2_32.lib")

void get_filename(char *uri,char *filename)
{
	if(strlen(uri)<=1){
		strcpy(filename,"index.html");
		return ;
	}
	int i = 0, j = 0;
	for(i=1; i<strlen(uri); i++,j++){
		if(uri[i] == '/'){
			filename[j] = '\\';
		}else
			filename[j] = uri[i];
	}
}

void headMsg(SOCKET acceptfd,char *code,char *msg,int lenOfBody)
{
    char head[MAXLEN] = {0};
    sprintf(head,"HTTP/1.0 %s %s\r\n",code,msg);
    send(acceptfd,head,strlen(head),0);
    sprintf(head,"Content-type:text/html\r\n");
    send(acceptfd,head,strlen(head),0);
    sprintf(head,"Content-length: %d\r\n\r\n",lenOfBody);
    send(acceptfd,head,strlen(head),0);
}

void errorMsg(SOCKET acceptfd,char *cause,char *errcode,char *msg)
{
    char body[MAXLEN] = {0};
    sprintf(body,"<html><title>Error</title>");
    sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
    sprintf(body,"%s%s: %s\r\n",body,errcode,msg);
    sprintf(body,"%s<p>%s: %s\r\n",body,msg,cause);
    sprintf(body,"%s<hr><em>The web server by whc</em>\r\n",body);
    headMsg(acceptfd,errcode,msg,strlen(body));
    send(acceptfd,body,strlen(body),0);
}

void handle(SOCKET acceptfd,char *buf,int len)
{
    char method[MAXLEN] = {0};
    char uri[MAXLEN] = {0};
    char version[MAXLEN] = {0};
    sscanf(buf,"%s %s %s",method,uri,version);
    /* if the request is not the GET, return 501.*/
    if(NULL == strstr(method,"GET")){
        errorMsg(acceptfd,method,"501","Not Implemented");
        return ;
    }

	char filename[50] = {0};
	get_filename(uri,filename);
    FILE *pFile = fopen(filename,"r+");

	/* if the file is not exist, return 404.*/
    if(NULL == pFile){
        errorMsg(acceptfd,method,"404","Not Found");
		return ;
    }

	/* if the file existed, return 200.*/
    char file[1024] = {0};
    while(NULL !=fgets(file,1024,pFile)){
		headMsg(acceptfd,"200","OK",strlen(file));
        send(acceptfd,file,strlen(file),0);
        memset(file,0,1024);
    }
    
    return ;
}



DWORD WINAPI ClientThread(LPVOID lpParameter)
{
    SOCKET acceptfd = (SOCKET)lpParameter;
    int Ret = 0;
	char recvBuffer[1024] = {0};

    recv(acceptfd, recvBuffer, 1024, 0);
    cout << "Reavived From Web Browser:" << recvBuffer << endl;

    handle(acceptfd,recvBuffer,strlen(recvBuffer));
    closesocket(acceptfd);

	return 0;
}


int main(){
    WSADATA  Ws;
    SOCKET ServerSocket, ClientSocket;
    struct sockaddr_in LocalAddr, ClientAddr;
    int Ret = 0;
    int AddrLen = 0;
    HANDLE hThread = NULL;

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

    cout << "Web Server Start" << endl;
	cout << "Server port: " << PORT << endl;

    while ( true )
    {
        AddrLen = sizeof(ClientAddr);
        ClientSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddr, &AddrLen);
        if ( ClientSocket == INVALID_SOCKET )
        {
            cout << "Accept Failed::" << GetLastError() << endl;
            break;
        }


        hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)ClientSocket, 0, NULL);
        if ( hThread == NULL )
        {
            cout << "Create Thread Failed!" << endl;
            break;
        }
		CloseHandle(hThread);
    }

    closesocket(ServerSocket);
    WSACleanup();

    return 0;

}

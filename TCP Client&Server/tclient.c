#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include <openssl/md5.h>
#include <string.h>
#include <stdlib.h>

#define SERVER_PORT 1500
/* change above port number if required */

#define MAX_MSG 100

typedef struct Argument{
	char hostname[50];
	int port;
	char request[10];
	int offset;
	int length;
	char filename[100];
	char saveasfilename[100];
}Argument;
Argument argut = {"\0",25000,"\0",0,-1,"\0","\0"};

int min(int a,int b){
	return a<b?a:b;
}


void filetypeRspHandle(int sockfd){
	unsigned char buff[1024] = {0};
	buff[0] = 0xea;
	int temp = strlen(argut.filename);
	int length = htonl(temp);
	memcpy(&buff[1],&length,4);
	strcpy(&buff[5],argut.filename);
	send(sockfd, buff, 6+temp, 0);
	
	memset(buff,0,1024);
	int len = recv(sockfd,buff,1024,0);
	if(buff[0] == 0xe9){
		int i;
		for(i=5; i<len; i++){
			if(buff[i] > 0x7F)
				break;
		}
		if(i == len)
			printf("%s\n",&buff[5]);
		else
			printf("Invalid characters detected in a FILETYPE_RSP message.\n");
	}else if(buff[0] == 0xe8){
		printf("FILETYPE_ERR received from the server\n");
	}	
}

void checksumRspHandle(int sockfd){
	unsigned char buff[1024] = {0};
	buff[0] = 0xca;
	int temp = strlen(argut.filename)+8;
	int dataLength = htonl(temp);
	int offset = htonl(argut.offset);
	int length = htonl(argut.length);
	memcpy(&buff[1],&dataLength,4);
	memcpy(&buff[5],&offset,4);
	memcpy(&buff[9],&length,4);
	strcpy(&buff[13],argut.filename);
	send(sockfd, buff, 6+temp, 0);
	
	memset(buff,0,1024);
	int len = recv(sockfd,buff,1024,0);
	
	if(buff[0] == 0xc9){
		int temp;
		memcpy(&temp,&buff[1],4);
		int dataLength = ntohl(temp);
		if(dataLength != 16)
			printf("Invalid DataLength detected in a CHECKSUM_RSP message.\n");
		else{
			int i;
			for(i=5; i<21; i++)
				printf("%02x",buff[i]);
			printf("\n");
		}
	}else if(buff[0] == 0xc8){
		printf("CHECKSUM_ERR received from the server\n");
	}	
}

void downloadRspHandle(int sockfd){
	unsigned char buff[4096] = {0};
	buff[0] = 0xaa;
	int temp = strlen(argut.filename)+8;
	int dataLength = htonl(temp);
	int offset = htonl(argut.offset);
	int length = htonl(argut.length);
	memcpy(&buff[1],&dataLength,4);
	memcpy(&buff[5],&offset,4);
	memcpy(&buff[9],&length,4);
	strcpy(&buff[13],argut.filename);
	send(sockfd, buff, 6+temp, 0);
	
	memset(buff,0,1024);
	int len = recv(sockfd,buff,5,0);
	
	if(buff[0] == 0xa9){
		int temp;
		memcpy(&temp,&buff[1],4);
		int dataLength = ntohl(temp);
		FILE *fp = NULL;
		char stdFilename[20] = {0};
		if(dataLength > 0){
			if(strlen(argut.saveasfilename) > 0){
				if(access(argut.saveasfilename,F_OK) == 0){
					printf("File %s already exists, would you like to overwrite it? [yes/no](n) ",argut.saveasfilename);
					fflush(stdin);
					char ans;
					scanf("%c",&ans);
					scanf("%*[^\n]");
					if(ans == 'y'){
						fp = fopen(argut.saveasfilename,"w+");
					}else{
						printf("Download canceled per user's request.\n");
						return ;
					}
				}else{
					fp = fopen(argut.saveasfilename,"w+");
				}
				strcpy(stdFilename,argut.saveasfilename);
			}else{
				argut.filename;
				
				int i = strlen(argut.filename);
				for(; i>=0; i--){
					if(argut.filename[i] == '/'){
						break;
					}
				}
				strcpy(stdFilename,&argut.filename[i+1]);
				fp = fopen(&argut.filename[i+1],"w+");
			}
		}
		
		MD5_CTX c;
		char buf[4096] = {0};
		unsigned char md5_sum[MD5_DIGEST_LENGTH];
		MD5_Init(&c);
		
		int remaining = dataLength;
		while(remaining > 0){
			memset(buf,0,4096);
			int bytes_to_read = min(remaining,4096);
			int bytes_read = read(sockfd,buf,bytes_to_read);
			
			fwrite(buf,bytes_read,1,fp);
			printf(".");
			fflush(stdout);
			remaining -= bytes_read;
			MD5_Update(&c,buf,bytes_read);
		}
		MD5_Final(md5_sum,&c);
		printf("Downloaded data have been successfully written into '%s' (MD5=",stdFilename);
		int j;
		for(j=0; j<MD5_DIGEST_LENGTH; j++){
			printf("%02x",md5_sum[j]);
		}
		printf("\n");
	}else if(buff[0] == 0xa8){
		printf("DOWNLOAD_ERR received from the server\n");
	}	
}

int search_substring(int argc, char *argv[],char *subString,int *num){
	int i=0;
	for(i=1; i<argc; i++){
		if(strcmp(argv[i],subString) == 0){
			*num = atoi(argv[i+1]);
			return 1;
		}
	}
	return 0;
}



int main (int argc, char *argv[]) {
	gethostname(argut.hostname, 50);
	char temp[50] = {0};
	strcpy(temp,argv[1]);
	if(strstr(temp,":")){
		char *para = strtok(temp,":");
		strcpy(argut.hostname,para);
		para = strtok(NULL,":");
		argut.port = atoi(para);
	}else
		argut.port = atoi(temp);
	strcpy(argut.request,argv[2]);

	int index = 3;
	if(search_substring(argc,argv,"-o",&argut.offset))
		index += 2;
	if(search_substring(argc,argv,"-l",&argut.length))
		index += 2;
	strcpy(argut.filename,argv[index++]);
	if(argc > index){
		strcpy(argut.saveasfilename,argv[index]);
	}
	//printf("%s:%d %s %d %d %s %s\n",argut.hostname,argut.port,argut.request,argut.offset,argut.length,argut.filename,argut.saveasfilename);
	
	int sd, rc, i;
	struct sockaddr_in localAddr, servAddr;
	struct hostent *h;
 
	h = gethostbyname(argut.hostname);
	if(h==NULL) {
		printf("%s: unknown host '%s'\n",argv[0],argv[1]);
		exit(1);
	}

	servAddr.sin_family = h->h_addrtype;
	memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	servAddr.sin_port = htons(argut.port);

	/* create socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd<0) {
		perror("cannot open socket ");
		exit(1);
	}
			
	/* connect to server */
	rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
	if(rc<0) {
		perror("cannot connect ");
		exit(1);
	}
	
	char buff[1024] = {0};
	if(strstr(argut.request,"filetype"))
		filetypeRspHandle(sd);
	else if(strstr(argut.request,"checksum"))
		checksumRspHandle(sd);
	else if(strstr(argut.request,"download"))
		downloadRspHandle(sd);
	

return 0;
  
}


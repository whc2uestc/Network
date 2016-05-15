#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> 
#include <pthread.h>
#include <openssl/md5.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define SUCCESS 0
#define ERROR   1

#define END_LINE 0x0
#define SERVER_PORT 1500
/* change above port number if required */

#define MAX_MSG 1024

/* function readline */
int read_line();
typedef struct Argument{
	int debug;
	int shutdown;
	int port;
}Argument;
Argument argut = {0,300,25000};
int sd;

int min(int a,int b){
	return a<b?a:b;
}

int check_symbol(char c){
	if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='/'||c==','||c=='+'||c=='.')
		return 1;
	return 0;
}

int check_filename(char *filename){
	int i=0;
	for(i=0; i<strlen(filename); i++){
		if(check_symbol(filename[i]) == 0)
			return 0;
	}
	return 1;
}

void filetypeReqHandle(int acceptfd,char *buff){
	if(argut.debug){
		int temp;
		memcpy(&temp,&buff[1],4);
		int req_dataLength = ntohl(temp);
		printf("FILETYPE_REQ received with DataLength = %d, Data = '%s'\n",req_dataLength,&buff[5]);	
	}
	char cmd[80] = {0};
	char tmp[1024] = {0};
	unsigned char req_message[1024] = {0};
	
	if(check_filename(&buff[5]) == 0){
		req_message[0] = 0xe8;
		if(argut.debug){
			printf("FILETYPE_ERR sent with DataLength = 0\n");	
		}
		send(acceptfd, req_message, 5, 0);
		return ;
	}
	FILE *pfp = NULL;
	sprintf(cmd,"file %s",&buff[5]);
	if((pfp=(FILE*)popen(cmd,"r")) == NULL){
		fprintf(stderr,"Cannot execute '%s'.\n",cmd);
		req_message[0] = 0xe8;
		if(argut.debug){
			printf("FILETYPE_ERR sent with DataLength = 0\n");	
		}
		send(acceptfd, req_message, 5, 0);
	}else{
		while(fgets(tmp,sizeof(tmp),pfp) != NULL){
			//fprintf(stdout,"%s",tmp);
			strcat(req_message,tmp);
		}
		pclose(pfp);
		if(req_message[strlen(req_message)-1] == '\n')
			req_message[strlen(req_message)-1] = '\0';
		
		req_message[0] = 0xe9;
		int length = htonl(strlen(&req_message[5]));
		memcpy(&req_message[1],&length,4);
		int dataLength = ntohl(length);
		if(argut.debug){
			printf("FILETYPE_RSP sent with DataLength = %d, Data = '%s'\n",dataLength,&req_message[5]);	
		}
		send(acceptfd, req_message, strlen(&req_message[5])+6, 0);
	}
	
}

void send_checksum_err(int acceptfd){
	char req_message[5] = {0xc8,0,0,0,0};
	send(acceptfd, req_message, 5, 0);
	if(argut.debug){
		printf("CHECKSUM_ERR sent with DataLength = 0\n");	
	}
}
void send_checksum_rsp(int acceptfd,char *filename,int offset,int length){
	unsigned char req_message[1024] = {0};
	FILE *fp = fopen(filename,"r");
	int rsp_length;
	if(fp == NULL){
		send_checksum_err(acceptfd);
	}else{
		MD5_CTX c;
		char buf[4096] = {0};
		unsigned char md5_sum[MD5_DIGEST_LENGTH];
		MD5_Init(&c);

		fseek(fp,offset,SEEK_SET);
		int read_bytes;
		while((read_bytes = min(4096,length)) > 0){
			memset(buf,0,4096);
			fread(buf,read_bytes,1,fp);
			MD5_Update(&c,buf,read_bytes);
			length -= 4096;
		}
		MD5_Final(md5_sum,&c);
		fclose(fp);
		
		req_message[0] = 0xc9;
		rsp_length= htonl(MD5_DIGEST_LENGTH);
		memcpy(&req_message[1],&rsp_length,4);
		memcpy(&req_message[5],md5_sum,MD5_DIGEST_LENGTH);
		if(argut.debug){
			printf("CHECKSUM_RSP sent with DataLength = %d, checksum = ",ntohl(rsp_length));	
			int i = 0;
			for(i=0; i<MD5_DIGEST_LENGTH; i++)
				printf("%02x",md5_sum[i]);
			printf("\n");
		}
		send(acceptfd, req_message, MD5_DIGEST_LENGTH+5, 0);
	}
}
int get_filesize(char* filename)
{
    FILE *fp = fopen(filename,"r");
    if(!fp) return -1;
    fseek(fp,0L,SEEK_END);
    int size = ftell(fp);
    fclose(fp);
    
    return size;
}

int check_argv_valid(char *filename,int offset,int length){
	int filesize = get_filesize(filename);
	if(filesize < 0)
		return 0;
	if(check_filename(filename)==0 || offset<0 || offset+length>filesize || length>=filesize)
		return 0;
	return 1;
}


void checksumReqHandle(int acceptfd,char *buff){
	int offset,length,temp;
	memcpy(&temp,&buff[1],4);
	int datalength = ntohl(temp);
	if(datalength <= 8){
		send_checksum_err(acceptfd);
		return ;
	}
	memcpy(&temp,&buff[5],4);
	offset = ntohl(temp);
	memcpy(&temp,&buff[9],4);
	length = ntohl(temp);
	char filename[100] = {0};
	strcpy(filename,&buff[13]);
	if(argut.debug){ 
		printf("CHECKSUM_REQ received with DataLength = %d, offset = %d, length = %d, filename = '%s'\n",datalength,offset,length,filename);	
	}
	if(check_argv_valid(filename,offset,length) == 0){
		send_checksum_err(acceptfd);
		return ;
	}
	if(length < 0)
		length = get_filesize(filename) - offset;
	send_checksum_rsp(acceptfd,filename,offset,length);
	
}

void send_download_err(int acceptfd){
	char req_message[5] = {0xa8,0,0,0,0};
	send(acceptfd, req_message, 5, 0);
	if(argut.debug){
		printf("DOWNLOAD_ERR sent with DataLength = 0\n");	
	}
}

void send_download_rsp(int acceptfd,char *filename,int offset,int length){
	char *req_message = (char*)malloc(length+5);
	FILE *fp = fopen(filename,"r");
	if(fp == NULL){
		send_download_err(acceptfd);
	}else{
		int rsp_length = htonl(length);
		memcpy(&req_message[1],&rsp_length,4);
		fseek(fp,offset,SEEK_SET);
		req_message[0] = 0xa9;
		fread(&req_message[5],length,1,fp);
		send(acceptfd, req_message, length+5, 0);
		if(argut.debug){
			printf("DOWNLOAD_RSP sent with DataLength = %d\n",ntohl(rsp_length));	
		}
		fclose(fp);
	}	
	free(req_message);
}

void downloadReqHandle(int acceptfd,char *buff){
	int offset,length;
	int temp;
	memcpy(&temp,&buff[1],4);
	int datalength = ntohl(temp);
	if(datalength <= 8){
		send_download_err(acceptfd);
		return ;
	}
	
	memcpy(&temp,&buff[5],4);
	offset = ntohl(temp);
	memcpy(&temp,&buff[9],4);
	length = ntohl(temp);
	
	char filename[100] = {0};
	strcpy(filename,&buff[13]);
	if(argut.debug){ 
		printf("DOWNLOAD_REQ received with DataLength = %d, offset = %d, length = %d, filename = '%s'\n",datalength,offset,length,filename);	
	}
	
	if(check_argv_valid(filename,offset,length) == 0){
		send_download_err(acceptfd);
		return ;
	}
	if(length < 0)
		length = get_filesize(filename) - offset;
	send_download_rsp(acceptfd,filename,offset,length);
}

void display_timer(int sig){
	if(SIGALRM == sig){
		shutdown(sd,SHUT_RDWR);
		close(sd);
		exit(0);
	}
}


void thread_handle(void *fd){
	int acceptfd = *((int*)fd);
	unsigned char buff[MAX_MSG] = {0};
	
	int rc = recv(acceptfd, buff, 1024, 0);
	if(rc<0) {
		perror("cannot recv data ");
		close(acceptfd);
		exit(1);
	} 
	switch(buff[0]){
		case 0xea: filetypeReqHandle(acceptfd,buff); break;
		case 0xca: checksumReqHandle(acceptfd,buff); break;
		case 0xaa: downloadReqHandle(acceptfd,buff); break;
	}
	
}


int main (int argc, char *argv[]) {
	int ch;
	opterr = 0;
	while((ch = getopt(argc, argv, "t:d")) != -1){
	   switch(ch)
	   {
		  case 't':
			argut.shutdown = atoi(optarg);
			break;
		  case 'd':
			argut.debug = 1;
			break;
		  default:
			 printf("other option :%c\n", ch);
	   }
	}
	argut.port = atoi(argv[argc-1]);
   
	int newSd;
	struct sockaddr_in servAddr;
	char line[MAX_MSG];

	/* create socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd<0) {
		perror("cannot open socket ");
		return ERROR;
	}
  
	/* bind server port */
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(argut.port);
  
	if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
		perror("cannot bind port ");
		return ERROR;
	}

	listen(sd,5);
  
	signal(SIGALRM,display_timer);
	alarm(argut.shutdown);
	while(1) {
		newSd = accept(sd, NULL, NULL);
		alarm(argut.shutdown);
		if(newSd<0) {
		perror("cannot accept connection ");
		return ERROR;
		}
    
		pthread_t tid;
		if(pthread_create(&tid, NULL, (void *)thread_handle, (void *)&newSd) < 0){
			  printf("thread_handle thread create ERROR!\n");
		}
		
    
	} /* while (1) */

}


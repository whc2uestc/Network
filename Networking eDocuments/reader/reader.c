#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define EXUPERY 0
#define JOYCE 1
#define SHELLEY 2

typedef struct Reader{
	char user[20];
	char cur_book_name[20];
	int cur_page;
	char mode[5];
	int polling_interval;
}Reader;
Reader reader;
int sockfd;

typedef struct Post{
	char book_name[20];
	int page;
	int line;
	char serial_name[20];
	char user[20];
	char content[100];
	int index;
	struct Post *next;
}Post;
typedef struct Line{
	Post *post;
	int index;
}Line;
typedef struct Page{
	Line line[9];
}Page;
typedef struct DataBase{
	char book_name[20];
	Page page[4];
}DataBase;
DataBase db[3];
DataBase push_db[3];
char push_buff[1024] = {0};
int push_flag = 0;
void StrLTrim(char *pStr)  
{  
    char *pTmp = pStr;  
      
    while (*pTmp == ' ')   
    {  
        pTmp++;  
    }  
    while(*pTmp != '\0')  
    {  
        *pStr = *pTmp;  
        pStr++;  
        pTmp++;  
    }  
    *pStr = '\0';  
}


void display_handle(char *pStr){
	char buff[1024] = {0};
	sprintf(buff,"%s %s",pStr,reader.user);
	int rc = send(sockfd, buff, strlen(buff) + 1, 0);
	if(rc<0) {
		perror("cannot send data ");
		close(sockfd);
		exit(1);
	}
	char *para = strtok(pStr," ");
	if(para == NULL){
		printf("The input is error\n");
		return ;
	}
	para = strtok(NULL," ");
	if(para == NULL){
		printf("The input is error\n");
		return ;
	}
	strcpy(reader.cur_book_name,para);
	para = strtok(NULL," ");
	if(para == NULL){
		printf("The input is error\n");
		return ;
	}
	reader.cur_page = atoi(para);
	int book_index = 0;
	if(strstr(reader.cur_book_name,"exupery"))
		book_index = EXUPERY;
	else if(strstr(reader.cur_book_name,"joyce"))
		book_index = JOYCE;
	else if(strstr(reader.cur_book_name,"shelley"))
		book_index = SHELLEY;
	memset(buff,0,1024);
	rc = recv(sockfd,buff,1024,0);
	
	char data[1024] = {0};
	char *temp = buff;
	while(*temp!='\n')
		temp ++;
	temp ++;
	strcpy(data,temp);
	char flag[9] = {0};
	para = strtok(buff,",");
	if(atoi(para) > db[book_index].page[reader.cur_page-1].line[0].index)
		flag[0] = 'n';
	else if(db[book_index].page[reader.cur_page-1].line[0].index != 0)
		flag[0] = 'm';
	else
		flag[0] = ' ';
	
	
	
	int i = 1;
	for(i=1; i<9; i++){
		para = strtok(NULL,",");
		if(atoi(para) > db[book_index].page[reader.cur_page-1].line[i].index)
			flag[i] = 'n';
		else if(db[book_index].page[reader.cur_page-1].line[i].index != 0)
			flag[i] = 'm';
		else
			flag[i] = ' ';
	}
	data[0] = flag[0];
	int line_cnt = 0;
	for(i=0; i<strlen(data); i++){
		if(data[i] == '\n'){
			line_cnt ++;
			data[i+1] = flag[line_cnt];
			if(line_cnt == 8)
				break;
		}
	}
	printf("%s\n",data);
	
	alarm(reader.polling_interval);
}

void post_to_forum_handle(char *pStr){
	char *command = "post_to_forum";
	pStr = pStr + 14;
	char para[5] = {0};
	int index = 0;
	while(*pStr != ' '){
		para[index ++] = *pStr;
		pStr ++;
	}
	int line_number = atoi(para);
	char buff[1024] = {0};
	sprintf(buff,"%s %s %d %d %s %s",command,reader.cur_book_name,reader.cur_page,line_number,reader.user,pStr);
	/* send command to server */
	int rc = send(sockfd, buff, strlen(buff) + 1, 0);
	if(rc<0) {
		perror("cannot send data ");
		close(sockfd);
		exit(1);
	}
}

void read_post_handle(char *pStr){
	char *command = strtok(pStr," ");
	if(command == NULL){
		printf("The input is error\n");
		return ;
	}
	char *para = strtok(NULL," ");
	if(para == NULL){
		printf("The input is error\n");
		return ;
	}
	int line_number = atoi(para);
	int book_index = 0;
	if(strstr(reader.cur_book_name,"exupery"))
		book_index = EXUPERY;
	else if(strstr(reader.cur_book_name,"joyce"))
		book_index = JOYCE;
	else if(strstr(reader.cur_book_name,"shelley"))
		book_index = SHELLEY;
	
	char buff[1024] = {0};
	/* send command to server */
	sprintf(buff,"%s %s %d %d %d",command,reader.cur_book_name,reader.cur_page,line_number,db[book_index].page[reader.cur_page].line[line_number-1]);
	int rc = send(sockfd, buff, strlen(buff) + 1, 0);
	if(rc<0) {
		perror("cannot send data ");
		close(sockfd);
		exit(1);
	}
	/* read posts from server */
	memset(buff,0,1024);
	rc = recv(sockfd,buff,1024,0);
	char hint[50] = {0};
	sprintf(hint,"Book by %s, Page %d, Line number %d:",reader.cur_book_name,reader.cur_page,line_number);
	printf("%s\n",hint);
	printf("%s",buff);
	int i;
	int update_index = 0;
	for(i=strlen(buff)-1; i>=0; i--){
		if(buff[i] == '\n')
			update_index ++;
	}
	db[book_index].page[reader.cur_page-1].line[line_number-1].index += update_index;
}
void display_timer(int sig){
	if(SIGALRM == sig){
		char command[100] = {0};
		sprintf(command,"display %s %d",reader.cur_book_name,reader.cur_page);
		display_handle(command);
	}
}

void pull_handle(){
	while(1){
		char buff[1024] = {0};
		gets(buff);
		StrLTrim(buff);
		if(strstr(buff,"display") == buff)
			display_handle(buff);
		else if(strstr(buff,"post_to_forum") == buff)
			post_to_forum_handle(buff);
		else if(strstr(buff,"read_post"))
			read_post_handle(buff);
	}
}

void push_display_handle(char *pStr){
	char buff[1024] = {0};
	sprintf(buff,"%s %s",pStr,reader.user);
	int rc = send(sockfd, buff, strlen(buff) + 1, 0);
	if(rc<0) {
		perror("cannot send data ");
		close(sockfd);
		exit(1);
	}
	
	char *para = strtok(pStr," ");
	if(para == NULL){
		printf("The input is error\n");
		return ;
	}
	para = strtok(NULL," ");
	if(para == NULL){
		printf("The input is error\n");
		return ;
	}
	strcpy(reader.cur_book_name,para);
	para = strtok(NULL," ");
	if(para == NULL){
		printf("The input is error\n");
		return ;
	}
	reader.cur_page = atoi(para);
	int book_index = 0;
	if(strstr(reader.cur_book_name,"exupery"))
		book_index = EXUPERY;
	else if(strstr(reader.cur_book_name,"joyce"))
		book_index = JOYCE;
	else if(strstr(reader.cur_book_name,"shelley"))
		book_index = SHELLEY;
	
	char data[1024] = {0};
	//rc = recv(sockfd,buff,1024,0);
	
	while(push_flag == 0)
		;
	push_flag = 0;
	strcpy(data,push_buff);
	memset(push_buff,0,1024);
	char flag[9] = {0};
	int i = 0;
	for(i=0; i<9; i++){
		if(push_db[book_index].page[reader.cur_page-1].line[i].index > db[book_index].page[reader.cur_page-1].line[i].index)
			flag[i] = 'n';
		else if(db[book_index].page[reader.cur_page-1].line[i].index != 0)
			flag[i] = 'm';
		else
			flag[i] = ' ';
	}
	
	data[0] = flag[0];
	int line_cnt = 0;
	for(i=0; i<strlen(data); i++){
		if(data[i] == '\n'){
			line_cnt ++;
			data[i+1] = flag[line_cnt];
			if(line_cnt == 8)
				break;
		}
	}
	printf("%s\n",data);
}
void push_post_to_forum_handle(char *pStr){
	char *command = "post_to_forum";
	pStr = pStr + 14;
	char para[5] = {0};
	int index = 0;
	while(*pStr != ' '){
		para[index ++] = *pStr;
		pStr ++;
	}
	int line_number = atoi(para);
	char buff[1024] = {0};
	sprintf(buff,"%s %s %d %d %s %s",command,reader.cur_book_name,reader.cur_page,line_number,reader.user,pStr);
	/* send command to server */
	int rc = send(sockfd, buff, strlen(buff) + 1, 0);
	if(rc<0) {
		perror("cannot send data ");
		close(sockfd);
		exit(1);
	}
}

void push_read_post_handle(char *pStr){
	char *command = strtok(pStr," ");
	if(command == NULL){
		printf("The input is error\n");
		return ;
	}
	char *para = strtok(NULL," ");
	if(para == NULL){
		printf("The input is error\n");
		return ;
	}
	int line_number = atoi(para);
	int book_index = 0;
	if(strstr(reader.cur_book_name,"exupery"))
		book_index = EXUPERY;
	else if(strstr(reader.cur_book_name,"joyce"))
		book_index = JOYCE;
	else if(strstr(reader.cur_book_name,"shelley"))
		book_index = SHELLEY;
	
	char buff[1024] = {0};
	/* send command to server */
	sprintf(buff,"%s %s %d %d %d",command,reader.cur_book_name,reader.cur_page,line_number,db[book_index].page[reader.cur_page].line[line_number-1]);
	int rc = send(sockfd, buff, strlen(buff) + 1, 0);
	if(rc<0) {
		perror("cannot send data ");
		close(sockfd);
		exit(1);
	}
	/* read posts from server */
	memset(buff,0,1024);
	//rc = recv(sockfd,buff,1024,0);
	while(push_flag == 0)
		;
	push_flag = 0;
	strcpy(buff,push_buff);
	memset(push_buff,0,1024);
	char hint[50] = {0};
	sprintf(hint,"Book by %s, Page %d, Line number %d:",reader.cur_book_name,reader.cur_page,line_number);
	printf("%s\n",hint);
	printf("%s",buff);
	int i;
	int update_index = 0;
	for(i=strlen(buff)-1; i>=0; i--){
		if(buff[i] == '\n')
			update_index ++;
	}
	db[book_index].page[reader.cur_page-1].line[line_number-1].index += update_index;
}
void push_thread_handle(){
	while(1){
		int rc = recv(sockfd, push_buff, 1024, 0);
		if(rc<0) {
			perror("cannot send data ");
			close(sockfd);
			exit(1);
		}
		if(strstr(push_buff,"&push")){
			strtok(push_buff," ");
			char *para = strtok(NULL," ");
			int book_index = 0;
			char book_name[10] = {0};
			strcpy(book_name,para);
			if(strstr(para,"exupery"))
				book_index = 0;
			else if(strstr(para,"joyce"))
				book_index = 1;
			else if(strstr(para,"shelley"))
				book_index = 2;
			para = strtok(NULL," ");
			int page = atoi(para);
			para = strtok(NULL," ");
			int line = atoi(para)-1;
			para = strtok(NULL," ");
			int index = atoi(para);
			if(strstr(book_name,reader.cur_book_name) && page==reader.cur_page){
				printf("There are new posts.\n");
			}
			
			push_db[book_index].page[page-1].line[line].index ++;
			
			memset(push_buff,0,1024);
		}else{
			push_flag = 1;	
			//while(push_flag  == 1)
			//	;
		}			
	}
}

void push_handle(){
	char login[50] = {0};
	sprintf(login,"push %s",reader.user);
	int rc = send(sockfd, login, strlen(login) + 1, 0);
	if(rc<0) {
		perror("cannot send data ");
		close(sockfd);
		exit(1);
	}
	
	char init_buff[1024] = {0};
	rc = recv(sockfd, init_buff, 1024, 0);
	if(rc<0) {
		perror("cannot recv data ");
		close(sockfd);
		exit(1);
	}
	strtok(init_buff,",");
	int book_i = 0,page_i = 0,line_i = 0;
	for(book_i = 0; book_i < 3; book_i ++){
		for(page_i=0; page_i<4; page_i++){
			for(line_i=0; line_i<9; line_i++){
				char *para = strtok(NULL,",");
				push_db[book_i].page[page_i].line[line_i].index = atoi(para);
			}
		}
	}
	
	pthread_t tid;
	if(pthread_create(&tid, NULL, (void *)push_thread_handle, NULL) == 0){
		  //printf("thread_handle thread create OK!\n");
	}
	
	while(1){
		char buff[1024] = {0};
		gets(buff);
		StrLTrim(buff);
		if(strstr(buff,"display") == buff)
			push_display_handle(buff);
		else if(strstr(buff,"post_to_forum") == buff)
			push_post_to_forum_handle(buff);
		else if(strstr(buff,"read_post"))
			push_read_post_handle(buff);
	}
}

int main(int argc,char *argv[]){
	if(argc < 6) {
		printf("usage: %s <server> <data1> <data2> ... <dataN>\n",argv[0]);
		exit(1);
	}
	strcpy(reader.user,argv[3]);
	strcpy(reader.mode,argv[1]);
	reader.polling_interval = atoi(argv[2]);
	
	int rc;
	struct sockaddr_in cliAddr, servAddr;
	struct hostent *h;
 
	h = gethostbyname(argv[4]);
	if(h==NULL) {
		printf("%s: unknown host '%s'\n",argv[0],argv[4]);
		exit(1);
	}

	int server_port = atoi(argv[5]);
	servAddr.sin_family = h->h_addrtype;
	memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	servAddr.sin_port = htons(server_port);
	
	/* create socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror("cannot open socket ");
		exit(1);
	}	
	
	/* connect to server */
	rc = connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr));
	if(rc < 0) {
		perror("cannot connect ");
		exit(1);
	}
	if(strstr(argv[1],"pull")){
		signal(SIGALRM,display_timer);
		pull_handle();
	}
	else if(strstr(argv[1],"push"))
		push_handle();
	
	
	
	
	close(sockfd);
	
	return 0;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <pthread.h>

#define MAX_MSG 100
#define END_LINE 0x0
#define ERROR 1
#define SECCESS 0

#define EXUPERY 0
#define JOYCE 1
#define SHELLEY 2

int read_line(int newSd, char *line_to_return);

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
	int size;
}Line;

typedef struct Page{
	Line line[9];
}Page;

typedef struct DataBase{
	char book_name[20];
	Page page[4];
}DataBase;
DataBase db[3];

typedef struct PushNode{
	char user[20];
	int acceptfd;
	struct PushNode *next;
}PushNode;
typedef struct PushList{
	PushNode *pList;
	int size;
}PushList;
PushList pl;
void add_to_pushList(int acceptfd,char *user){
	PushNode *node = (PushNode*)malloc(sizeof(PushNode));
	memset(node,0,sizeof(PushNode));
	node->acceptfd = acceptfd;
	strcpy(node->user,user);
	if(pl.pList == NULL)
		pl.pList = node;
	else{
		PushNode *cur = pl.pList;
		PushNode *pre;
		while(cur){
			pre = cur;
			cur = cur->next;
		}
		pre->next = node;
	}
	pl.size ++;
}

void display_handle(int acceptfd,char *pStr){
	strtok(pStr," ");
	char *book_name = strtok(NULL," ");
	char *para = strtok(NULL," ");
	int page = atoi(para);
	char *user = strtok(NULL," ");
	char book_path[20] = {0};
	sprintf(book_path,"%s_page%d",book_name,page);
	int book_index = 0;
	if(strstr(book_name,"exupery"))
		book_index = 0;
	else if(strstr(book_name,"joyce"))
		book_index = 1;
	else if(strstr(book_name,"shelley"))
		book_index = 2;
	PushNode *cur = pl.pList;
	int push_list_flag = 0;
	while(cur){
		if(strstr(cur->user,user)){
			push_list_flag = 1;
			break;
		}
		cur = cur->next;
	}
	FILE *fp = fopen(book_path,"r+");
	if(NULL == fp){
		printf("open file error!\n");
		exit(1);
	}
	char buff[1024] = {0};
	char data[1000] = {0};
	fread(data,1024,1,fp);
	fclose(fp);
	if(push_list_flag == 0){
		printf("A query is received from %s for posts associated with page %d of the book %s.\n",user,page,book_name);
	
		sprintf(buff,"%d,%d,%d,%d,%d,%d,%d,%d,%d\n%s",db[book_index].page[page-1].line[0].size,\
			db[book_index].page[page-1].line[1].size,db[book_index].page[page-1].line[2].size,\
			db[book_index].page[page-1].line[3].size,db[book_index].page[page-1].line[4].size,\
			db[book_index].page[page-1].line[5].size,db[book_index].page[page-1].line[6].size,\
			db[book_index].page[page-1].line[7].size,db[book_index].page[page-1].line[8].size,data);
		
		int rc = send(acceptfd, buff, strlen(buff) + 1, 0);
		if(rc<0) {
			perror("cannot send data \n");
			close(acceptfd);
			exit(1);
		}
	}else{
		int rc = send(acceptfd, data, strlen(data) + 1, 0);
		if(rc<0) {
			perror("cannot send data \n");
			close(acceptfd);
			exit(1);
		}
	}
}

void post_to_forum_handle(int acceptfd,char *pStr){
	Post *pPost = (Post*)malloc(sizeof(Post));
	memset(pPost,0,sizeof(Post));
	int cnt = 0;
	char *ppStr = pStr;
	while(*ppStr!='\0' && cnt<5){
		if(*ppStr == ' ')
			cnt ++;
		ppStr ++;
	}
	ppStr ++;
	strcpy(pPost->content,ppStr);
	strtok(pStr," ");
	char *para = strtok(NULL," ");
	strcpy(pPost->book_name,para);
	int book_index = 0;
	if(strstr(pPost->book_name,"exupery"))
		book_index = EXUPERY;
	else if(strstr(pPost->book_name,"joyce"))
		book_index = JOYCE;
	else if(strstr(pPost->book_name,"shelley"))
		book_index = SHELLEY;
	para = strtok(NULL," ");
	pPost->page = atoi(para);
	para = strtok(NULL," ");
	pPost->line = atoi(para) - 1;
	para = strtok(NULL," ");
	strcpy(pPost->user,para);

	
	pPost->index = db[book_index].page[pPost->page-1].line[pPost->line].size+1;
	sprintf(pPost->serial_name,"%s,%d,%d,%d",pPost->book_name,pPost->page,pPost->line+1,pPost->index);
	
	db[book_index].page[pPost->page-1].line[pPost->line].size ++;
	Post *cur = db[book_index].page[pPost->page-1].line[pPost->line].post;
	if(cur == NULL){
		db[book_index].page[pPost->page-1].line[pPost->line].post = pPost;
	}
	else{
		Post *pre = cur;
		while(cur){
			pre = cur;
			cur = cur->next;
		}
		pre->next = pPost;
	}
	
	printf("New post received from %s.\n",pPost->user);
	printf("Post added to the database and given serial number (%s).\n",pPost->serial_name);
	if(pl.size == 0)
		printf("Push list empty, No actioon required.\n");
	else{
		PushNode *cur = pl.pList;
		while(cur){
			printf("%s on the push list, push the message to her reader.\n",cur->user);
			char buff[1024] = {0};
			sprintf(buff,"&push %s %d %d %d %s: %s",pPost->book_name,pPost->page,pPost->line+1,pPost->index,pPost->user,pPost->content);
			int rc = send(cur->acceptfd, buff, strlen(buff) + 1, 0);
			if(rc<0) {
				perror("cannot send data ");
				close(cur->acceptfd);
				exit(1);
			}
			cur = cur->next;
		}
	}
	
}

void read_post_handle(int acceptfd,char *pStr){

	strtok(pStr," ");
	char *book_name = strtok(NULL," ");
	char *para = strtok(NULL," ");
	int page = atoi(para);
	para = strtok(NULL," ");
	int line = atoi(para) - 1;
	para = strtok(NULL," ");
	int post_deadline = atoi(para);
	int book_index = 0;
	
	if(strstr(book_name,"exupery"))
		book_index = 0;
	else if(strstr(book_name,"joyce"))
		book_index = 1;
	else if(strstr(book_name,"shelley"))
		book_index = 2;

	Post *temp = db[book_index].page[page-1].line[line].post;
	int post_count = 0;
	char buff[1024] = {0};
	while(temp){
		if(post_count >= post_deadline){
			char data[100] = {0};
			sprintf(data,"%d %s: %s\n",temp->index,temp->user,temp->content);
			strcat(buff,data);
		}
		post_count ++;
		temp = temp->next;
	}
	int rc = send(acceptfd, buff, strlen(buff) + 1, 0);
	if(rc<0) {
		perror("cannot send data \n");
		close(acceptfd);
		exit(1);
	}
}

void push_request_handle(int acceptfd,char *pStr){
	strtok(pStr," ");
	char *user = strtok(NULL," ");
	add_to_pushList(acceptfd,user);
	printf("Received a request from %s's reader to work in push mode \
		and has added it to the push list.\n",user);
	printf("Received a summary from Amy's reader.\n");
	int book_i = 0,page_i = 0,line_i = 0;
	
	int cnt = 0;
	char book_name[3][10] = {"exupery","joyce","shelley"};
	for(book_i = 0; book_i < 3; book_i ++){
		for(page_i=0; page_i<4; page_i++){
			for(line_i=0; line_i<9; line_i++){
				if(db[book_i].page[page_i].line[line_i].size == 0)
					continue;
				cnt ++;
				if(cnt == 1)
					printf("Forwarded the new posts to %s's reader:\n",user);
				Post *cur = db[book_i].page[page_i].line[line_i].post;
				printf("Book by %s, Page %d, Line %d\n",book_name[book_i],page_i+1,line_i+1);
				while(cur){
					printf("%d %s: %s\n",cur->index,cur->user,cur->content);
					cur = cur->next;
				}
			}
		}
	}
	char buff[1024] = {0};
	strcat(buff,"push,");
	for(book_i = 0; book_i < 3; book_i ++){
		for(page_i=0; page_i<4; page_i++){
			char buff0[100] = {0};
			sprintf(buff0,"%d,%d,%d,%d,%d,%d,%d,%d,%d,",db[book_i].page[page_i].line[0].size,\
					db[book_i].page[page_i].line[1].size,db[book_i].page[page_i].line[2].size,\
					db[book_i].page[page_i].line[3].size,db[book_i].page[page_i].line[4].size,\
					db[book_i].page[page_i].line[5].size,db[book_i].page[page_i].line[6].size,\
					db[book_i].page[page_i].line[7].size,db[book_i].page[page_i].line[8].size);
			strcat(buff,buff0);
		}
	}
	int rc = send(acceptfd, buff, strlen(buff)+1, 0);
	if(rc<0) {
		perror("cannot send data \n");
		close(acceptfd);
		exit(1);
	}
}


void thread_handle(void *fd){
	int acceptfd = *((int*)fd);
	char line[MAX_MSG];
	
	/* init line */
	memset(line,0x0,MAX_MSG);

	/* receive segments */
	while(read_line(acceptfd,line)!=ERROR) {  
		//printf("received: %s\n", line);
			
		if(strstr(line,"display") == line)
			display_handle(acceptfd,line);
		else if(strstr(line,"post_to_forum") == line)
			post_to_forum_handle(acceptfd,line);
		else if(strstr(line,"read_post") == line)
			read_post_handle(acceptfd,line);	
		else if(strstr(line,"push") == line)
			push_request_handle(acceptfd,line);
		/* init line */
		memset(line,0x0,MAX_MSG);
  
	} /* while(read_line) */
}


int main(int argc,char *argv[]){
	if(argc != 2){
		printf("The argc is error!\n");
		return 1;
	}
	int server_port = atoi(argv[1]);
	
	int sockfd,acceptfd;
    struct sockaddr_in cliAddr, servAddr;

	
	/* create socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror("cannot open socket!\n");
		return 1;
	}
	/* bind server port */
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(server_port);
  
	if(bind(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
		perror("cannot bind port \n");
		return 1;
	}
	listen(sockfd,5);
	
	printf("The server is listening on port number %d\n",server_port);
	printf("The database for discussion posts has been intialised\n");
	
	while(1) {
		int addr_len = sizeof(cliAddr);
		acceptfd = accept(sockfd, (struct sockaddr *) &cliAddr, &addr_len);
		if(acceptfd<0) {
			perror("cannot accept connection \n");
			return 1;
		}
    
		pthread_t tid;
		if(pthread_create(&tid, NULL, (void *)thread_handle, (void *)&acceptfd) < 0){
			  printf("thread_handle thread create ERROR!\n");
		}
		
    
	} /* while (1) */
	return 0;
}


/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING       */
/* this function is experimental.. I don't know yet if it works  */
/* correctly or not. Use Steven's readline() function to have    */
/* something robust.                                             */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING       */

/* rcv_line is my function readline(). Data is read from the socket when */
/* needed, but not byte after bytes. All the received data is read.      */
/* This means only one call to recv(), instead of one call for           */
/* each received byte.                                                   */
/* You can set END_CHAR to whatever means endofline for you. (0x0A is \n)*/
/* read_lin returns the number of bytes returned in line_to_return       */
int read_line(int newSd, char *line_to_return) {
  
  static int rcv_ptr=0;
  static char rcv_msg[MAX_MSG];
  static int n;
  int offset;

  offset=0;

  while(1) {
    if(rcv_ptr==0) {
      /* read data from socket */
      memset(rcv_msg,0x0,MAX_MSG); /* init buffer */
      n = recv(newSd, rcv_msg, MAX_MSG, 0); /* wait for data */
      if (n<0) {
	perror(" cannot receive data ");
	return ERROR;
      } else if (n==0) {
	printf(" connection closed by client\n");
	close(newSd);
	return ERROR;
      }
    }
  
    /* if new data read on socket */
    /* OR */
    /* if another line is still in buffer */

    /* copy line into 'line_to_return' */
    while(*(rcv_msg+rcv_ptr)!=END_LINE && rcv_ptr<n) {
      memcpy(line_to_return+offset,rcv_msg+rcv_ptr,1);
      offset++;
      rcv_ptr++;
    }
    
    /* end of line + end of buffer => return line */
    if(rcv_ptr==n-1) { 
      /* set last byte to END_LINE */
      *(line_to_return+offset)=END_LINE;
      rcv_ptr=0;
      return ++offset;
    } 
    
    /* end of line but still some data in buffer => return line */
    if(rcv_ptr <n-1) {
      /* set last byte to END_LINE */
      *(line_to_return+offset)=END_LINE;
      rcv_ptr++;
      return ++offset;
    }

    /* end of buffer but line is not ended => */
    /*  wait for more data to arrive on socket */
    if(rcv_ptr == n) {
      rcv_ptr = 0;
    } 
    
  } /* while */
}

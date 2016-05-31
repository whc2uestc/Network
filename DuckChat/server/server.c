/*
 * 	server.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "message_handler.h"
static time_t lasttime;


int main(int argc, char** argv) {
	if(argc < 3) {
		printf("Usage:%s ip/hostname port\n",argv[0]);
		return -1;
	}	
	srand(time(0));
	adj_server = NULL;
	server_tree = NULL;
	int i;
	for(i=3; i<3+(argc-3)/2; i++){
		int tmp = atoi(argv[i+1+(i-3)]);
		insert_into_list(tmp,argv[i+(i-3)]);
	}
	
	char *ptr,**pptr;
	struct hostent *hptr;
	char str[1024];	
	ptr = argv[1];
	hptr = gethostbyname(ptr);
	pptr = hptr->h_addr_list;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if(*ptr >= '0' && *ptr <= '9') {
		inet_pton(AF_INET,ptr,&servaddr.sin_addr);
	}
	else {
		inet_pton(AF_INET,inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)), &servaddr.sin_addr);
	}
	servaddr.sin_port = htons(atoi(argv[2]));
	bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	timeout_init();
	struct pollfd fds;
	fds.fd = sockfd;
	fds.events = POLLIN;
	fds.revents = 0;

	struct sigaction act;
    union sigval tsval;
    act.sa_handler = renew_timer;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(50, &act, NULL);
    time(&lasttime);
	while(1) {
		
		struct sockaddr_in *cliaddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
		int len = sizeof(struct sockaddr_in);
		char buf[100];
		long timeout = sch_timer();
		out.last = time(NULL);
		int ret = poll(&fds,1,timeout*1000);
		if(ret < 0) {
			printf("something goes wrong\n");
			return 0;
		}
		else if(ret == 0) {
			handle_timer(NULL);
		}
		else if(fds.revents & POLLIN) {
			recvfrom(sockfd,buf,sizeof(buf),0,(struct sockaddr*)cliaddr,&len);
			Read_Handler(buf,cliaddr);
		}
		time_t nowtime;
        /*获取当前时间*/
        time(&nowtime);
		if (nowtime - lasttime >= 50)
        {
            /*向主进程发送信号，实际上是自己给自己发信号*/
            sigqueue(getpid(), 50, tsval);
            lasttime = nowtime;
        }
	}
	
	return 0;
}

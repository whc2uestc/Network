/*
 * 	common.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "common.h"
#include "raw.h"
#include <time.h>
/*when client start, before go to coer cycle, there are something need to be init,including user name, server socket etc.*/
int client_init(int argc, char** argv) {
	char *ptr, **pptr;
	struct hostent *hptr;
	char str[1024];

	ptr = argv[1];
	hptr = gethostbyname(ptr);
	pptr = hptr->h_addr_list;
	client_cycle.User_Name = argv[3];
	bzero(&client_cycle.servaddr,sizeof(client_cycle.servaddr));
	client_cycle.servaddr.sin_family = AF_INET;
	client_cycle.servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET,inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)), &client_cycle.servaddr.sin_addr);
	

	client_cycle.Active_Channel = (header_t*)malloc(sizeof(header_t));
	client_cycle.Active_Channel->Num_Channel = 0;
	client_cycle.Active_Channel->next =NULL;

	client_cycle.flag = 1;
	timer.flag = -1;
	timer.count = 0;

	Com_Parse("/Login");
	
	return 0;
}
/*using poll to listen both server and user,and call different method to handle timeout and message from server and command from keyboard*/
int client_core_cycle() {	
	struct pollfd fds[2];
	fds[0].fd = 0;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	fds[1].fd = client_cycle.server;
	fds[1].events = POLLIN;
	fds[1].revents = 0;
	int ret;
	long loop = 60000;
	time_t time1,time2;
	int timeout = 0;
	

	while(client_cycle.flag) {
		printf(">");
		fflush(stdout);
		time1 = time((time_t*)0);
		ret = poll(fds, 2, loop);
		if(ret < 0) {
			return -1;
		}
		else if(ret == 0) {
			timeout = 1;
		}
		else if(fds[0].revents & POLLIN) {
			Write_Handler();
			loop = 60000;
			timeout = 0;
		}

		else if(fds[1].revents & POLLIN) {
			time2 = time((time_t*)0);
			loop = loop + (time1 - time2)*1000;
			if(loop <= 0)
				timeout = 1;
			time1 = time((time_t*)0);
			Read_Handler();
			time2 = time((time_t*)0);
			loop = loop + (time1 - time2)*1000;
			if(loop <= 0)
				timeout = 1;
		}

		if(timeout) {
			Keep_Alive_Handler();
			timeout = 0;
			loop = 60000;
			printf("\b");
		}


	}

	return 0;
}

int client_cleanup() {
	if(client_cycle.Active_Channel->next) {
		clean(client_cycle.Active_Channel->next);
	}
	close(client_cycle.server);
	client_cycle.flag = 0;
	return 0;
}

void Read_Handler() {
	printf("\b");
	raw_mode();
	struct sockaddr_in cliaddr;
	int len = sizeof(cliaddr);
	char buf[1024];
	recvfrom(client_cycle.server,buf,sizeof(buf),0,(struct sockaddr*)&cliaddr,&len);
	int txt_type = *(int*)buf;
	switch(txt_type) {
		case TXT_SAY:
			Handler_Say(buf);
			break;
		case TXT_LIST:
			Handler_List(buf);
			break;
		case TXT_WHO:
			Handler_Who(buf);
			break;
		case TXT_ERROR:
			Handler_Error(buf);
			break;
		default:
			return ;
	}
	return ;
}

void Write_Handler() {
	char buf[65];
	gets(buf);
	Com_Parse(buf);
	
	return ;
}

void clean(channel_t* cptr) {
	if(cptr->next)
		clean(cptr->next);
	free(cptr);
	return ;
}

/*
 * 	command_handler.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "command_handler.h"

//handle longin command include connection ,join Common channel etc.
void Login_Handler() {
	client_cycle.server = socket(AF_INET,SOCK_DGRAM, 0);

	login_t login;
	login.req_type = REQ_LOGIN;
	strcpy(login.req_username,client_cycle.User_Name);

	sendto(client_cycle.server,(char*)&login,sizeof(login),0,(struct sockaddr*)&client_cycle.servaddr,sizeof(client_cycle.servaddr));
	
	client_cycle.Cur_Channel = Add_Channel(client_cycle.Active_Channel,"Common");

	

	return ;

}
//handle exit command
void Logout_Handler() {
	logout_t logout;
	logout.req_type = REQ_LOGOUT;
	sendto(client_cycle.server,(char*)&logout,sizeof(logout),0,(struct sockaddr*)&client_cycle.servaddr,sizeof(client_cycle.servaddr));
	client_cleanup();

	return ;
}
//handle join command
void Join_Handler(char* command) {
	join_t join;
	join.req_type = REQ_JOIN;
	strcpy(join.req_channel, command);

	sendto(client_cycle.server,(char*)&join,sizeof(join),0,(struct sockaddr*)&client_cycle.servaddr,sizeof(client_cycle.servaddr));

	client_cycle.Cur_Channel = Add_Channel(client_cycle.Active_Channel,command);

	return ;
}
//handle leave command
void Leave_Handler(char* command) {
	leave_t leave;
	leave.req_type = REQ_LEAVE;
	strcpy(leave.req_channel,command);

	sendto(client_cycle.server,(char*)&leave,sizeof(leave),0,(struct sockaddr*)&client_cycle.servaddr,sizeof(client_cycle.servaddr));

	if(client_cycle.Cur_Channel == Sch_Channel(client_cycle.Active_Channel,command)) {
		printf("NOTIC:Now you leave your current channel,you should join a channel first!\n");
		client_cycle.Cur_Channel = NULL;
	}
	Del_Channel(client_cycle.Active_Channel,command);
	channel_t *ptr = client_cycle.Active_Channel->next;	

	return ;
}
//hanlde send text to server
void Say_Handler(char* command) {
	if(!client_cycle.Cur_Channel) {
		printf("ERROR:You should join a channel first\n");
		return ;
	}
	say_t say;
	say.req_type = REQ_SAY;
	strcpy(say.req_channel,client_cycle.Cur_Channel->name);
	strcpy(say.req_text,command);

	sendto(client_cycle.server,(char*)&say,sizeof(say),0,(struct sockaddr*)&client_cycle.servaddr,sizeof(client_cycle.servaddr));

	return ;
}
//send message to server for all the channels
void List_Handler() {
	list_t list;
	list.req_type = REQ_LIST;

	sendto(client_cycle.server,(char*)&list,sizeof(list),0,(struct sockaddr*)&client_cycle.servaddr,sizeof(client_cycle.servaddr));
	Read_Handler();
	return ;
}
//get current channel existing users
void Who_Handler(char* command) {
	who_t who;
	who.req_type = REQ_WHO;
	strcpy(who.req_channel,command);

	sendto(client_cycle.server,(char*)&who,sizeof(who),0,(struct sockaddr*)&client_cycle.servaddr,sizeof(client_cycle.servaddr));
	Read_Handler();

	return ;
}
//try to notify server do not disconnect
void Keep_Alive_Handler() {

	keep_alive_t keep;
	keep.req_type = REQ_KEEP_ALIVE;

	sendto(client_cycle.server,(char*)&keep,sizeof(keep),0,(struct sockaddr*)&client_cycle.servaddr,sizeof(client_cycle.servaddr));
	if(timer.flag == -1) {	
		timer.flag = time(NULL);
		timer.count = 1;
	}
	else {
		//printf("\b");
		//printf("Oops!Server has down,you can exit client!\n");
	}

	return ;
}

int Switch_Handler(char* command) {
	channel_t *ptr = Sch_Channel(client_cycle.Active_Channel, command);
	if(ptr == NULL) {
		printf("ERROR:You should join that channel first\n");
		return -1;	
	}
	else if(ptr == client_cycle.Cur_Channel) {
		printf("ERROR:You already join this channel\n");	
	}
	else client_cycle.Cur_Channel = ptr;
	channel_t *ptr_1 = client_cycle.Active_Channel->next;	
	while(ptr_1)
	{
		printf("exist channel:%s\n",ptr_1->name);
		ptr_1 = ptr_1->next;
	}
	printf("cur channel %s\n",client_cycle.Cur_Channel->name);

	return 0;
}

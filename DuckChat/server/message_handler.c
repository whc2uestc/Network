/*
 * 	message_handler.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "message_handler.h"
typedef struct iden_node{
	long long idenf[10000];
	int len;
	int index;
}iden_node;
iden_node iden_tree;

int s2s_renew_join(){
	tree_node *ptree = server_tree;
	while(ptree){
		text_join_s2s buf;
		buf.txt_type = TXT_JOIN_S2S;
		strcpy(buf.channelname,ptree->channelname);
		
		server_node *pserver = adj_server;
		while(pserver){
			sendto(sockfd,(char*)&buf,sizeof(text_join_s2s)+1,0,(struct sockaddr*)&(pserver->addr),sizeof(struct sockaddr_in));
			printf("%s:%d %s:%d send S2S Join %s\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(pserver->addr.sin_addr),ntohs(pserver->addr.sin_port),ptree->channelname);
			pserver = pserver->next;
		}
		ptree = ptree->next;
	}
	
	return 0;
}


void renew_timer(int sig)
{
	s2s_renew_join();
	//alarm(60);       //we contimue set the timer	
}


int search_iden_tree(long long num){
	int i;
	for(i=0;i<iden_tree.len;i++){
		if(iden_tree.idenf[i] == num)
			return 1;
	}
	return 0;
}
int insert_iden_tree(long long num){
	if(search_iden_tree(num) == 0){
		iden_tree.idenf[iden_tree.index++] = num;
		iden_tree.index = iden_tree.index%10000;
		iden_tree.len ++;
		if(iden_tree.len > 10000)
			iden_tree.len = 10000;
		return 0;
	}else
		return 1;
}


int s2s_join(struct sockaddr_in* fromaddr,char *channelname,int type){
	insert_into_tree(channelname,adj_server);
	text_join_s2s buf;
	buf.txt_type = TXT_JOIN_S2S;
	strcpy(buf.channelname,channelname);
	server_node *cur = adj_server;	
	if(type == 0){ //c2s
		while(cur){
			sendto(sockfd,(char*)&buf,sizeof(text_join_s2s)+1,0,(struct sockaddr*)&(cur->addr),sizeof(struct sockaddr_in));
			printf("%s:%d %s:%d send S2S Join %s\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cur->addr.sin_addr),ntohs(cur->addr.sin_port),channelname);
			cur = cur->next;
		}
	}else{//s2s
		while(cur){
			if(cur->addr.sin_addr.s_addr!=fromaddr->sin_addr.s_addr || cur->addr.sin_port!=fromaddr->sin_port){
				sendto(sockfd,(char*)&buf,sizeof(text_join_s2s)+1,0,(struct sockaddr*)&(cur->addr),sizeof(struct sockaddr_in));
				printf("%s:%d %s:%d send S2S Join %s\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cur->addr.sin_addr),ntohs(cur->addr.sin_port),channelname);
			}
			cur = cur->next;
		}
		
	}
	return 0;
}

int s2s_leave(struct sockaddr_in* fromaddr,char *channelname,int type){
	text_leave_s2s buf;
	buf.txt_type = TXT_LEAVE_S2S;
	strcpy(buf.channelname,channelname);
	
	
	if(delete_from_tree(fromaddr,channelname)==0){
		sendto(sockfd,(char*)&buf,sizeof(text_leave_s2s)+1,0,(struct sockaddr*)fromaddr,sizeof(struct sockaddr_in));
		printf("%s:%d %s:%d send S2S Leave %s\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(fromaddr->sin_addr),ntohs(fromaddr->sin_port),channelname);
	}
	
	return 0;
}
long long read_random_nunmber(){
	FILE *fp;
	int byte_count = 8;
	long long data;
	fp = fopen("/dev/urandom", "r");
	fread(&data, 1, byte_count, fp);
	fclose(fp);
	return data;
}

int s2s_say(struct sockaddr_in* fromaddr,char *channelname,int type,char *bbuf){
	text_say_s2s buf;
	buf.txt_type = TXT_SAY_S2S;
	
	tree_node *head = server_tree;
	while(head && strcmp(head->channelname,channelname)!=0)
		head = head->next;
	server_node *cur = adj_server;//head->server_list;
	if(type == 0){ //c2s
		buf.txt_idenf = read_random_nunmber();
		//while(insert_iden_tree(buf.txt_idenf)){
		//	buf.txt_idenf = read_random_nunmber();
		//}
		strcpy(buf.txt_username,((struct text_say*)bbuf)->txt_username);
		strcpy(buf.txt_channel,((struct text_say*)bbuf)->txt_channel);
		strcpy(buf.txt_text,((struct text_say*)bbuf)->txt_text);
		while(cur){
			sendto(sockfd,(char*)&buf,sizeof(text_say_s2s)+1,0,(struct sockaddr*)&(cur->addr),sizeof(struct sockaddr_in));
			printf("%s:%d %s:%d send S2S Say %s %s \"%s\"\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cur->addr.sin_addr),ntohs(cur->addr.sin_port),buf.txt_username,buf.txt_channel,buf.txt_text);
			cur = cur->next;
		}
	}else{ // s2s
		buf.txt_idenf = ((struct text_say_s2s*)bbuf)->txt_idenf;
		strcpy(buf.txt_username,((struct text_say_s2s*)bbuf)->txt_username);
		strcpy(buf.txt_channel,((struct text_say_s2s*)bbuf)->txt_channel);
		strcpy(buf.txt_text,((struct text_say_s2s*)bbuf)->txt_text);
		if(search_iden_tree(buf.txt_idenf)){
			s2s_leave(fromaddr,buf.txt_channel,1);
			printf("leave1\n");
			return 1;
		}else
			insert_iden_tree(buf.txt_idenf);
		int flag = 0;
		while(cur){
			if(cur->addr.sin_addr.s_addr!=fromaddr->sin_addr.s_addr || cur->addr.sin_port!=fromaddr->sin_port){
				sendto(sockfd,(char*)&buf,sizeof(text_say_s2s)+1,0,(struct sockaddr*)&(cur->addr),sizeof(struct sockaddr_in));
				printf("%s:%d %s:%d send S2S Say %s %s \"%s\"\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cur->addr.sin_addr),ntohs(cur->addr.sin_port),buf.txt_username,buf.txt_channel,buf.txt_text);
				flag = 1;
			}
			cur = cur->next;
		}
		if(flag == 0){
			s2s_leave(fromaddr,buf.txt_channel,1);
			printf("leave2");
		}
	}
	
}

int S2S_Handle_join(user_t* user,char* buf,struct sockaddr_in *cliaddr) {
	char* ptr = ((struct text_join_s2s*)buf)->channelname;
	//if(Sch_channel_list(ptr) != NULL)
		//return 1;
	printf("%s:%d %s:%d recv S2S Join %s\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cliaddr->sin_addr),ntohs(cliaddr->sin_port),ptr);
	channel_t* channel = Sch_channel_list(ptr);
	if(channel == NULL) {
		s2s_join(cliaddr,ptr,1); //s2s
		Add_channel_list(ptr);
		insert_into_tree(ptr,adj_server);
	}
	return 0;
}

int S2S_Handle_Leave(user_t* user,char* buf,struct sockaddr_in *cliaddr) {
	char* ptr = ((struct text_leave_s2s*)buf)->channelname;
	delete_from_tree(cliaddr,ptr);
	printf("%s:%d %s:%d recv S2S Leave %s\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cliaddr->sin_addr),ntohs(cliaddr->sin_port),ptr);
	return 0;
}

int S2S_Handle_Say(user_t* user,char* buf,struct sockaddr_in *cliaddr) {
	char *ptr = ((struct text_say_s2s*)buf)->txt_channel;
	channel_t* channel = Sch_channel_list(ptr);

	struct text_say text;
	text.txt_type = TXT_SAY;
	strcpy(text.txt_channel,((struct text_say_s2s*)buf)->txt_channel);
	strcpy(text.txt_username,((struct text_say_s2s*)buf)->txt_username);
	strcpy(text.txt_text,((struct text_say_s2s*)buf)->txt_text);
	printf("%s:%d %s:%d recv S2S Say %s %s \"%s\"\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cliaddr->sin_addr),ntohs(cliaddr->sin_port),text.txt_username,ptr,text.txt_text);
	Say_Handler_ss((char*)&text,sizeof(text));
	s2s_say(cliaddr,ptr,1,buf); //s2s

	return 0;
}
int Read_Handler(char* buf,struct sockaddr_in* cliaddr) {	
	int ret;
	user_t* user = Sch_Userbysock(cliaddr);
	int type = *(int*)buf;
	switch(type) {
		case REQ_LOGIN:
			ret = Handle_Login(buf, cliaddr);
			break;
		case REQ_LOGOUT:
			ret = Handle_Logout(user);
			break;
		case REQ_JOIN:
			ret = Handle_join(user, buf,cliaddr);
			break;
		case REQ_LEAVE:
			ret = Handle_Leave(user, buf,cliaddr);
			break;
		case REQ_SAY:
			ret = Handle_Say(user, buf,cliaddr);
			break;
		case REQ_LIST:
			ret = Handle_List(user, cliaddr);
			break;
		case REQ_WHO:
			ret = Handle_Who(user, buf,cliaddr);
			break;
		case REQ_KEEP_ALIVE:
			ret = Handle_Keep_Alive(user, buf,cliaddr);
			break;
		case TXT_JOIN_S2S:
			S2S_Handle_join(user, buf,cliaddr);
			break;
		case TXT_LEAVE_S2S:
			S2S_Handle_Leave(user, buf,cliaddr);
			break;
		case TXT_SAY_S2S:
			S2S_Handle_Say(user, buf,cliaddr);
			break;
		default: {
			char* err = "Unkown command.";
			int len = strlen(err);
			if(user)
				Error_Handler(err,user->cliaddr,len);
			else
				Error_Handler(err,cliaddr,len);
			break;
		}
	}
	return ret;
}

int Say_Handler_ss(char* buf,int len) {
	char *ptr = ((struct text_say*)buf)->txt_channel;
	channel_t* channel = Sch_channel_list(ptr);
	channel_user_t* cuser = channel->user_list;
	while(cuser) {
		struct sockaddr_in* cliaddr = cuser->user->cliaddr;
		printf("%s:%d %s:%d send Request say %s %s \"%s\"\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cliaddr->sin_addr),ntohs(cliaddr->sin_port),cuser->user->user_name,ptr,((struct text_say*)buf)->txt_text);
		sendto(sockfd,buf,len,0,(struct sockaddr*)cuser->user->cliaddr,sizeof(struct sockaddr_in));
		cuser = cuser->next;
	}
	return 0;

}

int Say_Handler(char* buf,int len) {
	char *ptr = ((struct text_say*)buf)->txt_channel;
	channel_t* channel = Sch_channel_list(ptr);
	channel_user_t* cuser = channel->user_list;
	while(cuser) {
		struct sockaddr_in* cliaddr = cuser->user->cliaddr;
		sendto(sockfd,buf,len,0,(struct sockaddr*)cuser->user->cliaddr,sizeof(struct sockaddr_in));
		cuser = cuser->next;
	}
	return 0;

}

int List_Handler(user_t* user, char* buf, int len) {
	int ret;
	ret = sendto(sockfd,buf,len,0,(struct sockaddr*)user->cliaddr,sizeof(struct sockaddr_in));
	return ret;

}

int Who_Handler(user_t* user, char* buf, int len) {
	int ret = sendto(sockfd,buf,len,0,(struct sockaddr*)user->cliaddr,sizeof(struct sockaddr_in));
	return ret;
}

int Error_Handler(char* err,struct sockaddr_in *cliaddr,int len) {
	struct text_error error;
	error.txt_type = TXT_ERROR;
	strcpy(error.txt_error,err);
	int ret = sendto(sockfd,(char*)&error,sizeof(error),0,(struct sockaddr*)cliaddr,sizeof(struct sockaddr_in));
	return 0;
}

int Handle_Login(char* buf, struct sockaddr_in *cliaddr) {
		
	user_t *user = Sch_Userbysock(cliaddr);
	if(user) {
		char *err = "User already exist!";
		int len = strlen(err);
		Error_Handler(err,user->cliaddr,len);
		free(cliaddr);
		handle_timer(NULL);
		return -1;
	}
	char* ptr = ((struct request_login*)buf)->req_username;
	printf("*server:user %s login server\n",ptr);
	printf("%s:%d %s:%d recv Request login %s\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cliaddr->sin_addr),ntohs(cliaddr->sin_port),ptr);
	
	if(Sch_channel_list("Common")==NULL){
		s2s_join(cliaddr,"Common",0); //s2s 
		Add_channel_list("Common");
	}
	Add_user_users(ptr,cliaddr);
	Add_user_channel(ptr,"Common");
	user = Sch_Userbysock(cliaddr);
	add_timer(user);
	
	return 0;
}

int Handle_Logout(user_t* user) {
	channel_t* channel = serv_list.channel_list;
	printf("*server:user:%s logout\n",user->user_name);

	if(channel) {
		while(channel) {
			channel_user_t *uptr = Sch_user_channel(channel,user->user_name);
			if(uptr) {
				if(user == uptr->user) {
					Del_user_channel(user->user_name,channel->channel_name);
					channel = serv_list.channel_list;
				}
			}
			else {
				channel = channel->next;
			}
		}
	}

	int i = 0;
	struct time_user *t = NULL;
	for(; i < 120;i++) {
		if(out.time_arr[i]) {
			t = out.time_arr[i];
			while(t->next && t->user != user) 
				t = t->next;
			if(t->next) break;
			else {
				if(t->user == user) break;
			}
		}				
	}
	if(i < 120) {
		if(out.time_arr[i] == t) {
			if(t->next) {
				out.time_arr[i] = t->next;
				free(t);
			}
			else {
				out.time_arr[i] = NULL;
				free(t);
			}
		}
		else {
			struct time_user* f = out.time_arr[i];
			while(f->next != t)
				f =f->next;
			if(!t->next) {
				f->next = NULL;
				free(t);
			}
			else {
				f = t->next;
				t->user = f->user;
				t->next = f->next;
				free(f);
			}
		}
	}
	Del_user_users(user->user_name);

	return 0;
}

int Handle_join(user_t* user,char* buf,struct sockaddr_in *cliaddr) {
	if(!user) {
		char *err = "User already exist!";
		int len = strlen(err);
		Error_Handler(err,user->cliaddr,len);
		return -1;
	}
	char* ptr = ((struct request_join*)buf)->req_channel;
	printf("*server:user %s join channel %s\n",user->user_name,ptr);
	printf("%s:%d %s:%d recv Request join %s %s\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cliaddr->sin_addr),ntohs(cliaddr->sin_port),user->user_name,ptr);
	
	channel_t* channel = Sch_channel_list(ptr);
	if(channel) {
		Add_user_channel(user->user_name,channel->channel_name);
	}
	else {
		s2s_join(cliaddr,ptr,0); //s2s
		channel = Add_channel_list(ptr);
		Add_user_channel(user->user_name,ptr);
	}
	
	channel_t* p = serv_list.channel_list;
	add_timer(user);
	
	return 0;
}

int Handle_Leave(user_t* user,char* buf,struct sockaddr_in *cliaddr) {
	if(!user) {
		char *err = "User do not exit";
		int len = strlen(err);
		Error_Handler(err,user->cliaddr,len);
		return -1;
	}
	char* ptr = ((struct request_leave*)buf)->req_channel;
	
	printf("*server:user %s leave channel %s\n",user->user_name,ptr);
	channel_t* channel = Sch_channel_list(ptr);
	if(channel) {
		Del_user_channel(user->user_name,channel->channel_name);
		//s2s_leave(cliaddr,channel->channel_name,0);
		add_timer(user);
		return 0;
	}
	else {
		char* err = "You don't in that channel";
		int len = strlen(err);
		Error_Handler(err,user->cliaddr,len);
		add_timer(user);
		return -1;
	}
}

int Handle_Say(user_t* user,char* buf,struct sockaddr_in *cliaddr) {
	if(!user) {
		char *err = "You are not login!";
		int len = strlen(err);
		Error_Handler(err,cliaddr,len);
		return -1;
	}

	char *ptr = ((struct request_say*)buf)->req_channel;
	printf("*server:user %s send a message\n",user->user_name);
	printf("%s:%d %s:%d recv Request say %s %s \"%s\"\n",inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),inet_ntoa(cliaddr->sin_addr),ntohs(cliaddr->sin_port),user->user_name,ptr,((struct request_say*)buf)->req_text);
	
	channel_t* channel = Sch_channel_list(ptr);
	
	if(user != (Sch_user_channel(channel,user->user_name))->user) {
		char* err = "You don't in that channel";
		int len = strlen(err);
		Error_Handler(err,user->cliaddr,len);
		return -1;
	}
	
	struct text_say text;
	text.txt_type = TXT_SAY;
	strcpy(text.txt_channel,ptr);
	strcpy(text.txt_username,user->user_name);
	strcpy(text.txt_text,((struct request_say*)buf)->req_text);
	
	
	Say_Handler((char*)&text,sizeof(text));
	s2s_say(cliaddr,ptr,0,(char*)&text); //s2s
	add_timer(user);
	return 0;
}

int Handle_List(user_t* user,struct sockaddr_in *cliaddr) {
	if(!user) {
		char *err = "User already exist!";
		int len = strlen(err);
		Error_Handler(err,user->cliaddr,len);
		return -1;
	}
	struct text_list *list = (struct text_list*)malloc(sizeof(struct text_list) + sizeof(struct channel_info)*serv_list.nchannels);
	printf("*server:user %s request channel list\n",user->user_name);
	list->txt_type = TXT_LIST;
	list->txt_nchannels = serv_list.nchannels;
	int i;
	struct channel_info *info = (struct channel_info*)(list->txt_channels);
	channel_t* channel = serv_list.channel_list;
	for(i = 0; i < list->txt_nchannels; i++) {
		strcpy(info[i].ch_channel,channel->channel_name);
		printf("current channel %s\n",info[i].ch_channel);
		channel = channel->next;
	}
	List_Handler(user,(char*)list,sizeof(struct text_list) + sizeof(struct channel_info)*serv_list.nchannels);
	add_timer(user);
	return 0;
}

int Handle_Who(user_t* user,char* buf,struct sockaddr_in *cliaddr) {
	if(!user) {
		char *err = "User already exist!";
		int len = strlen(err);
		Error_Handler(err,user->cliaddr,len);
		return -1;
	}
	
	char* ptr = ((struct request_who*)buf)->req_channel;
	channel_t *channel = Sch_channel_list(ptr);
	if(channel) {
		printf("*server:user %s request who in the channel:%s\n",user->user_name,channel->channel_name);
		int n = channel->nusers;
		struct text_who *who = (struct text_who*)malloc(sizeof(struct text_who) + sizeof(struct user_info)*n);
		who->txt_type = TXT_WHO;
		who->txt_nusernames = n;
		strcpy(who->txt_channel,ptr);
		int i;
		struct user_info* info = (struct user_info*)(who->txt_users);
		channel_user_t* uptr = channel->user_list;
		for(i = 0; i < n; i++) {
			strcpy(info[i].us_username,uptr->user->user_name);
			uptr = uptr->next;
		}

		int ret = Who_Handler(user,(char*)who,sizeof(struct text_who) + sizeof(struct user_info)*n);
		add_timer(user);
		return ret;
	}
	else {
		printf("*server:user %s request who in the channel which is not exist\n",user->user_name);
		char* err = "There is no channel.";
		int len = strlen(err);
		Error_Handler(err,user->cliaddr,len);
		add_timer(user);
		return 0;
	}
}

int Handle_Keep_Alive(user_t* user,struct sockaddr_in*cliaddr) {
	if(!user) {
		char* err = "You do not login";
		int len = strlen(err);
		Error_Handler(err,cliaddr,len);
		free(cliaddr);
		return -1;
	}
	else {
		printf("*server: user %s sends keep alive\n",user->user_name);
		/*channel_t *chnl_list = serv_list.channel_list;
		while(chnl_list){
			channel_user_t *user_list = chnl_list->user_list;
			while(user_list){
				if(strcmp(user_list->user->user_name,user->user_name) == 0)
					s2s_join(cliaddr,chnl_list->channel_name,0);
				user_list = user_list->next;
			}
			chnl_list = chnl_list->next;
		}*/
		
		//char* err = "i know";
		//int len = strlen(err);
		//Error_Handler(err,user->cliaddr,len);
		add_timer(user);
		return 0;
	}
}

user_t* Sch_Userbysock(struct sockaddr_in* cliaddr) {
	user_t *user = login_users.user_list;
	if(!user) 
		return NULL;
	else {
		while(user) {
			if(user->cliaddr->sin_port == cliaddr->sin_port && user->cliaddr->sin_addr.s_addr == cliaddr->sin_addr.s_addr) {			
				return user;
			}
			else user = user->next;
		}
		return NULL;
	}
}

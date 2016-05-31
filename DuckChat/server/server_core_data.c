/*
 * 	server_core_data.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "server_core_data.h"
user_t* Add_user(char* username, struct sockaddr_in *cliaddr) {
	user_t *user = (user_t*)malloc(sizeof(user_t));
	strcpy(user->user_name,username);
	user->cliaddr = cliaddr;
	user->next = NULL;
	return user;
}
void Add_user_users(char* username,struct sockaddr_in* cliaddr) {
	
	if(!(Sch_user_users(username))) {

		user_t *user = Add_user(username,cliaddr);
		
		user_t *uptr = login_users.user_list;


		if(uptr == NULL) {
			login_users.user_list = user;
			login_users.nusers = 1;

		}
		else {
			while(uptr->next)
				uptr = uptr->next;
			uptr->next = user;
			login_users.nusers++;

		}
	}

	return ;
}
void Add_user_channel(char* username, char* channelname) {
	channel_t *cptr = Sch_channel_list(channelname);
	if(cptr) {
		channel_user_t* uptr = Sch_user_channel(cptr,username);
		if(uptr) {
			return ;
		}
		else {
			user_t* tptr =Sch_user_users(username);
			if(tptr) {
				channel_user_t* ttptr = cptr->user_list;
				if(ttptr) {
					while(ttptr->next)
						ttptr = ttptr->next;
					channel_user_t* cuptr = (struct channel_user*)malloc(sizeof(struct channel_user));
					cuptr->user = tptr;
					cuptr->next = NULL;
					ttptr->next = cuptr;
					cptr->nusers++;
				}
				else {
					channel_user_t* cuptr = (struct channel_user*)malloc(sizeof(struct channel_user));
					cuptr->user = tptr;
					cuptr->next = NULL;
					cptr->user_list = cuptr;
					cptr->nusers = 1;
				}
			}
			return ;
		}
	}
	else {
		cptr = Add_channel_list(channelname);
		user_t* uptr = Sch_user_users(username);
		if(uptr) {
			channel_user_t* cuptr = (struct channel_user*)malloc(sizeof(struct channel_user));
			cuptr->user = uptr;
			cuptr->next = NULL;
			cptr->user_list = cuptr;
			cptr->nusers = 1;
			return ;
		}

	}
}
channel_t* Add_channel_list(char* channelname) {
	channel_t* cptr = (channel_t*)malloc(sizeof(channel_t));
	channel_t *tcptr = serv_list.channel_list;
	cptr->nusers = 0;
	cptr->user_list = NULL;
	cptr->next = NULL;
	strcpy(cptr->channel_name,channelname);
	if(tcptr == NULL) {
		serv_list.channel_list = cptr;
		serv_list.nchannels = 1;
		return cptr;
	}
	else {
		channel_t *p = Sch_channel_list(cptr->channel_name);
		if(!p) {
			while(tcptr->next) {
				tcptr = tcptr->next;
			}
			tcptr->next = cptr;
			serv_list.nchannels++;
			return cptr;
		}
	}
}
void Del_user(user_t* user ,channel_user_t* cuser) {
	if(user) {
		if(user->next == NULL) {
			free(user->cliaddr);
			free(user);
			return ;
		}
		else {			
			user_t* uptr = user->next;
			free(user->cliaddr);
			user->cliaddr = NULL;
			strcpy(user->user_name,uptr->user_name);
			user->cliaddr = uptr->cliaddr;
			user->next = uptr->next;
			channel_t* channel = serv_list.channel_list;
			if(channel) {
				while(channel) {
					channel_user_t *cuptr = Sch_user_channel(channel,uptr->user_name);
					if(cuptr) {
						if(uptr == cuptr->user) {
							cuptr->user = user;
						}
					}
					channel = channel->next;
				}
			}
			free(uptr);
			return ;
		}
	}
	else {
		if(cuser->next == NULL) {
			return ;
		}
		else {
			channel_user_t* uptr = cuser->next;
			cuser->user = uptr->user;
			cuser->next = uptr->next;
			free(uptr);
			return ;
		}
	}
}
void Del_user_users(char* username) {
	user_t* user = Sch_user_users(username);
	if(user) {
		if(!user->next) {
			user_t* uptr = login_users.user_list;
			if(user == uptr) {
				login_users.user_list = NULL;
				Del_user(user,0);
				login_users.nusers--;
			}
			else {
				while(uptr->next && uptr->next != user)
					uptr = uptr->next;
				uptr->next =NULL;
				Del_user(user,0);
				login_users.nusers--;
			}
		}
		else {
			if(user == login_users.user_list)
				Del_user(login_users.user_list,0);
			else Del_user(user,0);
			login_users.nusers--;
		}
	}
}
void Del_user_channel(char* username,char* channelname) {
	channel_t* channel = Sch_channel_list(channelname);
	
	if(channel) {
		channel_user_t* cuser = Sch_user_channel(channel,username);
		if(cuser) {
			if(cuser->next) {
				if(channel->user_list == cuser) {
					Del_user(0,channel->user_list);
				}
				else Del_user(0,cuser);
				channel->nusers--;
				if(!channel->nusers) {
					Del_channel_list(channel);
				}
			}
			else {
				channel_user_t* uptr = channel->user_list;
				if(uptr == cuser) {
					channel->user_list = NULL;
					Del_user(0,cuser);
					channel->nusers--;
				}
				else {
					while(uptr->next && uptr->next != cuser)
						uptr = uptr->next;
					uptr->next = NULL;
					Del_user(0,cuser);
					channel->nusers--;
				}
				if(!channel->nusers) {
						Del_channel_list(channel);
				}
			}
		}
	}
}
void Del_channel_list(channel_t* channel) {
	if(!channel->next) {
		channel_t* cptr = serv_list.channel_list;
		if(channel == cptr) {
			serv_list.channel_list = NULL;
			free(channel);
			serv_list.nchannels--;
			return ;
		}
		else {
			while(cptr->next && cptr->next != channel)
				cptr = cptr->next;
			cptr->next = NULL;
			free(channel);
			serv_list.nchannels--;
			return ;
		}
	}
	else {
		channel_t *cptr = channel->next;
		channel->nusers = cptr->nusers;
		strcpy(channel->channel_name,cptr->channel_name);
		channel->user_list = cptr->user_list;
		channel->next = cptr->next;
		free(cptr);
		serv_list.nchannels--;
		return ;
	}
}
user_t* Sch_user_users(char *username) {
	user_t* user = login_users.user_list;
	if(user) {
		while(user->next &&(strcmp(user->user_name,username) != 0)) {
			user = user->next;
		}
		if(!user->next) {
			if((strcmp(user->user_name,username) != 0))
				return NULL;
		}
		return user;
	}
	return NULL;
}
channel_user_t* Sch_user_channel(channel_t* channelptr,char*username) {	
	channel_user_t* cuser = channelptr->user_list;
	if(cuser) {
		while(cuser->next && (strcmp(cuser->user->user_name,username) != 0 )) {
			cuser = cuser->next;
		}
		if(!cuser->next) {
			if((strcmp(cuser->user->user_name,username) != 0))
				return NULL;
		}
		return cuser;
	}
	return NULL;
}
channel_t* Sch_channel_list(char* channelname) {
	channel_t* channel = serv_list.channel_list;
	if(channel) {
		while(channel->next && (strcmp(channel->channel_name,channelname) != 0)) {
			channel = channel->next;
		}
		if(!channel->next) {
			if((strcmp(channel->channel_name,channelname)) != 0)
				return NULL;
		}
		return channel;
	}
	return NULL;
}

void timeout_init() {
	out.last = -1;
	int i;
	for(i = 0; i < 120; i++){
		out.time_arr[i] = NULL;
	}
}
void add_timer(user_t *puser) {
	handle_timer(puser);
	struct time_user* tuptr = out.time_arr[0];
	if(tuptr == NULL) {
		struct time_user* tu = (struct time_user*)malloc(sizeof(struct time_user));
		tu->user = puser;
		tu->next = NULL;
		out.time_arr[0] = tu;
	}
	else {
		while(tuptr->next) 
			tuptr = tuptr->next;
		struct time_user* tu = (struct time_user*)malloc(sizeof(struct time_user));
		tu->user = puser;		
		tu->next = NULL;
		tuptr->next = tu;
	}
}
int sch_timer() {
	int i;
	for(i = 119;i > -1; i--) {
		if(out.time_arr[i]) {
			struct time_user* p = out.time_arr[i];
			while(p) {
				if(Sch_user_users(p->user->user_name))
					return 120 - i;
				p = p->next;
			}
		}	
	}
	return 120;
	
}
void handle_timer(user_t *puser) {
	time_t now = time(NULL);
	int  past = now - out.last;
	if(past == 0) return;
	int i; 
	for(i = 119; i + past >= 0; i--) {
		if(i + past > 119) {
			if(out.time_arr[i]) {
				struct time_user* tuptr = out.time_arr[i];
				send_timeout_message(puser,tuptr);
				out.time_arr[i] = NULL;
			}			
		}
		else {
			if(i >= 0) {
				struct time_user *t = out.time_arr[i];
				while(t) {
					if(t->user == puser)	{
						if(t->next) {
							struct time_user *p = t->next;
							t->user = p->user;
							t->next = p->next;
							free(p);
							break;
						}
						else {
							struct time_user *p = out.time_arr[i];
							if(p == t) out.time_arr[i] = NULL;
							else {
								while(p->next != t)
									p = p->next;
								p->next = NULL;
							}
							free(t);
							break;						
						}
					}
						t = t->next;				
				}
				out.time_arr[i+past] = out.time_arr[i];
				out.time_arr[i] = NULL;
			}
			else out.time_arr[i+past] = NULL;
		}
	}
	out.last = now;
}
void send_timeout_message(user_t *puser,struct time_user* tuptr) {
	if(tuptr->next)
		send_timeout_message(puser,tuptr->next);
	tuptr->next = NULL;
	if(tuptr->user != puser) {
		char* err = "Time Out";
		int len  = strlen(err);
		if(Sch_user_users(tuptr->user->user_name)) {
			Error_Handler(err,tuptr->user->cliaddr,len);
			printf("*server:forcibly removing user %s\n",tuptr->user->user_name);
			Handle_Logout(tuptr->user);
		}
			tuptr = NULL;
		return ;
	}
	else {
		free(tuptr);
		return ;
	}
}


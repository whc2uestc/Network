/*
 * 	server_core_data.h
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "s2s_handler.h"
#ifndef _SERVER_CORE_DATA_H_
#define _SERVER_CORE_DATA_H_

typedef struct list 	list_t;
typedef struct channel 	channel_t;
typedef struct users 	users_t;
typedef struct user		user_t;
typedef struct channel_user	channel_user_t;  

struct list {
	int nchannels;
	struct channel *channel_list;
};

struct channel {
	int nusers;
	char channel_name[32];
	struct channel_user *user_list;
	struct channel *next;
};

struct users {
	int nusers;
	struct user *user_list;
};

struct user {
	char user_name[32];
	struct sockaddr_in *cliaddr;
	struct user* next;
};

struct channel_user {
	user_t* user;
	struct channel_user *next;
};
struct time_user {
	user_t* user;
	struct time_user*next;
};
struct timeout {
	time_t last;
	struct time_user* time_arr[120];
};

struct timeout out;

list_t serv_list;
users_t login_users;

user_t* Add_user(char* username, struct sockaddr_in *cliaddr);
void Add_user_users(char* username,struct sockaddr_in* cliaddr);
void Add_user_channel(char* username, char* channelname);
channel_t* Add_channel_list(char* channelname);
void Del_user(user_t* user, channel_user_t* cuser);
void Del_user_users(char* username);
void Del_user_channel(char* username,char* channelname);
void Del_channel_list(channel_t* channel);
user_t* Sch_user_users(char *username);
channel_user_t* Sch_user_channel(channel_t* channelptr,char*username);
channel_t* Sch_channel_list(char* channelname);

void timeout_init();
void add_timer(user_t *puser);
int sch_timer();
void handle_timer(user_t *puser);
void send_timeout_message(user_t *puser,struct time_user* tuptr);

#endif


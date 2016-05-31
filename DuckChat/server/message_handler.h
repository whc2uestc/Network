/*
 * 	message_handler.h
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "server_core_data.h"
#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

int Read_Handler(char* buf,struct sockaddr_in* cliaddr);
int Say_Handler(char* buf,int len);
int List_Handler(user_t* user, char* buf, int len);
int Who_Handler(user_t* user, char* buf, int len);
int Error_Handler(char* err,struct sockaddr_in *cliaddr,int len);
int Handle_Login(char* buf, struct sockaddr_in *cliaddr);
int Handle_Logout(user_t* user);
int Handle_join(user_t* user,char* buf,struct sockaddr_in *cliaddr);
int Handle_Leave(user_t* user,char* buf,struct sockaddr_in *cliaddr);
int Handle_Say(user_t* user,char* buf,struct sockaddr_in *cliaddr);
int Handle_List(user_t* user,struct sockaddr_in *cliaddr);
int Handle_Who(user_t* user,char* buf,struct sockaddr_in *cliaddr);
int Handle_Keep_Alive();
void renew_timer(int sig);
user_t* Sch_Userbysock(struct sockaddr_in* cliaddr);

#endif

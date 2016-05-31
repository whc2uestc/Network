/*
 * 	command_handler.h
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "duckchat.h"

#ifndef _COMMAND_HANDLER_H_
#define _COMMAND_HANDLER_H_
/*when client recive a command from keyboard, after parse the command ,client calls this function to send the message to server*/
void Login_Handler();
void Logout_Handler();
void Join_Handler(char* command);
void Leave_Handler(char* command);
void Say_Handler(char* command);
void List_Handler();
void Who_Handler(char* command);
void Keep_Alive_Handler();
int  Switch_Handler(char* command);

#endif

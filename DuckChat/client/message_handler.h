/*
 * 	message_handler.h
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "duckchat.h"

#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

int i;

void Read_Handler(char* message);
void Write_Hnadler(char* message);
void Handler_Say(char* message);
void Handler_List(char* message);
void Handler_Who(char* message);
void Handler_Error(char* message);

#endif

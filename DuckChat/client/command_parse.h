/*
 * 	command_parse.h
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "duckchat.h"

#ifndef _COMMAND_PARSE_H_
#define _COMMAND_PARSE_H_

#include "command_handler.h"
//These method use for Parse the command which client recive from keyboard
int Com_Parse(char* command);
void Com_Handler(int request, char* command);
int Parse(char* command);

#endif
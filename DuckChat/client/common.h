/*
 * 	common.h
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include "duckchat.h"
#include "command_parse.h"
#include "channel.h"
#include <poll.h>
//The core cycle for client to handle the both messages sended by server and recived from user

int client_init(int argc, char** argv);
int client_core_cycle();
int client_cleanup();

void Read_Handler();
void Write_Handler();

void clean(channel_t* cptr);

#endif

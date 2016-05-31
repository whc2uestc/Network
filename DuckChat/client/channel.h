/*
 * 	channel.h
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct channel channel_t;
typedef struct header header_t;
//using list to store local channel info
struct channel {
	char name[32];
	struct header* head;
	struct channel *next;
};

//the header of channel list 
struct header {
	unsigned int Num_Channel;
	struct channel *next;
};

//the actions which the channel list support,including add a channel,find a particular channel and delete a channel

channel_t* Add_Channel(header_t* head, char* channel_name);

int Del_Channel(header_t* head, char* channel_name);

channel_t* Sch_Channel(header_t* head, char* channel_name);



#endif

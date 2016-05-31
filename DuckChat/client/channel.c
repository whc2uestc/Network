/*
 * 	channel.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "channel.h"
/*add a channel*/
channel_t* Add_Channel(header_t* head, char* channel_name) {
	channel_t* cur_channel = (channel_t*)malloc(sizeof(channel_t));
	
	strcpy(cur_channel->name,channel_name);
	cur_channel->head = NULL;
	cur_channel->next = NULL;
	if(head->next == NULL) {
		head->next = cur_channel;
		head->Num_Channel = 1;
		return cur_channel;
	}
	else {
		int flag = 1;
		channel_t* channel = head->next;
		while(channel->next) {
			if(!(strcmp(channel->name,cur_channel->name))) {
				flag = 0;
				break;		
			}
			else 
				channel = channel->next;
		}
		if(!(strcmp(channel->name,cur_channel->name))) {
			flag = 0;
		}
		if(flag) {
			channel->next = cur_channel;
			head->Num_Channel++;
			return cur_channel;
		}
		else return channel;
	
	}

}
/*delete a channel,because of using list, there is a O(1) method to delete a pointer in channel list*/
int Del_Channel(header_t* head, char* name) {
	if(!head->next)
		return -1;
	channel_t* channel = Sch_Channel(head, name);
	if(!channel)
		return -1;
	
	else if(channel->next == NULL) {
		channel_t* temp = head->next;
		
		if(temp == channel) {
			free(channel);
			head->next = NULL;
			return 0;
		}
		
		while(temp->next != channel)
			temp = temp->next;
		
		temp->next = NULL;
		free(channel);

		return 0;
	}
	else {
		channel_t* temp = channel->next;
		
		strcpy(channel->name,temp->name);
		channel->next = temp->next;
		
		free(temp);
		
		return 0;
	}
}
/*find a particular channel using channel name*/
channel_t* Sch_Channel(header_t* head, char* name) {
	if(!head->next)
		return NULL;
	channel_t* channel = head->next;
	while((strcmp(channel->name, name)) != 0 && channel->next)
		channel = channel->next;
	if(!channel->next && (strcmp(channel->name, name)) != 0)
		return NULL;
	else
		return channel;
}

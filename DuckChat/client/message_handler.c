/*
 * 	message_handler.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "message_handler.h"
#include "raw.h"

void Handler_Say(char* message) {
	struct text_say *txt = (struct text_say*)message;
	
	printf("[%s][%s]:[%s]\n",txt->txt_channel,txt->txt_username,txt->txt_text);

	cooked_mode();

	return ;
}

void Handler_List(char* message) {
	struct text_list *txt = (struct text_list*)message;
	struct channel_info *info = (struct channel_info*)(txt->txt_channels);
	int n = txt->txt_nchannels;
	printf("Existing channels:\n");
	for(i = 0; i < n; i++)
		printf("\t%s\n",info[i].ch_channel);
	cooked_mode();


}

void Handler_Who(char* message) {
	struct text_who* txt = (struct text_who*)message;
	struct user_info* info = (struct user_info*)(txt->txt_users);
	printf("Users on channel %s:\n",txt->txt_channel);
	int n = txt->txt_nusernames;
	for(i = 0; i < n; i++) {
		printf("\t%s\n",info[i].us_username);
	}
	cooked_mode();


}

void Handler_Error(char* message) {
	struct text_error *txt = (struct text_error*)message;
	if((strcmp(txt->txt_error,"i know")) == 0) {
		timer.flag = -1;
		timer.count = 0;
	}
	else {
		//printf("Error message:\n");
		//printf("%s\n",txt->txt_error);
	}
	cooked_mode();


}

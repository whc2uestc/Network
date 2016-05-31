/*
 * 	command_parse.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "command_parse.h"
//the interface for parse command
int Com_Parse(char* command) {
	int request  = Parse(command);
	if(request != -1) {
		Com_Handler(request, command);
		return 0;
	}
	else return -1;
}
//annalize command
int Parse(char* command) {
	char* m_command = command;
	if(*m_command != '/')
		return REQ_SAY;
	else {
		m_command++;
		if(*m_command == 'L') 
			return REQ_LOGIN;

		else if(*m_command == 'e')
			return REQ_LOGOUT;

		else if(*m_command == 'j')
			return REQ_JOIN;

		else if(*m_command == 'w')
			return REQ_WHO;

		else if(*m_command == 's')
			return REQ_SWITCH;

		else if(*m_command == 'l') {
			m_command++;
			if(*m_command == 'e')
				return REQ_LEAVE;
			else 
				return REQ_LIST;
		}
		else if(*m_command == 'k')
			return REQ_KEEP_ALIVE;
		else
			return -1;
	}
}
//after parse command ,call those method to handle the command and send message to server
void Com_Handler(int requset, char* command) {
	switch(requset) {
		case REQ_LOGIN:
			Login_Handler();
			break;
		case REQ_LOGOUT:
			Logout_Handler();
			break;
		case REQ_JOIN:
			Join_Handler(command + 6);
			break;
		case REQ_LEAVE:
			Leave_Handler(command + 7);
			break;
		case REQ_SAY:
			Say_Handler(command);
			break;
		case REQ_LIST:
			List_Handler();
			break;
		case REQ_WHO:
			Who_Handler(command + 5);
			break;
		case REQ_KEEP_ALIVE:
			Keep_Alive_Handler();
			break;
		case REQ_SWITCH:
			Switch_Handler(command + 8);
			break;
		default: return;
	}
}

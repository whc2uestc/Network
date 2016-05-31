/*
 * 	s2s_handler.h
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#ifndef S2S_HANDLER_H_
#define S2S_HANDLER_H_
#include "duckchat.h"

void insert_into_list(int port,char *ptr);
void delete_from_list(unsigned short port,in_addr_t addr);
tree_node* insert_into_tree(char *channelname,server_node* plist);
int delete_from_tree(struct sockaddr_in *cliaddr,char* channelname);
#endif
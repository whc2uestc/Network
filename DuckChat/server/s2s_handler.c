/*
 * 	s2s_handler.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "s2s_handler.h"
void insert_into_list(int port,char *ptr){
	if(NULL == adj_server){
		adj_server = (server_node*)malloc(sizeof(server_node));
		bzero(&adj_server->addr,sizeof(adj_server->addr));
		adj_server->addr.sin_family = AF_INET;
		adj_server->addr.sin_port = htons(port);
		char **pptr;
		struct hostent *hptr;
		char str[1024];	
		hptr = gethostbyname(ptr);
		pptr = hptr->h_addr_list;
		if(*ptr >= '0' && *ptr <= '9') {
			inet_pton(AF_INET,ptr,&adj_server->addr.sin_addr);
		}
		else {
			inet_pton(AF_INET,inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)), &adj_server->addr.sin_addr);
		}
		adj_server->next = NULL;
		return ;
	}
	server_node *cur = adj_server;
	server_node *pre = NULL;
	while(cur){
		if(cur->addr.sin_port==htons(port) && cur->addr.sin_addr.s_addr==inet_addr(ptr))
			return ;
		pre = cur;
		cur = cur->next;
	}

	cur = (server_node*)malloc(sizeof(server_node));
	bzero(&cur->addr,sizeof(cur->addr));
	cur->addr.sin_family = AF_INET;
	cur->addr.sin_port = htons(port);

	char **pptr;
	struct hostent *hptr;
	char str[1024];	
	hptr = gethostbyname(ptr);
	pptr = hptr->h_addr_list;

	if(*ptr >= '0' && *ptr <= '9') {
		inet_pton(AF_INET,ptr,&cur->addr.sin_addr);
	}
	else {
		inet_pton(AF_INET,inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)), &cur->addr.sin_addr);
	}
	cur->next = NULL;
	pre->next = cur;

}

void delete_from_list(unsigned short port,in_addr_t addr){
	if(NULL == adj_server)
		return ;
	server_node *cur = adj_server;
	if(adj_server->addr.sin_port==port && adj_server->addr.sin_addr.s_addr==addr){
		adj_server = adj_server->next;
		free(cur);
		return ;
	}
	cur = cur->next;
	server_node *pre = adj_server;
	while(cur){
		if(cur->addr.sin_port==port && cur->addr.sin_addr.s_addr==addr){
			pre->next = cur->next;
			free(cur);
			return ;
		}
		pre = cur;
		cur = cur->next;
	}
}

server_node* copy_list(server_node* phead){
	if(NULL == phead)
		return NULL;
	server_node* pcopy = NULL;
	server_node* res = NULL;
	while(phead){
		server_node* cur = (server_node*)malloc(sizeof(server_node));
		memcpy(&(cur->addr),&(phead->addr),sizeof(struct sockaddr_in));
		cur->next = NULL;
		if(res == NULL){
			res = cur;
			pcopy = cur;
		}else{
			pcopy->next = cur;
			pcopy = pcopy->next;
		}
		phead = phead->next;
	}
	return res;
}

tree_node* insert_into_tree(char *channelname,server_node* plist){
	tree_node* phead = (tree_node*)malloc(sizeof(tree_node));
	strcpy(phead->channelname,channelname);
	phead->server_list = copy_list(plist);
	phead->next = NULL;
	if(server_tree == NULL){
		server_tree = phead;
	}else{
		tree_node* cur = server_tree;
		while(cur->next){
			cur = cur->next;
		}
		cur->next = phead;
	}
	return server_tree;
}

int delete_from_tree(struct sockaddr_in *cliaddr,char* channelname){
	tree_node* cur = server_tree;
	while(cur && strcmp(cur->channelname,channelname)!=0)
		cur = cur->next;
	server_node *head = cur->server_list;
	if(NULL == head)
		return 1;
	if(head->addr.sin_port==cliaddr->sin_port && head->addr.sin_addr.s_addr==cliaddr->sin_addr.s_addr){
		cur->server_list = cur->server_list->next;
		free(head);
		return 0;
	}
	head = head->next;
	server_node *pre = cur->server_list;
	while(head){
		if(head->addr.sin_port==cliaddr->sin_port && head->addr.sin_addr.s_addr==cliaddr->sin_addr.s_addr){
			pre->next = head->next;
			free(head);
			return 0;
		}
		pre = head;
		head = head->next;
	}
	return 1;
}





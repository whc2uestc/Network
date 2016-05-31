/*
 * 	client.c
 *	author: Hancheng Wang
 *    date: 2016.5.26
 */
#include "common.h"

int main(int argc, char** argv) {
	
	if(argc < 4) {
		printf("Usage:%s hostname port username\n",argv[0]);
		exit(0);
	}
	
	client_init(argc,argv);

	client_core_cycle();
	
	return 0;
}

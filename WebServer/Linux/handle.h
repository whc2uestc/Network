#ifndef HANDLE_H_
#define HANDLE_H_
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define MAXLEN 1024


void handle(int acceptfd,char *buf,int len);
void errorMsg(int acceptfd,char *cause,char *errcode,char *msg);
void headMsg(int acceptfd,char *code,char *msg,int lenOfBody);

#endif

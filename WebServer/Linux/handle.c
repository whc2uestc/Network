#include "handle.h"

void handle(int acceptfd,char *buf,int len){
    char method[MAXLEN] = {0};
    char uri[MAXLEN] = {0};
    char version[MAXLEN] = {0};
    sscanf(buf,"%s %s %s",method,uri,version);
    /* if the request is not the GET, return 501.*/
    if(NULL == strstr(method,"GET")){
        errorMsg(acceptfd,method,"501","Not Implemented");
        return ;
    }

    /* if the uri is '/root', return the dynamic web.
     * else return the static html. */
    if(strstr(uri,"/root")){
        errorMsg(acceptfd,method,"501","Not Implemented");
        return ;
    }
    else{
        FILE *pFile = fopen("index.html","r+");
        if(NULL == pFile){
            exit(1);
        }
        char file[1024] = {0};
        while(NULL !=fgets(file,1024,pFile)){
            headMsg(acceptfd,"200","OK",strlen(file));
            if(write(acceptfd,file,strlen(file)) < 0)
                exit(1);
            memset(file,0,1024);
        }
    }
    return ;
}

void errorMsg(int acceptfd,char *cause,char *errcode,char *msg){
    char body[MAXLEN] = {0};
    sprintf(body,"<html><title>Error</title>");
    sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
    sprintf(body,"%s%s: %s\r\n",body,errcode,msg);
    sprintf(body,"%s<p>%s: %s\r\n",body,msg,cause);
    sprintf(body,"%s<hr><em>The web server by whc</em>\r\n",body);
    headMsg(acceptfd,errcode,msg,strlen(body));
    write(acceptfd,body,strlen(body));
}
    
void headMsg(int acceptfd,char *code,char *msg,int lenOfBody){
    char head[MAXLEN] = {0};
    sprintf(head,"HTTP/1.0 %s %s\r\n",code,msg);
    write(acceptfd,head,strlen(head));
    sprintf(head,"Content-type:text/html\r\n");
    write(acceptfd,head,strlen(head));
    sprintf(head,"Content-length: %d\r\n\r\n",lenOfBody);
    write(acceptfd,head,strlen(head));
}

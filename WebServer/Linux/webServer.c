#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
int main(){
    
    int sockfd;
    struct sockaddr_in servAddr;

    if((sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0){
        exit(0);    
    }
    printf("sock ok!\n");
    
    bzero(&servAddr,sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(65500);
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(sockfd,(struct sockaddr*)&servAddr,sizeof(struct sockaddr)) < 0){
        exit(0);
    }
    printf("bind ok!\n");
    
    while(1){
        if(listen(sockfd,10) < 0){

        }
        printf("listen ok!\n");
        
        int acceptfd;
        if((acceptfd=accept(sockfd,NULL,NULL)) < 0){
            exit(0);
        }
        printf("accept ok!\n");
        if(fork() == 0){
            char buff[1024] = {0};
            if(read(acceptfd,buff,1024) > 0){
                printf("%s",buff);
            }
            handle(acceptfd,buff,strlen(buff));
            close(acceptfd);
            exit(0);
        }
        close(acceptfd);
    } 
    close(sockfd);
    return 0;
}

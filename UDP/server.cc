#include<sys/socket.h>
#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <arpa/inet.h> 
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
//#include <signal.h>
#include <errno.h>
#include<sys/wait.h>

#define PORT 2772

void do_echo(int sock_fd, struct sockaddr_in* s_addr, socklen_t s_len);

int main(int argc, const char* argv[]) {
    int s_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    if (inet_aton("127.0.0.1", &server_addr.sin_addr)==0){
        printf("inet_aton error\n");
        exit(1);
    }
    if (bind(s_fd, (sockaddr*)&server_addr, sizeof(server_addr))==-1) {
        printf("bind error\n");
        exit(1);
    }
    struct sockaddr_in client_addr;
    socklen_t c_len = sizeof(client_addr);
    do_echo(s_fd, &client_addr, c_len);
    
    return 0;
}


void do_echo(int sock_fd, struct sockaddr_in* c_addr, socklen_t len) {
    socklen_t s_len=len;
    char buffer[1024];
    int n;
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        
        n=recvfrom(sock_fd, buffer, sizeof(buffer), 0, (sockaddr*)c_addr, &s_len);

        printf("recv from %s:%d\n", inet_ntoa(c_addr->sin_addr), ntohs(c_addr->sin_port));
        printf("recv: %s\n", buffer);

        sendto(sock_fd, buffer, sizeof(buffer), 0, (sockaddr*)c_addr, s_len);
    }
}

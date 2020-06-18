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
    const int on= 1;
    setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    int recv_size;
    socklen_t recv_len = sizeof(recv_size);
    getsockopt(s_fd, SOL_SOCKET, SO_RCVBUF, &recv_size, &recv_len);
    printf("before set: %d\n", recv_size);
    recv_size=10+8*2; // 不知为啥 一定要2倍的udp头部大小
    int rl_recv_size;
    setsockopt(s_fd, SOL_SOCKET, SO_RCVBUF, &recv_size, sizeof(recv_size));
    getsockopt(s_fd, SOL_SOCKET, SO_RCVBUF, &rl_recv_size, &recv_len);
    printf("after set: %d\n", recv_size);
    if (bind(s_fd, (sockaddr*)&server_addr, sizeof(server_addr))==-1) {
        printf("bind error\n");
        exit(1);
    }
    // 不需要listen
    struct sockaddr_in client_addr;
    socklen_t c_len = sizeof(client_addr);
    do_echo(s_fd, &client_addr, c_len);
    
    return 0;
}


void do_echo(int sock_fd, struct sockaddr_in* c_addr, socklen_t len) {
    socklen_t s_len=len;
    char buffer[10];
    int n;
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        
        n=recvfrom(sock_fd, buffer, sizeof(buffer), 0, (sockaddr*)c_addr, &s_len);

        printf("recv %d from %s:%d\n", n, inet_ntoa(c_addr->sin_addr), ntohs(c_addr->sin_port));
        printf("recv: %s\n", buffer);

        sendto(sock_fd, buffer, sizeof(buffer), 0, (sockaddr*)c_addr, s_len); // 不同于TCP，不管有没有数据都会发送 sizeof(buffer)个数据
    }
}

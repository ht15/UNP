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

int main(int argc, const char* argv[]) {
    int c_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    socklen_t s_len = sizeof(server_addr);

    connect(c_fd, (sockaddr*)&server_addr, s_len);
    char buffer[1024];
    int n;
    while(fgets(buffer, sizeof(buffer), stdin) != NULL) {
        //sendto(c_fd, buffer, sizeof(buffer), 0, (sockaddr*)&server_addr, s_len);
        //sendto(c_fd, buffer, sizeof(buffer), 0, NULL, 0);
        write(c_fd, buffer, sizeof(buffer));
        memset(buffer, 0, sizeof(buffer));
        //n=recvfrom(c_fd, buffer, sizeof(buffer), 0, NULL, NULL);
        n=read(c_fd, buffer, sizeof(buffer));
        buffer[n]=0;
        fputs(buffer, stdout);
        memset(buffer, 0, sizeof(buffer));
    }
    return 0;
}

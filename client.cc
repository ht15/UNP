#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
 
#define MYPORT 2727
#define BUFFER_SIZE 1024
 
int main()
{
    int c_fd = socket(AF_INET,SOCK_STREAM, 0);
    
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MYPORT); //服务器端口
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    myaddr.sin_port = htons(37773);
    // 自己绑定指定地址，不由内核在connect时指定。bind两次以上同一个端口，除了第一次能bind到指定端口，后面都由内核分配一个临时端口.
    bind(c_fd, (sockaddr *)&myaddr, sizeof(myaddr));

    if (connect(c_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    struct sockaddr_in my_addr;
    socklen_t len=sizeof(my_addr); // 同样需要指定长度
 
    getsockname(c_fd,(struct sockaddr *)&my_addr,&len);
    printf("ip=%s\n",inet_ntoa(my_addr.sin_addr));
    printf("port=%d\n",ntohs(my_addr.sin_port));

    
    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {/*每次读取一行，读取的数据保存在buf指向的字符数组中，成功，则返回第一个参数buf；*/
        send(c_fd, sendbuf, strlen(sendbuf),0); ///发送
        if(strcmp(sendbuf,"exit\n")==0)
            break;
        recv(c_fd, recvbuf, sizeof(recvbuf),0); ///接收
        fputs(recvbuf, stdout);
    
        memset(sendbuf, 0, sizeof(sendbuf));//接受或者发送完毕后把数组中的数据全部清空（置0）
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(c_fd);
    return 0;
}

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
#include "mysignal.hpp"
#include <sys/un.h>
#include <fcntl.h>

#define PORT 2727
#define QUEUE_NUM 10

#define UNIX_PATH "/tmp/test_unix.sock"

void do_echo(int);


void sig_chld(int signo){
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0){
	printf("child %d terminated\n", pid);
    	printf("stat is: %d\n", stat);
    }
}


int main(int argc, const char* argv[]) {
    printf("AF_INET: %d\n", AF_INET);
    printf("SOCK_STREAM: %d\n", SOCK_STREAM);
    /***
     * 协议族（协议域）包括: 
     *  AF_INET(ipv4)
     *  AF_INET6(ipb6)
     *  AF_LOCAL(unix域协议）
     * socket类型包括：
     *  SOCK_STREAM(字节流套接字）
     *  SOCK_DGRAM(数据报套接字）
     *  SOCK_RAW(原始套接字 TODO)
     * 协议类型
     *  IPPROTO_TCP(tcp传输协议）
     *  IPPROTO_UDP(UDP传输协议）
     *  0(由协议族和socket类型确定的默认协议类型）
     ***/
    //int s_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 使用IPPROTO_UDP时 创建socket会报错
    int s_fd = socket(AF_LOCAL, SOCK_STREAM, 0); // unix域套接字, 协议项需设为零
    if(s_fd == -1){
        printf("create socket error\n");
        exit(1);
    }
    /***
     * IPv4套接字地址结构
     *struct sockaddr_in {
     *  uint8_t sin_len;                //长度字段，一般无需设置
     *  sa_family_t sin_family;         //协议族，如AF_INET
     *  in_port_t sin_port;             //16位的二进制网络字节序端口号
     *  struct in_addr sin_addr;        //32位的二进制网络字节序ip地址, struct in_addr{in_addr_t s_addr;}只有一个成员变量,由于历史原因才这样写
     *  char sin_zero[8];               //一般不使用
     *}
     *
     * 通用套接字地址结构
     *struct sockddr {
     *  uint8_t sa_len;
     *  sa_family_t sa_family;
     *  char sa_data[14];
     *  }
     *所由API几乎都将通用套接字地址结构类型（或指针）作为参数，然后在函数通过协议族来转换成真实类型，所有在调用的时候要进行强制类型转换。（这一切都是为了接口重用）
     ***/
    //struct sockaddr_in server_addr;
    //memset(&server_addr, 0, sizeof(server_addr)); // 一般先把一个套接字地址结构初始化为0
    //server_addr.sin_port = htons(PORT);
    //server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = inet_addr("127.0.0.1")已经废弃
    //if(inet_aton("127.0.0.1", &server_addr.sin_addr) == 0) {
        //printf("inet_aton error\n");
        //exit(1);
    //}
    struct sockaddr_un server_addr; // unix域套接字地址
    memset(&server_addr, 0, sizeof(server_addr)); // 一般先把一个套接字地址结构初始化为0
    server_addr.sun_family = AF_LOCAL;
    unlink(UNIX_PATH);
    strcpy(server_addr.sun_path, UNIX_PATH);
    /***
     * bind可以指定一个ip和port，或者指定其中一个，或者都不指定。客户端connect前一般不bind，服务端listen前一般先bind
     * 如果不bind一个指定port，connect或listen时内核要为相应套接字选择一个临时端口
     ***/
    const int on=1;
    setsockopt(s_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    if(bind(s_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1){ 
        printf("bind error\n");
        exit(1);
    }
    /***
     * listen做了两件事：
     * 1. socket从主动变成被动，状态从CLOSED变成LISTEN，等待客户端的连接
     * 2. 维护两个队列：a. 未完成连接的队列(收到了SYN分节，但为收到ACK, 处于SYN_RCVD, 保持RTT时间) b.完成连接的队列(完成了3次握手，还未被accept)
     *    两个队列之和不大于第二个参数
     ***/
    if(listen(s_fd, QUEUE_NUM) == -1){
        printf("listen error\n");
        exit(1);
    }
    mySignal(SIGCHLD, sig_chld);
    int child_pid;
    while(1){
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr); // 不指定大小，返回accept获取到的地址会有问题。
        /***
        * accept 从已完成连接的队列的头部返回一个连接，并由内核关联一个全新的套接字描述符
        ***/
        int conn = accept(s_fd, (sockaddr*)&client_addr, &client_addr_len);
        if(conn < 0){
            if(errno == EINTR) {
                printf("accept error be interrupted\n");
                continue;
            } else {
                printf("accept error\n");
                exit(1);
            }
        }
        if((child_pid=fork())==0){ // 子进程
            int dest_size;
            socklen_t  buf_send_len;
            getsockopt(conn, SOL_SOCKET, SO_SNDBUF, &dest_size, &buf_send_len);
            printf("send buff length before set: %d\n", dest_size);
            dest_size = 50;
            setsockopt(conn, SOL_SOCKET, SO_SNDBUF, &dest_size, sizeof(dest_size));
            getsockopt(conn, SOL_SOCKET, SO_SNDBUF, &dest_size, &buf_send_len);
            printf("send buff length after set: %d\n", dest_size);

            int flags;
            flags = fcntl(conn, F_GETFL, 0);
            fcntl(conn, F_SETFL, flags|O_NONBLOCK);
            close(s_fd);//关闭监听套接字描述符
            struct sockaddr_in sub_server_addr;
            socklen_t sub_server_addr_len = sizeof(sub_server_addr);
            printf("accept from client:(%s,%d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            getsockname(conn, (sockaddr*)&sub_server_addr, &sub_server_addr_len);
            printf("conn addr is:(%s,%d)\n", inet_ntoa(sub_server_addr.sin_addr), ntohs(sub_server_addr.sin_port));
            do_echo(conn);
            exit(0);
        }
        close(conn);
    }
    return 0;
}

class Test {
public:
    int age;
};

void do_echo(int conn) {
    if(conn == -1){
        printf("accept error\n");
        exit(1);
    }
    char buffer[1024];
    int send_size;
    while(1)
    {
        memset(buffer, 0 ,sizeof(buffer));
        int dest_size;
        int buf_send_len = sizeof(dest_size);
        getsockopt(conn, SOL_SOCKET, SO_SNDBUF, &dest_size, (socklen_t*)&buf_send_len);
        int len = recv(conn, buffer, dest_size, 0);//从TCP连接的另一端接收数据。
        if(strcmp(buffer, "exit\n") == 0)
        {
            break;
        } else if(len == 0) { // 对端close，并非是会产生read事件!!! 而是此时tcp处于半关闭状态， 己方收到了FIN，对端关闭了写，所以己方认为没有数据可读了，所以立马返回0
            printf("get close info, will exit\n");
            break;
        }
        int old_len = len;
        if(len>0){
            printf("%s\n", buffer);//如果有收到数据则输出数据
            printf("get info len from client: %d\n", len);
            Test *t = (Test*)buffer;
            printf("get Test at age: %d\n", t->age);
        }
        send_size=0;
        while(len > 0){
            if(len > dest_size){
                len = dest_size;
            }
            int send_num = send(conn, buffer+ send_size, len , 0);//向TCP连接的另一端发送数据。
            if(send_num==-1){
                if(errno == EWOULDBLOCK){
                    printf("buffer lack\n");
                    continue;
                }
            }
            printf("send %d to client\n", send_num);
            send_size += send_num;
            len = old_len-send_num;
            printf("after send len:%d, send_size:%d\n", len, send_size);
            old_len = len;
        }
    }
    close(conn);//因为accpet函数连接成功后还会生成一个新的套接字描述符，结束后也需要关闭
}

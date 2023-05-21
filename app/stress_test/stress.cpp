/***************************************************************************************************
* FTP服务器压力测试程序
***************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <vector>
#include <string>

#include "../../code/progress_bar/prog_bar.h"

#define SEDN_RECV_BUF_SIZE 65535
#define EVENTS_NUMBER 10000
#define FILE_SIZE_LENGTH 20

int recv_file_size[20000] = {0};
int file_size[20000] = {0};

std::vector<std::string > file_name = {"testfile.txt", "testfile2.txt", "testfile3.txt"};
// FILE *file_fp = fopen(file_name[0].c_str(), "w+");

//  设置文件描述符为非阻塞
int set_non_block(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//  将fd文件描述添加到epoll监听事件中
void add_fd(int epoll_fd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLOUT | EPOLLET | EPOLLRDHUP;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    set_non_block(fd);
}

//  向服务器发送数据(发送需要下载的文件名)
bool write_nbytes(int sock_fd, const char* buffer, int len)
{
    int bytes_write = 0;
    printf("Socket %d for download file name is: %s\n", sock_fd, buffer);
    while (1)
    {
        bytes_write = send(sock_fd, buffer, len, 0);
        if (bytes_write == -1 || bytes_write == 0)
        {
            return false;
        }

        len -= bytes_write;
        buffer = buffer + bytes_write;
        if (len <= 0)
        {
            return true;
        }
    }
    return true;
}

// //  从服务器读取数据 (即接收服务器传输的文件)  
// bool read_file(int sock_fd, char *buffer, int len)
// {
//     char file_length[FILE_SIZE_LENGTH];
//     memset(buffer, '\0', len);

//     //  判断服务器是否有该文件，接收到"-1"为未找到文件，否则接收到为文件的大小
//     bzero(file_length, FILE_SIZE_LENGTH);
//     int recv_size = recv(sock_fd, file_length, FILE_SIZE_LENGTH, 0);

//     int file_size = 0;              //  获取文件大小
//     file_size = atoi(file_length);  
//     recv_size = 0;
//     printf("file_size: %d\n", file_size);
    
//     while (1)
//     {
//         //  接收文件数据
//         int tmp_recv_size = recv(sock_fd, buffer, len, 0);
//         printf("tmp_recv_size: %d\n", tmp_recv_size);
//         // if (tmp_recv_size <= 0)
//         //     break;

//         if(tmp_recv_size > 0)
//             recv_size += tmp_recv_size;
        
//         //  文件传输完成
//         if (recv_size == file_size)
//         {
//             printf("read %d bytes from socket %d !!!\n", recv_size, sock_fd);
//             return true;
//         }
//     }
//     //close(sock_fd);
//     return false;
// }

//  从服务器读取数据 (即接收服务器传输的文件)  
bool read_file(int sock_fd, char * buffer, int len)
{
    
    //int size = 0;
    memset(buffer, '\0', len);

    if (file_size[sock_fd] == 0)
    {
        char file_length[FILE_SIZE_LENGTH];
        int recv_size = recv(sock_fd, file_length, FILE_SIZE_LENGTH, 0);
        file_size[sock_fd] = atoi(file_length);
    }
    //int file_size = 0;              //  获取文件大小

    while(1)
    {
        //printf("size: %d\n", recv_file_size[sock_fd]);
        int recv_size = recv(sock_fd, buffer, len, 0);
        if (recv_size <= 0)
            break;
        
        recv_file_size[sock_fd] += recv_size;
        
        // printf("file_size: %d\n", file_size[sock_fd]);
        if (recv_file_size[sock_fd] == file_size[sock_fd])
        {
            printf("read int %d bytes from socket %d\n", recv_file_size[sock_fd], sock_fd);
            return true;
        }
    }
    
    return false;
}

//  向服务器发起num个TCP连接, 可根据改变num的大小来调整测试压力
void start_conn(int epoll_fd, int num, const char* ip, int port)
{
    int ret = 0;
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;            //  ipv4
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);

    for (int i = 0; i < num; ++i)
    {
        sleep(0.5);               //  延时1s，建立一个连接
        int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0)
        {
            continue;
        }

        if (connect(sock_fd, (struct sockaddr*) &server_address, sizeof(server_address)) == 0)
        {
            printf("Socket %d build connect !!!\n", sock_fd);
            add_fd(epoll_fd, sock_fd);
        }
    }
}

//  关闭连接，将连接的fd从epoll监听中删除
void close_conn(int epoll_fd, int sock_fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock_fd, 0);
    close(sock_fd);
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        printf("usage: %s ip_address port_number conn_number \n", basename(argv[0]));
        return 1;
    }
    char buffer[65535];

    int epoll_fd = epoll_create(100);
    start_conn(epoll_fd, atoi(argv[3]), (argv[1]), atoi(argv[2]));
    //start_conn(epoll_fd, atoi(argv[1]), ip, port);
    epoll_event events[EVENTS_NUMBER];

    while (1)
    {
        int events_count = epoll_wait(epoll_fd, events, EVENTS_NUMBER, 2000);    //  2000ms
        for (int i = 0; i < events_count; i++)
        {
            int sock_fd = events[i].data.fd;

            if (events[i].events & (EPOLLERR | EPOLLRDHUP | EPOLLHUP))  
            {
                close_conn(epoll_fd, sock_fd);
            }
            else if (events[i].events & EPOLLIN)
            {
                if (!read_file(sock_fd, buffer, 65535))    
                {
                    struct epoll_event event;
                    event.events = EPOLLIN | EPOLLET | EPOLLERR;
                    event.data.fd = sock_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event);
                }
                else
                {
                    struct epoll_event event;
                    event.events = EPOLLOUT | EPOLLET | EPOLLERR;
                    event.data.fd = sock_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event);
                }               
            }
            else if (events[i].events & EPOLLOUT)
            {
                if (!write_nbytes(sock_fd, file_name[0].c_str(), file_name[0].size()))
                {
                    close_conn(epoll_fd, sock_fd);
                }

                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET | EPOLLERR;
                event.data.fd = sock_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event);
            }
            
        }
    }
}

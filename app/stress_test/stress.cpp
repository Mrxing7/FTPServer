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

//  从服务器读取数据 (即接收服务器传输的文件)  
bool read_file(int sock_fd, char * file_name)
{
    int recv_size = 0;
    char normal_data[SEDN_RECV_BUF_SIZE];
    char file_length[FILE_SIZE_LENGTH];

    bzero(normal_data, SEDN_RECV_BUF_SIZE);
    bzero(file_length, FILE_SIZE_LENGTH);

    //  打开（创建）文件
    FILE *file_fp = fopen(file_name, "w+");
    if (!file_fp)
    {
        printf("Fail to create file !!!\n");
        return 0;
    }

    //  发送文件名
    int send_size = send(sock_fd, file_name, strlen(file_name), 0);
    if (send_size < 0)
    {
        printf("Error send !!!\n");
        return 0;
    }

    //  判断服务器是否有该文件，接收到"-1"为未找到文件，否则接收到为文件的大小
    bzero(file_length, FILE_SIZE_LENGTH);
    recv_size = recv(sock_fd, file_length, FILE_SIZE_LENGTH, 0);
    if (!strcmp(file_length, "-1"))
    {
        printf("File name not found!!\n");
        return 0;
    }

    int file_size = 0;              //  获取文件大小
    file_size = atoi(file_length);  
    recv_size = 0;
    printf("file_size: %d\n", file_size);
    //  计算文件下载的花费时间
    timespec start, end;
    
    //  接收文件内容
    printf("File downloading...\n");
    //printf(YELLOW);
    sleep(1);
    clock_gettime(CLOCK_MONOTONIC, &start);         //  获取下载开始时间
    while (1)
    {
        //  接收文件数据
        bzero(normal_data, SEDN_RECV_BUF_SIZE);
        int tmp_recv_size = recv(sock_fd, normal_data, SEDN_RECV_BUF_SIZE, 0);
        if (tmp_recv_size <= 0)
            break;
        // 写入文件中
        fwrite(normal_data, sizeof(char), tmp_recv_size, file_fp);

        // 显示进度条
        recv_size += tmp_recv_size;
        //prog_bar_show((HKA_U32) (((double)recv_size / (double)file_size) * 100));

        //  文件传输完成
        if (recv_size == file_size)
        {
            break;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);         //  获取下载结束时间
    long long time_comsumption = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;  //  单位为us
    printf("time consuming: %lldus\n", time_comsumption);
    //printf(WHITE);
    fclose(file_fp);
    printf("Socket %d received the file successfully !!!\n", sock_fd);
    printf("\n");
    //close(sock_fd);
    return true;
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
        sleep(1);               //  延时1s，建立一个连接
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
    // char *ip = "192.168.33.141";
    // int port = 9999;

    int epoll_fd = epoll_create(100);
    start_conn(epoll_fd, atoi(argv[3]), (argv[1]), atoi(argv[2]));
    //start_conn(epoll_fd, atoi(argv[1]), ip, port);
    epoll_event events[EVENTS_NUMBER];

    while (1)
    {
        int events_count = epoll_wait(epoll_fd, events, EVENTS_NUMBER, 0);    //  2000ms
        for (int i = 0; i < events_count; i++)
        {
            int sock_fd = events[i].data.fd;
            if (events[i].events & EPOLLIN)
            {
                if (!read_file(sock_fd, (char *)file_name[0].c_str()))    //  若下载文件成功，则关闭连接
                {
                    close_conn(epoll_fd, sock_fd);
                    continue;
                }

                struct epoll_event event;
                event.events = EPOLLOUT;
                event.data.fd = sock_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event);
            }
            else if (events[i].events & EPOLLOUT)
            {
                if (!write_nbytes(sock_fd, file_name[0].c_str(), file_name[0].size()))
                {
                    close_conn(epoll_fd, sock_fd);
                }

                struct epoll_event event;
                event.events = EPOLLIN;
                event.data.fd = sock_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event);
            }
            else if (events[i].events & (EPOLLERR | EPOLLRDHUP | EPOLLHUP))
            {
                close_conn(epoll_fd, sock_fd);
            }
        }
    }
}
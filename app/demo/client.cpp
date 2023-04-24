/***************************************************************************************************
* 客户端程序
***************************************************************************************************/

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <signal.h>
#include <unistd.h> 
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h> 
#include <string.h>
#include <ctime>

#include "../../code/progress_bar/prog_bar.h"

#define SEDN_RECV_BUF_SIZE 65535
#define FILE_NAME_LENGTH 1000
#define FILE_SIZE_LENGTH 20

struct FILEINFO {
    char file_name[FILE_NAME_LENGTH];           //  文件名
    int offset;                                 //  断点重传的文件偏移量，正常情况=0
};

//  设置文件描述符为非阻塞
int set_non_block(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main (int argc, char *argv[])
{
    if (argc <= 3)
    {
        printf("usage: %s ip_address port_number file_name\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    // if (argc <= 1)
    // {
    //     printf("usage: %s file_name \n", basename(argv[0]));
    //     return 1;
    // }

    // const char *ip = "192.168.43.254";
    // int port = 10001;

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;

    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);
    //set_non_block(sockfd);

    int ret = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (ret < 0)
    {
        printf("connection fail !!!\n");
        return 0;
    }
    else
    {
        char normal_data[SEDN_RECV_BUF_SIZE];
        char file_length[FILE_SIZE_LENGTH];
        int recv_size = 0;
        bzero(normal_data, SEDN_RECV_BUF_SIZE);

        //char file_name[FILE_NAME_LENGTH] = "testfile.txt";
        FILEINFO file_info;
        strcpy(file_info.file_name, argv[3]);       //  argv[3]
        // bzero(file_name, FILE_NAME_LENGTH);
        // printf("Please input file name: ");
        // scanf("%s", file_name);

        //  打开（创建）文件
        FILE *file_fp = fopen(file_info.file_name, "a");
        if (!file_fp)
        {
            printf("Fail to create file !!!\n");
            return 0;
        }

        //  获取本地文件大小
        fseek(file_fp, 0, SEEK_END);
        file_info.offset = ftell(file_fp);

        printf("local file_size: %d\n", file_info.offset);

        //  发送文件名
        int send_size = send(sockfd, (char *) &file_info, sizeof(file_info), 0);
        if (send_size < 0)
        {
            printf("Error send !!!\n");
            return 0;
        }

        //  计算文件下载的花费时间
        timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);         //  获取下载开始时间

        //  判断服务器是否有该文件，接收到"-1"为未找到文件，否则接收到为文件的大小
        bzero(file_length, FILE_SIZE_LENGTH);
        recv_size = recv(sockfd, file_length, FILE_SIZE_LENGTH, 0);
        if (!strcmp(file_length, "-1"))
        {
            printf("File name not found!!\n");
            return 0;
        }

        if (!strcmp(file_length, "0"))
        {
            printf("File already exist!!\n");
            return 0;
        }

        int file_size = 0;              //  获取文件大小
        file_size = atoi(file_length);  
        recv_size = 0;
        printf("service file_size: %d\n", file_size);
        
        //  接收文件内容
        printf("File downloading...\n");
        printf(YELLOW);
        while (1)
        {
            //  接收文件数据
            bzero(normal_data, SEDN_RECV_BUF_SIZE);
            int tmp_recv_size = recv(sockfd, normal_data, SEDN_RECV_BUF_SIZE, 0);
            if (tmp_recv_size <= 0)
                break;
            // 写入文件中
            fwrite(normal_data, sizeof(char), tmp_recv_size, file_fp);

            // 显示进度条
            recv_size += tmp_recv_size;
            prog_bar_show((HKA_U32) (((double)recv_size / (double)file_size) * 100));

            //  文件传输完成
            if (recv_size == file_size)
            {
                break;
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end);         //  获取下载结束时间
        long long time_comsumption = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;  //  单位为us
        printf("\n");
        printf("time consuming: %lldus\n", time_comsumption);
        printf(WHITE);
        fclose(file_fp);
        //sleep(20);
    }
    close(sockfd);
    return 0;

}
/***************************************************************************************************
* main.c
***************************************************************************************************/
#include "./ftp_server/ftp_server.h"
#include "./progress_bar/prog_bar.h"
#include "./type/basic_type.h"


int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("usage : %s port time_out thread_num\n", basename(argv[0]));
    }

    int port = atoi(argv[1]);
    int time_out = atoi(argv[2]);
    int thread_num = atoi(argv[3]);

    FTPServer server(port, time_out, thread_num);   //  端口号  超时时间  线程池中的线程数量
    server.start();
    
    return 0;
}
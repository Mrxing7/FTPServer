/***************************************************************************************************
* main.c
***************************************************************************************************/
#include "./ftp_server/ftp_server.h"
#include "./progress_bar/prog_bar.h"
#include "./type/basic_type.h"


int main(int argc, char *argv[])
{
    FTPServer server(9876, 10000, 8);   //  端口号  超时时间  线程池中的线程数量
    server.start();
    
    return 0;
}
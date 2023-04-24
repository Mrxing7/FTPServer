/***************************************************************************************************
* 服务器头文件
***************************************************************************************************/

#ifndef _SERVER_H_
#define _SERVER_H_

#include "../common/common.h"
#include "../type/basic_type.h"
#include "../ftp_conn/ftp_conn.h"
#include "epoller.h"
#include "../pool/thread_pool.h"
#include "../timer/heap_timer.h"

// 定义FTPSever类
class FTPServer
{
public:
    FTPServer(HKA_U32 port, HKA_U32 timeout_ms, HKA_S32 thread_num);
    ~FTPServer();
    HKA_VOID start();

private:
    HKA_STATUS init_socket_();                              //  初始化socket
    HKA_VOID deal_listen_();                                //  处理新的连接
    HKA_VOID deal_read_(FTPConn *client);                   //  将客户端请求加入线程池
    HKA_VOID deal_write_(FTPConn *client);                  //  将响应客户端加入线程池 

    HKA_VOID on_read_(FTPConn *client);                     //  处理客户端请求
    HKA_VOID on_write_(FTPConn *client);                    //  返回数据给客户端 

    HKA_VOID add_client_(HKA_S32 fd, sockaddr_in addr);     //  新增客户端       

    HKA_VOID close_conn_(FTPConn *client);                  //  关闭连接

    HKA_VOID extent_time(FTPConn* client);                  //  更新超时时间
    HKA_STATUS set_fd_nonblock(HKA_S32 fd);                 //  设置fd为非阻塞

    HKA_U32 port_;                                      //  服务器端口号
    struct sockaddr_in  ip_address_;                    //  服务器ip
    HKA_BOOL is_close_;                                 //  服务端是否关闭标志
    HKA_S32 listen_fd_;                                 //  监听描述符
    HKA_S08* src_dir_;                                  //  服务器文件的路径
    HKA_U32 timeout_ms_;                                //  超时时间(单位：ms)

    std::unique_ptr<HeapTimer> timer_;                  //  定时器
    std::unique_ptr<Epoller> epoller_;                  //  I/O复用的文件描述符
    std::unique_ptr<ThreadPool> thread_pool_;           //  线程池
    std::unordered_map<int, FTPConn> clients_;          //  连接的客户端
    
};  

#endif // end of _SERVER_H_
/***************************************************************************************************
* 服务器程序
***************************************************************************************************/

#include "ftp_server.h"

using namespace std;

/***************************************************************************************************
* 功  能：FTPSever类的构造函数
* 参  数：*
*         port          -I  服务器的端口号
*         timeout_ms    -I  超时时间，单位ms
*         thread_num    -I  线程池中线程的数量
* 形参列表：*
*           port_           端口号
*           is_close_       服务器是否关闭标志位，true为关闭，false为打开  
*           timeout_ms_     超时时间
*           timer_          定时器 
*           epoller_        I/O复用
*           thread_pool_    线程池
* 返回值：无         
* 备  注：
***************************************************************************************************/
FTPServer::FTPServer(HKA_U32 port, HKA_U32 timeout_ms, HKA_S32 thread_num) : 
            port_(port), is_close_(false), timeout_ms_(timeout_ms), 
            timer_(new HeapTimer), epoller_(new Epoller()), 
            thread_pool_(new ThreadPool(thread_num))
{
    HKA_STATUS ret;
    FTPConn::client_count = 0;              //  客户端连接的数量

    //  获取服务器保存文件的路径
    src_dir_ = (HKA_S08*) getcwd(NULL, 256);
    assert(src_dir_);
    strcat((char *)src_dir_, "/../data/");
    FTPConn::src_dir = src_dir_;

    //  初始化socket
    ret = init_socket_();
    if(ret < 0)
    {
        is_close_ = true;
    }
}

/***************************************************************************************************
* 功  能：FTPSever类的析构函数
* 参  数：*
*        无
* 返回值：无         
* 备  注：
***************************************************************************************************/
FTPServer::~FTPServer()
{
    close(listen_fd_);      
    is_close_ = true;
}

/***************************************************************************************************
* 功  能：初始化服务器的socket
* 参  数：*
*        无
* 返回值：状态码    HKA_STATUS         
* 备  注：
***************************************************************************************************/
HKA_STATUS FTPServer::init_socket_()
{
    HKA_STATUS ret = HKA_STS_ERR;

    //  端口号检测是否符合范围
    if (port_ < 1024 || port_ > 65535)
    {
        return ret;
    }

    //  初始化服务器的ip地址和端口号
    bzero(&ip_address_, sizeof(ip_address_));
    ip_address_.sin_family = AF_INET;
    ip_address_.sin_addr.s_addr = (INADDR_ANY);
    ip_address_.sin_port = htons(port_);

    //  创建socket
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd_ < 0) {
        // LOG_ERROR("Create socket error!", port_);
        return ret;
    }

    //  绑定socket
    ret = bind(listen_fd_, (struct sockaddr *)&ip_address_, sizeof(ip_address_));
    if (ret < 0)
    {
        close(listen_fd_);
        return ret;
    }

    //  监听socket
    ret = listen(listen_fd_, 5);
    if (ret < 0)
    {
        close(listen_fd_);
        return ret;
    }

    // 将listen_fd_添加到epoll监听事件中 (I/O多路复用)
    ret = epoller_->add_fd(listen_fd_, EPOLLIN | EPOLLET | EPOLLRDHUP);
    if (ret < 0)
    {
        close(listen_fd_);
        return ret;
    }

    //  设置I/O非阻塞
    set_fd_nonblock(listen_fd_);

    //  服务器初始化成功
    printf("Server initialization successful!!!\n"); 

    return ret;
}

/***************************************************************************************************
* 功  能：服务器开始运行
* 参  数：*
*        无
* 返回值：HKA_VOID         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPServer::start()
{   
    int time_ms = 0;        //  epoll等待时间，也是超时关闭的客户端连接的时间
    while (!is_close_)      //  服务器未关闭
    {
        if(timeout_ms_ > 0) {
            time_ms = timer_->get_next_tick();
        }
        int event_count = epoller_->wait(time_ms);        //  epoll监听I/O事件 (超时连接到了返回)
        for (int i = 0; i < event_count; i++)
        {
            int curfd = epoller_->get_event_fd(i);          //  获取事件的fd
            uint32_t events = epoller_->get_events(i);      //  获取事件的类型

            if (curfd == listen_fd_)                //  有新的客户端连接
            {
                deal_listen_();     //  处理新的客户端连接
            }        
            else if (events & EPOLLOUT)     //  服务器发送数据（epoll监听到写事件）
            {
                deal_write_(&clients_[curfd]);  //  处理写
                //on_write_(&clients_[curfd]);
            }
            else if (events & EPOLLIN)      //  服务器接受数据（epoll监听到读事件）
            {
                deal_read_(&clients_[curfd]);   //  处理读
                //on_read_(&clients_[curfd]);
                
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))    //  有异常关闭与客户端的连接
            {
                close_conn_(&clients_[curfd]);  //  关闭客户端连接
            }
            else
            {
                printf("Unexpected event!!!\n");
            }
        }
    }

    //  关闭服务器的监听端口
    close(listen_fd_);
}

/***************************************************************************************************
* 功  能：服务器处理新的客户端连接
* 参  数：*
*        无
* 返回值：HKA_VOID         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPServer::deal_listen_()
{
    //  有新的客户端连接
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);

    //  建立接收客户端连接
    int connfd = accept(listen_fd_, (struct sockaddr *) &client, &client_addrlength);
    if (connfd < 0)
    {
        return;
    }

    //  增加新的客户端
    add_client_(connfd, client);
}

/***************************************************************************************************
* 功  能：添加新的客户端
* 参  数：*
*        fd     -I      客户端连接的fd
*        addr   -I      客户端ip、port   
* 返回值：HKA_VOID         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPServer::add_client_(HKA_S32 fd, sockaddr_in addr)
{
    assert(fd > 0);
    clients_[fd].init_(fd, addr);                               //  初始化客户端
    epoller_->add_fd(fd, EPOLLIN | EPOLLET | EPOLLRDHUP);       //  设置为ET模式（边沿触发，只通知一次）
    set_fd_nonblock(fd);                                        //  非阻塞

    //  给新的客户端设定超时时间
    if(timeout_ms_ > 0) {
        timer_->add(fd, timeout_ms_, std::bind(&FTPServer::close_conn_, this, &clients_[fd]));
    }
}

/***************************************************************************************************
* 功  能：服务器处理读（接收客户端的请求）
* 参  数：*
*        client     -I      客户端连接 
* 返回值：HKA_VOID         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPServer::deal_read_(FTPConn *client)                   
{
    assert(client);

    //  将客户端请求任务加入线程池
    thread_pool_->append(std::bind(&FTPServer::on_read_, this, client));   
}

/***************************************************************************************************
* 功  能：服务器处理写（响应客户端的请求）
* 参  数：*
*        client     -I      客户端连接 
* 返回值：HKA_VOID         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPServer::deal_write_(FTPConn *client)                  //  将响应客户端任务加入线程池 
{
    extent_time(client);        //  更新超时时间
    thread_pool_->append(std::bind(&FTPServer::on_write_, this, client));   //  将发送任务交给子线程之后，主线程继续监听连接
    epoller_->mod_fd(client->get_fd_(), EPOLLIN);   //  修改epoller监听的事件类型
}

/***************************************************************************************************
* 功  能：服务器接收客户端数据并进行解析
* 参  数：*
*        client     -I      客户端连接 
* 返回值：HKA_VOID         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPServer::on_read_(FTPConn *client)       //  在子线程中处理
{
    HKA_S32 recv_size = client->recv_();        //  读取客户端信息
    if (recv_size < 0)      //  客户端未发送数据
    {
        return;
    }
    else if (recv_size == 0)    //  客户端关闭连接
    {
        close_conn_(client);
        return;
    }
    extent_time(client);
    client->process_();
    epoller_->mod_fd(client->get_fd_(), EPOLLOUT);
}

/***************************************************************************************************
* 功  能：服务器向发送客户端发送文件
* 参  数：*
*        client     -I      客户端连接 
* 返回值：HKA_VOID         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPServer::on_write_(FTPConn *client)      //  在子线程中处理
{
    assert(client);

    //  传输文件
    client->set_is_transport(true);                 //  设置传输文件标志（正在传输文件）
    client->send_file_(client->get_file_fd_());
}

/***************************************************************************************************
* 功  能：服务器关闭与客户端的连接
* 参  数：*
*        client     -I      客户端连接 
* 返回值：HKA_VOID         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPServer::close_conn_(FTPConn *client)
{
    epoller_->delete_fd(client->get_fd_()); 
    client->close_();
}

/***************************************************************************************************
* 功  能：设置fd为非阻塞
* 参  数：*
*        fd     -I      文件描述符
* 返回值：状态码 （失败返回-1）       
* 备  注：
***************************************************************************************************/
HKA_STATUS FTPServer::set_fd_nonblock(HKA_S32 fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

/***************************************************************************************************
* 功  能：增加超时时间（当客户端在超时时间内与服务端进行了通信，则将其超时时间重新调整）
* 参  数：*
*        client     -I      客户端连接 
* 返回值：HKA_VOID         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPServer::extent_time(FTPConn* client) {
    assert(client);
    if(timeout_ms_ > 0) { timer_->adjust(client->get_fd_(), timeout_ms_); }
}




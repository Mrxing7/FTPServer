/***************************************************************************************************
* ftp_conn类（客户端连接）
***************************************************************************************************/
#ifndef _FTP_CONN_H_
#define _FTP_CONN_H_

#include "../common/common.h"
#include "../type/basic_type.h"

#define BUFFER_SIZE 1024
#define FILE_NAME_LENGTH 1000
#define FILE_SIZE_LENGTH 20
#define SEND_FILE_BLOCK_SIZE 65535

struct FILEINFO {
    char file_name[FILE_NAME_LENGTH];           //  文件名
    int offset;                                 //  断点重传的文件偏移量，正常情况=0
};

class FTPConn
{
public:
    FTPConn(HKA_S32 buffer_size = BUFFER_SIZE);
    ~FTPConn();

    HKA_VOID init_(HKA_S32 fd, const sockaddr_in& client_addr);     //  初始化

    HKA_BOOL process_();                //  处理业务逻辑

    HKA_S32 get_fd_();                  //  获得连接的fd
    HKA_S32 get_port_();                //  获得客户端端口号
    const char* get_ip_();              //  获得客户端ip
    sockaddr_in get_addr_();            //  获得客户端addr
    HKA_VOID close_();                  //  关闭连接

    HKA_S32 get_file_fd_();                     //  获得文件fd
    HKA_VOID set_file_fd(HKA_S32 file_fd);      //  设置文件fd

    ssize_t recv_();                            //  接受数据
    ssize_t send_();                            //  发送数据
    HKA_BOOL send_file_(HKA_S32 file_fd);       //  发送文件

    HKA_BOOL get_is_transport();                //  获取是否发送文件标志
    HKA_VOID set_is_transport(HKA_BOOL is);     //  设置是否发送文件标志

    static HKA_U32 client_count;        //  客户端连接的数量
    static HKA_S08* src_dir;            //  文件的路径

private:
    HKA_S32 fd_;                        //  与客户端发送数据的fd
    struct  sockaddr_in client_addr_;   //  客户端ip、port等信息

    std::vector<HKA_S08> recv_buffer_;  //  接受缓存
    std::vector<HKA_S08> send_buffer_;  //  发送缓存

    HKA_S32 file_fd_;                   //  传输的文件fd
    long long restart_file_pos;         //  文件传输的断点
    long long file_size;                //  文件大小
    HKA_BOOL is_close;                  //  是否断开连接
    HKA_BOOL is_transport;              //  是否在传输文件
    HKA_BOOL is_first_transport;        //  是否第一次传输文件
};

#endif // end of _FTP_CONN_H_
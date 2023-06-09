/***************************************************************************************************
* ftp_conn类（客户端连接）
***************************************************************************************************/
#include "ftp_conn.h"

HKA_U32 FTPConn::client_count;
HKA_S08* FTPConn::src_dir;

/***************************************************************************************************
* 功  能：FTPConn类的构造函数
* 参  数：*
*        buffer_size    -I      缓冲区大小
* 返回值：无         
* 备  注：
***************************************************************************************************/
FTPConn::FTPConn(HKA_S32 buffer_size)
{
    fd_ = -1;
    client_addr_ = { 0 };
    is_close = true;
    is_transport = false;
    is_first_transport = true;
    restart_file_pos = 0;
    file_size = 0;
    recv_buffer_.resize(buffer_size,'\0');
    send_buffer_.resize(FILE_SIZE_LENGTH,'\0');
}

/***************************************************************************************************
* 功  能：FTPConn类的析构函数
* 参  数：*
*        无
* 返回值：无         
* 备  注：
***************************************************************************************************/
FTPConn::~FTPConn()
{
    close_();       //  关闭连接
}

/***************************************************************************************************
* 功  能：FTPConn类的初始化
* 参  数：*
*        fd             -I     连接的文件描述符fd
*        client_addr    -I     客户端连接信息    
* 返回值：无         
* 备  注：
***************************************************************************************************/
HKA_VOID FTPConn::init_(HKA_S32 fd, const sockaddr_in& client_addr)
{
    assert(fd > 0);
    client_count++;
    client_addr_ = client_addr;
    fd_ = fd;
    is_close = false;
    is_transport = false;
    is_first_transport = true;
    restart_file_pos = 0;
    file_size = 0;
    recv_buffer_.resize(BUFFER_SIZE,'\0');
    send_buffer_.resize(FILE_SIZE_LENGTH,'\0');
}

/***************************************************************************************************
* 功  能：FTPConn类的请求处理
* 参  数：*
*        无   
* 返回值：是否正确处理 HKA_BOOL        
* 备  注：
***************************************************************************************************/
HKA_BOOL FTPConn::process_()
{
    FILEINFO *file_info =(FILEINFO *) &(recv_buffer_[0]);

    //  获取文件路径
    char file_path[FILE_NAME_LENGTH];
    strcpy(file_path, (char *)src_dir);

    if (recv_buffer_.size() > BUFFER_SIZE)     //  文件名太长
    {
        return false;
    }
    
    //strcat(file_path, (char *)&(file_info->file_name[0]));   
    //restart_file_pos = (file_info->offset);
    //strcat(file_path, (char *)&(recv_buffer_[0])); 
    strcat(file_path, file_info->file_name);        //  获取文件名
    restart_file_pos = (file_info->offset);         //  获取文件偏移量

    //  打开文件
    HKA_S32 file_fd = open(file_path, O_RDONLY);
    
    set_file_fd(file_fd);
    return true;
}        

/***************************************************************************************************
* 功  能：获取连接的文件描述符fd
* 参  数：*
*        无   
* 返回值：文件描述符fd        
* 备  注：
***************************************************************************************************/
HKA_S32 FTPConn::get_fd_()
{
    return fd_;
}

/***************************************************************************************************
* 功  能：获取客户端的端口号
* 参  数：*
*        无   
* 返回值：端口号       
* 备  注：
***************************************************************************************************/
HKA_S32 FTPConn::get_port_()
{
    return client_addr_.sin_port;
}

/***************************************************************************************************
* 功  能：获取客户端的ip地址
* 参  数：*
*        无   
* 返回值：ip地址      
* 备  注：
***************************************************************************************************/
const char* FTPConn::get_ip_()
{
    return inet_ntoa(client_addr_.sin_addr);
}

/***************************************************************************************************
* 功  能：获取客户端的信息
* 参  数：*
*        无   
* 返回值：客户端的信息      
* 备  注：
***************************************************************************************************/
sockaddr_in FTPConn::get_addr_()
{
    return client_addr_;
}

/***************************************************************************************************
* 功  能：获取客户端下载文件的fd
* 参  数：*
*        无   
* 返回值：文件的fd     
* 备  注：
***************************************************************************************************/
HKA_S32 FTPConn::get_file_fd_()
{
    return file_fd_;
}

/***************************************************************************************************
* 功  能：设置客户端下载文件的fd
* 参  数：*
*        file_fd        -I      文件描述符   
* 返回值：空   
* 备  注：
***************************************************************************************************/
HKA_VOID FTPConn::set_file_fd(HKA_S32 file_fd)
{
    assert(file_fd);
    file_fd_ = file_fd;
}

/***************************************************************************************************
* 功  能：接收客户端数据
* 参  数：*
*        无   
* 返回值：客户端数据的大小
* 备  注：
***************************************************************************************************/
ssize_t FTPConn::recv_()
{
    recv_buffer_.assign(BUFFER_SIZE,'\0');
    return recv(fd_, &recv_buffer_[0], (int)recv_buffer_.size(), 0);
}

/***************************************************************************************************
* 功  能：向客户端发送数据（主要发送请求的文件长度信息）
* 参  数：*
*        无   
* 返回值：发送数据的大小
* 备  注：
***************************************************************************************************/
ssize_t FTPConn::send_()
{
    return send(fd_, &send_buffer_[0], (int)send_buffer_.size(), 0);
}

/***************************************************************************************************
* 功  能：向客户端发送文件数据（更高效）
* 参  数：*
*        无   
* 返回值：是否发送成功
* 备  注：
***************************************************************************************************/
HKA_BOOL FTPConn::send_file_(HKA_S32 file_fd)
{
    if (is_transport == false)      //  传输文件标志
        return false;
    
    if(file_fd < 0)         //  文件不存在(文件已经传完), 向客户端发送-1
    {
        const char *reply_message = "-1";
        send(fd_, reply_message, strlen(reply_message), 0);
        return false;
    }   
    else                    //  文件存在, 向客户端发送文件的大小
    {
        struct stat stat_buf;
        fstat(file_fd, &stat_buf);
        long long file_size = stat_buf.st_size;     //  剩余的文件字节

        //  判断断点续传
        int offset = restart_file_pos;
        if (offset != 0)
        {
            file_size -= offset;
            lseek(file_fd, offset, SEEK_SET);
        }

        //  判断是否是第一次传输
        if(is_first_transport == true)
        {
            //  发送文件的大小
            sprintf((char *)&send_buffer_[0], "%lld", file_size);
            int send_size = send_();        //  将send_buffer_内容发送给客户端
            if (send_size < 0)
            {
                return send_size;
            }
            is_first_transport = false;
        }
        
        //  发送文件内容
        int block_size = 0;
        int send_size = 0;
        // const int fix_size = 65536;
        char *client_ip = inet_ntoa(client_addr_.sin_addr);
        printf("file_size: %d\n", file_size);
        while(file_size > 0)
        {
            //block_size = (file_size > fix_size) ? fix_size : file_size;   //读取字节数
            block_size = file_size;
            send_size = sendfile(fd_, file_fd, NULL, block_size);

            if (send_size < 0)
            {
                is_first_transport = false;
                is_transport = false;
                return false;
            }

            file_size -= send_size;
            restart_file_pos += send_size;
            printf("Send %d bytes file to %s:%d \n", send_size, client_ip, client_addr_.sin_port);
        }
        close(file_fd);             //  关闭文件描述符
        is_transport = false;
        return true;
    }
}

/***************************************************************************************************
* 功  能：获取是否在传输文件
* 参  数：*
*        无   
* 返回值：true、false
* 备  注：
***************************************************************************************************/
HKA_BOOL FTPConn::get_is_transport()
{
    return is_transport;
}

/***************************************************************************************************
* 功  能：设置是否在传输文件
* 参  数：*
*        is         -I  是否在传输文件   
* 返回值：空
* 备  注：
***************************************************************************************************/
HKA_VOID FTPConn::set_is_transport(HKA_BOOL is)
{
    is_transport = is;
}

/***************************************************************************************************
* 功  能：关闭连接
* 参  数：*
*        无 
* 返回值：空
* 备  注：
***************************************************************************************************/
HKA_VOID FTPConn::close_()
{
    if(is_close == false)
    {
        is_close = true;
        client_count--;
        close(file_fd_);
        close(fd_);
    }       
}
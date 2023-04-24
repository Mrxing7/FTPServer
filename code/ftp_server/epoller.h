/***************************************************************************************************
* epoll类（I/O复用）
***************************************************************************************************/
#ifndef _EPOLLER_H_
#define _EPOLLER_H_

#include "../common/common.h"
#include "../type/basic_type.h"

class Epoller 
{
public:
    explicit Epoller(HKA_S32 max_event = 1024);         
    ~Epoller(); 
    HKA_BOOL add_fd(HKA_S32 fd, uint32_t events);       //  添加epoll_fd_上的注册事件
    HKA_BOOL mod_fd(HKA_S32 fd, uint32_t events);       //  修改epoll_fd_上的注册事件
    HKA_BOOL delete_fd(HKA_S32 fd);                     //  删除epoll_fd_上的注册事件
    HKA_S32 wait(HKA_S32 timeout_ms = -1);              //  epoll_wait(), 默认为阻塞
    HKA_S32 get_event_fd(size_t index);                 //  获取events_[index]的fd
    uint32_t get_events(size_t index);                  //  获取events_[index]的events

private:
    HKA_S32 epoll_fd_;                                  //  epoll I/O多路复用的文件描述符
    std::vector<struct epoll_event> events_;            //  epoll_wait()返回的可操作的事件
    HKA_U32 events_length_;                             //  epoll_wait()返回的events_的长度
    HKA_U32 events_length_old_;                         //  上一次返回的events_长度（用来更新当前的events_length_）
    HKA_U32 events_length_old_old_;                     //  上上一次返回的events_长度（用来更新当前的events_length_）
};

#endif      // end of _EPOLLER_H_
/***************************************************************************************************
* epoll类（I/O复用）
***************************************************************************************************/
#include "epoller.h"

/***************************************************************************************************
* 功  能：Epoller类的构造函数
* 参  数：*
*        max_event      -I      监听的事件数量
* 形参列表：*
            events_                 epoll_wait()返回的可操作的事件
            events_length_          epoll_wait()返回的可操作的事件数量    
            events_length_old_      上次epoll_wait()返回的可操作的事件数量   
            events_length_old_old_  上上次epoll_wait()返回的可操作的事件数量 
* 返回值：无         
* 备  注：
***************************************************************************************************/
Epoller::Epoller(HKA_S32 max_event): epoll_fd_(epoll_create(1024)), events_(max_event),
                                    events_length_(1024), events_length_old_(0), 
                                    events_length_old_old_(0)
{
    assert(epoll_fd_ >= 0 && events_.size() > 0);
}

/***************************************************************************************************
* 功  能：Epoller类的析构函数
* 参  数：*
*        无
* 返回值：无         
* 备  注：
***************************************************************************************************/
Epoller::~Epoller() 
{
    close(epoll_fd_);       //  关闭epoll的文件描述符
}

/***************************************************************************************************
* 功  能：向epoll中添加待监听的事件
* 参  数：*
*        fd         -I      待监听的文件描述符
*        events     -I      待监听的事件类型
* 返回值：是否添加成功  HKA_BOOL         
* 备  注：
***************************************************************************************************/
HKA_BOOL Epoller::add_fd(HKA_S32 fd, uint32_t events)
{
    if(fd < 0) 
    {
        return false;
    }

    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);  
}

/***************************************************************************************************
* 功  能：向epoll中修改监听的事件类型
* 参  数：*
*        fd         -I      文件描述符
*        events     -I      事件类型
* 返回值：是否修改成功  HKA_BOOL         
* 备  注：
***************************************************************************************************/
HKA_BOOL Epoller::mod_fd(HKA_S32 fd, uint32_t events)
{
    if(fd < 0) 
    {
        return false;
    }

    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
}

/***************************************************************************************************
* 功  能：向epoll中删除监听的事件
* 参  数：*
*        fd         -I      文件描述符
*        events     -I      事件类型
* 返回值：是否删除成功  HKA_BOOL         
* 备  注：
***************************************************************************************************/
HKA_BOOL Epoller::delete_fd(HKA_S32 fd)
{
    if(fd < 0) 
    {
        return false;
    }
    
    epoll_event ev = {0};
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev);
}

/***************************************************************************************************
* 功  能：等待监听的事件
* 参  数：*
*        timeout_ms         -I      超时时间
* 返回值：返回事件发生的数量        
* 备  注：
***************************************************************************************************/
HKA_S32 Epoller::wait(HKA_S32 timeout_ms)
{
    //  用来更新当前的events_的长度（实时更新: 当前长度 = 上上次长度*0.2 + 上次长度*0.8）
    if (events_length_old_ != 0 && events_length_old_old_ != 0)
    {
        events_.resize(0.2 * events_length_old_old_ + 0.8 * events_length_old_);
    }

    //  系统调用epoll_wait函数
    events_length_ = epoll_wait(epoll_fd_, &events_[0], (int)(events_.size()), timeout_ms);
    events_length_old_old_ = events_length_old_;
    events_length_old_ = events_length_;
    return events_length_;
}

/***************************************************************************************************
* 功  能：获取事件的文件描述符fd
* 参  数：*
*        index         -I      事件下标
* 返回值：事件的文件描述符fd        
* 备  注：
***************************************************************************************************/
HKA_S32 Epoller::get_event_fd(size_t index)
{
    assert(index < events_length_ && index >= 0);
    return events_[index].data.fd;
}

/***************************************************************************************************
* 功  能：获取事件的类型
* 参  数：*
*        index         -I      事件下标
* 返回值：事件的类型       
* 备  注：
***************************************************************************************************/
uint32_t Epoller::get_events(size_t index)
{
    assert(index < events_length_ && index >= 0);
    return events_[index].events;
}
/***************************************************************************************************
* 实现线程池的同步机制
***************************************************************************************************/
#ifndef _LOCKER_H_
#define _LOCKER_H_

#include "../common/common.h"
#include "../type/basic_type.h"

/***************************************************************************************************
* 信号量类
***************************************************************************************************/
class Sem
{
public:
    //  创建并初始化信号量
    Sem()
    {
        if (sem_init( &m_sem_, 0, 0) != 0)
        {
            //  构造函数没有返回值，可以通过抛出异常来报告错误
            throw std::exception();
        }
    }

    // 销毁信号量
    ~Sem()
    {
        sem_destroy(&m_sem_);
    }

    //  等待信号量，P操作
    HKA_BOOL wait()
    {
        return sem_wait(&m_sem_) == 0;
    }

    //  增加信号量，V操作
    HKA_BOOL post()
    {
        return sem_post(&m_sem_) == 0;
    }

private:
    sem_t m_sem_;
};

/***************************************************************************************************
* 互斥锁类
***************************************************************************************************/
class Locker
{
public:
    //  创建并初始化互斥锁
    Locker()
    {
        if (pthread_mutex_init(&m_mutex_, NULL) != 0)
        {
            throw std::exception();
        }
    }

    //  销毁互斥锁
    ~Locker()
    {
        pthread_mutex_destroy(&m_mutex_);
    }

    //  获取互斥锁
    HKA_BOOL lock()
    {
        return pthread_mutex_lock(&m_mutex_) == 0;
    }

    // 释放互斥锁
    HKA_BOOL unlock()
    {
        return pthread_mutex_unlock(&m_mutex_) == 0;
    }

private:
    pthread_mutex_t m_mutex_;
};

/***************************************************************************************************
* 条件变量类
***************************************************************************************************/
class Cond
{
public:
    //  创建并初始化条件变量
    Cond()
    {
        if (pthread_mutex_init(&m_mutex_, NULL) != 0)
        {
            throw std::exception();
        }
        if (pthread_cond_init(&m_cond_, NULL) != 0)
        {
            pthread_mutex_destroy(&m_mutex_);
            throw std::exception();
        }
    }

    //  销毁条件变量
    ~Cond()
    {
        pthread_mutex_destroy(&m_mutex_);
        pthread_cond_destroy(&m_cond_);
    }

    //  等待条件变量
    HKA_BOOL wait()
    {
        int ret = 0;
        pthread_mutex_lock(&m_mutex_);
        ret = pthread_cond_wait(&m_cond_, &m_mutex_);
        pthread_mutex_unlock(&m_mutex_);
        return ret == 0;
    }

    //  唤醒等待条件变量的线程
    HKA_BOOL signal()
    {
        return pthread_cond_signal(&m_cond_) == 0;
    }

private:
    pthread_mutex_t m_mutex_;
    pthread_cond_t m_cond_;
};

#endif      //  end of _LOCKER_H_
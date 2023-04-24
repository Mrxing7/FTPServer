/***************************************************************************************************
* 线程池类
***************************************************************************************************/
#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "../common/common.h"
#include "locker.h"

//  线程池类
class ThreadPool
{
public:
    //  参数thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的，等待处理的请求数量
    ThreadPool(HKA_S32 thread_number = 2, HKA_S32 max_requests = 10000);
    ~ThreadPool();

    template<class F>
    HKA_BOOL append(F&& task)       //  往请求队列中添加任务
    {
        queue_locker_.lock();
        if ((HKA_S32)work_queue_.size() > max_requests_)
        {
            queue_locker_.unlock();
            return false;
        }
        work_queue_.push(std::forward<F>(task));        //  将任务放入工作队列中
        queue_locker_.unlock();             //  解锁
        queue_stat_.post();                 //  V操作，信号量++
        return true;
    }

private:
    static HKA_VOID* worker(void *arg); //  工作线程运行的函数，它不断从工作队列中取出任务并执行
    HKA_VOID run();                     //  被worker()函数调用

private:
    HKA_S32 thread_number_;                                 //  线程池中的线程数
    HKA_S32 max_requests_;                                  //  请求队列中允许的最大请求数
    pthread_t* threads_;                                    //  描述线程池的数组，其大小为thread_number_
    std::queue< std::function<HKA_VOID()> > work_queue_;    //  请求队列
    Locker queue_locker_;                                   //  保护请求队列的互斥锁
    Sem queue_stat_;                                        //  是否有任务需要处理（信号量，记录的是任务队列中的数量）
    HKA_BOOL stop_;                                         //  是否结束线程
};


#endif      //  end of _THREAD_POOL_H_
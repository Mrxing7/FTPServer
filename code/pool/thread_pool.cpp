/***************************************************************************************************
* 线程池类
***************************************************************************************************/
#include "thread_pool.h"

/***************************************************************************************************
* 功  能：ThreadPool类的构造函数
* 参  数：*
*       thread_number       -I      线程数量
*       max_requests        -I      请求数量
* 返回值：无         
* 备  注：
***************************************************************************************************/
ThreadPool::ThreadPool(HKA_S32 thread_number, HKA_S32 max_requests) :
            thread_number_(thread_number), max_requests_(max_requests),
            threads_(NULL), stop_(false) 
{
    if (thread_number <= 0 || max_requests <= 0)
    {
        throw std::exception();
    }

    //  申请线程池的空间
    threads_ = new pthread_t[thread_number];
    if (!threads_)
    {
        throw std::exception();
    }

    //  创建thread_number个线程
    for (int i = 0; i < thread_number; ++i)
    {
        if (pthread_create(threads_ + i, NULL, worker, this) != 0)
        {
            delete [] threads_;
            throw std::exception();
        }
        if (pthread_detach(threads_[i]))    //  将其设置为脱离线程(可自行回收资源)
        {
            delete [] threads_;
            throw std::exception(); 
        }
    }
}

/***************************************************************************************************
* 功  能：ThreadPool类的析构函数
* 参  数：*
*        无
* 返回值：无         
* 备  注：
***************************************************************************************************/
ThreadPool:: ~ThreadPool()
{
    delete [] threads_;
    stop_ = true;
}

/***************************************************************************************************
* 功  能：线程的任务
* 参  数：*
*        arg       -I   线程池的参数
* 返回值：无         
* 备  注：
***************************************************************************************************/
HKA_VOID *ThreadPool::worker(void *arg)
{
    ThreadPool* pool = (ThreadPool *) arg;
    pool->run();
    return pool;
}

/***************************************************************************************************
* 功  能：线程的任务的执行
* 参  数：*
*        无
* 返回值：空         
* 备  注：
***************************************************************************************************/
void ThreadPool::run()
{
    while(!stop_)
    {
        queue_stat_.wait();     //  P操作, 信号量--
        queue_locker_.lock();   //  加锁

        if (work_queue_.empty())    //  如果任务队列为空，则释放锁，可让其往队列中添加任务
        {
            queue_locker_.unlock();
            continue;
        }

        auto task = std::move(work_queue_.front());  
        work_queue_.pop();                  //  任务出队 
        task();                             //  执行队列中的任务（往队列添加什么函数，就会执行什么函数）
        queue_locker_.unlock();             //  解锁
    }
}
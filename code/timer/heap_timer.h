/***************************************************************************************************
* 小根堆定时器类
***************************************************************************************************/
#ifndef _HEAP_TIMER_H_
#define _HEAP_TIMER_H_

#include "../common/common.h"
#include "../type/basic_type.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {              //  定时节点
    int id;                     //  连接的fd
    TimeStamp expires;          //  超时时间
    TimeoutCallBack cb;         //  超时回调函数

    //  重载比较运算符
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};

class HeapTimer
{
private:
    /* data */
public:
    HeapTimer();     

    ~HeapTimer();

    HKA_VOID adjust(HKA_S32 id, HKA_S32 newExpires);                //  堆节点的调整

    HKA_VOID add(HKA_S32 id, HKA_S32 timeOut, const TimeoutCallBack& cb);   //  添加定时节点

    HKA_VOID do_work(HKA_S32 id);           // 删除指定id结点，并触发回调函数 

    HKA_VOID clear();                       //  清除heap_、ref_

    HKA_VOID tick();                        //  清除超时结点            

    HKA_VOID pop();                         //  将超时结点pop出数组

    HKA_S32 get_next_tick();                //  获取下一个距离最近的超时节点

private:

    HKA_VOID del_(size_t i);                //  删除超时节点
        
    HKA_VOID sift_up_(size_t i);            //  堆向上翻转

    HKA_BOOL sift_down_(size_t index, size_t n);    //  堆向下反转

    HKA_VOID swap_node_(size_t i, size_t j);        //  交换时间节点

    std::vector<TimerNode> heap_;               //  数组保存堆的时间节点  

    std::unordered_map<int, size_t> ref_;       // 保存fd对应的heap_数组下标
};


#endif      //  end of _HEAP_TIMER_H_

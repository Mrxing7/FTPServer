/***************************************************************************************************
* 小根堆定时器类
***************************************************************************************************/

#include "heap_timer.h"

/***************************************************************************************************
* 功  能：HeapTimer类的构造函数（初始化HeapTimer类的成员）
* 参  数：* 
*        无 
* 返回值：无         
* 备  注：
***************************************************************************************************/
HeapTimer::HeapTimer() 
{ 
    heap_.reserve(64); 
}

/***************************************************************************************************
* 功  能：HeapTimer类的析构函数（删除HeapTimer类的成员）
* 参  数：*
*        无
* 返回值：无         
* 备  注：
***************************************************************************************************/
HeapTimer::~HeapTimer() 
{ 
    clear(); 
}

/***************************************************************************************************
* 功  能：
* 参  数：*
*        无
* 返回值：无         
* 备  注：
***************************************************************************************************/
HKA_VOID HeapTimer::sift_up_(size_t child_node_index) {
    assert(child_node_index >= 0 && child_node_index < heap_.size());
    size_t father_node_index = (child_node_index - 1) / 2;
    while(father_node_index >= 0) 
    {
        if(heap_[father_node_index] < heap_[child_node_index]) 
        { 
            break; 
        }
        swap_node_(child_node_index, father_node_index);
        child_node_index = father_node_index;
        father_node_index = (child_node_index - 1) / 2;
    }
}

HKA_VOID HeapTimer::swap_node_(size_t child_node_index, size_t father_node_index) {
    assert(child_node_index >= 0 && child_node_index < heap_.size());
    assert(father_node_index >= 0 && father_node_index < heap_.size());
    std::swap(heap_[child_node_index], heap_[father_node_index]);
    ref_[heap_[child_node_index].id] = child_node_index;
    ref_[heap_[father_node_index].id] = father_node_index;
} 

HKA_BOOL HeapTimer::sift_down_(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while(j < n) 
    {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        swap_node_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

HKA_VOID HeapTimer::add(HKA_S32 id, HKA_S32 time_out, const TimeoutCallBack& cb) {
    assert(id >= 0);
    size_t i;
    if(ref_.count(id) == 0) {
        /* 新节点：堆尾插入，调整堆 */
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(time_out), cb});
        sift_up_(i);
    } 
    else {
        /* 已有结点：调整堆 */
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(time_out);
        heap_[i].cb = cb;
        if(!sift_down_(i, heap_.size())) {
            sift_up_(i);
        }
    }
}

HKA_VOID HeapTimer::do_work(int id) {
    /* 删除指定id结点，并触发回调函数 */
    if(heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();      //  回调函数
    del_(i);        //  删除定时节点
}

HKA_VOID HeapTimer::del_(size_t index) {
    /* 删除指定位置的结点 */
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if(i < n) {
        swap_node_(i, n);
        if(!sift_down_(i, n)) {
            sift_up_(i);
        }
    }
    /* 队尾元素删除 */
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::adjust(int id, int time_out) {
    /* 调整指定id的结点 */
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(time_out);
    sift_down_(ref_[id], heap_.size());
}

void HeapTimer::tick() {
    /* 清除超时结点 */
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimerNode node = heap_.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break;  //  还未超时
        }
        node.cb();  //  调用超时回调函数
        pop();
    }
}

void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);
}

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

int HeapTimer::get_next_tick() {
    tick();
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;     //  返回下一个距离最近的超时节点
}
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <list>
#include "locker.h"
#include <exception>
#include <cstdio>

template <typename T>
class threadpool
{
public:
    // 构造函数
    threadpool(int thread_number = 8,int max_requests = 10000);

    // 析构函数
    ~threadpool();

    // 添加任务
    bool append(T* request);

private:
    // 回调函数，工作线程
    static void* worker(void* arg);

    // 开始执行线程池
    void run();
  
private:
    // 线程的数量
    int m_thread_number;

    // 线程池数组，大小为m_thread_number
    pthread_t *m_threads;

    // 请求队列最多允许的，等待处理的请求数量
    int m_max_requests;

    // 请求队列
    std::list<T *> m_workqueue;

    // 互斥锁
    locker m_queuelocker;

    // 信号量用来判断是否有任务需要处理
    sem m_queuestat;

    // 是否结束线程
    bool m_stop;
};

#endif
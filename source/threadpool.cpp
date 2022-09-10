#include "threadpool.h"

// 工作线程
template<typename T>
void* threadpool<T>::worker(void* arg)
{
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}

// 构造函数
template<typename T>
threadpool<T>::threadpool(int thread_number,int max_requests):m_thread_number(thread_number),
m_max_requests(max_requests),m_stop(false),m_threads(NULL) // 初始化
{
    if((thread_number <= 0) || (max_requests <= 0)) // 初始化给的数值不正确，抛出异常
    {
        throw std::exception();
    }

    // 创建线程池数组
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads) // 创建不成功
    {
        throw std::exception();
    }

    // 创建thread_number个线程，并将它们设置为线程脱离
    for(int i = 0; i < thread_number; ++i)
    {
        printf("create the %sth thread\n",i);
        
        if(pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            // 创建失败
            delete[] m_threads;
            throw std::exception();
        }

        if(pthread_detach(m_threads[i]) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

// 析构函数
template<typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
    m_stop = true;
}

// 添加任务
template<typename T>
bool threadpool<T>::append(T* request)
{
    // 操作工作队列时一定要加锁，因为它被所有线程所共享
    m_queuelocker.lock();
    if(m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }

    m_workqueue.push_back(request);
    m_queuestat.post();
    m_queuelocker.unlock();
    return true;
}



// 开始执行线程池
template<typename T>
void threadpool<T>::run()
{
    while(!m_stop)
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }

        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();

        if(!request)
        {
            continue;
        }

        request->process();
    }
}



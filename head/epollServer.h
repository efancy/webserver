#ifndef EPOLLSERVER_H
#define EPOLLSERVER_H

#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_EVENT_NUMBER 10000

class epollServer
{
public:
    epollServer();

    ~epollServer();

    // 添加fd到m_epollfd
    void addfd(int fd, bool one_shot);

    // 从m_epollfd中移除fd
    void removefd(int fd);

    // 设置fd非阻塞
    int setnoblocking(int fd);

    // 修改文件描述符,重置socket上的EPOLLONESHOT事件，确保下一次数据可读时，EPOLLIN事件能被触发
    void modfd(int fd,int ev);

    // 测试m_epollfd
    int start();

private:
    struct epoll_event events[MAX_EVENT_NUMBER];
    int m_epollfd;
};

#endif
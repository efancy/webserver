#ifndef EPOLLSERVER_H
#define EPOLLSERVER_H

#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "tcpServer.h"

#define MAX_EVENT_NUMBER 10000

// 添加fd到m_epollfd
extern void addfd(int fd, bool one_shot);

// 从m_epollfd中移除fd
extern void removefd(int fd);

// 修改文件描述符,重置socket上的EPOLLONESHOT事件，确保下一次数据可读时，EPOLLIN事件能被触发
extern void modfd(int fd, int ev);

// 设置fd非阻塞
extern int setnoblocking(int fd);

class epollServer
{
public:
    epollServer();


    ~epollServer();

   

    // 测试m_epollfd
    int start();

    // 获取事件
    epoll_event *getevents();

    static int m_epollfd; // 所有的socket上的事件都被注册到同一个epoll内核事件中，所有设置成静态

private:
    struct epoll_event events[MAX_EVENT_NUMBER];
};

#endif
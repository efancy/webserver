#include "epollServer.h"

epollServer::epollServer()
{
    this->m_epollfd = 0;
    this->m_epollfd = epoll_create(8);
}

epollServer::~epollServer()
{
}

// 添加fd到m_epollfd
void epollServer::addfd(int fd, bool one_shot)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;

    // 将事件设置为oneshot 防止同一个通信被不同的线程处理
    if (one_shot)
    {
        event.events |= EPOLLONESHOT;
    }

    int ret = epoll_ctl(this->m_epollfd, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1)
    {
        perror("epoll_ctl");
        exit(-1);
    }

    // 将文件描述符设置为非阻塞
    setnoblocking(fd);
}

// 从m_epollfd中移除fd
void epollServer::removefd(int fd)
{
    int ret = epoll_ctl(this->m_epollfd, EPOLL_CTL_DEL, fd, 0);
    if (ret == -1)
    {
        perror("epoll_ctl");
        exit(-1);
    }

    close(fd);
}

// 设置fd非阻塞
int epollServer::setnoblocking(int fd)
{
    int oldFlags = fcntl(fd, F_GETFL);
    int newFlags = oldFlags | O_NONBLOCK;
    fcntl(fd, F_SETFL, newFlags);

    return oldFlags;
}

// 修改文件描述符,重置socket上的EPOLLONESHOT事件，确保下一次数据可读时，EPOLLIN事件能被触发
void epollServer::modfd(int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    int ret = epoll_ctl(this->m_epollfd,EPOLL_CTL_MOD,fd,&event);
    if(ret == -1)
    {
        perror("epoll_ctl");
        exit(-1);
    }
}

// 测试m_epollfd
int epollServer::start()
{
    int number = epoll_wait(this->m_epollfd,this->events,MAX_EVENT_NUMBER,-1);
    if(number == -1)
    {
        perror("epoll_wait");
        exit(-1);
    }
}
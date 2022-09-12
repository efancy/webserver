#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <string.h>
#include <assert.h>
#include "threadpool.h"
#include "epollServer.h"
#include "tcpServer.h"
#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "conn_http.h"

#define MAX_FD 65536 // 最大客户端个数

void addSig(int sig, void(handler)(int))
{
    struct sigaction sa;
    bzero(&sa, sizeof(sa)); // 初始化
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);

    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("usage: %s port_number\n", basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);
    addSig(SIGPIPE, SIG_IGN);

    threadpool<conn_http>* pool = NULL;
    try
    {
        pool = new threadpool<conn_http>;
    }
    catch (...)
    {
        return 1;
    }

    // 创建监听socket
    tcpServer *listenfd = NULL;
    try
    {
        listenfd = new tcpServer(port);
    }
    catch (...)
    {
        return 1;
    }

    listenfd->start(); // 创建完成

    // 客户端个数
    conn_http *users = new conn_http[MAX_FD];

    // 将监听socket添加到epoll对象中
    epollServer *epoll = NULL;
    try
    {
        epoll = new epollServer;
    }
    catch (...)
    {
        return 1;
    }

    epoll->addfd(listenfd->getListenfd(), false); // 添加

    while (true)
    {
        // 检测 判断
        int number = epoll->start();
        if ((number < 0) && (errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }

        for (int i = 0; i < number; i++)
        {
            int sockfd = epoll->getevents()[i].data.fd;
            int lisfd = listenfd->getListenfd();
            if (sockfd == lisfd)
            {
                // 有客户端进来
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int connfd = accept(lisfd, (struct sockaddr *)&client_addr, &len);
                if (connfd < 0)
                {
                    printf("erron is: %d\n", errno);
                    continue;
                }
                if (conn_http::m_user_count >= MAX_FD)
                {
                    close(connfd);
                    continue;
                }
                listenfd->reuseAddr(connfd);             // 端口复用
                users[connfd].init(connfd, client_addr); // 初始化连接
                epoll->addfd(connfd, true);              // 添加进epoll
            }
            else if (epoll->getevents()[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 关闭连接
                if (users[sockfd].getSocket() == -1)
                {
                    epoll->removefd(sockfd);
                    users[sockfd].setSocket(-1);
                    conn_http::m_user_count--;
                }
            }
            else if (epoll->getevents()[i].events & EPOLLIN)
            {
                if (users[sockfd].read()) // 一次读完数据
                {
                    pool->append(users + sockfd);
                }
                else
                {
                    // 关闭连接
                    if (users[sockfd].getSocket() == -1)
                    {
                        epoll->removefd(sockfd);
                        users[sockfd].setSocket(-1);
                        conn_http::m_user_count--;
                    }
                }
            }
            else if (epoll->getevents()[i].events & EPOLLOUT)
            {
                if (!users[sockfd].write()) // 一次读完数据
                {
                    // 关闭连接
                    if (users[sockfd].getSocket() == -1)
                    {
                        epoll->removefd(sockfd);
                        users[sockfd].setSocket(-1);
                        conn_http::m_user_count--;
                    }
                }
            }
        }
    }

    close(epoll->m_epollfd);
    close(listenfd->getListenfd());
    delete [] users;
    delete pool;
    return 0;
}
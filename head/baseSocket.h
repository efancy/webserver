#ifndef BASESOCKET_H
#define BASESOCKET_H

#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

class baseSocket
{
public:
    baseSocket();

    ~baseSocket();

    // 获取监听文件描述符
    int getListenfd();

    // 开始创建监听文件描述符
    void start();

    // 纯虚函数，在子类中去实现
    virtual void socketRun() = 0;
    virtual void socketStop() = 0;
protected:
    int m_listen_fd;
};





#endif
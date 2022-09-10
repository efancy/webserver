#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "baseSocket.h"
#include "hostAddress.h"

class tcpServer : public baseSocket
{
public:
    tcpServer(unsigned short port);
    ~tcpServer();

    // 绑定本地地址信息
    void socketRun();

    // 关闭监听文件描述符
    void socketStop();

    // 设置本地地址
    void setAddress(hostAddress* address);

    // 给文件描述符添加端口复用
    void reuseAddr(int fd);

    // 获取本地地址
    hostAddress* getAddress();

private:
    hostAddress* m_address;
};


#endif
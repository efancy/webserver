#ifndef HOSTADDRESS_H
#define HOSTADDRESS_H

#include<arpa/inet.h>

#define LISTEN_MAX_NUM 5

class hostAddress
{
public:
    hostAddress(unsigned short port);

    ~hostAddress();

    // 设置本地端口
    void setPort(unsigned short port);

    // 获取本地端口
    unsigned short getPort();

    // 获取本地地址信息的长度
    socklen_t getLen();

    // 获取本地信息
    struct sockaddr_in getAddr_in();

    struct sockaddr* getAddr();

private:
    unsigned short m_port;
    socklen_t len;
    struct sockaddr_in server_address;
};



#endif
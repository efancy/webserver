#include "hostAddress.h"

hostAddress::hostAddress(unsigned short port)  // 初始化本地地址信息
{
    this->m_port = htons(port);

    this->server_address.sin_family = AF_INET;
    this->server_address.sin_port = this->m_port;
    this->server_address.sin_addr.s_addr = INADDR_ANY;

    this->len = sizeof(this->server_address);
}

hostAddress::~hostAddress()
{

}

void hostAddress::setPort(unsigned short port) // 设置本地端口号
{
    this->m_port = htons(port);
}

unsigned short hostAddress::getPort() // 获取端口号
{
    return this->m_port;
}

struct sockaddr_in hostAddress::getAddr_in() // 获取本地地址信息
{
    return this->server_address;
}

struct sockaddr* hostAddress::getAddr()
{
    return (struct sockaddr*)&this->server_address;
}

socklen_t hostAddress::getLen() // 获取本地地址信息长度
{
    return this->len;
}
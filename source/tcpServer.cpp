#include "tcpServer.h"

tcpServer::tcpServer(unsigned short port) // 调用本地地址信息类 传入端口初始化本地地址信息
{
    this->m_address = new hostAddress(port);
    if(!this->m_address)
    {
        throw std::exception();
    }
}

tcpServer::~tcpServer()
{

}

void tcpServer::setAddress(hostAddress* address) // 设置本地地址信息
{
    this->m_address = address;
}

hostAddress* tcpServer::getAddress() // 获取本地地址信息
{
    return this->m_address;
}

// 设置端口复用
void tcpServer::reuseAddr(int fd)
{
    int optv = 1;
    int ret = setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optv,sizeof(optv));
    if(ret == -1)
    {
        perror("setsockopt");
        exit(-1);
    }
}

void tcpServer::socketRun() // 完成socket绑定及监听
{
    // 端口复用
    this->reuseAddr(this->m_listen_fd);

    // 绑定
    int ret = bind(this->m_listen_fd,this->m_address->getAddr(),this->m_address->getLen());
    if(ret == -1)
    {
        perror("bind");
        exit(-1);
    }

    // 监听
    ret = listen(this->m_listen_fd,LISTEN_MAX_NUM);
    if(ret == -1)
    {
        perror("listen");
        exit(-1);
    }
}

void tcpServer::socketStop() // 关闭监听文件描述符
{
    if(this->getListenfd() != 0)
    {
        close(this->getListenfd());
        this->m_listen_fd = 0;
    }
}
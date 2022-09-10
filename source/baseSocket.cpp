#include "baseSocket.h"

baseSocket::baseSocket()
{
    this->m_listen_fd = 0; // 初始化为0
}

baseSocket::~baseSocket()
{

}

// 获取监听文件描述符
int baseSocket::getListenfd()
{
    return this->m_listen_fd;
}

// 创建监听文件描述符
void baseSocket::start()
{
    this->m_listen_fd = socket(PF_INET,SOCK_STREAM,0);
    if(this->m_listen_fd == -1)
    {
        perror("socket");
        exit(-1);
    }

    // 继续操作绑定、监听等
    this->socketRun();
}
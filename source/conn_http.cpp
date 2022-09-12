#include "conn_http.h"

int conn_http::m_user_count = 0;

// 初始化连接

conn_http::conn_http()
{
    this->m_socket = -1;
}

conn_http::~conn_http(){};

void conn_http::init(int sockfd, const sockaddr_in &addr)
{
    this->m_socket = sockfd;
    this->m_client_addr = addr;

    this->m_user_count++;
    this->init();
}

void conn_http::init()
{

}

int conn_http::getSocket()
{
    return this->m_socket;
}

void conn_http::setSocket(int fd)
{
    this->m_socket = fd;
}

bool conn_http::read() // 非阻塞的读
{
    printf("一次性读完所有数据\n");
    return true;
}

bool conn_http::write() // 非阻塞的写
{
    printf("一次性写完所有数据\n");
    return true;
}

// 有线程池中的工作线程调用，这是处理HTTP请求的函数入口
void conn_http::process() // 处理客户端请求
{
    // 解析HTTP请求

    printf("parse request, create response\n");

    // 生成响应
}
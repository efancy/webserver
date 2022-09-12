#ifndef CONN_HTTP_H
#define CONN_HTTP_H

#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

class conn_http
{
public:
    conn_http();
    ~conn_http();

    // 初始化连接
    void init(int sockfd, const sockaddr_in &addr);

    static int m_user_count; // 统计用户的数量

    void init();

    int getSocket();

    void setSocket(int fd);

    void process(); // 处理客户端请求

    bool read(); // 非阻塞的读

    bool write(); // 非阻塞的写

    // 关闭连接
    void close_conn();

private:
    sockaddr_in m_client_addr;
    int m_socket; // 该HTTP连接的socket和对方的socket地址
};

#endif
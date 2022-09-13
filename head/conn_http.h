#ifndef CONN_HTTP_H
#define CONN_HTTP_H

#include <arpa/inet.h>
#include <errno.h>
#include "epollServer.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

class conn_http
{
public:
    conn_http();
    ~conn_http();

    static const int READ_BUFFER_SIZE = 2048; // 读缓存区的大小
    static const int WRITE_BUFFER_SIZE = 1024; // 写缓存区的大小

    // HTTP的请求方法，但我们只支持GET
    enum METHOD{GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};

    /* 解析客户端请求时，主状态机的状态
    CHECK_STATE_REQUESTLINE:当前正在分析请求行
    CHECK_STATE_HEADER:当前正在分析头部字段
    CHECK_STATE_CONTENT:正在解析请求体 */
    enum CHECK_STATE{CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT};

    /* 从状态机的三种可能状态， 即行的读取状态，分别表示
    LINE_OK:读取到一个完整的行
    LINE_BAD:行出错
    LINE_OPEN:行数据尚且不完整 */
    enum LINE_STATUS{LINE_OK = 0, LINE_BAD, LINE_OPEN};

    /* 服务器处理HTTP请求的可能结果，报文解析的结果
    NO_REQUEST             :     请求不完整，需要继续读取客户数据
    GET_REQUEST            :     获得了一个完整的客户请求
    BAD_REQUEST            :     客户端请求语法错误
    NO_RESOURCE            :     服务器没有资源
    FORBIDDEN_REQUEST      :     客户对资源没有访问的权限
    FILE_REQUEST           :     文件请求，获取文件成功
    INTERNAL_ERROR         :     服务器内部错误
    CLOSED_CONNECTION      :     客户端已经关闭连接 */
    enum HTTP_CODE{NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_REQUEST, CLOSED_CONNECTION};

    
    // 初始化连接
    void init(int sockfd, const sockaddr_in &addr);

    static int m_user_count; // 统计用户的数量

    void init(); // 初始化状态机或标志位的信息

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
    char m_read_buf[READ_BUFFER_SIZE]; // 读缓存区
    int m_read_idx; // 记录读缓存区读取大小的下标
    epollServer m_epoll;

    int m_checked_idx; // 记录当前分析的字符处于缓存区的位置
    int m_start_line;  // 当前解析的行的起始位置
    char* m_url;       // 请求目标文件的文件名
    char* m_version;   // 协议版本，只支持HTTP1.1
    METHOD m_method;   // 请求方法
    char* m_host;      // 主机名
    bool m_connection; // 判断HTTP请求是否要保持连接
    int m_content_length; // 请求体的长度

    CHECK_STATE m_check_state; // 主状态机当前的状态

    HTTP_CODE process_read(); // 解析HTTP请求
    HTTP_CODE parse_request_line(char* text); // 解析请求首行
    HTTP_CODE parse_headers(char* text); // 解析请求头
    HTTP_CODE parse_content(char* text); // 解析请求体

    LINE_STATUS parse_line(); // 解析行

    char* get_line(){ return m_read_buf + m_start_line;} // 获取一行数据 内联函数

    HTTP_CODE  do_request();
};

#endif
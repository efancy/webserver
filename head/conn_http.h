#ifndef CONN_HTTP_H
#define CONN_HTTP_H

#include <arpa/inet.h>
#include <errno.h>
#include "epollServer.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/uio.h>



class conn_http
{
public:
    conn_http();
    ~conn_http();

    static const int READ_BUFFER_SIZE = 2048; // 读缓存区的大小
    static const int WRITE_BUFFER_SIZE = 1024; // 写缓存区的大小
    static const int FILENAME_LEN = 200; // 文件名最大长度

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
    char m_write_buf[WRITE_BUFFER_SIZE]; // 写缓存区
    int m_write_idx; // 记录写缓存区写入多少的下标

    int m_checked_idx; // 记录当前分析的字符处于缓存区的位置
    int m_start_line;  // 当前解析的行的起始位置
    char* m_url;       // 请求目标文件的文件名
    char* m_version;   // 协议版本，只支持HTTP1.1
    METHOD m_method;   // 请求方法
    char* m_host;      // 主机名
    bool m_connection; // 判断HTTP请求是否要保持连接
    int m_content_length; // 请求体的长度

    char m_real_file[FILENAME_LEN]; // 客户请求的目标文件的完整路径，其内容等于 doc_root + url，doc_root是网站根目录
    struct stat m_file_stat; // 保存文件属性
    char* m_file_address;  // 内存映射地址

    CHECK_STATE m_check_state; // 主状态机当前的状态

    // 下面这一组函数被process_read调用以分析HTTP请求
    HTTP_CODE process_read(); // 解析HTTP请求
    HTTP_CODE parse_request_line(char* text); // 解析请求首行
    HTTP_CODE parse_headers(char* text); // 解析请求头
    HTTP_CODE parse_content(char* text); // 解析请求体
    char* get_line(){ return m_read_buf + m_start_line;} // 获取一行数据 内联函数
    LINE_STATUS parse_line(); // 解析行
    HTTP_CODE  do_request(); // 将客户请求的文件资源创建内存映射，并将映射地址存储到m_file_address
    void unmap(); // 对内存映射区进行munmap操作


    // 这一组函数被process_write调用以填充HTTP应答。
    bool process_write(HTTP_CODE ret); // 填充HTTP应答
    bool add_response(const char* format, ...); // 添加响应到写缓存区
    bool add_content(const char* content); // 将消息体添加到响应 
    bool add_content_type();  // 将消息体类型添加到响应 被add_headers调用
    bool add_status_line(int status, const char* title); // 添加状态行到响应
    void add_headers(int content_length); // 添加头部字段
    bool add_content_length(int content_length); // 添加消息体长度到响应 被add_headers调用
    bool add_connection(); // 添加连接状态到响应 被add_headers调用
    bool add_blank_line(); // 添加空白行 被add_headers调用

    struct iovec m_iv[2]; // 我们将采用writev来执行写操作，所以定义下面两个成员，其中m_iv_count表示被写内存块的数量。
    int m_iv_count;

};

#endif
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

// 初始化状态机或标志位的信息
void conn_http::init()
{
    this->m_check_state = CHECK_STATE_REQUESTLINE; // 初始化为正在解析当前首行
    this->m_checked_idx = 0;                       // 正在解析的字符在缓存区的位置初始化为0
    this->m_start_line = 0;                        // 解析行的开始位置初始化为0
    this->m_read_idx = 0;                          // 读取缓存区数据开始的位置初始化为0

    this->m_url = 0;            // 请求目标文件的文件名
    this->m_version = 0;        // 协议版本，只支持HTTP1.1
    this->m_method = GET;       // 请求方法，初始化为GET
    this->m_host = 0;           // 主机名
    this->m_connection = false; // HTTP请求是否保持连接 初始化为false
    this->m_content_length = 0; // 将请求体长度初始化为0

    bzero(this->m_read_buf, READ_BUFFER_SIZE); // 将读缓存区清空
}

int conn_http::getSocket()
{
    return this->m_socket;
}

void conn_http::setSocket(int fd)
{
    this->m_socket = fd;
}

// 非阻塞的读
bool conn_http::read()
{
    // 一次性读取读缓存区
    if (this->m_read_idx >= READ_BUFFER_SIZE)
    {
        return false;
    }
    int read_byte = 0; // 定义一个变量用来记录一次循环所读取的数据
    while (true)
    {
        // 从m_read_buf+m_read_idx索引处开始保存数据，大小是READ_BUFFER_SIZE-m_read_idx
        read_byte = recv(this->m_socket, this->m_read_buf + this->m_read_idx, READ_BUFFER_SIZE - this->m_read_idx, 0);
        if (read_byte == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 没有数据
                break;
            }
            return false;
        }
        else if (read_byte == 0)
        {
            // 读取结束 对方关闭连接
            return false;
        }

        this->m_read_idx += read_byte;
    }

    printf("读取到到数据：%s\n", this->m_read_buf);

    return true;
}

// 解析HTTP请求
conn_http::HTTP_CODE conn_http::process_read()
{
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;

    char *text = 0;
    // 当解析状态为：当前正在解析请求体 并且 解析到一行完整的数据
    // 或者 获取到的解析行状态为已获取到完整行
    while (((this->m_check_state == CHECK_STATE_CONTENT) && (line_status == LINE_OK)) || ((line_status = this->parse_line()) == LINE_OK))
    {
        // 获取一行数据
        text = get_line();
        m_start_line = m_read_idx;

        printf("got 1 http line: %s\n", text);

        switch (m_check_state)
        {
        case CHECK_STATE_REQUESTLINE:
        {
            ret = parse_request_line(text);
            if (ret == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            break;
        }

        case CHECK_STATE_HEADER:
        {
            ret = parse_headers(text);
            if (ret == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            else if (ret == GET_REQUEST)
            {
                return do_request();
            }
        }

        case CHECK_STATE_CONTENT:
        {
            ret = parse_content(text);
            if (ret == GET_REQUEST)
            {
                return do_request();
            }
            line_status = LINE_OPEN; // 如果获取到一个完整的客户请求失败，说明数据不完整，改变状态
            break;
        }
        default:
        {
            return INTERNAL_REQUEST;
        }
        }
        return NO_REQUEST;
    }

    return NO_REQUEST;
}

conn_http::HTTP_CODE conn_http::do_request()
{
}

// 解析请求首行  获得请求方法，目标URL，HTTP版本
conn_http::HTTP_CODE conn_http::parse_request_line(char *text)
{
    //  GET / HTTP/1.1
    m_url = strpbrk(text, " \t"); // strpbrk函数比较两个字符串中是否有相同的字符，有的话返回第一个相同字符的指针
    if (!m_url)                   // 没有空格或者\t肯定有问题的不
    {
        return BAD_REQUEST;
    }

    //  GET\0/ HTTP/1.1
    *m_url++ = '\0';

    char *method = text;
    if (strcasecmp(method, "GET") == 0) // 仅支持GET方法
    {
        m_method = GET;
    }
    else
    {
        return BAD_REQUEST;
    }

    m_url += strspn(m_url, " \t"); // 排除首个字符就是空格或\t
    m_version = strpbrk(m_url, " \t");
    if (!m_version)
    {
        return BAD_REQUEST;
    }
    *m_version++ = '\0';
    m_version += strspn(m_version, " \t");      // 排除首个字符就是空格或\t
    if (strcasecmp(m_version, "HTTP/1.1") != 0) // 仅支持HTTP/1.1
    {
        return BAD_REQUEST;
    }

    // 检查URL是否合法     http://192.168.0.107:10000/index.html
    if (strncasecmp(m_url, "http://", 7) == 0)
    {
        m_url += 7;                 // 192.168.0.107:10000/index.html
        m_url = strchr(m_url, '/'); // /index.html
    }

    if (!m_url || m_url[0] != '/')
    {
        return BAD_REQUEST;
    }

    m_check_state = CHECK_STATE_HEADER; // 主状态机检测状态变更为检测请求头

    return NO_REQUEST;
}

// 解析请求头
conn_http::HTTP_CODE conn_http::parse_headers(char *text)
{
    // 遇到空行，表示头部字段解析完毕
    if (text[0] == '\0')
    {
        // 如果HTTP请求有消息体，则还需要读取m_conten_length字节消息体
        // 将状态机的状态转换到CHECK_STATE_CONTENT状态
        if (m_content_length != 0)
        {
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        // 如果没有消息体,说明已经得到了一个完整的请求
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        // 处理Host头部字段
        text += 5;
        text += strspn(text, " \t"); // 第一个字符跳过空格和\t
        m_host = text;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0)
    {
        // 解析Connection头部字段
        text += 11;
        text += strspn(text, " \t"); // 第一个字符跳过空格和\t
        if (strcasecmp(text, "keep-alive") == 0)
        {
            m_connection = true;
        }
    }
    else if (strncasecmp(text, "Content-Length:", 15) == 0)
    {
        // 解析Content-Length头部字段
        text += 15;
        text += strspn(text, " \t"); // 第一个字符跳过空格和\t
        m_content_length = atol(text);
    }
    else
    {
        printf("oop! unknow header %s\n", text);
    }
    return NO_REQUEST;
}

// 我们没有真正的HTTP请求的消息体，所以只判断它是否被完整的读入了
conn_http::HTTP_CODE conn_http::parse_content(char *text) // 解析请求体
{
    if(m_read_idx >= (m_checked_idx + m_content_length))
    {
        if(text[m_content_length] == '\0')
        {
            return GET_REQUEST;
        }
    }

    return NO_REQUEST;
}

// 解析一行，判断依据\r\n
conn_http::LINE_STATUS conn_http::parse_line() // 解析行
{
    char temp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx)
    {
        temp = m_read_buf[m_checked_idx]; // 获取到当前分析的字符
        // 如果当前字符是'\r'，即回车符，则说明可能取到一个完整的行
        if (temp == '\r')
        {
            // 如果'\r'碰巧是目前buf中最后一个被读入的客户数据，那么这次分析没有取到一个完整的行，需要继续读取数据
            if ((m_checked_idx + 1) == m_read_idx)
            {
                return LINE_OPEN;
                // 如果'\r'字符后面一个字符是'\n'，说明取到了一个完整的行
            }
            else if (m_read_buf[m_checked_idx + 1] == '\n')
            {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';

                return LINE_OK;
            }
            // 如果以上都不是，那就存在语法错误
            return LINE_BAD;
            // 如果当前字符是'\n',即换行符，则说明可能取到了一个完整的行
        }
        else if (temp == '\n')
        {
            // 因为'\r''\n'一起用，所以还要判断前一个字符是否为'\r'
            if ((m_checked_idx > 1) && (m_read_buf[m_checked_idx - 1] == '\r'))
            {
                // 前一个字符是'\r'的话，说明取到了一个完整行
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';

                return LINE_OK;
            }
            // 如果前一个字符不是'\r'，那就存在语法错误
            return LINE_BAD;
        }
    }

    return LINE_OPEN;
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
    HTTP_CODE read_ret = this->process_read();
    if (read_ret == NO_REQUEST) // 请求不完整，需要继续读取客户数据
    {
        m_epoll.modfd(this->m_socket, EPOLLIN);
        return;
    }

    // 生成响应
}
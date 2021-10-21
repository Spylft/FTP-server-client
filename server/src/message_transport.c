/*

*/
#include "command_define.h"
#include "command.h"
#include "file_transport.h"
#include "info_handle.h"
#include "message_transport.h"
#include "status.h"

/*
发送message信息至连接connection_id
参数：
    connection_id：连接ID
    message：信息字符串头
    len：信息长度
返回：
    0成功 1失败
*/
bool Write_Message(int connection_id, char *message)
{
    printf("Write %s\n", message);
    int len = strlen(message);
    int p = 0;
    while (p < len)
    {
        int n = write(connection_id, message + p, len - p);
        // printf("%d %d %d\n", p, n, len);
        if (n < 0)
        {
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return 1;
        }
        else
        {
            p += n;
            if (message[p - 1] == '\n' || message[p - 1] == '\r')
                break;
        }
    }
    return 0;
}

/*
连接cont读取信息，储存在cont的message字符串中
参数：
    cont：连接结构体
返回：
    0成功 1失败
*/
bool Read_Message(struct Connection *cont)
{
    puts("Begin read");
    int p = 0;
    while (1)
    {
        int n = read(cont->connection_id, cont->message + p, socket_maxlen - p);
        if (n < 0)
        {
            printf("Error read(): %s(%d)\n", strerror(errno), errno);
            return 1;
        }
        else if (n == 0)
        {
            break;
        }
        else
        {
            p += n;
            if (cont->message[p - 1] == '\n' || cont->message[p - 1] == '\r')
                break;
        }
    }
    while (cont->message[p - 1] == '\r' || cont->message[p - 1] == '\n')
        p--;
    cont->message[p] = '\0';
    printf("Read %s %d\n", cont->message, p);
    return 0;
}
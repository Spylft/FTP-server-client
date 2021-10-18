#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

#include "command.h"
#include "file_transport.h"
#include "message_transport.h"
#include "status.h"

/*
检查该连接是否能进行RETR操作
参数：
    cont：连接结构体
返回：
    0成功 1失败
*/
bool RETR_Check(struct Connection *cont)
{
    if (cont->connection_mode == NONE_MODE)
    {
        char message_fail[30] = "503 Need PORT or PASV.\r\n";
        Write_Message(cont->connection_id, message_fail);
        return 1;
    }
    return 0;
}

/*
进行PORT连接
参数：
    cont：连接结构体
返回：
    0成功 1失败
*/
bool Connect_PORT(struct Connection *cont)
{
    struct sockaddr_in addr;
    if ((cont->connection_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cont->port);
    printf("connection %d: ip:port %s:%d\n", cont->connection_id, cont->ip, cont->port);
    if (inet_pton(AF_INET, cont->ip, &addr.sin_addr) <= 0)
    {
        printf("Error inet_pton(): %s:(%d)\n", strerror(errno), errno);
        return 1;
    }
    if (connect(cont->connection_data, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
    printf("connection %d: connect PORT success\n", cont->connection_id);
    return 0;
}

/*
RETR命令，将cont的message中的路径文件发送到客户端
参数：
    cont：连接结构体
*/
void Command_RETR(struct Connection *cont)
{
    if (RETR_Check(cont))
    {
        printf("connection %d: retr error\n", cont->connection_id);
        return;
    }
    printf("connection %d: start data connect\n", cont->connection_id);
    {
        char message_start_connect[30] = "150 Start connecting.\r\n";
        Write_Message(cont->connection_id, message_start_connect);
    }
    if (cont->connection_mode == PORT_MODE)
    {
        if (Connect_PORT(cont))
        {
            printf("connection %d: connect fail\n", cont->connection_id);
            char message_connect_fail[30] = "425 Can't open data connection.\r\n";
            Write_Message(cont->connection_id, message_connect_fail);
            return;
        }
        else
        {
            struct File_Transport_Info *info = Get_File_Transport_Info(cont);
            Close_connectionid(cont->connection_listen);
            cont->connection_mode = NONE_MODE;
            pthread_t data_transport_thread;
            pthread_create(&data_transport_thread, NULL, File_Transport, (void *)info);
            pthread_detach(data_transport_thread);
        }
    }
    else if (cont->connection_mode == PASV_MODE)
    {
    }
}

/*
文件传输
参数：
    info：文件传输所需信息
*/
void File_Transport(struct File_Transport_Info *info)
{
    // TODO
}

/*
新建文件传输所需信息
参数：
    cont：连接结构体
返回：
    info：文件传输所需信息
*/
struct File_Transport_Info *Get_File_Transport_Info(struct Connection *cont)
{
    // TODO
    struct File_Transport_Info *info = (struct File_Transport_Info *)malloc(sizeof(struct File_Transport_Info));

    return info;
}
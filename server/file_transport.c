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
进行PASV连接
参数：
    cont：连接结构体
返回：
    0成功 1失败
*/
bool Connect_PASV(struct Connection *cont)
{
    if ((cont->connection_data = accept(cont->connection_listen, NULL, NULL)) == -1)
    {
        printf("connection %d: pasv listen error\n", cont->connection_id);
        return 1;
    }
    printf("connection %d: connect PASV success\n", cont->connection_id);
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
    }
    else if (cont->connection_mode == PASV_MODE)
    {
        if (Connect_PASV(cont))
        {
            printf("connection %d: connect fail\n", cont->connection_id);
            char message_connect_fail[30] = "425 Can't open data connection.\r\n";
            Write_Message(cont->connection_id, message_connect_fail);
            return;
        }
    }
    else
    {
        char message_error[30] = "503 Need PORT or PASV command.\r\n";
        Write_Message(cont->connection_id, message_error);
        return;
    }

    struct File_Transport_Info *info = Get_File_Transport_Info(cont);
    if (info == NULL)
    {
        char message_trans_fail[30] = "504 Filepath too long.\r\n";
        Write_Message(cont->connection_id, message_trans_fail);
        return;
    }
    Close_connectionid(cont->connection_listen);
    cont->connection_mode = NONE_MODE;
    pthread_t data_transport_thread;
    pthread_create(&data_transport_thread, NULL, File_Transport, (void *)info);
    pthread_detach(data_transport_thread);
}

/*
打开位于dir的文件
参数：
    dir：文件地址
返回：
    文件结构体
*/
FILE *Get_File(char *dir)
{
}

/*
文件传输
参数：
    info：文件传输所需信息
*/
void File_Transport(void *info_)
{
    struct File_Transport_Info *info = (struct File_Transport_Info *)info_;
    FILE *file = Get_File(info->file_dir);
    if (file == NULL)
    {
        char message_trans_fail[30] = "451 File open error.\r\n";
        Write_Message(info->file_dir, message_trans_fail);
        close(info->connection_data);
        free(info);
        pthread_exit(0);
    }

    int index = fseek(file, 0, SEEK_SET);
    if (index != 0)
    {
        fclose(file);
        char message_trans_fail[30] = "451 File find error.\r\n";
        Write_Message(info->connection_id, message_trans_fail);
        close(info->connection_data);
        free(info);
        pthread_exit(0);
    }

    char Buffer[socket_maxlen];
    memset(Buffer, 0, sizeof(char) * socket_maxlen);
    while (!feof(file))
    {
        int len = fread(Buffer, sizeof(char), socket_maxlen, file);
        int sent_len = 0;
        while (sent_len < len)
        {
            int len_sent = write(info->connection_data, Buffer + sent_len, len - sent_len);
            if (len_sent < 0)
            {
                fclose(file);
                char message_trans_fail[30] = "426 Transport error.\r\n";
                Write_Message(info->connection_id, message_trans_fail);
                close(info->connection_data);
                free(info);
                pthread_exit(0);
            }
            else
            {
                sent_len += len_sent;
            }
        }
    }

    fclose(file);
    char message_trans_success[30] = "226 Transport success.\r\n";
    Write_Message(info->connection_id, message_trans_success);
    close(info->connection_data);
    free(info);
    pthread_exit(0);
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
    info->connection_id = cont->connection_id;
    info->connection_data = cont->connection_data;
    char *file_path = cont->message + 5;
    int len1 = strlen(cont->dir), len2 = strlen(file_path), len3 = strlen(root);
    // if (len1 + len2 > root_maxlen)
    // {
    //     free(info);
    //     printf("connection %d: error path\n", cont->connection_id);
    //     return NULL;
    // }
    if (file_path[0] == '/')
    {
        if (len3 + len2 > root_maxlen)
        {
            free(info);
            printf("connection %d: error path\n", cont->connection_id);
            return NULL;
        }
        strcpy(info->file_dir, root);
        strcpy(info->file_dir + len3, file_path);
        info->file_dir[len2 + len3] = '\0';
    }
    else
    {
        if (len1 + len2 > root_maxlen)
        {
            free(info);
            printf("connection %d: error path\n", cont->connection_id);
            return NULL;
        }
        strcpy(info->file_dir, cont->dir);
        strcpy(info->file_dir + len1, file_path);
        info->file_dir[len1 + len2] = '\0';
    }
    return info;
}
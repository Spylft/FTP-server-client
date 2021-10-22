#include "command_define.h"
#include "command.h"
#include "file_transport.h"
#include "info_handle.h"
#include "message_transport.h"
#include "status.h"

/*
检查该连接是否能进行文件传输操作
参数：
    cont：连接结构体
返回：
    0成功 1失败
*/
bool File_Transport_Check(struct Connection *cont)
{
    if (cont->connection_mode == NONE_MODE)
    {
        char message_fail[message_maxlen] = "503 Need PORT or PASV.\r\n";
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
    if (Connect_Data(cont))
    {
        char message_connect_fail[message_maxlen] = "425 Can't open data connection.\r\n";
        Write_Message(cont->connection_id, message_connect_fail);
        return;
    }

    struct File_Transport_Info *info = Get_File_Transport_Info(cont);
    if (info == NULL)
    {
        char message_trans_fail[message_maxlen] = "504 Filepath too long.\r\n";
        Write_Message(cont->connection_id, message_trans_fail);
        return;
    }
    Close_Connectionid(cont->connection_listen);
    cont->connection_mode = NONE_MODE;
    pthread_t data_transport_thread;
    pthread_create(&data_transport_thread, NULL, File_Transport_Data, (void *)info);
    pthread_detach(data_transport_thread);
}

/*
STOR命令，将客户端发送的文件接收到cont的message中的路径的文件里
参数：
    cont：连接结构体
*/
void Command_STOR(struct Connection *cont)
{
    if (Connect_Data(cont))
    {
        char message_connect_fail[message_maxlen] = "425 Can't open data connection.\r\n";
        Write_Message(cont->connection_id, message_connect_fail);
        return;
    }

    struct File_Transport_Info *info = Get_File_Transport_Info(cont);
    if (info == NULL)
    {
        char message_trans_fail[message_maxlen] = "504 Filepath too long.\r\n";
        Write_Message(cont->connection_id, message_trans_fail);
        return;
    }
    Close_Connectionid(cont->connection_listen);
    cont->connection_mode = NONE_MODE;
    pthread_t data_transport_thread;
    pthread_create(&data_transport_thread, NULL, File_Transport_Receive, (void *)info);
    pthread_detach(data_transport_thread);
}

/*
LIST命令，将cont的message中的路径或文件的信息发送到客户端
参数：
    cont：连接结构体
*/
void Command_LIST(struct Connection *cont)
{
    if (Connect_Data(cont))
    {
        char message_connect_fail[message_maxlen] = "425 Can't open data connection.\r\n";
        Write_Message(cont->connection_id, message_connect_fail);
        return;
    }

    struct File_Transport_Info *info = Get_File_Transport_Info(cont);
    if (info == NULL)
    {
        char message_trans_fail[message_maxlen] = "504 Filepath too long.\r\n";
        Write_Message(cont->connection_id, message_trans_fail);
        return;
    }
    char message_list[message_maxlen] = "226 LIST success.\r\n";
    Write_Message(cont->connection_id, message_list);
    Close_Connectionid(cont->connection_listen);
    cont->connection_mode = NONE_MODE;
    pthread_t data_transport_thread;
    pthread_create(&data_transport_thread, NULL, LIST_Transport, (void *)info);
    pthread_detach(data_transport_thread);
}

/*
向info传输file内的内容
参数：
    info：传输参数
    file：传输内容
返回：
    0成功 1失败
*/
bool File_Transport(struct File_Transport_Info *info, FILE *file)
{
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
                char message_trans_fail[message_maxlen] = "426 Transport error.\r\n";
                Write_Message(info->connection_id, message_trans_fail);
                close(info->connection_data);
                free(info);
                return 1;
            }
            else
            {
                sent_len += len_sent;
            }
        }
    }
    return 0;
}

/*
文件传输
参数：
    info：文件传输所需信息
*/
void *File_Transport_Data(void *info_)
{
    struct File_Transport_Info *info = (struct File_Transport_Info *)info_;
    puts(info->file_dir);
    FILE *file = fopen(info->file_dir, "rb");
    if (file == NULL)
    {
        char message_trans_fail[message_maxlen] = "451 File open error.\r\n";
        Write_Message(info->connection_id, message_trans_fail);
        close(info->connection_data);
        free(info);
        pthread_exit(0);
    }

    // int index = fseek(file, 0, SEEK_SET);
    // if (index != 0)
    // {
    //     fclose(file);
    //     char message_trans_fail[message_maxlen] = "451 File find error.\r\n";
    //     Write_Message(info->connection_id, message_trans_fail);
    //     close(info->connection_data);
    //     free(info);
    //     pthread_exit(0);
    // }

    if (File_Transport(info, file))
        pthread_exit(0);

    fclose(file);
    char message_trans_success[message_maxlen] = "226 Data Transport success.\r\n";
    Write_Message(info->connection_id, message_trans_success);
    close(info->connection_data);
    free(info);
    pthread_exit(0);
    return NULL;
}

/*
文件传输
参数：
    info：文件传输所需信息
*/
void *File_Transport_Receive(void *info_)
{
    struct File_Transport_Info *info = (struct File_Transport_Info *)info_;
    FILE *file = fopen(info->file_dir, "wb+");
    if (file == NULL)
    {
        char message_trans_fail[message_maxlen] = "451 File open error.\r\n";
        Write_Message(info->connection_id, message_trans_fail);
        close(info->connection_data);
        free(info);
        pthread_exit(0);
    }

    // int index = fseek(file, 0, SEEK_SET);
    // if (index != 0)
    // {
    //     fclose(file);
    //     char message_trans_fail[message_maxlen] = "451 File find error.\r\n";
    //     Write_Message(info->connection_id, message_trans_fail);
    //     close(info->connection_data);
    //     free(info);
    //     pthread_exit(0);
    // }

    char Buffer[socket_maxlen];
    memset(Buffer, 0, sizeof(char) * socket_maxlen);
    while (1)
    {
        int len = read(info->connection_data, Buffer, socket_maxlen);
        if (len < 0)
        {
            fclose(file);
            char message_receive_fail[message_maxlen] = "426 Connection error.\r\n";
            Write_Message(info->connection_id, message_receive_fail);
            close(info->connection_data);
            free(info);
            pthread_exit(0);
        }
        else if (len == 0)
        {
            break;
        }
        fwrite(Buffer, sizeof(char), len, file);
    }

    fclose(file);
    char message_trans_success[message_maxlen] = "226 Data Receive success.\r\n";
    Write_Message(info->connection_id, message_trans_success);
    close(info->connection_data);
    free(info);
    pthread_exit(0);
}

/*
向客户端传输目录下所有文件信息或者单个文件信息
参数：
    info：传输信息
 */
void *LIST_Transport(void *info_)
{
    struct File_Transport_Info *info = (struct File_Transport_Info *)info_;

    char cmd[root_maxlen] = "ls -l ";
    strcat(cmd, info->file_dir);
    FILE *file = popen(cmd, "r");
    if (file == NULL)
    {
        printf("connection %d: popen fail\n", info->connection_id);
        char message_popen_fail[message_maxlen] = "451 Can't get file list.\r\n";
        Write_Message(info->connection_id, message_popen_fail);
        close(info->connection_data);
        free(info);
        pthread_exit(0);
    }

    if (File_Transport(info, file))
        pthread_exit(0);

    pclose(file);
    char message_trans_success[message_maxlen] = "226 LIST Transport success.\r\n";
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
    if (strlen(cont->message) == 4)
    {
        if (len1 + len3 + 1 > root_maxlen)
        {
            free(info);
            printf("connection %d: error path\n", cont->connection_id);
            return NULL;
        }
        sprintf(info->file_dir, "%s/%s", root, cont->dir);
    }
    // if (len1 + len2 > root_maxlen)
    // {
    //     free(info);
    //     printf("connection %d: error path\n", cont->connection_id);
    //     return NULL;
    // }
    if (file_path[0] == '/')
    {
        if (len3 + len2 + 1 > root_maxlen)
        {
            free(info);
            printf("connection %d: error path\n", cont->connection_id);
            return NULL;
        }
        sprintf(info->file_dir, "%s/%s", root, file_path);
    }
    else
    {
        if (len1 + len2 + len3 + 2 > root_maxlen)
        {
            free(info);
            printf("connection %d: error path\n", cont->connection_id);
            return NULL;
        }
        sprintf(info->file_dir, "%s/%s/%s", root, cont->dir, file_path);
    }
    return info;
}

/*
连接cont的数据连接
参数：
    cont：连接结构体
返回：
    0成功 1失败
*/
bool Connect_Data(struct Connection *cont)
{
    if (File_Transport_Check(cont))
    {
        printf("connection %d: connect error\n", cont->connection_id);
        return 1;
    }
    printf("connection %d: start data connect\n", cont->connection_id);
    {
        char message_start_connect[message_maxlen] = "150 Start connecting.\r\n";
        Write_Message(cont->connection_id, message_start_connect);
    }
    if (cont->connection_mode == PORT_MODE)
    {
        if (Connect_PORT(cont))
        {
            printf("connection %d: connect fail\n", cont->connection_id);
            char message_connect_fail[message_maxlen] = "425 Can't open data connection.\r\n";
            Write_Message(cont->connection_id, message_connect_fail);
            return 1;
        }
    }
    else if (cont->connection_mode == PASV_MODE)
    {
        if (Connect_PASV(cont))
        {
            printf("connection %d: connect fail\n", cont->connection_id);
            char message_connect_fail[message_maxlen] = "425 Can't open data connection.\r\n";
            Write_Message(cont->connection_id, message_connect_fail);
            return 1;
        }
    }
    return 0;
}
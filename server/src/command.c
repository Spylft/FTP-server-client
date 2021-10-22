#include "command_define.h"
#include "command.h"
#include "file_transport.h"
#include "info_handle.h"
#include "message_transport.h"
#include "status.h"

/*
输入命令字符串，处理返回命令对应id
参数：
    command：命令字符串
返回：
    0：命令不合法
    命令id
*/
int Identify_Commandid(char *command)
{
    int len = 0;
    while (command[len] != ' ' && command[len] != '\0')
        len++;
    if (len == 3)
    {
        if (command[0] == 'M' && command[1] == 'K' && command[2] == 'D')
            return MKD;
        else if (command[0] == 'C' && command[1] == 'W' && command[2] == 'D')
            return CWD;
        else if (command[0] == 'P' && command[1] == 'W' && command[2] == 'D')
            return PWD;
        else if (command[0] == 'R' && command[1] == 'M' && command[2] == 'D')
            return RMD;
    }
    else if (len == 4)
    {
        if (command[0] == 'U' && command[1] == 'S' && command[2] == 'E' && command[3] == 'R')
            return USER;
        else if (command[0] == 'P' && command[1] == 'A' && command[2] == 'S' && command[3] == 'S')
            return PASS;
        else if (command[0] == 'R' && command[1] == 'E' && command[2] == 'T' && command[3] == 'R')
            return RETR;
        else if (command[0] == 'S' && command[1] == 'T' && command[2] == 'O' && command[3] == 'R')
            return STOR;
        else if (command[0] == 'Q' && command[1] == 'U' && command[2] == 'I' && command[3] == 'T')
            return QUIT;
        else if (command[0] == 'S' && command[1] == 'Y' && command[2] == 'S' && command[3] == 'T')
            return SYST;
        else if (command[0] == 'T' && command[1] == 'Y' && command[2] == 'P' && command[3] == 'E')
            return TYPE;
        else if (command[0] == 'P' && command[1] == 'O' && command[2] == 'R' && command[3] == 'T')
            return PORT;
        else if (command[0] == 'P' && command[1] == 'A' && command[2] == 'S' && command[3] == 'V')
            return PASV;
        else if (command[0] == 'L' && command[1] == 'I' && command[2] == 'S' && command[3] == 'T')
            return LIST;
        else if (command[0] == 'R' && command[1] == 'N' && command[2] == 'F' && command[3] == 'R')
            return RNFR;
        else if (command[0] == 'R' && command[1] == 'N' && command[2] == 'T' && command[3] == 'O')
            return RNTO;
    }
    return 0;
}
/*
处理server创建参数（端口，根目录地址）
参数：
    argc：参数个数
    argv：参数字符串
返回：
    0：读取成功
    1：读取失败
*/
bool Decode_Arg(int argc, char *argv[])
{
    char a_port[10] = "-port";
    char a_root[10] = "-root";
    char default_root[10] = "/tmp";
    root = (char *)calloc(root_maxlen, sizeof(char));
    if (argc == 1)
    {
        port = 21;
        strcpy(root, default_root);
        return 0;
    }
    else if (argc == 3)
    {
        if (!strcmp(a_port, argv[1]))
        {
            port = atoi(argv[2]);
            strcpy(root, default_root);
            return 0;
        }
        else if (!strcmp(a_root, argv[1]))
        {
            port = 21;
            strcpy(root, argv[2]);
            return 0;
        }
    }
    else if (argc == 5)
    {
        if (!strcmp(a_port, argv[1]) && !strcmp(a_root, argv[3]))
        {
            port = atoi(argv[2]);
            strcpy(root, argv[4]);
            return 0;
        }
        else if (!strcmp(a_root, argv[1]) && !strcmp(a_port, argv[3]))
        {
            port = atoi(argv[4]);
            strcpy(root, argv[2]);
            return 0;
        }
    }
    return 1;
}

/*
新建地址
参数：
    cont：连接结构体
返回：
    0成功 -1失败
 */
bool Make_Dir(struct Connection *cont)
{
    char cmd[root_maxlen * 3 + 30];
    sprintf(cmd, "mkdir -p %s/%s/%s", root, cont->dir, cont->message + 4);
    return system(cmd);
}

/*
转换目录
参数：
    cont：连接结构体
*/
void Command_CWD(struct Connection *cont)
{
    char path[root_maxlen * 3 + 30];
    if (cont->message[4] != '/')
    {
        sprintf(path, "%s/%s/%s", root, cont->dir, cont->message + 4);
    }
    else
    {
        sprintf(path, "%s/%s", root, cont->message + 4);
    }
    if ((cont->message[4] == '.' && cont->message[5] == '.' && cont->message[6] == '\0') || (access(path, 0)))
    {
        printf("connection %d: cwd fail\n", cont->connection_id);
        char message_cwd_fail[message_maxlen] = "451 Path error.\r\n";
        Write_Message(cont->connection_id, message_cwd_fail);
    }
    else
    {
        if (cont->message[4] != '/')
            strcat(cont->dir, cont->message + 4);
        else
            strcpy(cont->dir, cont->message + 4);
        printf("connection %d: dir:%s\n", cont->connection_id, cont->dir);
        char message_cwd_success[message_maxlen] = "250 CWD success.\r\n";
        Write_Message(cont->connection_id, message_cwd_success);
    }
}

/*
删除指定目录
参数：
    cont：连接结构体
*/
void Command_RMD(struct Connection *cont)
{
    char path[root_maxlen * 3 + 30];
    if (cont->message[4] != '/')
    {
        sprintf(path, "%s/%s/%s", root, cont->dir, cont->message + 4);
    }
    else
    {
        sprintf(path, "%s/%s", root, cont->message + 4);
    }
    if (access(path, 0))
    {
        printf("connection %d: rmd fail\n", cont->connection_id);
        char message_cwd_fail[message_maxlen] = "451 Path error.\r\n";
        Write_Message(cont->connection_id, message_cwd_fail);
    }
    else
    {
        rmdir(path);
        printf("connection %d: rmd:%s\n", cont->connection_id, path);
        char message_rmd_success[message_maxlen] = "250 RMD success.\r\n";
        Write_Message(cont->connection_id, message_rmd_success);
    }
}

/*
确定重命名文件/目录
参数：
    cont：连接结构体
*/
void Command_RNFR(struct Connection *cont)
{
    if (cont->message[5] == '/')
    {
        sprintf(cont->rename_dir, "%s/%s", root, cont->message + 5);
    }
    else
    {
        sprintf(cont->rename_dir, "%s/%s/%s", root, cont->dir, cont->message + 5);
    }
    if (access(cont->rename_dir, 0))
    {
        printf("connection %d: access path error\n", cont->connection_id);
        char message_rnfr_error[message_maxlen] = "550 File or directory can't access.\r\n";
        Write_Message(cont->connection_id, message_rnfr_error);
        return;
    }
    char message_rnfr_success[message_maxlen] = "350 RNFR okay.\r\n";
    Write_Message(cont->connection_id, message_rnfr_success);
}

/*
确定重命名文件/目录名称
参数：
    cont：连接结构体
*/
void Command_RNTO(struct Connection *cont)
{
    if (strlen(cont->message) <= 5)
    {
        printf("connection %d: parament error\n", cont->connection_id);
        char message_para_error[message_maxlen] = "504 Parament error.\r\n";
        Write_Message(cont->connection_id, message_para_error);
        return;
    }

    char cmd[root_maxlen * 5 + 30], nowpath[root_maxlen * 3 + 30];
    if (cont->message[5] == '/')
    {
        sprintf(nowpath, "%s/%s", root, cont->message + 5);
    }
    else
    {
        sprintf(nowpath, "%s/%s/%s", root, cont->dir, cont->message + 5);
    }
    sprintf(cmd, "rm %s %s", cont->rename_dir, nowpath);
    if (system(cmd))
    {
        printf("connection %d: rename fail\n", cont->connection_id);
        char message_rename_fail[message_maxlen] = "451 Fail to rename.\r\n";
        Write_Message(cont->connection_id, message_rename_fail);
    }
    else
    {
        printf("connection %d: rename success\n", cont->connection_id);
        char message_rename_success[message_maxlen] = "250 Rename success.\r\n";
        Write_Message(cont->connection_id, message_rename_success);
    }
}
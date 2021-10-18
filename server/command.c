#include <string.h>

#include "command.h"
#include "file_transport.h"
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
    while (command[len] != ' ' || command[len] != '\0')
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
    0：读取失败
    1：读取成功
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
        return 1;
    }
    else if (argc == 3)
    {
        if (!strcmp(a_port, argv[1]))
        {
            port = atoi(argv[2]);
            strcpy(root, default_root);
            return 1;
        }
        else if (!strcmp(a_root, argv[1]))
        {
            port = 21;
            strcpy(root, argv[2]);
            return 1;
        }
    }
    else if (argc == 5)
    {
        if (!strcmp(a_port, argv[1]) && !strcmp(a_root, argv[3]))
        {
            port = atoi(argv[2]);
            strcpy(root, argv[4]);
            return 1;
        }
        else if (!strcmp(a_root, argv[1]) && !strcmp(a_port, argv[3]))
        {
            port = atoi(argv[4]);
            strcpy(root, argv[2]);
            return 1;
        }
    }
    return 0;
}
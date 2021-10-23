#ifndef _STATUS_H_
#define _STATUS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <memory.h>
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <pthread.h>

#include "command_define.h"

extern int port;
extern int listenid;
extern char *root;
struct Connection
{
    int connection_id;     //连接ID
    int connection_mode;   //连接种类 0PORT 1PASV 初始为-1
    int connection_data;   //数据连接 若无连接则为-1
    int connection_listen; //被动模式监听连接 若无连接则为-1
    char message[socket_maxlen];
    char ip[ip_maxlen];
    int port;
    char dir[root_maxlen];
    char rename_dir[root_maxlen];
    int is_RNFR; //标识是否为RNFR操作
};
struct Connection *New_Connection(int connection_id);
bool Server_Init();
int Get_Connection();
void *Run_Connection(void *arg);
void Connection_Login(struct Connection *cont);
void Connection_Running(struct Connection *cont);
bool Set_IP_Port(int ip, int port, int mode, struct Connection *cont);
void Close_Connection(struct Connection *cont);
void Close_Connectionid(int id);
int Get_Random_Port();

#endif
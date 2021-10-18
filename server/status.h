#ifndef _STATUS_H_
#define _STATUS_H_

#include <stdbool.h>
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

#include "command.h"
#include "file_transport.h"
#include "message_transport.h"
#include "info_handle.h"

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
};
struct Connection *New_Connection(int connection_id);
bool Server_Init();
int Get_Connection();
void Run_Connection(void *arg);
void Connection_Login(struct Connection *cont);
void Connection_Runing(struct Connection *cont);
void Set_IP_Port(int ip, int port, int mode, struct Connection *cont);
void Close_connection(struct Connection *cont);

#endif
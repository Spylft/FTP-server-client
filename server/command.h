/*
    定义命令编号
*/
#ifndef _COMMAND_H_
#define _COMMAND_H_

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

#define USER 1
#define PASS 2
#define RETR 3
#define STOR 4
#define QUIT 5
#define SYST 6
#define TYPE 7
#define PORT 8
#define PASV 9
#define MKD 10
#define CWD 11
#define PWD 12
#define LIST 13
#define RMD 14
#define RNFR 15
#define RNTO 16
#define ABOR 17

#define socket_maxlen 8192
#define root_maxlen 300
#define ip_maxlen 30

#define PORT_MODE 0
#define PASV_MODE 1
#define NONE_MODE -1

extern int port;
extern char *root;

int Identify_Commandid(char *command);

bool Decode_Arg(int argc, char *argv[]);
bool Make_Dir(struct Connection *cont);
void Command_CWD(struct Connection *cont);
void Command_RMD(struct Connection *cont);
void Command_RNFR(struct Connection *cont);
void Command_RNTO(struct Connection *cont);

#endif
/*
    定义命令编号
*/
#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <stdbool.h>

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

#endif
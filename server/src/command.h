/*
    定义命令编号
*/
#ifndef _COMMAND_H_
#define _COMMAND_H_

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
#include <errno.h>
#include <pthread.h>

#include "status.h"

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
#ifndef _COMMAND_DEFINE_H_
#define _COMMAND_DEFINE_H_

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

#define socket_maxlen 4096
#define root_maxlen 300
#define ip_maxlen 30
#define message_maxlen 300

#define PORT_MODE 0
#define PASV_MODE 1
#define NONE_MODE -1

#endif
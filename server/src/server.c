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

#include "command_define.h"
#include "command.h"
#include "file_transport.h"
#include "info_handle.h"
#include "message_transport.h"
#include "status.h"

int main(int argc, char *argv[])
{
    if (Decode_Arg(argc, argv))
    {
        printf("arg invaild\n");
        return 0;
    }
    printf("root:%s\n", root);
    if (Server_Init())
    {
        return 0;
    }
    while (1)
    {
        int new_link = Get_Connection();
        if (new_link == -1)
            continue;
        pthread_t new_thread;
        pthread_create(&new_thread, NULL, Run_Connection, (void *)&new_link);
    }
    return 0;
}
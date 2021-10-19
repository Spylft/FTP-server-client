#ifndef _FILE_TRANSPORT_H_
#define _FILE_TRANSPORT_H_

#include "status.h"

struct File_Transport_Info
{
    char file_dir[root_maxlen];
    int connection_data;
    int connection_id;
};

struct File_Transport_Info *Get_File_Transport_Info(struct Connection *cont);
void Command_RETR(struct Connection *cont);
void Command_STOR(struct Connection *cont);
bool File_Transport_Check(struct Connection *cont);
bool Connect_PORT(struct Connection *cont);
bool Connect_PASV(struct Connection *cont);
void File_Transport(void *info_);
void File_Transport_Receive(void *info_);

#endif
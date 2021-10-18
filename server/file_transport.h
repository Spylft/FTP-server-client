#ifndef _FILE_TRANSPORT_H_
#define _FILE_TRANSPORT_H_

#include "status.h"

struct File_Transport_Info
{
    char dir[root_maxlen];
    char file_name[root_maxlen];
};

struct File_Transport_Info *Get_File_Transport_Info(struct Connection *cont);
void Command_RETR(struct Connection *cont);
bool RETR_Check(struct Connection *cont);
bool Connect_PORT(struct Connection *cont);
void File_Transport(struct File_Transport_Info *info);

#endif
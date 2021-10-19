#include "status.h"
#include "file_transport.h"
#include "message_transport.h"

int listenid;

/*
创建一个连接结构体，储存对应信息
参数：
    connection_id：连接ID
返回：
    一个指针，指向连接结构体
*/
struct Connection *New_Connection(int connection_id)
{
    struct Connection *ret = (struct Connection *)malloc(sizeof(struct Connection));
    ret->connection_id = connection_id;
    ret->connection_mode = -1;
    ret->connection_data = -1;
    ret->connection_listen = -1;
    ret->is_RNFR = 0;
    strcpy(ret->dir, root);
    return ret;
}

/*
初始化server
返回：
    成功返回0，失败返回1
*/
bool Server_Init()
{
    struct sockaddr_in addr;
    char sentence[socket_maxlen];

    if ((listenid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenid, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return 0;
    }

    if (listen(listenid, 10) == -1)
    {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return 0;
    }

    return 1;
}
/*
获得连接ID
返回：
    连接ID
*/
int Get_Connection()
{
    int id;
    while (1)
    {
        if (id = accept(listenid, NULL, NULL) == -1)
        {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            continue;
        }
        else
        {
            char message_welcome[30] = "220 Anonymous FTP server ready.\r\n";
            printf("connection %d: connected\n", id);
            Write_Message(id, message_welcome);
            break;
        }
    }
    return id;
}
/*
运行该连接（先登录，再进行其他操作）
*/
void Run_Connection(void *arg)
{
    int connection_id = *(int *)arg;
    struct Connection *cont = New_Connection(connection_id);
    Connection_Login(cont);
    Connection_Running(cont);
}

/*
连接进行登录操作
*/
void Connection_Login(struct Connection *cont)
{
    while (1)
    {
        Read_Message(cont);
        int command_id = Identify_Commandid(cont->message);
        if (command_id != USER)
        {
            printf("connection %d: Invalid command!\n", cont->connection_id);
            char message_unlogged[30] = "530 Not logged in.\r\n";
            Write_Message(cont->connection_id, message_unlogged);
        }
        else
        {
            char anonymous[30] = "anonymous";
            if (!strcmp(anonymous, cont->message + 5))
            {
                char message_askpassword[30] = "331 User name okay, need password.\r\n";
                Write_Message(cont->connection_id, message_askpassword);
                break;
            }
            else
            {
                printf("connection %d: Invalid user\n", cont->connection_id);
                char message_unpermitted[30] = "530 Need anonymous to login.\r\n";
                Write_Message(cont->connection_id, message_unpermitted);
                continue;
            }
        }
    }
    while (1)
    {

        Read_Message(cont);
        int command_id = Identify_Commandid(cont->message);
        if (command_id != PASS)
        {
            printf("connection %d: Need password\n", cont->connection_id);
            char message_login_failed[30] = "503 Need PASS command.\r\n";
            Write_Message(cont->connection_id, message_login_failed);
            continue;
        }
        else
        {
            printf("connection %d: login\n", cont->connection_id);
            char message_greeting[30] = "230 Login successful.\r\n";
            Write_Message(cont->connection_id, message_greeting);
            return;
        }
    }
}

/*
连接进行其他操作
*/
void Connection_Running(struct Connection *cont)
{
    while (1)
    {
        Read_Message(cont);
        int command_id = Identify_Commandid(cont->message);
        switch (command_id)
        {
        case USER:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: logged already\n", cont->connection_id);
            char message_logged[30] = "503 Logged already.\r\n";
            Write_Message(cont->connection_id, message_logged);
            break;
        }
        case PASS:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: logged already\n", cont->connection_id);
            char message_logged[30] = "503 Logged already.\r\n";
            Write_Message(cont->connection_id, message_logged);
            break;
        }

        case RETR:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: retr %s\n", cont->connection_id, cont->message + 5);
            Command_RETR(cont);
            break;
        }
        case STOR:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: stor %s\n", cont->connection_id, cont->message + 5);
            Command_STOR(cont);
            break;
        }
        case QUIT:
        {
            if (strlen(cont->message) > 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: logout\n", cont->connection_id);
            char message_QUIT[30] = "221 Goodbye.\r\n";
            Write_Message(cont->connection_id, message_QUIT);
            Close_connection(cont);
            close(cont->connection_id);
            free(cont);
            break;
        }
        case ABOR:
        {
            if (strlen(cont->message) > 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: logout\n", cont->connection_id);
            char message_QUIT[30] = "221 Goodbye.\r\n";
            Write_Message(cont->connection_id, message_QUIT);
            Close_connection(cont);
            close(cont->connection_id);
            free(cont);
            break;
        }
        case SYST:
        {
            if (strlen(cont->message) > 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            char message_SYST[30] = "215 UNIX Type: L8\r\n";
            Write_Message(cont->connection_id, message_SYST);
            break;
        }
        case TYPE:
        {
            if (cont->message[5] == 'I' && strlen(cont->message) == 6)
            {
                char message_type[30] = "200 Type set to I.\r\n";
                Write_Message(cont->connection_id, message_type);
            }
            else
            {
                char message_wrong_type[30] = "504 Error TYPE.\r\n";
                Write_Message(cont->connection_id, message_wrong_type);
            }
            break;
        }
        case PORT:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            long long ip_port = Get_IP_Port(cont->message);
            if (ip_port == -1)
            {
                printf("connection %d: parameter error\n", cont->connection_id);
                char message_parameter_error[30] = "504 Command not implemented for that parameter.\r\n";
                Write_Message(cont->connection_id, message_parameter_error);
                break;
            }
            Close_connection(cont);
            int port = ip_port >> 32;
            int ip = ip_port ^ (((long long)port) << 32);
            Set_IP_Port(ip, port, PORT_MODE, cont);
            printf("connection %d: ip:%s port:%d mode:port\n", cont->ip, cont->port);
            char message_port[30] = "200 Command PORT okay.\r\n";
            Write_Message(cont->connection_id, message_port);
            break;
        }
        case PASV:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            long long ip_port = Get_IP_Port(cont->message);
            if (ip_port == -1)
            {
                printf("connection %d: parameter error\n", cont->connection_id);
                char message_parameter_error[30] = "504 Command not implemented for that parameter.\r\n";
                Write_Message(cont->connection_id, message_parameter_error);
                break;
            }
            Close_connection(cont);
            int port = ip_port >> 32;
            int ip = ip_port ^ (((long long)port) << 32);
            if (Set_IP_Port(ip, port, PASV_MODE, cont))
            {
                printf("connection %d: pasv fail\n", cont->connection_id);
                char message_pasv_fail[30] = "425 PASV connection fail.\r\n";
                Write_Message(cont->connection_id, message_pasv_fail);
                break;
            }
            printf("connection %d: ip:%s port:%d mode:pasv\n", cont->ip, cont->port);
            char message_pasv[60] = "227 Entering Passive Mode (";
            int len = strlen(message_pasv);
            for (int i = 0; i < 4; i++)
            {
                int tmp = (ip >> ((3 - i) * 8)) & 255;
                Write_Digit(tmp, message_pasv, &len);
                message_pasv[len++] = ',';
            }
            int tmp = port >> 8;
            Write_Digit(tmp, message_pasv, &len);
            message_pasv[len++] = ',';
            tmp = port & 255;
            Write_Digit(tmp, message_pasv, &len);
            message_pasv[len++] = ')';
            message_pasv[len++] = '.';
            message_pasv[len++] = '\r';
            message_pasv[len++] = '\n';
            Write_Message(cont->connection_id, message_pasv);
            break;
        }
        case MKD:
        {
            if (strlen(cont->message) <= 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            if (!Make_Dir(cont))
            {
                printf("connection %d: mkd success\n", cont->connection_id);
                char message_mkd_success[root_maxlen];
                int beg = 4;
                char mid1 = cont->dir[strlen(cont->dir) - 1], mid2 = cont->message[4];
                if (mid1 == '/' && mid2 == '/')
                    beg++;
                if (mid1 != '/' && mid2 != '/')
                    beg--, cont->message[beg] = '/';
                sprintf(message_mkd_success, "257 \"%s%s\" directory created.\r\n", cont->dir, cont->message + beg);
                Write_Message(cont->connection_id, message_mkd_success);
            }
            else
            {
                printf("connection %d: mkd fail\n", cont->connection_id);
                char message_mkd_fail[30] = "451 Error make dir.\r\n";
                Write_Message(cont->connection_id, message_mkd_fail);
            }
            break;
        }
        case CWD:
        {
            if (strlen(cont->message) <= 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            Command_CWD(cont);
            break;
        }
        case PWD:
        {
            if (strlen(cont->message) <= 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: dir:%s\n", cont->connection_id, cont->dir);
            char message_PWD[root_maxlen];
            sprintf(message_PWD, "200 Current Path is %s", cont->dir);
            Write_Message(cont->connection_id, message_PWD);
            break;
        }
        case LIST:
        {
            Command_LIST(cont);
            break;
        }
        case RMD:
        {
            if (strlen(cont->message) <= 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            Command_RMD(cont);
            break;
        }
        case RNFR:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            Command_RNFR(cont);
            break;
        }
        case RNTO:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[30] = "504 Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            Command_RNTO(cont);
            break;
        }
        default:
        {
            char message_command_invaild[30] = "500 Command unrecognized.\r\n";
            printf("connection %d: command unrecognized\n", cont->connection_id);
            Write_Message(cont->connection_id, message_command_invaild);
            break;
        }
        }
    }
}

/*
获得一个随机端口
返回：
    随机端口值
*/
int Get_Random_Port()
{
    return 20000 + rand() % 45536;
}

/*
设置连接的ip与端口
参数：
    ip：ip数字
    port：端口数字
    mode：设置连接模式
    cont：连接结构体
返回：
    0成功 1失败
*/
bool Set_IP_Port(int ip, int port, int mode, struct Connection *cont)
{
    Close_Connection(cont);
    int len = 0;
    for (int i = 0; i < 4; i++)
        Write_Digit((ip >> ((3 - i) * 8)) & 255, cont->ip, &len), cont->ip[len++] = '.';
    cont->ip[len - 1] = '\0';
    cont->port = port;
    cont->connection_mode = mode;
    if (mode == PASV_MODE)
    {
        cont->port = Get_Random_Port();
        struct sockaddr_in Address;
        if ((cont->connection_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        {
            printf("Error socket(): %s(%d)\n", strerror(errno), errno);
            return 1;
        }

        memset(&Address, 0, sizeof(Address));
        Address.sin_family = AF_INET;
        Address.sin_port = htons(cont->port);

        if (bind(cont->connection_listen, (struct sockaddr *)&Address, sizeof(Address)))
        {
            printf("Error bind(): %s(%d)\n", strerror(errno), errno);
            return 1;
        }

        if (listen(cont->connection_listen, 10) == -1)
        {
            printf("Error listen(): %s(%d)\n", strerror(errno), errno);
            return 1;
        }
    }
    return 0;
}

/*
关闭连接id
参数：
    id：连接id
*/
void Close_connectionid(int id)
{
    if (id != -1)
    {
        close(id);
    }
}

/*
关闭连接的主动或者被动数据连接
参数：
    cont：连接结构体
*/
void Close_connection(struct Connection *cont)
{
    Close_connectionid(cont->connection_data);
    Close_connectionid(cont->connection_listen);
}
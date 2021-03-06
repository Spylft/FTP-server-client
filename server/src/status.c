#include "command_define.h"
#include "command.h"
#include "file_transport.h"
#include "info_handle.h"
#include "message_transport.h"
#include "status.h"

int port;
int listenid;
char *root;

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
    ret->dir[0] = '/';
    ret->dir[1] = '\0';
    // strcpy(ret->dir, root);
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
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // printf("%d\n", port);

    if (bind(listenid, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    if (listen(listenid, 10) == -1)
    {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    return 0;
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
        if ((id = accept(listenid, NULL, NULL)) == -1)
        {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            continue;
        }
        else
        {
            char message_welcome[message_maxlen] = "220 Anonymous FTP server ready.\r\n";
            printf("connection %d: connected\n", id);
            Write_Message(id, message_welcome);
            break;
        }
    }
    // printf("%d\n", id);
    return id;
}
/*
运行该连接（先登录，再进行其他操作）
*/
void *Run_Connection(void *arg)
{
    int connection_id = *(int *)arg;
    struct Connection *cont = New_Connection(connection_id);
    Connection_Login(cont);
    Connection_Running(cont);
    return NULL;
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
            char message_unlogged[message_maxlen] = "530 Not logged in.\r\n";
            Write_Message(cont->connection_id, message_unlogged);
        }
        else
        {
            char anonymous[message_maxlen] = "anonymous";
            if (!strcmp(anonymous, cont->message + 5))
            {
                char message_askpassword[message_maxlen] = "331 User name okay, need password.\r\n";
                Write_Message(cont->connection_id, message_askpassword);
                break;
            }
            else
            {
                printf("connection %d: Invalid user\n", cont->connection_id);
                char message_unpermitted[message_maxlen] = "530 Need anonymous to login.\r\n";
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
            char message_login_failed[message_maxlen] = "503 Need PASS command.\r\n";
            Write_Message(cont->connection_id, message_login_failed);
            continue;
        }
        else
        {
            printf("connection %d: login\n", cont->connection_id);
            char message_greeting[message_maxlen] = "230 Login successful.\r\n";
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
                char message_para_error[message_maxlen] = "504 Command USER Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: logged already\n", cont->connection_id);
            char message_logged[message_maxlen] = "503 Logged already.\r\n";
            Write_Message(cont->connection_id, message_logged);
            break;
        }
        case PASS:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[message_maxlen] = "504 Command PASS Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: logged already\n", cont->connection_id);
            char message_logged[message_maxlen] = "503 Logged already.\r\n";
            Write_Message(cont->connection_id, message_logged);
            break;
        }

        case RETR:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[message_maxlen] = "504 Command RETR Parament error.1\r\n";
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
                char message_para_error[message_maxlen] = "504 Command STOR Parament error.\r\n";
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
                char message_para_error[message_maxlen] = "504 Command QUIT Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: logout\n", cont->connection_id);
            char message_QUIT[message_maxlen] = "221 Goodbye.\r\n";
            Write_Message(cont->connection_id, message_QUIT);
            Close_Connection(cont);
            close(cont->connection_id);
            free(cont);
            pthread_exit(0);
            break;
        }
        case ABOR:
        {
            if (strlen(cont->message) > 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[message_maxlen] = "504 Command ABOR Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            printf("connection %d: logout\n", cont->connection_id);
            char message_QUIT[message_maxlen] = "221 Goodbye.\r\n";
            Write_Message(cont->connection_id, message_QUIT);
            Close_Connection(cont);
            close(cont->connection_id);
            free(cont);
            pthread_exit(0);
            break;
        }
        case SYST:
        {
            if (strlen(cont->message) > 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[message_maxlen] = "504 Command SYST Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            char message_SYST[message_maxlen] = "215 UNIX Type: L8\r\n";
            Write_Message(cont->connection_id, message_SYST);
            break;
        }
        case TYPE:
        {
            if (cont->message[5] == 'I' && strlen(cont->message) == 6)
            {
                char message_type[message_maxlen] = "200 Type set to I.\r\n";
                Write_Message(cont->connection_id, message_type);
            }
            else
            {
                char message_wrong_type[message_maxlen] = "504 Error TYPE.\r\n";
                Write_Message(cont->connection_id, message_wrong_type);
            }
            break;
        }
        case PORT:
        {
            if (strlen(cont->message) <= 5)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[message_maxlen] = "504 Command PORT Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            long long ip_port = Get_IP_Port(cont->message);
            if (ip_port == -1ll)
            {
                printf("connection %d: parameter error\n", cont->connection_id);
                char message_parameter_error[message_maxlen] = "504 Command not implemented for that parameter.\r\n";
                Write_Message(cont->connection_id, message_parameter_error);
                break;
            }
            Close_Connection(cont);
            int port = ip_port >> 32;
            int ip = ip_port ^ (((long long)port) << 32);
            Set_IP_Port(ip, port, PORT_MODE, cont);
            printf("2connection %d: ip:%s port:%d mode:port\n", cont->connection_id, cont->ip, cont->port);
            char message_port[message_maxlen] = "200 Command PORT okay.\r\n";
            Write_Message(cont->connection_id, message_port);
            break;
        }
        case PASV:
        {
            if (strlen(cont->message) > 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[message_maxlen * 30] = "504 Command PASV Parament error.\r\n";
                sprintf(message_para_error, "%lu%s\n", strlen(cont->message), cont->message);
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            Close_Connection(cont);
            if (Set_IP_Port(0, 0, PASV_MODE, cont))
            {
                printf("connection %d: pasv fail\n", cont->connection_id);
                char message_pasv_fail[message_maxlen] = "425 PASV connection fail.\r\n";
                Write_Message(cont->connection_id, message_pasv_fail);
                break;
            }
            // long long ip_port = Get_IP_Port(cont->message);
            // if (ip_port == -1ll)
            // {
            //     printf("connection %d: parameter error\n", cont->connection_id);
            //     char message_parameter_error[message_maxlen] = "504 Command not implemented for that parameter.\r\n";
            //     Write_Message(cont->connection_id, message_parameter_error);
            //     break;
            // }
            printf("3connection %d: ip:%s port:%d mode:pasv\n", cont->connection_id, cont->ip, cont->port);
            char message_pasv[message_maxlen]; // = "227 Entering Passive Mode (";
            sprintf(message_pasv, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", 127, 0, 0, 1, ((cont->port) >> 8) & 255, (cont->port) & 255);
            // int len = strlen(message_pasv);
            // Write_Digit(127, message_pasv, &len); // IP：127.0.0.1
            // message_pasv[len++] = ',';
            // Write_Digit(0, message_pasv, &len);
            // message_pasv[len++] = ',';
            // Write_Digit(0, message_pasv, &len);
            // message_pasv[len++] = ',';
            // Write_Digit(1, message_pasv, &len);
            // message_pasv[len++] = ',';
            // // for (int i = 0; i < 4; i++)
            // // {
            // //     int tmp = (ip >> ((3 - i) * 8)) & 255;
            // //     Write_Digit(tmp, message_pasv, &len);
            // //     message_pasv[len++] = ',';
            // // }
            // int tmp = ((cont->port) >> 8) & 255;
            // Write_Digit(tmp, message_pasv, &len);
            // message_pasv[len++] = ',';
            // tmp = port & 255;
            // Write_Digit(tmp, message_pasv, &len);
            // message_pasv[len++] = ')';
            // message_pasv[len++] = '.';
            // message_pasv[len++] = '\r';
            // message_pasv[len++] = '\n';
            // if (len > message_pasv)
            //     while (1)
            //         ;
            // message_pasv[len++] = '\0';
            // while (1)
            //     ;
            // sprintf(message_pasv, "sadsaf.\r\n");
            Write_Message(cont->connection_id, message_pasv);
            break;
        }
        case MKD:
        {
            if (strlen(cont->message) <= 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[message_maxlen] = "504 Command MKD Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            if (!Make_Dir(cont))
            {
                printf("connection %d: mkd success\n", cont->connection_id);
                char message_mkd_success[root_maxlen * 2 + 30];
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
                char message_mkd_fail[message_maxlen] = "451 Error make dir.\r\n";
                Write_Message(cont->connection_id, message_mkd_fail);
            }
            break;
        }
        case CWD:
        {
            if (strlen(cont->message) <= 4)
            {
                printf("connection %d: parament error\n", cont->connection_id);
                char message_para_error[message_maxlen] = "504 Command CWD Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            Command_CWD(cont);
            break;
        }
        case PWD:
        {
            printf("connection %d: dir:%s\n", cont->connection_id, cont->dir);
            char message_PWD[root_maxlen + 30];
            sprintf(message_PWD, "257 Current Path is \"%s\"", cont->dir);
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
                char message_para_error[message_maxlen] = "504 Command RMD Parament error.\r\n";
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
                char message_para_error[message_maxlen] = "504 Command RNFR Parament error.\r\n";
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
                char message_para_error[message_maxlen] = "504 Command RNTO Parament error.\r\n";
                Write_Message(cont->connection_id, message_para_error);
                return;
            }
            Command_RNTO(cont);
            break;
        }
        default:
        {
            char message_command_invaild[message_maxlen] = "500 Command unrecognized.\r\n";
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
    // return 35555 + rand() % (37000 - 35555);
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
int getIfaceName(char *iface_name, int len)
{
    int r = -1;
    int flgs, ref, use, metric, mtu, win, ir;
    unsigned long int d, g, m;
    char devname[20];
    FILE *fp = NULL;

    if ((fp = fopen("/proc/net/route", "r")) == NULL)
    {
        perror("fopen error!\n");
        return -1;
    }

    if (fscanf(fp, "%*[^\n]\n") < 0)
    {
        fclose(fp);
        return -1;
    }

    while (1)
    {
        r = fscanf(fp, "%19s%lx%lx%X%d%d%d%lx%d%d%d\n",
                   devname, &d, &g, &flgs, &ref, &use,
                   &metric, &m, &mtu, &win, &ir);
        if (r != 11)
        {
            if ((r < 0) && feof(fp))
            {
                break;
            }
            continue;
        }

        strncpy(iface_name, devname, len);
        fclose(fp);
        return 0;
    }

    fclose(fp);

    return -1;
}

int getIpAddress(char *iface_name, char *ip_addr, int len)
{
    int sockfd = -1;
    struct ifreq ifr;
    struct sockaddr_in *addr = NULL;

    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, iface_name);
    addr = (struct sockaddr_in *)&ifr.ifr_addr;
    addr->sin_family = AF_INET;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("create socket error!\n");
        return -1;
    }

    if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0)
    {
        strncpy(ip_addr, inet_ntoa(addr->sin_addr), len);
        close(sockfd);
        return 0;
    }

    close(sockfd);

    return -1;
}

bool Set_IP_Port(int ip, int port, int mode, struct Connection *cont)
{
    Close_Connection(cont);
    if (mode == PASV_MODE)
    {
        cont->port = Get_Random_Port();
        cont->connection_mode = mode;
        if (!isdigit(cont->ip[0]))
        {
            char iface_name[20];
            if (getIfaceName(iface_name, sizeof(iface_name)) < 0)
            {
                printf("get ip error\n");
                return 1;
            }
            if (getIpAddress(iface_name, cont->ip, 15) < 0)
            {
                printf("get ip error\n");
                return 1;
            }
        }
        // sprintf(cont->ip, "%s", "127.0.0.1");
        struct sockaddr_in Address;
        if ((cont->connection_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        {
            printf("Error socket(): %s(%d)\n", strerror(errno), errno);
            return 1;
        }

        memset(&Address, 0, sizeof(Address));
        Address.sin_family = AF_INET;
        Address.sin_port = htons(cont->port);

        printf("ip:%s\n", cont->ip);

        if (inet_pton(AF_INET, cont->ip, &Address.sin_addr) <= 0)
        {
            printf("Error inetpton(): %s(%d)\n", strerror(errno), errno);
            return 1;
        }

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
    else
    {
        int len = 0;
        for (int i = 0; i < 4; i++)
            Write_Digit((ip >> ((3 - i) * 8)) & 255, cont->ip, &len), cont->ip[len++] = '.';
        cont->ip[len - 1] = '\0';
        cont->port = port;
        cont->connection_mode = mode;
    }
    return 0;
}

/*
关闭连接id
参数：
    id：连接id
*/
void Close_Connectionid(int id)
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
void Close_Connection(struct Connection *cont)
{
    Close_Connectionid(cont->connection_data);
    cont->connection_data = -1;
    Close_Connectionid(cont->connection_listen);
    cont->connection_listen = -1;
}
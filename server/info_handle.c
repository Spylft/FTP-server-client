#include "command_define.h"
#include "command.h"
#include "file_transport.h"
#include "info_handle.h"
#include "message_transport.h"
#include "status.h"

/*
给定字符串以及当前位置p，求p开始的数字为多少，并且将p最终置该数字后
参数：
    digit：字符串
    len：字符串长度
    p：初始下标
返回：
    数字为多少
    若数字为-1，则一开始就不是数字
*/
int Get_Digit(char *digit, int len, int *p)
{
    if (!isdigit(digit[*p]))
        return -1;
    int ret = 0;
    while (*p < len && isdigit(digit[*p]))
    {
        ret = ret * 10 + digit[*p] - '0';
        (*p)++;
    }
    return ret;
}
/*
输入字符格式的ip和端口，返回longlong格式
参数：
    h1,h2,h3,h4,p1,p2：表示ip以及端口
返回：
    p1p2h1h2h3h4：一个longlong的数 大的16位是端口 小的32位是ip
    若返回-1即参数有误
*/
long long Get_IP_Port(char *ip_port)
{
    long long ret = 0;
    int cnt = 0, len = strlen(ip_port), i = 0, beg = 5;
    while (i < 4 && beg < len)
    {
        int tmp = Get_Digit(ip_port, len, &beg);
        if (ip_port[beg] != ',')
            return -1;
        if (tmp >= 256 || tmp < 0)
            return -1;
        ret |= ((long long)tmp) << (8 * (3 - i));
        beg++;
    }
    int tmp = Get_Digit(ip_port, len, &beg);
    if (ip_port[beg] != ',')
        return -1;
    if (tmp >= 256 || tmp < 0)
        return -1;
    ret |= ((long long)tmp) << 40;
    beg++;
    tmp = Get_Digit(ip_port, len, &beg);
    if (tmp >= 256 || tmp < 0)
        return -1;
    ret |= ((long long)tmp) << 32;
    return ret;
}

/*
将digit写入str字符串的len位置之后
参数：
    digit：数字
    str：需要写入的字符串
    len：开始写的位置
*/
void Write_Digit(int digit, char *str, int *len)
{
    int stk[10], sz = 0;
    while (digit)
        stk[sz++] = digit % 10, digit /= 10;
    if (sz == 0)
        stk[sz++] = 0;
    while (sz--)
        str[*len] = stk[sz], (*len)++;
}
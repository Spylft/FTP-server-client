#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <memory.h>
#include <unistd.h>		/* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>	/* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>	/* IP address conversion stuff */
#include <netdb.h>		/* gethostbyname */
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>

int sockfd;
struct sockaddr_in addr;
char sentence[8192];
void Write_Message(char *message)
{
	//把键盘输入写入socket
	int p = 0, len = strlen(message);
	printf("Write %s\n", message);
	while (p < len)
	{
		// printf("%d\n", p);
		int n = write(sockfd, message + p, len - p); // write函数不保证所有的数据写完，可能中途退出
		// printf("%d %d %d\n", p, n, len);
		if (n < 0)
		{
			printf("Error write(): %s(%d)\n", strerror(errno), errno);
			return;
		}
		else
		{
			p += n;
		}
	}
}
void Read_Message(char *message)
{
	int p = 0;
	while (1)
	{
		int n = read(sockfd, message + p, 8192 - p);
		if (n < 0)
		{
			printf("Error read(): %s(%d)\n", strerror(errno), errno); // read不保证一次读完，可能中途退出
			return;
		}
		else if (n == 0)
		{
			break;
		}
		else
		{
			p += n;
			if (message[p - 1] == '\n')
			{
				break;
			}
		}
	}
	message[p] = '\0';
}
int main(int argc, char **argv)
{

	//创建socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//设置目标主机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(25543);
	if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
	{ //转换ip地址:点分十进制-->二进制
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//连接上目标主机（将socket和目标主机连接）-- 阻塞函数
	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	printf("connected\n");
	puts("1");
	Read_Message(sentence);
	printf("Read %s\n", sentence);
	Write_Message("USER anonymous\n");
	puts("2");
	Read_Message(sentence);
	printf("Read %s\n", sentence);
	Write_Message("PASS 12345\n");
	puts("3");
	Read_Message(sentence);
	printf("Read %s\n", sentence);
	// TODO
	int listenfd, connfd;
	char message[8192];
	listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(143 * 256 + 1);
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //监听"0.0.0.0"
	bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
	listen(listenfd, 10);

	Write_Message("PORT 127,0,0,1,143,1\n");
	Read_Message(sentence);
	printf("Read %s\n", sentence);

	Write_Message("STOR test105.data\n");
	connfd = accept(listenfd, NULL, NULL);

	FILE *file = fopen("test105.data", "rb");
	char Buffer[8192];
	memset(Buffer, 0, sizeof(char) * 8192);
	while (!feof(file))
	{
		int len = fread(Buffer, sizeof(char), 8192, file);
		int sent_len = 0;
		while (sent_len < len)
		{
			int len_sent = write(connfd, Buffer + sent_len, len - sent_len);
			sent_len += len_sent;
		}
	}
	close(connfd);
	system("pause");

	// 	Write_Message("RETR test195.data\n");
	// connfd = accept(listenfd, NULL, NULL);

	// char Buffer[8192];
	// memset(Buffer, 0, sizeof(char) * 8192);
	// while (1)
	// {
	// 	int len = read(connfd, Buffer, 8192);
	// 	if (len < 0)
	// 	{
	// 		break;
	// 		// fclose(file);
	// 		// char message_receive_fail[message_maxlen] = "426 Connection error.\r\n";
	// 		// Write_Message(info->connection_id, message_receive_fail);
	// 		// close(info->connection_data);
	// 		// free(info);
	// 		// pthread_exit(0);
	// 	}
	// 	else if (len == 0)
	// 	{
	// 		break;
	// 	}
	// 	printf("%lu\n", strlen(Buffer));
	// 	// fwrite(Buffer, sizeof(char), len, file);
	// }
	// close(connfd);
	// printf("read end\n");

	// Write_Message("QUIT\n");
	// Read_Message(sentence);
	// printf("Read %s\n", sentence);

	close(sockfd);

	return 0;
}

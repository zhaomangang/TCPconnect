//TCPclient.c

#include <stdio.h>
#include <unistd.h>	//LINUX标准头文件，包含了各种LINUX系统服务函数原型和数据结构的定义
#include <string.h>
#include <sys/types.h>	//基本数据类型头文件，含有基本数据类型的定义
#include <sys/socket.h>	//提供套接字函数原型的定义
#include <netinet/in.h>	//提供数据结构sockaddr_in的定义
#include <netdb.h>	//含有hostent结构与gethostbyname函数的定义
#include <stdlib.h>

#define PORT 1234	//定义端口号
#define BUFSIZE 100	//定义缓冲区大小
void process(FILE *fp,int sockfd);	//声明用于处理与服务器之间的通信子函数
char *getMessage(char *sendline,int line,FILE *fp);	//声明用于实现接受用户键盘输入数据的子函数
int main(int argc,char *argv[])
{
	int fd;	//定义文件描述符变量
	struct hostent *he;	//定义hostent结构变量
	struct sockaddr_in server;	
	if(argc!=2)	//若用户输入的命令行参数错误则提示用法并退出系统
	{
		printf("Usage:%s<IP Adress>\n",argv[0]);
		exit(1);
	}
	if((he=gethostbyname(argv[1]))==NULL)	//调用gethostbyname()由用户输入的远程服务器的十进制ip地址获得其二进制的ip地址
	{
		printf("gethostbyname() error\n");
		exit(1);	
	
	}
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)	//创建套接字
	{
		printf("socket() error\n");
		exit(1);
	}
	bzero(&server,sizeof(server));	//清空服务器端点地址结构体变量

	//以下代码段用于对服务器端点地址结构体变量进行赋值
	server.sin_family = AF_INET;
	server.sin_port=htons(PORT);
	server.sin_addr = *((struct in_addr *)he->h_addr);
	if(connect(fd,(struct sockaddr *)&server,sizeof(struct sockaddr)) == -1)	//调用connect()函数向远程服务器发出连接请求
	{
		printf("connect() error\n");
		exit(1);
	}
	process(stdin,fd);	//调用子process()函数基于新创建的套接字与服务器之间进行交互
	close(fd);	//交互完毕，关闭套接字
	return 0;
}

void process(FILE *fp,int sockfd)
{
	char sendline[BUFSIZE],recvline[BUFSIZE];
	int numbytes;
	printf("Connected to server.\n");
	printf("Input name:");
	if(fgets(sendline,BUFSIZE,fp) == NULL) //调用fgets()函数接受用户键盘输入的客户端名字并存入缓冲区sendline
	{
		printf("\nExit\n");
		return;
	
	}
	send(sockfd,sendline,strlen(sendline),0);	//将sendline中缓存的客户端名字发送给服务器

	while(getMessage(sendline,BUFSIZE,fp)!=NULL)	//循环调用getMessage()函数接受用户键盘输入的信息并存入缓冲区sendline
	{
		send(sockfd,sendline,strlen(sendline),0);	//将缓存在sendline中的信息发送给服务器
		if((numbytes = recv(sockfd,recvline,BUFSIZE,0)) == 0)	//调用recv()函数接收服务器回送的信息并存入缓冲区recvline
		{
			printf("Server terminated.\n");
			return;
		}
		recvline[numbytes] = '\0';	//在字符串末尾添加字符串结束符号
		printf("Server Message:%s\n",recvline);	//打印输出服务器的回送信息内容
	
	}
	 printf("\nExit. \n");

}

char *getMessage(char *sendline,int len,FILE* fp)
{
	printf("Input string to server:");
	return(fgets(sendline,BUFSIZE,fp));	//调用fgets()函数接收用户键盘输入的信息并存入缓冲区sendline

}



























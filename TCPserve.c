//TCPserve.c

#include <stdio.h>
#include <unistd.h>	//linux标准头文件，包含了各种linux系统服务函数原型和数据结构的定义

#include <sys/types.h>	//基本数据类型头文件，包含有基本系统数据类型定义
#include <sys/socket.h>	//提供套接字函数原型与数据结构的定义
#include <netinet/in.h>	//提供数据结构sockaddr_in的定义
#include <arpa/inet.h>	//提供ip地址转函数原型的定义
#include <signal.h>

#include <stdlib.h>
#include <string.h>

#define PORT 1234	//定义服务器端端口
#define QLEN 10	//定义允许排队的连接数
#define BUFSIZE 1024	//定义缓冲区大小为1024b
void sig_chld(int);	//声明sig_chld()函数
void process_cli(int connectfd,struct sockaddr_in client);	//声明用于处理与客户之间通信的子函数
int main()
{
	int listenfd,connectfd;	//定义主套接字和从套接字
	pid_t pid;	//定义进程标识变量
	struct sockaddr_in server;	//定义服务器端点地址结构体变量
	struct sockaddr_in client;	//定义客户端端点地址结构体变量
	socklen_t ssize;
	if((listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1)	//创建主套接字
	{
		perror("Creating socket failed.");
		exit(1);
	}
	int opt = SO_REUSEADDR;	//设置与主套接字关联的选项，允许主套接字重用本地地址和端口
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	bzero(&server,sizeof(server));	//清空服务器端点地址结构体变量

	//以下代码段用于对服务器端点地址结构体变量进行赋值
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	//调用bind()函数将主套接字绑定到服务器的端点地址
	if(bind(listenfd,(struct sockaddr*)&server,sizeof(struct sockaddr)) == -1)
	{
		perror("bind error\n");
		exit(1);	//调用bind()函数出错则退出系统
	}
	if(listen(listenfd,QLEN) == -1)	//调用listen()函数使主套接字处于被动监听模式，并为主套接字建立一个数据输入队列
	{
		perror("listen() error\n");
		exit(1);	//调用listen()出错则退出系统
	}
	ssize = sizeof(struct sockaddr_in);
	signal(SIGCHLD,sig_chld);	//调用signal()函数为SIGCHLD信号安装handler
	while(1)	//循环调用accpet()函数接受客户连接请求，建立连接，并创建新的从套接字connectfd用于处理该连接
	{
		if((connectfd = accept(listenfd,(struct sockaddr*)&client,&ssize)) == -1)
		{
			perror("accept()error\n");
			exit(1);
		}
		if((pid = fork()) > 0)	//调用fork()函数创建新的从进程
		{
			close(connectfd);	//在父进程中关闭从淘金诶子描述符
			continue;	//父进程返回while循环
		}
		else if(pid == 0)
		{
			close(listenfd);	//在子进程中关闭主套接字描述符
			process_cli(connectfd,client);	//在子进程中调用process_cli()函数基于新建的从套接字connectfd来处理与客户之间的通信
			exit(0);	//处理完毕与客户的通信之后退出子进程
		}
		else
		{
			//调用fork()创建从进程出错退出系统
			printf("fork error\n");
			exit(0);
		}
	}
	close(listenfd);	//主进程结束时关闭主套接字
	return 0;
}

void process_cli(int connectfd,struct sockaddr_in client)
{
	int num;
	char recvbuf[BUFSIZE],sendbuf[BUFSIZE],cli_name[BUFSIZE];	//定义相关缓存区
	int len;
	printf("You got a connection from %s.",inet_ntoa(client.sin_addr));	//打印输出客户的ip地址
	num = recv(connectfd,cli_name,BUFSIZE,0);	//从从套接字中读取客户发送过来的数据（即接收该客户的名字）
	if(num == 0)	//若从套接字中数据读取完毕则关闭该套接字并返回
	{
		close(connectfd);
		printf("Client disconnected.\n");
		return ;
	}
	cli_name[num-1] = '\0';	//在字符串末尾添加字符串结束符
	printf("Client's name is %s.\n",cli_name);		//打印输出客户的名字
	while(num = recv(connectfd,recvbuf,BUFSIZE,0))	//循环接收来自客户的其他信息
	{
		recvbuf[num] = '\0';	//在字符串末尾添加字符串结束符
		printf("Recived client(%s)message:%s",cli_name,recvbuf);
		for(int i = 0;i<num-1;i++)	//将接收到的客户消息进行反转
		{
			sendbuf[i] = recvbuf[num-i -2];
		}
		sendbuf[num-1] = '\0';	//在字符串末尾添加字符串结束符
		len = strlen(sendbuf);
		send(connectfd,sendbuf,len,0);	//将反转后的数据回送给客户端
	}
	close(connectfd);	//关闭从套接字
}

void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	while((pid = waitpid(-1,&stat,WNOHANG))>0)	
	{
		/*
		 *
		 *调用waitpid()函数等待子进程结束，若
		  有子进程结束，则waitpid()函数将对其
		  回收，从而避免产生僵尸进程

		 * */
		printf("chile%d terminated\n",pid);
	}
	return;

}













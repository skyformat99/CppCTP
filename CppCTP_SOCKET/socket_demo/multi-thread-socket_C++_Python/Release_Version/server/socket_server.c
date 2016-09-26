#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "msg.h"

#define MAXCONNECTIONS 100
int sockfd;

void sig_handler(int signo) {
	if (signo == SIGINT) {
		printf("server close\n");
		close(sockfd);
		exit(1);
	}
}

/*输出连接上来的客户端的相关信息*/
void out_addr(struct sockaddr_in *clientaddr) {
	//将端口从网络字节序转换成主机字节序
	int port = ntohs(clientaddr->sin_port);
	char ip[16];
	memset(ip, 0, sizeof(ip));
	//将ip地址从网络字节序转换成点分十进制
	inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr, ip, sizeof(ip));
	printf("client: %s(%d) connected\n", ip, port);
}

/*输出服务器端时间*/
void do_service(int fd) {
	/*和客户端进行读写操作(双向通信)*/
	char buff[10 *1024];
	while (1)
	{
		memset(buff, 0, sizeof(buff));
		printf("start read and write...\n");
		size_t size;
		//printf("sizeof buff is %d = \n", sizeof(buff));
		if ((size = read_msg(fd, buff, sizeof(buff))) < 0) {
			print("protocal error");
			break;
		}
		else if (size == 0) {
			break;
		}
		else {
			printf("Server Received = %s\n", buff);
			//printf("socket_server send size = %d \n", strlen(buff));
			//printf("socket_server fd = %d \n", fd);
			//printf("socket_server send size = %d \n", sizeof(buff));
			//printf("socket_server send size = %d \n", strlen(buff));
			if (write_msg(fd, buff, sizeof(buff)) < 0) {
				printf("errorno = %d", errno);
				if (errno == EPIPE) {
					break;
				}
				perror("protocal error");
			}
		}
	}
}

void out_fd(int fd) {
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	//从fd中获取连接的客户端相关信息
	if (getpeername(fd, (struct sockaddr *)&addr, &len) < 0) {
		perror("getpeername error");
		return;
	}
	char ip[16];
	memset(ip, 0, sizeof(ip));
	int port = ntohs(addr.sin_port);
	inet_ntop(AF_INET, &addr.sin_addr.s_addr, ip, sizeof(ip));
	printf("%16s(%5d) closed!\n", ip, port);
}

void *th_fn(void *arg) {
	int fd = (int)arg;
	do_service(fd);
	out_fd(fd);
	close(fd);

	return (void *)0;
}

int main(int argc, char *argv[]) {
	
	if (argc < 2) {
		printf("usage: %s #port\n", argv[0]);
		exit(1);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		perror("signal sigint error");
		exit(1);
	}

	signal(SIGPIPE, SIG_IGN);

	/*步骤1:创建socket(套接字)*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/*步骤2:将socket和地址(包括ip,port)进行绑定*/
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	/*向地址中填入ip,port,internet地址簇类型*/
	serveraddr.sin_family = AF_INET; //ipv4
	serveraddr.sin_port = htons(atoi(argv[1])); //port
	serveraddr.sin_addr.s_addr = INADDR_ANY; //接收所有网卡地址
	if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		perror("bind error");
		exit(1);
	}

	/*步骤3:调用listen函数启动监听(指定port监听)
	通知系统去接受来自客户端的连接请求
	第二个参数:指定队列的长度*/
	if (listen(sockfd, MAXCONNECTIONS) < 0) {
		perror("listen error");
		exit(1);
	}

	/*步骤4:调用accept函数从队列中获得一个客户端的连接请求，
	并返回新的socket描述符*/
	struct sockaddr_in clientaddr;
	socklen_t clientaddr_len = sizeof(clientaddr);

	/*设置线程的分离属性*/
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	while (1)
	{
		int fd = accept(sockfd, NULL, NULL);
		if (fd < 0) {
			perror("accept error");
			continue;
		}
		/*步骤5:启动子线程去调用IO函数(read/write)和连接的客户端进行双向通信*/
		pthread_t th;
		int err;
		/*以分离状态启动子线程*/
		if ((err = pthread_create(&th, &attr, th_fn, (void *)fd)) != 0) {
			perror("pthread create error");
		}
		pthread_attr_destroy(&attr);
	}

	return 0;
}

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>

int sockfd; //套接字

void sig_handler(int signo)
{
	if (signo == SIGINT) {
		printf("Server close\n");
		close(sockfd);
		exit(1);
	}
}

//输出客户端信息
void out_addr(struct sockaddr_in *clientaddr) {
	//将端口从网络字节序转换成主机字节序
	int port = ntohs(clientaddr->sin_port);
	char ip[16];
	memset(ip, 0, sizeof(ip));
	//将ip地址从网络字节序转换成点分十进制
	inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr, ip, sizeof(ip));
	printf("client: %s(%d) connected\n", ip, port);
}

//和客户端进行通信
void do_service() {
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	// 接收客户端的数据报文
	if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientaddr, &len) < 0) {
		perror("recvfrom error");
	}
	else {
		out_addr(&clientaddr);
		printf("client send into: %s\n", buffer);

		//向客户端发送数据报文
		long int t = time(0);
		char *ptr = ctime(&t);
		size_t size = strlen(ptr) * sizeof(char);
		if (sendto(sockfd, ptr, size, 0, (struct sockaddr *)&clientaddr, len) < 0) {
			perror("sendto error");
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: %s port\n", argv[0]);
		exit(1);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		perror("signal sigint error");
		exit(1);
	}

	/*步骤1:创建socket*/
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket error");
		exit(1);
	}

	int ret;
	int opt = 1;
	//设置套接字选项
	if ((ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)) {
		perror("setsockopt error");
		exit(1);
	}

	/*步骤2:调用bind函数对socket和地址进行绑定*/
	struct sockaddr_in serveraddr;

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET; //ipv4
	serveraddr.sin_port = htons(atoi(argv[1])); //port
	serveraddr.sin_addr.s_addr = INADDR_ANY; //ip
	if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		perror("bind error");
		exit(1);
	}

	/*步骤3:服务端和客户端进行双向通信*/
	while (1) {
		do_service();
	}



	return 0;
}

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <signal.h>
#include <time.h>

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
	
	/*size_t len;
	char buff[20];
	if ((len = read(fd, buff, 20)) < 0) {
		perror("read error");
	}*/
	
	//获得系统时间
	long t = time(0);
	char *s = ctime(&t);
	size_t size = strlen(s) * sizeof(char);
	//将服务器端获得的系统时间写回到客户端
	if (write(fd, s, size) != size) {
		perror("write error");
	}
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
	while (1)
	{
		int fd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientaddr_len);
		if (fd < 0) {
			perror("accept error");
			continue;
		}
		/*步骤5:调用IO函数(read/write)和连接的客户端进行双向的通信*/
		out_addr(&clientaddr);
		do_service(fd);

		/*步骤6:关闭socket*/
		close(fd);
	}


	return 0;
}

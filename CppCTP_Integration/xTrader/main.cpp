#include <iostream>
#include <string>
#include <list>
#include <mongo/client/dbclient.h>
#include <stdio.h>

/*socket头文件*/
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>  // inet_ntoa

#include "ThostFtdcTraderApi.h"
#include "TdSpi.h"
#include "ThostFtdcMdApi.h"
#include "MdSpi.h"
#include "CTP_Manager.h"
#include "Utils.h"
#include "Debug.h"
#include "DBManager.h"
#include "Trader.h"
#include "FutureAccount.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "msg.h"

using std::cout;
using std::cin;
using namespace rapidjson;

/*宏定义*/
#define MAXCONNECTIONS 100
#define MAX_BUFFER_SIZE	30*1024

int sockfd;
CTP_Manager *ctp_m = NULL;

/*信号处理*/
void sig_handler(int signo) {
	if (signo == SIGINT) {
		printf("服务端已关闭!!!\n");
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
	char buff[MAX_BUFFER_SIZE];
	printf("服务端开始监听...\n");
	while (1)
	{
		memset(buff, 0, sizeof(buff));

		size_t size;
		//printf("sizeof buff is %d = \n", sizeof(buff));
		if ((size = read_msg(fd, buff, sizeof(buff))) < 0) {
			printf("protocal error");
			break;
		}
		else if (size == 0) {
			break;
		}
		else {
			//printf("服务端收到 = %s\n", buff);
			CTP_Manager::HandleMessage(fd, buff, ctp_m);
			//printf("socket_server send size = %d \n", strlen(buff));
			//printf("socket_server fd = %d \n", fd);
			//printf("socket_server send size = %d \n", sizeof(buff));
			//printf("socket_server send size = %d \n", strlen(buff));
			//if (write_msg(fd, buff, sizeof(buff)) < 0) {
			//	printf("先前客户端已断开!!!\n");
			//	//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
			//	if (errno == EPIPE) {
			//		break;
			//	}
			//	perror("protocal error");
			//}
		}
	}
}

/*输出文件描述符*/
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
	printf("来自ip地址:%16s(%5d) 已连接!!!\n", ip, port);
}

/*线程调用*/
void *th_fn(void *arg) {
	int fd = *(reinterpret_cast<int*>(arg));
	out_fd(fd);
	do_service(fd);
	close(fd);

	return (void *)0;
}

void printMenuEN() {
	cout << "|==========================|" << endl;
	cout << "|Please Input Your Choice: |" << endl;
	cout << "|i:Order Insert            |" << endl;
	cout << "|a:Order Action            |" << endl;
	cout << "|b:Break                   |" << endl;
	cout << "|e:Qry Exchange            |" << endl;
	cout << "|s:Qry Instrument          |" << endl;
	cout << "|o:Qry Order               |" << endl;
	cout << "|t:Qry Trading Account     |" << endl;
	cout << "|u:Qry Investor            |" << endl;
	cout << "|d:Qry Trade               |" << endl;
	cout << "|h:Qry Investor Position   |" << endl;
	cout << "|==========================|" << endl;
}

void printMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:交易员登陆              |" << endl;
	cout << "|【2】:管理员登陆              |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
}

void printContinue() {
	cout << "|======================|" << endl;
	cout << "|请输入【c】继续操作   |" << endl;
	cout << "|======================|" << endl;
}

void printContinueEN() {
	cout << "|======================|" << endl;
	cout << "|Input 'c' to Continue:|" << endl;
	cout << "|======================|" << endl;
}

void printLoginSuccessMenu() {
	cout << "|=============|" << endl;
	printf("|\033[47;30m【登陆成功!】\033[0m|\n");
	cout << "|=============|" << endl;
}

void printLoginFailedMenu() {
	cout << "|========================|" << endl;
	printf("|\033[47;31m【！！！登录失败！！！】\033[0m|\n");
	cout << "|========================|" << endl;
}

void printErrorInputMenu() {
	cout << "|========================|" << endl;
	printf("|\033[47;31m【！！！重新输入！！！】\033[0m|\n");
	cout << "|========================|" << endl;
}

void printWelcome() {
	cout << "|===========================|" << endl;
	//cout << "|欢迎登录期货多账户交易系统!|" << endl;
	printf("|\033[47;30m欢迎登录期货多账户交易系统!\033[0m|\n");
	cout << "|===========================|" << endl;
}

void printLoginTraderOperatorMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:账户查询                |" << endl;
	cout << "|【2】:持仓查询                |" << endl;
	cout << "|【3】:报单查询                |" << endl;
	cout << "|【4】:成交查询                |" << endl;
	cout << "|【5】:报单                    |" << endl;
	cout << "|【6】:撤单                    |" << endl;
	cout << "|【7】:订阅行情                |" << endl;
	cout << "|【8】:退订行情                |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
}

void printFutureAccountListMenu() {
	cout << "|==============================|" << endl;
	//cout << "|\033[40;32m当前您所管理的期货账户列表:   \033[0m|\n" << endl;
	printf("|\033[47;30m当前您所管理的期货账户列表:   \033[0m|\n");
	cout << "|==============================|" << endl;
}

void printTraderAccoutListMenu() {
	cout << "|==============================|" << endl;
	printf("|\033[47;30m当前您所管理的交易员列表:     \033[0m|\n");
	cout << "|==============================|" << endl;
}

void printAdminOperateMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:交易员管理              |" << endl;
	cout << "|【2】:期货账户管理            |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
}

void printTraderOperateMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:查看所有交易员          |" << endl;
	cout << "|【2】:增加交易员              |" << endl;
	cout << "|【3】:删除交易员              |" << endl;
	cout << "|【4】:修改交易员              |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
}

void printFutureAccountOperateMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:查看所有期货账户        |" << endl;
	cout << "|【2】:增加期货账户            |" << endl;
	cout << "|【3】:删除期货账户            |" << endl;
	cout << "|【4】:修改期货账户            |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
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

	// 初始化mongoDB
	mongo::client::initialize();

	// 初始化CTP_Manager
	ctp_m = new CTP_Manager();

	// 程序入口，初始化资源
	ctp_m->init();

	/*步骤1:创建socket(套接字)*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/*socket选项设置*/
	// a：设置套接字的属性使它能够在计算机重启的时候可以再次使用套接字的端口和IP
	int err, sock_reuse = 1;
	err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_reuse, sizeof(sock_reuse));
	if (err != 0) {
		printf("SO_REUSEADDR Setting Failed!\n");
		exit(1);
	}
	// b：设置接收缓冲区大小
	int nRecvBuf = MAX_BUFFER_SIZE; //数据最大长度
	err = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	if (err != 0) {
		printf("SO_RCVBUF Setting Failed!\n");
		exit(1);
	}
	// c：设置发送缓冲区大小
	int nSendBuf = MAX_BUFFER_SIZE; //数据最大长度
	err = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	if (err != 0) {
		printf("SO_SNDBUF Setting Failed!\n");
		exit(1);
	}

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
		if ((err = pthread_create(&th, &attr, th_fn, &fd)) != 0) {
			perror("pthread create error");
		}
		pthread_attr_destroy(&attr);
	}

	return 0;
}
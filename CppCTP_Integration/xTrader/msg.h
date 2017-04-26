#ifndef __MSG_H__
#define __MSG_H__
#include <sys/types.h>

#define MAX_BUFFER_SIZE	60*1024

typedef struct {
	//协议头部
	char head[13];
	char checknum; //校验码
	//协议体部
	char buff[MAX_BUFFER_SIZE]; //数据
} Msg;

/************************************************************************/
/* 发送一个基于自定义协议的message
 * 发送的数据存放在buff中*/
/************************************************************************/
int write_msg(int sockfd, char *buff, size_t len);

/************************************************************************/
/* 读取一个基于自定义协议的message
 * 读取的数据存放在buff中*/
/************************************************************************/
int read_msg(int sockfd, char *buff, size_t len);

#endif

#ifndef __MSG_H__
#define __MSG_H__
#include <sys/types.h>

typedef struct {
	//协议头部
	char head[13];
	char checknum; //校验码
	//协议体部
	char buff[1024]; //数据
} Msg;

/************************************************************************/
/* 发送一个基于自定义协议的message
 * 发送的数据存放在buff中*/
/************************************************************************/
extern int write_msg(int sockfd, char *buff, size_t len);

/************************************************************************/
/* 读取一个基于自定义协议的message
 * 读取的数据存放在buff中*/
/************************************************************************/
extern int read_msg(int sockfd, char *buff, size_t len);

#endif
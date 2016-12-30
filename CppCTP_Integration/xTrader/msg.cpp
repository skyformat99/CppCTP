#include "msg.h"
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <stdio.h>

/*计算校验码*/
static unsigned char msg_check(Msg *message) {
	//unsigned char s = 0;
	int s = 0;
	int i;
	//printf("message head = %s \n", message->head);
	//printf("message buff = %s \n", message->buff);
	for (i = 0; i < sizeof(message->head); i++) {
		//printf("i1 = %c \n", message->head[i]);
		s = ((s + message->head[i]) % 255);
		//printf("i1 checknum = %d \n", s);
	}
	/*for (i = 0; i < sizeof(message->buff); i++) {
		//printf("i2 = %c \n", message->buff[i]);
		s = ((s + message->buff[i]) % 255);
		//printf("i2 checknum = %d \n", s);
	}*/
	//printf("msg_check num is %d \n", s);
	return s;
}

/************************************************************************/
/* 发送一个基于自定义协议的message
 * 发送的数据存放在buff中*/
/************************************************************************/
int write_msg(int sockfd, char *buff, size_t len) {
	printf("开始发消息... \n");
	Msg message;
	memset(&message, 0, sizeof(message));
	//strcpy(message.head, "gmqh_sh_2016");
	memcpy(message.buff, buff, len);
	memcpy(message.head, "gmqh_sh_2016", strlen("gmqh_sh_2016") + 1);
	message.checknum = msg_check(&message);
	printf("write_msg2 \n");
	printf("write Message head %s\n", message.head);
	printf("write Message checknum %d\n", message.checknum);
	if (write(sockfd, &message, sizeof(message)) != sizeof(message)) {
		return -1;
	}
	printf("消息发送完毕 \n");
}

/************************************************************************/
/* 读取一个基于自定义协议的message
 * 读取的数据存放在buff中*/
/************************************************************************/
int read_msg(int sockfd, char *buff, size_t len) {
	//printf("read_msg1 \n");
	Msg message;
	memset(&message, 0, sizeof(message));
	size_t size;
	//printf("read_msg1-1 \n");
	if ((size = read(sockfd, &message, sizeof(message))) < 0) {
		//printf("read_msg1-2 \n");
		return -1;
	}
	else if (size == 0) {
		return 0;
	}
	//printf("read_msg2 \n");
	//进行校验码验证,判断接收到message是否完整
	unsigned char s = msg_check(&message);
	//printf("read_msg message.head = %s \n", message.head);
	//printf("read_msg message.checknum = %d \n", message.checknum);
	//printf("read_msg message.buff = %s \n", message.buff);
	//printf("read_msg checknum cal = %d \n", s);
	if ((s == (unsigned char)message.checknum) && (!strcmp("gmqh_sh_2016", message.head))) {
		//printf("read_msg3 \n");
		memcpy(buff, message.buff, len);
		return sizeof(message);
	}
	//printf("read_msg4 \n");
	return -1;
}
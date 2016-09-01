# -*- coding: utf-8 -*-

from collections import namedtuple
import socket
import sys
import struct

Message = namedtuple("Message", "head checknum buff")

# m = Message("gmqh", 0, "hello, world")

# 计算校验码
def msg_check(message):
    #将收到的head以及buff分别累加 % 255
    checknum = 0
    for i in message.head:
        # print("i1 = %c \n" % i)
        checknum = ((checknum + ord(i)) % 255)
        # print("i1 checknum = %d \n" % checknum)
    for i in message.buff:
        # print("i2 = %c \n" %i)
        checknum = ((checknum + ord(i)) % 255)
        # print("i2 checknum = %d \n" % checknum)
    return checknum

#发送数据
def write_msg(sockfd, buff):
    # print("send buff = ", buff)
    # print("send buff len = ", len(buff))
    # print("send buff = ", buff.encode())
    # print("send buff len = ", len(buff.encode()))
    #构造Message
    m = Message("gmqh_sh_2016", 0, buff)

    #数据发送前,将校验数据填入Message结构体
    checknum = msg_check(m)
    m = Message("gmqh_sh_2016", checknum, buff)

    # print("send m.buff = ", m.buff.encode())
    # print("send m.checknum = ", m.checknum)
    #打包数据(13位的head,1位校验码,不定长数据段)
    data = struct.pack(">13s1B" + str(len(m.buff.encode()) + 1) + "s", m.head.encode(), m.checknum, m.buff.encode());

    print("send data = ", data)
    #发送数据
    size = sockfd.send(data)

    # print(size)
    return size

#读取数据
def read_msg(sockfd):
    try:
        #接收数据1038个字节(与服务器端统一:13位head+1位checknum+1024数据段)
        data = sockfd.recv(1038)
    except socket.error as e:
        print(e)
    #解包数据
    head, checknum, buff = struct.unpack(">13s1B"+ str(len(data) - 14) +"s", data)
    # print(head, checknum, buff, '\n')
    #将解包的数据封装为Message结构体
    m = Message(head.decode().split('\x00')[0], checknum, buff.decode())
    tmp_checknum = msg_check(m)
    m = Message(head.decode().split('\x00')[0], tmp_checknum, buff.decode())

    #将收到的标志位与收到数据重新计算的标志位进行对比+head内容对比
    if ((m.checknum == checknum) and (m.head == "gmqh_sh_2016")):
        #打印接收到的数据
        print("receive data = ", m.buff)
        return 1
    else:
        return -1

if __name__ == '__main__':
    #创建socket套接字
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    if s:
        # 连接服务器: IP,port
        try:
            #进行与服务端的连接(ip地址根据实际情况进行更改)
            s.connect(("192.168.1.12", 8888))
        except socket.error as e:
            print("socket error", e)
            sys.exit(1)

        #输入提示符
        prompt = b'->'
        while True:
            buff = input(prompt)
            if buff == "":
                continue
            #发送数据
            if (write_msg(s, buff) < 0):
                print("write msg error")
                continue
            else:
                #接收数据
                if (read_msg(s) < 0):
                    print("read msg error")
                    continue
            pass

        s.close()
    else:
        print("socket error")



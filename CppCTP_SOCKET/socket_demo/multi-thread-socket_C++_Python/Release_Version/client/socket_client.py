# -*- coding: utf-8 -*-

from collections import namedtuple
import socket
import sys
import struct

Message = namedtuple("Message", "head checknum buff")

# m = Message("gmqh", 0, "hello, world")

# 计算校验码
def msg_check(message):
    checknum = 0
    for i in message.head:
        print("i1 = %c \n" % i)
        checknum = ((checknum + ord(i)) % 255)
        print("i1 checknum = %d \n" % checknum)
    for i in message.buff:
        print("i2 = %c \n" %i)
        checknum = ((checknum + ord(i)) % 255)
        print("i2 checknum = %d \n" % checknum)
    return checknum

def write_msg(sockfd, buff):
    print("send buff = ", buff)
    print("send buff len = ", len(buff))
    print("send buff = ", buff.encode())
    print("send buff len = ", len(buff.encode()))
    str_len = len(buff) + 1
    m = Message("gmqh_sh_2016", 0, buff)
    checknum = msg_check(m)
    m = Message("gmqh_sh_2016", checknum, buff)
    print("send m.buff = ", m.buff.encode())
    print("send m.checknum = ", m.checknum)
    data = struct.pack(">13s1B" + str(len(m.buff.encode()) + 1) + "s", m.head.encode(), m.checknum, m.buff.encode());
    print("send data = ", data)
    size = sockfd.send(data)
    print(size)
    return size

def read_msg(sockfd):
    try:
        data = sockfd.recv(1038)
    except socket.error as e:
        print(e)
    head, checknum, buff = struct.unpack(">13s1B"+ str(len(data) - 14) +"s", data)
    # print(head, checknum, buff, '\n')
    m = Message(head.decode().split('\x00')[0], checknum, buff.decode())
    tmp_checknum = msg_check(m)
    m = Message(head.decode().split('\x00')[0], tmp_checknum, buff.decode())
    if ((m.checknum == checknum) and (m.head == "gmqh_sh_2016")):
        print("receive data = ", m.buff)
        return 1
    else:
        return -1

if __name__ == '__main__':

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    if s:
        # 连接服务器: IP,port
        try:
            s.connect(("192.168.1.12", 8888))
        except socket.error as e:
            print("socket error", e)
            sys.exit(1)

        prompt = b'->'
        while True:
            buff = input(prompt)
            if buff == "":
                continue
            if (write_msg(s, buff) < 0):
                print("write msg error")
                continue
            else:
                if (read_msg(s) < 0):
                    print("read msg error")
                    continue
            pass

        s.close()
    else:
        print("socket error")



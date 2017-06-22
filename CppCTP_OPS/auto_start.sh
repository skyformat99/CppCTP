#!/bin/bash

#进入程序目录
cd ~/xTrader
#执行mongodb备份
sh mongo_backup.sh

#检查是否存在session为xts的session,关闭TS所在session
if screen -list | grep "xts"; then
    # run bash script
	echo "xts exists"
	screen -S xts -X quit
	echo "xts already exit"
fi

#在Screen中以分离方式启动TS程序
screen -U -S xts -d -m ./bin/quant_ctp_XTrader_no_debug_2017-06-22_20-11-24 8888 1
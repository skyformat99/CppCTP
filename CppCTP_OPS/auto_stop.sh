#!/bin/bash

#检查是否存在session为xts的session,关闭TS所在session
if screen -list | grep "xts"; then
    # run bash script
	echo "xts exists"
	screen -S xts -X quit
	echo "xts already exit"
fi

#进入程序目录
cd ~/xTrader
#执行mongodb备份
sh mongo_backup.sh
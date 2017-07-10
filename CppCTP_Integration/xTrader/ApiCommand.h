//封装api命令的接口
#ifndef QUANT_CTP_APICOMMAND_H
#define QUANT_CTP_APICOMMAND_H

#include "TdSpi.h"

class TdSpi;

class ApiCommand {
public:
	virtual ~ApiCommand();
	virtual int execute() = 0;
	int getCommandType();
protected:
	ApiCommand(TdSpi *tdapi, int command_type);
	TdSpi *tdapi;
	/************************************************************************/
	/* 命令类型
	0：查询类
	1：交易类*/
	/************************************************************************/
	int command_type;
};

#endif
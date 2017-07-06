//封装api命令的接口
#ifndef QUANT_CTP_APICOMMAND_H
#define QUANT_CTP_APICOMMAND_H

#include "../TdSpi.h"

class TdSpi;

class ApiCommand {
public:
	virtual ~ApiCommand();
	virtual int execute() = 0;
protected:
	ApiCommand(TdSpi *tdapi, int command_type);
	TdSpi *tdapi;
	int command_type;
};

#endif
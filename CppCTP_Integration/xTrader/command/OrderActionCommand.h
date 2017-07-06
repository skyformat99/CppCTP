#ifndef QUANT_CTP_ORDERACTIONCOMMAND_H
#define QUANT_CTP_ORDERACTIONCOMMAND_H

#include "../TdSpi.h"
#include "ApiCommand.h"

class OrderActionCommand : public ApiCommand{

public:
	OrderActionCommand(TdSpi *tdapi, CThostFtdcOrderField *action_order, int command_type);
	int execute() override;
private:
	CThostFtdcOrderField *action_order;
};

#endif
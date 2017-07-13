#ifndef QUANT_CTP_ORDERACTIONCOMMAND_H
#define QUANT_CTP_ORDERACTIONCOMMAND_H

#include "TdSpi.h"
#include "ApiCommand.h"

class OrderActionCommand : public ApiCommand{

public:
	OrderActionCommand(TdSpi *tdapi, CSgitFtdcOrderField *action_order, int command_type);
	int execute() override;
	
private:
	CSgitFtdcOrderField *action_order;
};

#endif
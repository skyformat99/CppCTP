#ifndef QUANT_CTP_ORDERINSERTCOMMAND_H
#define QUANT_CTP_ORDERINSERTCOMMAND_H

#include "../TdSpi.h"
#include "ApiCommand.h"

class OrderInsertCommand : public ApiCommand {

public:
	OrderInsertCommand(TdSpi *tdapi, User *user, CThostFtdcInputOrderField *insert_order, int command_type);
	int execute() override;
private:
	User *user;
	CThostFtdcInputOrderField *insert_order;
};

#endif
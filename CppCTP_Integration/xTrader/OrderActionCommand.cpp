#include "OrderActionCommand.h"

OrderActionCommand::OrderActionCommand(TdSpi *tdapi, CThostFtdcOrderField *action_order, int command_type) :ApiCommand(tdapi, command_type) {
	this->tdapi = tdapi;
	this->action_order = action_order;
	this->command_type = command_type;
}

int OrderActionCommand::execute() {
	int rsp_num = this->tdapi->OrderAction(action_order->ExchangeID, action_order->OrderRef, action_order->OrderSysID);
	return rsp_num;
}
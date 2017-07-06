#include "OrderInsertCommand.h"

OrderInsertCommand::OrderInsertCommand(TdSpi *tdapi, User *user, CThostFtdcInputOrderField *insert_order, int command_type) :ApiCommand(tdapi, command_type) {
	this->user = user;
	this->tdapi = tdapi;
	this->insert_order = insert_order;
}

int OrderInsertCommand::execute() {
	int rsp_num = this->tdapi->OrderInsert(this->user, this->insert_order);
	return rsp_num;
}
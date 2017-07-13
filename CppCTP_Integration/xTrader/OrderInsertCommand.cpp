#include "OrderInsertCommand.h"

OrderInsertCommand::OrderInsertCommand(TdSpi *tdapi, User *user, CSgitFtdcInputOrderField *insert_order, Strategy *stg, int command_type) :ApiCommand(tdapi, command_type) {
	this->user = user;
	this->tdapi = tdapi;
	this->insert_order = insert_order;
	this->command_type = command_type;
	this->stg = stg;
}

int OrderInsertCommand::execute() {
	int rsp_num = this->tdapi->OrderInsert(this->user, this->insert_order, this->stg);
	return rsp_num;
}
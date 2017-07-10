#include "QrySettlementInfoConfirmCommand.h"

QrySettlementInfoConfirmCommand::QrySettlementInfoConfirmCommand(TdSpi *tdapi, User *user, int command_type) :ApiCommand(tdapi, command_type) {
	this->user = user;
	this->tdapi = tdapi;
	this->command_type = command_type;
}

int QrySettlementInfoConfirmCommand::execute() {
	int rsp_num = this->tdapi->QrySettlementInfoConfirm(user);
	return rsp_num;
}
#include "LoginCommand.h"

LoginCommand::LoginCommand(TdSpi *tdapi, User *user, int command_type) :ApiCommand(tdapi, command_type) {
	this->user = user;
	this->tdapi = tdapi;
}

int LoginCommand::execute() {
	int rsp_num = this->tdapi->Login(user);
	return rsp_num;
}
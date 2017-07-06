#include "ApiCommand.h"

ApiCommand::~ApiCommand() {
	this->tdapi = NULL;
}

ApiCommand::ApiCommand(TdSpi *tdapi, int command_type) {
	this->tdapi = tdapi;
	this->command_type = command_type;
}
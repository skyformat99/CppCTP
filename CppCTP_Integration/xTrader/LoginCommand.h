#ifndef QUANT_CTP_LOGINCOMMAND_H
#define QUANT_CTP_LOGINCOMMAND_H

#include "TdSpi.h"
#include "ApiCommand.h"

class LoginCommand : public ApiCommand{

public:
	LoginCommand(TdSpi *tdapi, User *user, int command_type);
	int execute() override;
private:
	User *user;
};

#endif
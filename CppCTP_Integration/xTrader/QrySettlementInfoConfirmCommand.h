#ifndef QUANT_CTP_QRYSETTLEMENTINFOCONFIRMCOMMAND_H
#define QUANT_CTP_QRYSETTLEMENTINFOCONFIRMCOMMAND_H

#include "TdSpi.h"
#include "ApiCommand.h"

class QrySettlementInfoConfirmCommand : public ApiCommand{

public:
	QrySettlementInfoConfirmCommand(TdSpi *tdapi, User *user, int command_type);
	int execute() override;
private:
	User *user;
};

#endif
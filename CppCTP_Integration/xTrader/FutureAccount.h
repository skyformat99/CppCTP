#include <string>
#include "Trader.h"
#ifndef QUANT_ACCOUNT_H
#define QUANT_ACCOUNT_H

class FutureAccount {

public:
	FutureAccount();
	FutureAccount(string frontAddress, string userID, string password, string traderName, string isActive, string brokerID);
	~FutureAccount();
	void setUserID(string userID);
	string getUserID();
	void setPassword(string password);
	string getPassword();
	void setBrokerID(string brokerID);
	string getBrokerID();
	void setOperatorName(string traderName);
	string getOperatorName();

	void setTraderID(string traderid);
	string getTraderID();

	void setTrader(Trader *op);
	Trader *getTrader();
	
	void setIsActive(string isActive);
	string getIsActive();
	void setFrontAddress(string frontAddress);
	string getFrontAddress();

	void setOn_Off(int on_off);
	int getOn_Off();

private:
	string userID;
	string password;
	string brokerID;
	string traderID;
	string frontAddress;
	Trader *op;
	string isActive;
	int on_off;
};

#endif
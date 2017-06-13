#ifndef QUANT_MARKETCONFIG_H
#define QUANT_MARKETCONFIG_H
#include <string>
#include "Debug.h"

using std::string;

class MarketConfig
{

public:
	MarketConfig();
	MarketConfig(string market_id, string market_frontAddr, string broker_id, string user_id, string password, string isactive = "1");
	void setMarketFrontAddr(string market_frontAddr);
	void setBrokerID(string broker_id);
	void setUserID(string user_id);
	void setPassword(string password);
	void setMarketID(string market_id);
	void setIsActive(string isactive);

	string getMarketFrontAddr();
	string getBrokerID();
	string getUserID();
	string getPassword();
	string getMarketID();
	string getIsActive();

private:
	string market_id;
	string isactive;
	string market_frontAddr;
	string broker_id;
	string user_id;
	string password;
};
#endif
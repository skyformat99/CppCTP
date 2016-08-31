#ifndef QUANT_STRATEGY_H
#define QUANT_STRATEGY_H
#include "Trader.h"
#include "Algorithm.h"
#include "Debug.h"

class Strategy {

public:
	void get_tick(CThostFtdcDepthMarketDataField *pDepthMarketData);

	void setTrader(Trader *trader);
	Trader *getTrader();

	void setUser(User *user);
	User *getUser();

	void setStrategyId(string strategyid);
	string getStrategyId();

	void setAlgorithm(Algorithm *alg);
	Algorithm *getAlgorithm();

	void addInstrumentToList(string instrument);
	list<string> *getListInstruments();

	void setTraderID(string traderid);
	string getTraderID();

	void setUserID(string userid);
	string getUserID();

	void setIsActive(string isActive);
	string getIsActive();

private:
	Trader *trader;
	User *user;
	string strategyid;
	string userid;
	string traderid;
	string isActive;
	Algorithm *alg;
	list<string> *l_instruments;
	
};

#endif
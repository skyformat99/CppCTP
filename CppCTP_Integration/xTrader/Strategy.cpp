#include "Strategy.h"

void Strategy::get_tick(CThostFtdcDepthMarketDataField *pDepthMarketData) {
	USER_PRINT("Strategy::get_tick");
	cout << "===========================================" << endl;
	cout << "ondepthmarket data:" << ", ";
	cout << "trading day:" << pDepthMarketData->TradingDay << ", "
		<< "instrument id:" << pDepthMarketData->InstrumentID << ", "
		<< "last price:" << pDepthMarketData->LastPrice << ", "
		//<< "上次结算价:" << pDepthMarketData->PreSettlementPrice << endl
		//<< "昨收盘:" << pDepthMarketData->PreClosePrice << endl
		//<< "数量:" << pDepthMarketData->Volume << endl
		//<< "昨持仓量:" << pDepthMarketData->PreOpenInterest << endl
		<< "updateTime:" << pDepthMarketData->UpdateTime << ", "
		<< "UpdateMillisec:" << pDepthMarketData->UpdateMillisec << endl;
	//<< "申买价一：" << pDepthMarketData->BidPrice1 << endl
	//<< "申买量一:" << pDepthMarketData->BidVolume1 << endl
	//<< "申卖价一:" << pDepthMarketData->AskPrice1 << endl
	//<< "申卖量一:" << pDepthMarketData->AskVolume1 << endl
	//<< "今收盘价:" << pDepthMarketData->ClosePrice << endl
	//<< "当日均价:" << pDepthMarketData->AveragePrice << endl
	//<< "本次结算价格:" << pDepthMarketData->SettlementPrice << endl
	//<< "成交金额:" << pDepthMarketData->Turnover << endl
	//<< "持仓量:" << pDepthMarketData->OpenInterest << endl;
}

void Strategy::setTrader(Trader *trader) {
	this->trader = trader;
}

Trader * Strategy::getTrader() {
	return this->trader;
}

void Strategy::setUser(User *user) {
	this->user = user;
}

User * Strategy::getUser() {
	return this->user;
}

void Strategy::setStrategyId(string strategyid) {
	this->strategyid = strategyid;
}

string Strategy::getStrategyId() {
	return this->strategyid;
}

void Strategy::setAlgorithm(Algorithm *alg) {
	this->alg = alg;
}

Algorithm * Strategy::getAlgorithm() {
	return this->alg;
}

void Strategy::addInstrumentToList(string instrument) {
	this->l_instruments->push_back(instrument);
}

list<string> * Strategy::getListInstruments() {
	return this->l_instruments;
}

void Strategy::setTraderID(string traderid) {
	this->traderid = traderid;
}
string Strategy::getTraderID() {
	return this->traderid;
}

void Strategy::setUserID(string userid) {
	this->userid = userid;
}
string Strategy::getUserID() {
	return this->userid;
}

void Strategy::setIsActive(string isActive) {
	this->isActive = isActive;
}
string Strategy::getIsActive() {
	return this->isActive;
}
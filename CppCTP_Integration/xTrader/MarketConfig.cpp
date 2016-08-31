#include "MarketConfig.h"

MarketConfig::MarketConfig() {

}

MarketConfig::MarketConfig(string market_id, string market_frontAddr, string broker_id, string user_id, string password, string isactive) {
	this->market_frontAddr = market_frontAddr;
	this->market_id = market_id;
	this->isactive = isactive;
	this->broker_id = broker_id;
	this->user_id = user_id;
	this->password = password;
}

void MarketConfig::setMarketFrontAddr(string market_frontAddr) {
	this->market_frontAddr = market_frontAddr;
}

void MarketConfig::setBrokerID(string broker_id) {
	this->broker_id = broker_id;
}

void MarketConfig::setUserID(string user_id) {
	this->user_id = user_id;
}

void MarketConfig::setPassword(string password) {
	this->password = password;
}

void MarketConfig::setMarketID(string market_id) {
	this->market_id = market_id;
}

void MarketConfig::setIsActive(string isactive) {
	this->isactive = isactive;
}

string MarketConfig::getMarketFrontAddr() {
	return this->market_frontAddr;
}

string MarketConfig::getBrokerID() {
	return this->broker_id;
}

string MarketConfig::getUserID() {
	return this->user_id;
}

string MarketConfig::getPassword() {
	return this->password;
}

string MarketConfig::getMarketID() {
	return this->market_id;
}

string MarketConfig::getIsActive() {
	return this->isactive;
}
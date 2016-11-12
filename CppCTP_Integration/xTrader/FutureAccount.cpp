#include "FutureAccount.h"
#define ISACTIVE "1"
#define ISNOTACTIVE "0"

FutureAccount::FutureAccount() {
	this->frontAddress = "";
	this->userID = "";
	this->password = "";
	this->traderID = "";
	this->isActive = ISACTIVE;
	this->brokerID = "";
}
FutureAccount::FutureAccount(string frontAddress, string userID, string password, string traderID, string isActive, string brokerID) {
	this->frontAddress = frontAddress;
	this->userID = userID;
	this->password = password;
	this->traderID = traderID;
	this->isActive = isActive;
	this->brokerID = brokerID;
	this->on_off = 1;
}
FutureAccount::~FutureAccount() {
	delete this;
}

void FutureAccount::setUserID(string userID) {
	this->userID = userID;
}
string FutureAccount::getUserID() {
	return this->userID;
}
void FutureAccount::setPassword(string password) {
	this->password = password;
}
string FutureAccount::getPassword() {
	return this->password;
}
void FutureAccount::setBrokerID(string brokerID) {
	this->brokerID = brokerID;
}
string FutureAccount::getBrokerID() {
	return this->brokerID;
}

void FutureAccount::setTrader(Trader *op) {
	this->op = op;
	this->setTraderID(op->getTraderID());
}

Trader * FutureAccount::getTrader() {
	return this->op;
}

void FutureAccount::setTraderID(string traderid) {
	this->traderID = traderid;
}
string FutureAccount::getTraderID() {
	return this->traderID;
}

void FutureAccount::setIsActive(string isActive) {
	this->isActive = isActive;
}
string FutureAccount::getIsActive() {
	return this->isActive;
}

void FutureAccount::setFrontAddress(string frontAddress) {
	this->frontAddress = frontAddress;
}
string FutureAccount::getFrontAddress() {
	return this->frontAddress;
}

void FutureAccount::setOn_Off(int on_off) {
	this->on_off = on_off;
}
int FutureAccount::getOn_Off() {
	return this->on_off;
}
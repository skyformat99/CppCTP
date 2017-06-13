#include "Trader.h"
#include "Debug.h"
#define ISACTIVE "1"
#define ISNOTACTIVE "0"

Trader::Trader(string traderid, string password, string tradername, string isactive) {
	this->traderid = traderid;
	this->password = password;
	this->tradername = tradername;
	this->isactive = isactive;
	this->on_off = 0;
}

Trader::Trader() {
	this->traderid = "";
	this->password = "";
	this->tradername = "";
	this->isactive = ISACTIVE;
	this->on_off = 0;
}

Trader::~Trader() {
	delete this;
}

void Trader::setPassword(string password) {
	this->password = password;
}

string Trader::getPassword() {
	return this->password;
}

void Trader::setIsActive(string isactive) {
	this->isactive = isactive;
}
string Trader::getIsActive() {
	return this->isactive;
}

void Trader::setTraderName(string tradername) {
	this->tradername = tradername;
}
string Trader::getTraderName() {
	return this->tradername;
}

void Trader::setTraderID(string traderid) {
	this->traderid = traderid;
}
string Trader::getTraderID() {
	return this->traderid;
}

void Trader::setOn_Off(int on_off) {
	this->on_off = on_off;
}
int Trader::getOn_Off() {
	return this->on_off;
}
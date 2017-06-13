#include "Session.h"

Session::Session(string userid, int sessionid, int frontid, string tradingday) {
	this->userid = userid;
	this->sessionid = sessionid;
	this->frontid = frontid;
	this->tradingday = tradingday;
}
Session::~Session() {
	
}

void Session::setUserID(string userid) {
	this->userid = userid;
}

string Session::getUserID() {
	return this->userid;
}

void Session::setSessionID(int sessionid) {
	this->sessionid = sessionid;
}

int Session::getSessionID() {
	return this->sessionid;
}

void Session::setFrontID(int frontid) {
	this->frontid = frontid;
}

int Session::getFrontID() {
	return this->frontid;
}

void Session::setTradingDay(string tradingday) {
	this->tradingday = tradingday;
}

string Session::getTradingDay() {
	return this->tradingday;
}
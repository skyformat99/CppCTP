#ifndef QUANT_SESSIONID_H
#define QUANT_SESSIONID_H
#include <iostream>

using namespace std;
using std::string;

class Session
{
public:

	Session(string userid, int sessionid, int frontid, string tradingday);
	~Session();

	void setUserID(string userid);
	string getUserID();

	void setSessionID(int sessionid);
	int getSessionID();

	void setFrontID(int frontid);
	int getFrontID();

	void setTradingDay(string tradingday);
	string getTradingDay();


private:
	string userid;
	int sessionid;
	int frontid;
	string tradingday;
};

#endif
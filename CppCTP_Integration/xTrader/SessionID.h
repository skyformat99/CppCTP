#ifndef QUANT_SESSIONID_H
#define QUANT_SESSIONID_H
#include <iostream>

using namespace std;
using std::string;

class SessionID
{
public:

	SessionID(string userid, int sessionid, int frontid);
	~SessionID();

	void setUserID(string userid);
	string getUserID();

	void setSessionID(int sessionid);
	int getSessionID();

	void setFrontID(int frontid);
	int getFrontID();

private:
	string userid;
	int sessionid;
	int frontid;
};

#endif
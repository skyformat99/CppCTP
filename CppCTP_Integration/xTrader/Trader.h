#include <string>
#include <string.h>
#include <list>
#include "User.h"

using std::string;
using std::list;

class User;

#ifndef QUANT_TRADER_H
#define QUANT_TRADER_H

class Trader {
public:

	Trader(string userid, string password, string username, string isactive);
	Trader();
	~Trader();

	void setPassword(string password);
	string getPassword();

	void setIsActive(string isactive);
	string getIsActive();

	void setTraderName(string tradername);
	string getTraderName();

	void setTraderID(string traderid);
	string getTraderID();

	void addUserToLUser(User *user);
	void getLUser();

private:
	string tradername;
	string password;
	string traderid;
	string isactive;
	list<User *> l_user;
};


#endif
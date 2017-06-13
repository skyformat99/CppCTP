#include <iostream>
#include <string.h>
using namespace std;
using std::string;

#ifndef QUANT_CTP_LOGIN_H
#define QUANT_CTP_LOGIN_H

class Login {
public:
	Login();
	bool CheckIn(string username, string password);
	~Login();
private:
	string username;
	string password;
};

#endif
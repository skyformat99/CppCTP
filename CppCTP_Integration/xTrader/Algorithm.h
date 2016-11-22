

#ifndef QUANT_ALGORITHM_H
#define QUANT_ALGORITHM_H
#include <string>
#include <string.h>
using std::string;

class Algorithm {

public:

	Algorithm(string is_active = "1");

	void setAlgName(string name);
	string getAlgName();

	void setIsActive(string is_active);
	string getIsActive();

private:
	string name;
	string is_active;
};

#endif
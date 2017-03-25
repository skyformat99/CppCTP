#include <iostream>
#include "Lv2MD.h"
using namespace std;

int main(char *, char *argv[]) {
	int choice;
	_LTS_::Lv2MD *md = new _LTS_::Lv2MD("tcp://10.200.77.18:8800", "2011", "010000000083", "");
	while (true) {
		std::cin >> choice;
		std::cout << "choice = " << choice << std::endl;
	}
	return 0;
}
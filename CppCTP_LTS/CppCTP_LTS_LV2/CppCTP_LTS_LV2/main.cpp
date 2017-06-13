#include <iostream>
#include "Lv2MD.h"
using namespace std;

int main(char *, char *argv[]) {
	int choice;
	//Level2ÐÐÇéµØÖ·
	_LTS_::Lv2MD *md = new _LTS_::Lv2MD("tcp://222.66.55.171:8900", "2011", "010000040178", "7174516");
	while (true) {
		std::cin >> choice;
		std::cout << "choice = " << choice << std::endl;
	}
	return 0;
}
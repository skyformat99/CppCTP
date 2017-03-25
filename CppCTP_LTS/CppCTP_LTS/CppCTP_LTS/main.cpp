#include <iostream>
#include "MdClass.h"
using namespace std;
using std::cout;
using std::endl;

class MdClass;

int main(char *, char *argv[]) {
	int choice;
	std::cout << "hello, world" << std::endl;
	MdClass *md = new MdClass("tcp://211.144.195.163:54513", "2011", "010000040178", "", "conn/md");
	while (true)
	{
		std::cin >> choice;
		std::cout << "choice = " << choice << std::endl;
	}
	return 0;
}
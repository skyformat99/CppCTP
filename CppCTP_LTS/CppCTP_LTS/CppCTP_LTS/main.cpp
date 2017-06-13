#include <iostream>
#include "MdClass.h"
using namespace std;
using std::cout;
using std::endl;

class MdClass;

int main(char *, char *argv[]) {
	int choice;
	//Level1––«Èµÿ÷∑
	MdClass *md = new MdClass("tcp://180.166.216.228:24513", "2011", "010000040178", "7174516", "conn/md");
	/*list<string> list_instrumentid;
	string instrument_id = "SH000001";
	list_instrumentid.push_back(instrument_id);
	char* pExchageID = "SSE";
	md->SubMarketData(list_instrumentid, pExchageID);*/

	while (true)
	{
		std::cin >> choice;
		std::cout << "choice = " << choice << std::endl;
	}
	return 0;
}
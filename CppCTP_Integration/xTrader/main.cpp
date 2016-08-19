#include <iostream>
#include <string>
#include <list>
#include <mongo/client/dbclient.h>
#include <stdio.h>
#include "ThostFtdcTraderApi.h"
#include "TdSpi.h"
#include "ThostFtdcMdApi.h"
#include "MdSpi.h"
#include "CTP_Manager.h"
#include "Utils.h"
#include "Debug.h"
#include "DBManager.h"
#include "Trader.h"
#include "FutureAccount.h"

using std::cout;
using std::cin;

void printMenuEN() {
	cout << "|==========================|" << endl;
	cout << "|Please Input Your Choice: |" << endl;
	cout << "|i:Order Insert            |" << endl;
	cout << "|a:Order Action            |" << endl;
	cout << "|b:Break                   |" << endl;
	cout << "|e:Qry Exchange            |" << endl;
	cout << "|s:Qry Instrument          |" << endl;
	cout << "|o:Qry Order               |" << endl;
	cout << "|t:Qry Trading Account     |" << endl;
	cout << "|u:Qry Investor            |" << endl;
	cout << "|d:Qry Trade               |" << endl;
	cout << "|h:Qry Investor Position   |" << endl;
	cout << "|==========================|" << endl;
}

void printMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:交易员登陆              |" << endl;
	cout << "|【2】:管理员登陆              |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
}

void printContinue() {
	cout << "|======================|" << endl;
	cout << "|请输入【c】继续操作   |" << endl;
	cout << "|======================|" << endl;
}

void printContinueEN() {
	cout << "|======================|" << endl;
	cout << "|Input 'c' to Continue:|" << endl;
	cout << "|======================|" << endl;
}

void printLoginSuccessMenu() {
	cout << "|=============|" << endl;
	printf("|\033[47;30m【登陆成功!】\033[0m|\n");
	cout << "|=============|" << endl;
}

void printLoginFailedMenu() {
	cout << "|========================|" << endl;
	printf("|\033[47;31m【！！！登录失败！！！】\033[0m|\n");
	cout << "|========================|" << endl;
}

void printErrorInputMenu() {
	cout << "|========================|" << endl;
	printf("|\033[47;31m【！！！重新输入！！！】\033[0m|\n");
	cout << "|========================|" << endl;
}

void printWelcome() {
	cout << "|===========================|" << endl;
	//cout << "|欢迎登录期货多账户交易系统!|" << endl;
	printf("|\033[47;30m欢迎登录期货多账户交易系统!\033[0m|\n");
	cout << "|===========================|" << endl;
}

void printLoginTraderOperatorMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:账户查询                |" << endl;
	cout << "|【2】:持仓查询                |" << endl;
	cout << "|【3】:报单查询                |" << endl;
	cout << "|【4】:成交查询                |" << endl;
	cout << "|【5】:报单                    |" << endl;
	cout << "|【6】:撤单                    |" << endl;
	cout << "|【7】:订阅行情                |" << endl;
	cout << "|【8】:退订行情                |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
}

void printFutureAccountListMenu() {
	cout << "|==============================|" << endl;
	//cout << "|\033[40;32m当前您所管理的期货账户列表:   \033[0m|\n" << endl;
	printf("|\033[47;30m当前您所管理的期货账户列表:   \033[0m|\n");
	cout << "|==============================|" << endl;
}

void printTraderAccoutListMenu() {
	cout << "|==============================|" << endl;
	printf("|\033[47;30m当前您所管理的交易员列表:     \033[0m|\n");
	cout << "|==============================|" << endl;
}

void printAdminOperateMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:交易员管理              |" << endl;
	cout << "|【2】:期货账户管理            |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
}

void printTraderOperateMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:查看所有交易员          |" << endl;
	cout << "|【2】:增加交易员              |" << endl;
	cout << "|【3】:删除交易员              |" << endl;
	cout << "|【4】:修改交易员              |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
}

void printFutureAccountOperateMenu() {
	cout << "|==============================|" << endl;
	cout << "|请输入您的操作编号:           |" << endl;
	cout << "|【1】:查看所有期货账户        |" << endl;
	cout << "|【2】:增加期货账户            |" << endl;
	cout << "|【3】:删除期货账户            |" << endl;
	cout << "|【4】:修改期货账户            |" << endl;
	cout << "|【q】:退出                    |" << endl;
	cout << "|==============================|" << endl;
}

int main() {
	mongo::client::initialize();

	/************************************************************************/
	/*   标准CTP：
			Trade Front：180.168.146.187:10000，
			Market Front：180.168.146.187:10010；【电信】
		第二套：
			交易前置：180.168.146.187:10030，
			行情前置：180.168.146.187:10031；【7x24】                                                                   */
	/************************************************************************/

	string trade_frontAddr = "tcp://180.168.146.187:10000"; //仿真
	string broker_id = "9999";
	string user_id = "058176";
	string password = "669822";

	//string trade_frontAddr = "tcp://180.169.75.19:41205"; //实盘
	//string broker_id = "0187";
	//string user_id = "86001525";
	//string password = "206029";
	
	string market_frontAddr = "tcp://180.168.146.187:10011"; //实盘


	//CThostFtdcTraderApi *tdapi = CThostFtdcTraderApi::CreateFtdcTraderApi("./conn/user1/");
	//TdSpi *tdspi = new TdSpi("/conn/user1/");

	#if 0
	CThostFtdcMdApi *mdapi = CThostFtdcMdApi::CreateFtdcMdApi("./conn/market");
	MdSpi *mdspi = new MdSpi(mdapi);
	mdspi->Connect(const_cast<char *>(market_frontAddr.c_str())); //Standard
	sleep(1);
	mdspi->Login(const_cast<char *>(broker_id.c_str()), const_cast<char *>(user_id.c_str()), const_cast<char *>(password.c_str()));

	//订阅合约所以数量为3
	string array[] = { "cu1608", "cu1609", "zn1608", "zn1609" };
	cout << "total string size is:" << sizeof(array) / sizeof(string) << endl;
	int size = sizeof(array) / sizeof(string);
	char **instrumentID = new char *[size];
	int i;
	for (i = 0; i < size; i++) {
		const char *charResult = array[i].c_str();
		instrumentID[i] = new char[strlen(charResult) + 1];
		strcpy(instrumentID[i], charResult);
		//cout << instrumentID[i] << endl;
		//usleep(500000);
	}

	//cout << instrumentID << endl;
	//mdspi->SubMarketData(instrumentID, size);

	sleep(2);
#endif

	/************************************************************************/
	/* new multi accout test                                                */
	/************************************************************************/

	CTP_Manager *ctp_m = new CTP_Manager();
	//User *user1 = ctp_m->CreateAccount("tcp://180.168.146.187:10000", "9999", "058176", "669822");
	
	//sleep(1);
	
	//User *user2 = ctp_m->CreateAccount("tcp://180.168.146.187:10000", "9999", "063802", "123456");

	//sleep(3);

	/************************************************************************/
	/* 测试行情                                                              */
	/************************************************************************/
	/*MdSpi *mdspi1 = ctp_m->CreateMd(market_frontAddr, broker_id, user_id, password);

	list<string> l_instrument = ctp_m->addInstrument("cu1609", ctp_m->getL_Instrument());
	l_instrument = ctp_m->addInstrument("ag1612", l_instrument);
	l_instrument = ctp_m->addInstrument("cu1609", l_instrument);
	l_instrument = ctp_m->addInstrument("cu1609", l_instrument);
	l_instrument = ctp_m->addInstrument("cu1609", l_instrument);

	ctp_m->submarketData(mdspi1, l_instrument);*/


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//string instrument = "cu1701";
	//string instrument2 = "cu1609";
	//user1->getUserTradeSPI()->QryDepthMarketData(instrument);
	//sleep(1);
	//user1->getUserTradeSPI()->QryDepthMarketData(instrument2);
	//sleep(1);
	////sleep()
	//user2->getUserTradeSPI()->QryDepthMarketData(instrument);
	//sleep(1);
	//user2->getUserTradeSPI()->QryDepthMarketData(instrument2);

	//if (mdapi) {
	//	mdapi->RegisterSpi(NULL);
	//	mdapi->Release();
	//}
	//if (mdspi) {
	//	delete mdspi;
	//}

	/************************************************************************/
	/* 测试DBmanager                                                         */
	/************************************************************************/
	
	//DBManager *dbm = new DBManager();
	//DBManager dbm;

	//Trader *op = new Trader();
	//op->setIsActive("1");
	//op->setTraderID("1111");
	//op->setPassword("xxxx");
	//op->setTraderName("lucas");
	

	//dbm->CreateTrader(op);
	//dbm->DeleteTrader(op);
	//dbm->UpdateTrader(op);
	//dbm->SearchTraderByTraderID("1111");
	//dbm->SearchTraderByTraderName("lucas");
	//dbm->SearchTraderByTraderIdAndPassword("1111", "xxxx");
	//cout << dbm->FindTraderByTraderIdAndPassword("1", "xxxx") << endl;

	//FutureAccount *fa = new FutureAccount();
	//fa->setBrokerID("9999");
	//fa->setIsActive("1");
	//fa->setTrader(op);
	//fa->setUserID("058176");
	//fa->setPassword("123456");
	//fa->setFrontAddress("tcp://180.168.146.187:10011");

	//dbm->CreateFutureAccount(op, fa);
	//dbm->DeleteFutureAccount(op, fa);
	//dbm->UpdateFutureAccount(op, fa);
	//dbm->SearchFutrueByTraderID(op->getTraderID());
	//dbm->SearchFutrueByUserID(fa->getUserID());

	//list<FutureAccount *> *l_futureaccount = new list<FutureAccount *>();
	//ctp_m->getDBManager()->SearchFutrueListByTraderID("1", l_futureaccount);
	//list<FutureAccount *>::iterator itor;
	//for (itor = l_futureaccount->begin(); itor != l_futureaccount->end(); itor++) {
	//	cout << "l_futureaccount member : " << (*itor)->getUserID() << endl;
	//}

	string chooice;
	string userid; //期货账户id
	printWelcome();

	printMenu();
	cin >> chooice;
	
	Trader *op = new Trader();
	FutureAccount *fa = new FutureAccount();
	//list<FutureAccount *> *l_futureaccount = new list<FutureAccount *>();
	
	/*订阅行情*/

	MdSpi *mdspi1 = ctp_m->CreateMd(market_frontAddr, broker_id, user_id, password);
	list<string> l_Sub_instrument;
	list<string> l_UnSub_instrument;

	//= ctp_m->addInstrument("cu1609", ctp_m->getL_Instrument());
	

	while (chooice != "q") {
		/************************************************************************/
		/* 交易员登陆           CreateMd                                          */
		/************************************************************************/
		if (chooice == "1") {
			string traderid;
			string traderpassword;
			bool flag;
			cout << "请输入交易员账号:" << endl;
			cin >> traderid;
			cout << "请输入交易员密码:" << endl;
			cin >> traderpassword;

			flag = ctp_m->TraderLogin(traderid, traderpassword);
			if (flag) { //交易员登陆成功
				printLoginSuccessMenu();
				printFutureAccountListMenu();
				op->setTraderID(traderid);
				op->setPassword(traderpassword);

				bool create_flag = false;
				create_flag = ctp_m->checkInLTrader(traderid);
				list<FutureAccount *> *l_futureaccount = new list<FutureAccount *>();
				ctp_m->getDBManager()->SearchFutrueListByTraderID(traderid, l_futureaccount);
				list<FutureAccount *>::iterator itor;
				for (itor = l_futureaccount->begin(); itor != l_futureaccount->end(); itor++) {
					cout << "【期货账户 : " << (*itor)->getUserID() << ", "
						<< "经纪公司ID : " << (*itor)->getBrokerID() << ", "
						<< "密码 : " << (*itor)->getPassword() << ", "
						<< "交易前置地址 : " << (*itor)->getFrontAddress() << "】" << endl;
					if (!create_flag) {
						Trader *new_trader = new Trader();
						new_trader->setTraderID(traderid);
						new_trader->setPassword(password);
						ctp_m->CreateAccount((*itor)->getFrontAddress(), (*itor)->getBrokerID(), (*itor)->getUserID(), (*itor)->getPassword(), new_trader);
					}
				}
				//将已登录的账户存入登陆列表里
				ctp_m->addTraderToLTrader(traderid);

				/************************************************************************/
				/* 进入交易员操作界面                                                      */
				/************************************************************************/
				printLoginTraderOperatorMenu();
				cin >> chooice;

				while (chooice != "q") {
					if (chooice == "1") {
						cout << "账户查询" << endl;
						list<string> l_trader = ctp_m->getL_Trader();
						list<string>::iterator itor;
						for (itor = l_trader.begin(); itor != l_trader.end(); itor++) {
							// 打印交易员id
							cout << "【交易员账户 : " << (*itor) << "】" << endl;
							//根据交易员id获取账户
							list<FutureAccount *> *l_futureaccount = new list<FutureAccount *>();
							ctp_m->getDBManager()->SearchFutrueListByTraderID((*itor), l_futureaccount);
							list<FutureAccount *>::iterator inner_itor;
							for (inner_itor = l_futureaccount->begin(); inner_itor != l_futureaccount->end(); inner_itor++) {
								cout << "【期货账户:" << (*inner_itor)->getUserID() << "】" << endl;
								
							}
							delete l_futureaccount;
						}
					}
					else if (chooice == "2") {
						cout << "持仓查询" << endl;
						//根据不同的期货账户User查找对应的持仓信息
						if (ctp_m->getL_User()->size() == 0) {
							cout << "NULL" << endl;
						}
						else {
							cout << "NOT NULL" << endl;
							list<User *>::iterator itor;
							for (itor = ctp_m->getL_User()->begin(); itor != ctp_m->getL_User()->end(); itor++) {
								cout << "【期货账户:" << (*itor)->getUserID() << "】" << endl;
								(*itor)->getUserTradeSPI()->QryInvestorPosition();
								sleep(1);
								(*itor)->getUserTradeSPI()->QryInvestor();
							}
						}
						
					}
					else if (chooice == "3") {
						cout << "报单查询" << endl;
						cout << "请输入期货账户ID" << endl;
						cin >> userid;
						//根据不同的期货账户User查找对应的持仓信息
						if (ctp_m->getL_User()->size() == 0) {
							cout << "NULL" << endl;
						}
						else {
							cout << "NOT NULL" << endl;
							list<User *>::iterator itor;
							for (itor = ctp_m->getL_User()->begin(); itor != ctp_m->getL_User()->end(); itor++) {
								cout << "【期货账户:" << (*itor)->getUserID() << "】" << endl;
								if ((*itor)->getUserID() == userid) {
									(*itor)->getUserTradeSPI()->QryOrder();
								}
							}
						}
					}
					else if (chooice == "4") {
						cout << "成交查询" << endl;
						cout << "请输入期货账户ID" << endl;
						cin >> userid;
						//根据不同的期货账户User查找对应的持仓信息
						if (ctp_m->getL_User()->size() == 0) {
							cout << "NULL" << endl;
						}
						else {
							cout << "NOT NULL" << endl;
							list<User *>::iterator itor;
							for (itor = ctp_m->getL_User()->begin(); itor != ctp_m->getL_User()->end(); itor++) {
								cout << "【期货账户:" << (*itor)->getUserID() << "】" << endl;
								if ((*itor)->getUserID() == userid) { //成交查询
									(*itor)->getUserTradeSPI();
								}
							}
						}
					}
					else if (chooice == "5") {
						cout << "报单" << endl;
						cout << "请输入期货账户ID" << endl;
						cin >> userid;
						//根据不同的期货账户User查找对应的持仓信息
						if (ctp_m->getL_User()->size() == 0) {
							cout << "NULL" << endl;
						}
						else {
							cout << "NOT NULL" << endl;
							list<User *>::iterator itor;
							for (itor = ctp_m->getL_User()->begin(); itor != ctp_m->getL_User()->end(); itor++) {
								cout << "【期货账户:" << (*itor)->getUserID() << "】" << endl;
								if ((*itor)->getUserID() == userid) { // 开始下单
									/*order insert*/
									string order_InstrumentID;
									char order_CombOffsetFlag;
									char order_Direction;
									int order_Volume;
									double order_Price;
									string order_OrderRef;

									cout << "Order Insert Operation" << endl;
									/************************************************************************/
									/* Code Below  Order Insert                                             */
									/************************************************************************/
									cout << "Please input instrumentID, such as cu1609, zn1701..." << endl;
									cin >> order_InstrumentID;
									cout << "Please input Comboffsetflag" << endl;
									cout << "0:Open 1:Close 2:ForceClose 3:CloseToday 4:CloseYesterday" << endl;
									cin >> order_CombOffsetFlag;
									cout << "Please input Direction" << endl;
									cout << "0:Buy 1:Sell" << endl;
									cin >> order_Direction;
									cout << "Please input Volume, such as 10, 50, 100..." << endl;
									cin >> order_Volume;
									cout << "Please input Price" << endl;
									cin >> order_Price;
									cout << "Please input OrderRef, such as 1, 2, 3 or user specific" << endl;
									cin >> order_OrderRef;

									cout << "order_InstrumentID = " << order_InstrumentID << endl;
									cout << "order_CombOffsetFlag = " << order_CombOffsetFlag << endl;
									cout << "order_Direction = " << order_Direction << endl;
									cout << "order_Volume = " << order_Volume << endl;
									cout << "order_Price = " << order_Price << endl;
									cout << "order_OrderRef = " << order_OrderRef << endl;
									
									(*itor)->getUserTradeSPI()->OrderInsert((*itor), const_cast<char *>(order_InstrumentID.c_str()), order_CombOffsetFlag, order_Direction, order_Volume, order_Price, order_OrderRef);
								}
							}
						}

					}
					else if (chooice == "6") {
						cout << "撤单" << endl;
						cout << "请输入期货账户ID" << endl;
						cin >> userid;
						//根据不同的期货账户User查找对应的持仓信息
						if (ctp_m->getL_User()->size() == 0) {
							cout << "NULL" << endl;
						}
						else {
							cout << "NOT NULL" << endl;
							list<User *>::iterator itor;
							for (itor = ctp_m->getL_User()->begin(); itor != ctp_m->getL_User()->end(); itor++) {
								cout << "【期货账户:" << (*itor)->getUserID() << "】" << endl;
								if ((*itor)->getUserID() == userid) { // 开始撤单
									/*order action delete*/
									string action_ExchangeId;
									int action_int_ExchangeId;
									string action_OrderRef;
									string action_OrderSysId;

									cout << "撤单操作" << endl;
									/************************************************************************/
									/* Code Below  Order Action                                             */
									/************************************************************************/
									cout << "请选择期货交易所" << endl;
									cout << "1:上海期货交易所 2:大连商品交易所 3:郑州商品交易所 4:中国金融期货交易所" << endl;
									cin >> action_int_ExchangeId;
									cout << "请输入报单引用" << endl;
									cin >> action_OrderRef;
									cout << "请输入报单编号" << endl;
									cin.ignore(0x7fffffff, '\n');
									getline(cin, action_OrderSysId);

									switch (action_int_ExchangeId)
									{
									case 1:action_ExchangeId = "SHFE"; break;
									case 2:action_ExchangeId = "DCE"; break;
									case 3:action_ExchangeId = "CZCE"; break;
									case 4:action_ExchangeId = "CFFEX"; break;
									default:
										action_ExchangeId = "";
										break;
									}

									cout << "action_ExchangeId = " << action_ExchangeId << endl;
									cout << "action_OrderRef = " << action_OrderRef << endl;
									cout << "action_OrderSysId = " << action_OrderSysId << endl;

									(*itor)->getUserTradeSPI()->OrderAction(action_ExchangeId, action_OrderRef, action_OrderSysId);
								}
							}
						}
					}
					else if (chooice == "7") {
						cout << "订阅行情" << endl;
						string instrumentid;
						cout << "请输入你要订阅的合约id，例如cu1609" << endl;
						cin >> instrumentid;
						l_Sub_instrument = ctp_m->addSubInstrument(instrumentid, l_Sub_instrument);
						int count = ctp_m->calInstrument(instrumentid, l_Sub_instrument);
						USER_PRINT(count);
						if (count == 0 || count == 1) {
							ctp_m->SubmarketData(mdspi1, l_Sub_instrument);
						}
						else {
							cout << "已经订阅该行情!" << endl;
						}
						
					}
					else if (chooice == "8") {
						cout << "退订行情" << endl;
						string instrumentid;
						cout << "请输入你要退订的合约id，例如cu1609" << endl;
						cin >> instrumentid;
						l_Sub_instrument = ctp_m->delSubInstrument(instrumentid, l_Sub_instrument);
						if ((ctp_m->calInstrument(instrumentid, l_Sub_instrument) <= 0)) { // 如果合约列表里没有了该项,那么就取消订阅
							l_UnSub_instrument = ctp_m->addUnSubInstrument(instrumentid, l_UnSub_instrument);
							if (l_UnSub_instrument.size() > 0) {
								cout << "l_UnSub_instrument NOT NULL" << endl;
							}
							else {
								cout << "l_UnSub_instrument NULL" << endl;
							}
							ctp_m->UnSubmarketData(mdspi1, l_UnSub_instrument);
						}
					}
					else {
						printErrorInputMenu();
					}
					/************************************************************************/
					/* 进入交易员操作界面                                                      */
					/************************************************************************/
					printLoginTraderOperatorMenu();
					cin >> chooice;
				}

			}
			else { //登陆失败打印登陆失败菜单
				printLoginFailedMenu();
			}
			
		}
		/************************************************************************/
		/* 管理员登陆                                                             */
		/************************************************************************/
		else if (chooice == "2") {
			string adminid;
			string adminpassword;
			bool flag;
			cout << "请输入管理员账号:" << endl;
			cin >> adminid;
			cout << "请输入管理员密码:" << endl;
			cin >> adminpassword;
			
			flag = ctp_m->AdminLogin(adminid, adminpassword);
			if (flag) {
				printLoginSuccessMenu();
				printAdminOperateMenu();
				cin >> chooice;
				while (chooice != "q")
				{
					if (chooice == "1") { //交易员管理
						cout << "交易员管理" << endl;
						printTraderOperateMenu();
						cin >> chooice;
						while (chooice != "q") {
							if (chooice == "1") { //查询交易员
								//ctp_m->getDBManager()->getAllTrader();
								printTraderAccoutListMenu();
								ctp_m->getDBManager()->getAllTrader();
							}
							else if (chooice == "2") { //增加交易员

								cout << "当前系统已存在交易员" << endl;
								//ctp_m->getDBManager()->getAllTrader();
								printTraderAccoutListMenu();
								ctp_m->getDBManager()->getAllTrader();

								string tradername;
								string password;
								string traderid;
								string isactive;
								cout << "请输入交易员ID:" << endl;
								cin >> traderid;
								cout << "请输入交易员名字:" << endl;
								cin >> tradername;
								cout << "请输入交易员密码:" << endl;
								cin >> password;
								op->setTraderID(traderid);
								op->setTraderName(tradername);
								op->setPassword(password);
								
								//ctp_m->getDBManager()->CreateTrader(op);
								ctp_m->getDBManager()->CreateTrader(op);

								//delete op;
							}
							else if (chooice == "3") { //删除交易员
								cout << "当前系统已存在交易员" << endl;
								//打印当前存在的交易员
								printTraderAccoutListMenu();
								ctp_m->getDBManager()->getAllTrader();

								string traderid;
								cout << "请输入要删除的交易员的ID" << endl;
								cin >> traderid;
								op->setTraderID(traderid);
								ctp_m->getDBManager()->DeleteTrader(op);
								
							}
							else if (chooice == "4") { //修改交易员
								cout << "当前系统已存在交易员" << endl;
								//打印当前存在的交易员
								printTraderAccoutListMenu();
								ctp_m->getDBManager()->getAllTrader();

								string tradername;
								string password;
								string traderid;
								string new_traderid;
								string isactive;
								cout << "请输入交易员ID:" << endl;
								cin >> traderid;
								cout << "请输入交易员名字:" << endl;
								cin >> tradername;
								cout << "请输入交易员密码:" << endl;
								cin >> password;
								cout << "请输入交易员新ID:" << endl;
								cin >> new_traderid;
								cout << "请输入交易员激活状态" << endl;
								cin >> isactive;
								op->setTraderID(new_traderid);
								op->setTraderName(tradername);
								op->setPassword(password);
								op->setIsActive(isactive);
								ctp_m->getDBManager()->UpdateTrader(traderid, op);

							}
							else { //输入错误
								printErrorInputMenu();
							}
							//继续在交易员管理进行循环
							printTraderOperateMenu();
							cin >> chooice;
						}
					}
					else if (chooice == "2") { //期货账户管理
						cout << "期货账户管理" << endl;
						printFutureAccountOperateMenu();
						cin >> chooice;
						while (chooice != "q") {
							if (chooice == "1") { //查询期货账户
								printFutureAccountListMenu();
								ctp_m->getDBManager()->getAllFutureAccount();
							}
							else if (chooice == "2") { //增加期货账户
								cout << "当前系统已存在交易员" << endl;
								//打印当前存在的交易员
								printTraderAccoutListMenu();
								ctp_m->getDBManager()->getAllTrader();

								cout << "当前系统已存在期货账户" << endl;
								//打印当前存在的期货账户
								printFutureAccountListMenu();
								ctp_m->getDBManager()->getAllFutureAccount();

								string userID;
								string password;
								string brokerID;
								string traderID;
								string frontAddress;
								string isActive;
								cout << "请输入所属交易员ID:" << endl;
								cin >> traderID;
								cout << "请输入期货账户ID:" << endl;
								cin >> userID;
								cout << "请输入密码:" << endl;
								cin >> password;
								cout << "请输入BrokerID:" << endl;
								cin >> brokerID;
								cout << "请输入交易行情前置地址:" << endl;
								cin >> frontAddress;
								//cout << "请输入是否激活" << endl;
								//cin >> isActive;
								
								fa->setUserID(userID);
								fa->setPassword(password);
								fa->setBrokerID(brokerID);
								fa->setTraderID(traderID);
								fa->setFrontAddress(frontAddress);
								//fa->setIsActive(isActive);
								op->setTraderID(traderID);

								ctp_m->getDBManager()->CreateFutureAccount(op, fa);

							}
							else if (chooice == "3") { //删除期货账户
								cout << "当前系统已存在期货账户" << endl;
								//打印当前存在的期货账户
								printFutureAccountListMenu();
								ctp_m->getDBManager()->getAllFutureAccount();

								string userID;
								cout << "请输入账户ID:" << endl;
								cin >> userID;

								fa->setUserID(userID);
								ctp_m->getDBManager()->DeleteFutureAccount(fa);
							}
							else if (chooice == "4") { //修改期货账户
								cout << "当前系统已存在交易员" << endl;
								//打印当前存在的交易员
								printTraderAccoutListMenu();
								ctp_m->getDBManager()->getAllTrader();

								cout << "当前系统已存在期货账户" << endl;
								//打印当前存在的期货账户
								printFutureAccountListMenu();
								ctp_m->getDBManager()->getAllFutureAccount();

								string userID;
								string new_userID;
								string password;
								string brokerID;
								string traderID;
								string frontAddress;
								string isActive;
								cout << "请输入所属交易员ID:" << endl;
								cin >> traderID;
								cout << "请输入期货账户ID:" << endl;
								cin >> userID;
								cout << "请输入密码:" << endl;
								cin >> password;
								cout << "请输入BrokerID:" << endl;
								cin >> brokerID;
								cout << "请输入账户新ID:" << endl;
								cin >> new_userID;
								cout << "请输入交易行情前置地址:" << endl;
								cin >> frontAddress;
								cout << "请输入是否激活" << endl;
								cin >> isActive;

								fa->setUserID(new_userID);
								fa->setPassword(password);
								fa->setBrokerID(brokerID);
								fa->setTraderID(traderID);
								fa->setFrontAddress(frontAddress);
								fa->setIsActive(isActive);
								op->setTraderID(traderID);


								ctp_m->getDBManager()->UpdateFutureAccount(userID, op, fa);
							}
							else { //输入错误
								printErrorInputMenu();
							}
							//继续在期货账户管理进行循环
							printFutureAccountOperateMenu();
							cin >> chooice;
						}
					}
					else {
						printErrorInputMenu();
					}
					/************************************************************************/
					/* 进入管理员操作菜单                                                      */
					/************************************************************************/
					printAdminOperateMenu();
					cin >> chooice;
				}



			}
			else {
				printLoginFailedMenu();
			}
		}
		else {
			printErrorInputMenu();
		}

		printMenu();
		cin >> chooice;
	}
	return 0;
}
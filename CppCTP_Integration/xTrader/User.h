#ifndef QUANT_USER_H
#define QUANT_USER_H

#include <iostream>
#include <string>
#include <cstring>
#include <mongo/client/dbclient.h>
#include "ThostFtdcTraderApi.h"
#include "TdSpi.h"
#include "DBManager.h"
#include "Order.h"
#include "Trader.h"
#include "Strategy.h"
#include "CTP_Manager.h"

using namespace mongo;
using namespace std;
using std::string;
using mongo::DBClientConnection;
using mongo::BSONObjBuilder;

class CTP_Manager;

class User
{
public:
	User(string frontAddress, string BrokerID, string UserID, string Password, string nRequestID, int on_off, string TraderID = "");
	User(string BrokerID, string UserID, int nRequestID);
	~User();
	string getBrokerID();
	string getUserID();
	string getPassword();
	int getRequestID();
	bool getIsLogged();
	bool getIsFirstTimeLogged();
	bool getIsConfirmSettlement();
	int getLoginRequestID();
	CThostFtdcTraderApi *getUserTradeAPI();
	class TdSpi *getUserTradeSPI();
	string getFrontAddress();

	void setBrokerID(string BrokerID);
	void setUserID(string UserID);
	void setPassword(string Password);
	void setRequestID(int nRequestID);
	void setIsLogged(bool isLogged);
	void setIsFirstTimeLogged(bool isFirstTimeLogged);
	void setIsConfirmSettlement(bool isConfirmSettlement);
	void setLoginRequestID(int loginRequestID);
	void setUserTradeAPI(CThostFtdcTraderApi *UserTradeAPI);
	void setUserTradeSPI(TdSpi *UserTradeSPI);
	void setFrontAddress(string frontAddress);
	Trader *GetTrader();
	void setTrader(Trader *trader);
	string getTraderID();
	void setTraderID(string TraderID);

	/// 得到strategy_list
	list<Strategy *> *getListStrategy();
	/// 设置strategy_list
	void setListStrategy(list<Strategy *> *l_strategys);
	/// 添加strategy到list
	void addStrategyToList(Strategy *stg);


	/// 初始化合约撤单次数,例如"cu1601":0 "cu1701":0
	void init_instrument_id_action_counter(string instrument_id);

	/// 添加对应合约撤单次数计数器,例如"cu1602":1 "cu1701":1
	void add_instrument_id_action_counter(string instrument_id);

	/// 报单引用基准
	void setStgOrderRefBase(long long stg_order_ref_base);
	long long getStgOrderRefBase();

	/// 设置策略内合约最小跳价格
	void setStgInstrumnetPriceTick();

	/************************************************************************/
	/* 获取数据库连接                                                         */
	/************************************************************************/
	mongo::DBClientConnection * GetPositionConn();
	mongo::DBClientConnection * GetTradeConn();
	mongo::DBClientConnection * GetOrderConn();


	/************************************************************************/
	/* 完成Order的MongoDB操作                                                 */
	/************************************************************************/
	void DB_OrderInsert(mongo::DBClientConnection *conn, CThostFtdcInputOrderField *pInputOrder);
	void DB_OnRtnOrder(mongo::DBClientConnection *conn, CThostFtdcOrderField *pOrder);
	void DB_OnRtnTrade(mongo::DBClientConnection *conn, CThostFtdcTradeField *pTrade);
	void DB_OrderAction(mongo::DBClientConnection *conn, CThostFtdcInputOrderActionField *pOrderAction);
	void DB_OrderCombine(mongo::DBClientConnection *conn, CThostFtdcOrderField *pOrder);
	void DB_OnRspOrderAction(mongo::DBClientConnection *conn, CThostFtdcInputOrderActionField *pInputOrderAction); // CTP认为撤单参数错误
	void DB_OnErrRtnOrderAction(mongo::DBClientConnection *conn, CThostFtdcOrderActionField *pOrderAction); // 交易所认为撤单错误
	void DB_OnRspOrderInsert(mongo::DBClientConnection *conn, CThostFtdcInputOrderField *pInputOrder); // CTP认为报单参数错误
	void DB_OnErrRtnOrderInsert(mongo::DBClientConnection *conn, CThostFtdcInputOrderField *pInputOrder); // 交易所认为报单错误
	void DB_OnRspQryInvestorPosition(mongo::DBClientConnection *conn, CThostFtdcInvestorPositionField *pInvestorPosition); // 持仓信息

	/// 设置开关
	int getOn_Off();
	void setOn_Off(int on_off);

	/// 设置CTP_Manager
	void setCTP_Manager(CTP_Manager *o_ctp);
	CTP_Manager *getCTP_Manager();

	void setIsActive(string isActive);
	string getIsActive();

private:
	int on_off; //开关
	string BrokerID;
	string UserID;
	string Password;
	string frontAddress;
	int nRequestID;
	bool isLogged;
	bool isFirstTimeLogged;
	bool isConfirmSettlement;
	int loginRequestID;
	string TraderID;
	CThostFtdcTraderApi *UserTradeAPI;
	TdSpi *UserTradeSPI;
	Trader *trader;
	list<Strategy *> *l_strategys;
	map<string, int> *stg_map_instrument_action_counter;
	mongo::DBClientConnection * PositionConn;
	mongo::DBClientConnection * TradeConn;
	mongo::DBClientConnection * OrderConn;
	long long stg_order_ref_base;
	CTP_Manager *o_ctp;
	string isActive;
};

#endif
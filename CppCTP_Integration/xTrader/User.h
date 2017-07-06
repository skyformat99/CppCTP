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
#include "Session.h"
#include "xTradeStruct.h"
#include "concurrentqueue/blockingconcurrentqueue.h"
#include <spdlog/spdlog.h>
using namespace spdlog;
using spdlog::logger;

using namespace mongo;
using namespace std;
using std::string;
using mongo::DBClientConnection;
using mongo::BSONObjBuilder;
using namespace moodycamel;

class CTP_Manager;
class Session;

class User
{
public:
	User(string frontAddress, string BrokerID, string UserID, string Password, string nRequestID, int on_off, string TraderID = "", string stg_order_ref_base = "0");
	User(string BrokerID, string UserID, int nRequestID, string stg_order_ref_base = "0");
	~User();
	string getBrokerID();
	string getUserID();
	string getPassword();
	int getRequestID();
	bool getIsLogged();
	bool getIsLoggedError();
	bool getIsConnected();
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
	void setIsLoggedError(bool isLoggedError);
	void setIsConnected(bool isConnected);
	void setIsPositionRight(bool isPositionRight);
	bool getIsPositionRight();

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

	/// 拷贝持仓明细数据
	void CopyPositionDetailData(CThostFtdcInvestorPositionDetailField *dst, CThostFtdcInvestorPositionDetailField *src);

	/// 初始化合约撤单次数,例如"cu1601":0 "cu1701":0
	void init_instrument_id_action_counter(string instrument_id);

	/// 获得合约撤单次数,例如"cu1710":0
	int get_instrument_id_action_counter(string instrument_id);

	/// 添加对应合约撤单次数计数器,例如"cu1602":1 "cu1701":1
	void add_instrument_id_action_counter(CThostFtdcOrderField *pOrder);

	/// 报单引用基准
	void setStgOrderRefBase(long long stg_order_ref_base);

	/// 获取报单引用基准
	long long getStgOrderRefBase();

	/// 获取报单
	void OrderInsert(CThostFtdcInputOrderField *insert_order, string strategy_id);

	/// 设置策略内合约最小跳价格
	void setStgInstrumnetPriceTick();


	void setL_Position_Detail_From_CTP(list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_ctp);
	list<USER_INSTRUMENT_POSITION *> *getL_Position_Detail_From_CTP();
	void addL_Position_Detail_From_CTP(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail);

	void setL_Position_Detail_From_Local_Order(list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_order);
	list<USER_INSTRUMENT_POSITION *> *getL_Position_Detail_From_Local_Order();
	void addL_Position_Detail_From_Local_Order(USER_CThostFtdcOrderField *order);

	void setL_Position_Detail_From_Local_Trade(list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_trade);
	list<USER_INSTRUMENT_POSITION *> *getL_Position_Detail_From_Local_Trade();
	void addL_Position_Detail_From_Local_Trade(USER_CThostFtdcTradeField *order);

	/// 获取统计持仓明细结果
	void getL_Position_Detail_Data(list<USER_INSTRUMENT_POSITION *> *l_position_detail_cal);

	/************************************************************************/
	/* 获取数据库连接                                                         */
	/************************************************************************/


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
	// 更新报单引用
	void DB_UpdateOrderRef(string order_ref_base);

	/// 设置开关
	int getOn_Off();
	void setOn_Off(int on_off);

	/// 设置线程初始化状态，完成为true，否则false
	bool getThread_Init_Status();
	void setThread_Init_Status(bool thread_init_status);

	/// 设置CTP_Manager
	void setCTP_Manager(CTP_Manager *o_ctp);
	CTP_Manager *getCTP_Manager();

	void setIsActive(string isActive);
	string getIsActive();

	void setTradingDay(string stg_trading_day);
	string getTradingDay();

	void QueryTrade();

	void QueryOrder();

	// order_insert回调队列
	void thread_queue_OrderInsert();

	/// 得到数据库操作对象
	DBManager *getDBManager();
	void setDBManager(DBManager *dbm);

	int getSessionID();
	void setSessionID(int sid);

	list<Session *> * getL_Sessions();
	void setL_Sessions(list<Session *> *l_sessions);

	// 对比本地维护持仓结果与CTP API返回持仓对比结果
	bool ComparePositionTotal();

	// 设置系统xts_logger
	void setXtsLogger(std::shared_ptr<spdlog::logger> ptr);
	std::shared_ptr<spdlog::logger> getXtsLogger();

	// 网络是否曾经断过
	void setIsEverLostConnection(bool isEverLostConnection);
	bool getIsEverLostConnection();

private:
	int on_off; //开关
	string BrokerID;
	string UserID;
	string Password;
	string frontAddress;
	int nRequestID;
	bool isLogged;
	bool isConnected;
	bool isLoggedError;
	bool isPositionRight;
	bool isFirstTimeLogged;
	bool isConfirmSettlement;
	int loginRequestID;
	string TraderID;
	CThostFtdcTraderApi *UserTradeAPI;
	TdSpi *UserTradeSPI;
	Trader *trader;
	list<Strategy *> *l_strategys;
	map<string, int> *stg_map_instrument_action_counter;
	DBManager *dbm;
	long long stg_order_ref_base;
	CTP_Manager *o_ctp;
	string isActive;
	string trading_day;				// 交易日
	int sid;	// 会话ID
	list<Session *> *l_sessions;
	list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_ctp;
	list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_order;
	list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_trade;
	bool thread_init_status;
	bool isEverLostConnection;
	std::shared_ptr<spdlog::logger> xts_user_logger;
	sem_t sem_get_order_ref;					// 信号量,用来保证同一时间只能一处地方操作报单引用

	// 阻塞队列
	moodycamel::BlockingConcurrentQueue<CThostFtdcDepthMarketDataField *> queue_OrderInsert;	// 行情队列

};

#endif
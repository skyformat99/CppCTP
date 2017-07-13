#ifndef QUANT_USER_H
#define QUANT_USER_H

#include <iostream>
#include <string>
#include <cstring>
#include <mongo/client/dbclient.h>
#include "SgitFtdcTraderApi.h"
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
	CSgitFtdcTraderApi *getUserTradeAPI();
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
	void setUserTradeAPI(CSgitFtdcTraderApi *UserTradeAPI);
	void setUserTradeSPI(TdSpi *UserTradeSPI);
	void setFrontAddress(string frontAddress);
	Trader *GetTrader();
	void setTrader(Trader *trader);
	string getTraderID();
	void setTraderID(string TraderID);

	/// �õ�strategy_list
	list<Strategy *> *getListStrategy();
	/// ����strategy_list
	void setListStrategy(list<Strategy *> *l_strategys);
	/// ���strategy��list
	void addStrategyToList(Strategy *stg);

	/// �����ֲ���ϸ����
	void CopyPositionDetailData(CSgitFtdcInvestorPositionDetailField *dst, CSgitFtdcInvestorPositionDetailField *src);

	/// ��ʼ����Լ��������,����"cu1601":0 "cu1701":0
	void init_instrument_id_action_counter(string instrument_id);

	/// ��ú�Լ��������,����"cu1710":0
	int get_instrument_id_action_counter(string instrument_id);

	/// ��Ӷ�Ӧ��Լ��������������,����"cu1602":1 "cu1701":1
	void add_instrument_id_action_counter(CSgitFtdcOrderField *pOrder);

	/// ��Ӷ�Ӧ��Լ��������ͳ��(Sgit)
	void add_instrument_id_action_counter(CSgitFtdcInputOrderActionField *pInputOrderAction);

	/// �������û�׼
	void setStgOrderRefBase(long long stg_order_ref_base);

	/// ��ȡ�������û�׼
	long long getStgOrderRefBase();

	/// ��ȡ����
	void OrderInsert(CSgitFtdcInputOrderField *insert_order, Strategy *stg, string strategy_id);

	/// ���ò����ں�Լ��С���۸�
	void setStgInstrumnetPriceTick();


	void setL_Position_Detail_From_CTP(list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_ctp);
	list<USER_INSTRUMENT_POSITION *> *getL_Position_Detail_From_CTP();
	void addL_Position_Detail_From_CTP(CSgitFtdcInvestorPositionDetailField *pInvestorPositionDetail);

	void setL_Position_Detail_From_Local_Order(list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_order);
	list<USER_INSTRUMENT_POSITION *> *getL_Position_Detail_From_Local_Order();
	void addL_Position_Detail_From_Local_Order(USER_CSgitFtdcOrderField *order);

	void setL_Position_Detail_From_Local_Trade(list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_trade);
	list<USER_INSTRUMENT_POSITION *> *getL_Position_Detail_From_Local_Trade();
	void addL_Position_Detail_From_Local_Trade(USER_CSgitFtdcTradeField *order);

	/// ��ȡͳ�Ƴֲ���ϸ���
	void getL_Position_Detail_Data(list<USER_INSTRUMENT_POSITION *> *l_position_detail_cal);

	/************************************************************************/
	/* ��ȡ���ݿ�����                                                         */
	/************************************************************************/


	/************************************************************************/
	/* ���Order��MongoDB����                                                 */
	/************************************************************************/
	void DB_OrderInsert(mongo::DBClientConnection *conn, CSgitFtdcInputOrderField *pInputOrder);
	void DB_OnRtnOrder(mongo::DBClientConnection *conn, CSgitFtdcOrderField *pOrder);
	void DB_OnRtnTrade(mongo::DBClientConnection *conn, CSgitFtdcTradeField *pTrade);
	void DB_OrderAction(mongo::DBClientConnection *conn, CSgitFtdcInputOrderActionField *pOrderAction);
	void DB_OrderCombine(mongo::DBClientConnection *conn, CSgitFtdcOrderField *pOrder);
	void DB_OnRspOrderAction(mongo::DBClientConnection *conn, CSgitFtdcInputOrderActionField *pInputOrderAction); // CTP��Ϊ������������
	void DB_OnErrRtnOrderAction(mongo::DBClientConnection *conn, CSgitFtdcOrderActionField *pOrderAction); // ��������Ϊ��������
	void DB_OnRspOrderInsert(mongo::DBClientConnection *conn, CSgitFtdcInputOrderField *pInputOrder); // CTP��Ϊ������������
	void DB_OnErrRtnOrderInsert(mongo::DBClientConnection *conn, CSgitFtdcInputOrderField *pInputOrder); // ��������Ϊ��������
	void DB_OnRspQryInvestorPosition(mongo::DBClientConnection *conn, CSgitFtdcInvestorPositionField *pInvestorPosition); // �ֲ���Ϣ
	// ���±�������
	void DB_UpdateOrderRef(string order_ref_base);

	/// ���ÿ���
	int getOn_Off();
	void setOn_Off(int on_off);

	/// �����̳߳�ʼ��״̬�����Ϊtrue������false
	bool getThread_Init_Status();
	void setThread_Init_Status(bool thread_init_status);

	/// ����CTP_Manager
	void setCTP_Manager(CTP_Manager *o_ctp);
	CTP_Manager *getCTP_Manager();

	void setIsActive(string isActive);
	string getIsActive();

	void setTradingDay(string stg_trading_day);
	string getTradingDay();

	void QueryTrade();

	void QueryOrder();

	// order_insert�ص�����
	void thread_queue_OrderInsert();

	/// �õ����ݿ��������
	DBManager *getDBManager();
	void setDBManager(DBManager *dbm);

	int getSessionID();
	void setSessionID(int sid);

	list<Session *> * getL_Sessions();
	void setL_Sessions(list<Session *> *l_sessions);

	// �Աȱ���ά���ֲֽ����CTP API���سֲֶԱȽ��
	bool ComparePositionTotal();

	// ����ϵͳxts_logger
	void setXtsLogger(std::shared_ptr<spdlog::logger> ptr);
	std::shared_ptr<spdlog::logger> getXtsLogger();

	// �����Ƿ������Ϲ�
	void setIsEverLostConnection(bool isEverLostConnection);
	bool getIsEverLostConnection();

	// ��һ��orderref
	string getLastOrderRef();
	void setLastOrderRef(string last_order_ref);

	// ��ǰorder_ref
	string getCurrentOrderRef();
	void setCurrentOrderRef(string current_order_ref);

	// ���һ��orderrefͳ��
	int getLastOrderRefCal();
	void setLastOrderRefCal(int last_order_ref_cal);
	void autoIncrementLastOrderRefCal();
	void resetLastOrderRefCal();

	int getLastOrderRefCalTmp();
	void setLastOrderRefCalTmp(int last_order_ref_cal_tmp);
	void autoIncrementLastOrderRefCalTmp();
	void resetLastOrderRefCalTmp();

private:
	int on_off; //����
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
	CSgitFtdcTraderApi *UserTradeAPI;
	TdSpi *UserTradeSPI;
	Trader *trader;
	list<Strategy *> *l_strategys;
	map<string, int> *stg_map_instrument_action_counter;
	DBManager *dbm;
	long long stg_order_ref_base;
	CTP_Manager *o_ctp;
	string isActive;
	string trading_day;				// ������
	int sid;	// �ỰID
	list<Session *> *l_sessions;
	list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_ctp;
	list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_order;
	list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_trade;
	bool thread_init_status;
	bool isEverLostConnection;
	std::shared_ptr<spdlog::logger> xts_user_logger;
	sem_t sem_get_order_ref; // �ź���,������֤ͬһʱ��ֻ��һ���ط�������������
	sem_t sem_inc_last_order_ref_cal; // �ź���,������֤ͬһʱ��ֻ��һ���ۼӲ���
	sem_t sem_inc_last_order_ref_cal_tmp; // �ź���,������֤ͬһʱ��ֻ��һ���ۼӲ���

	// ��������
	moodycamel::BlockingConcurrentQueue<CSgitFtdcDepthMarketDataField *> queue_OrderInsert;	// �������
	string last_order_ref;	// ��һ��order_ref
	string current_order_ref;	// ��ǰorder_ref
	int last_order_ref_cal;	// ���һ��order_ref���մ���ͳ��
	int last_order_ref_cal_tmp;	// ���һ��order_ref���մ���ͳ��tmp

};

#endif
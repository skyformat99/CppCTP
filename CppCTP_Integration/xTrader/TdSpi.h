#ifndef QUANT_CTP_TRADE_TDSPI_H
#define QUANT_CTP_TRADE_TDSPI_H

#include <map>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <list>
#include <map>

#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiStruct.h"
#include "Strategy.h"
#include "CTP_Manager.h"
#include "Utils.h"

using std::string;
using std::list;

class User;
class Strategy;
class CTP_Manager;


#ifndef NULL
#define NULL 0
#endif

using std::map;
using std::string;

class TdSpi :public CThostFtdcTraderSpi{
public:

	//增加毫秒
	void timeraddMS(struct timeval *a, int ms);

	//协程控制
	int controlTimeOut(sem_t *t, int timeout = 2000);

    //构造函数
	TdSpi();

	//增加api
	void addApi(User *user, string flowpath);

	//建立连接
	void Connect(User *user, bool init_flag = false);

    //当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    void OnFrontConnected();

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	void OnFrontDisconnected(int nReason);

	//登录
	int Login(User *user);


    ///登录请求响应
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//登出
	void Logout(char *BrokerID, char *UserID);

    ///登出请求响应
    void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
                         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//等待线程结束
	void Join();



	//查询结算信息确认
	int QrySettlementInfoConfirm(User *user);

    //请求查询结算信息确认响应
    void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
                                       CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//查询结算信息
	void QrySettlementInfo(User *user);

    //请求查询投资者结算结果响应
    void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo,
                                CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//确认结算结果
	void ConfirmSettlementInfo(User *user);

    //投资者结算结果确认响应
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
                                    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//查询交易所
	void QryExchange();

	//响应查询交易所
	void OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//查询合约
	void QryInstrument(string exchangeid = "", string instrumentid = "");

	//查询行情
	void QryDepthMarketData(string instrumentid);

	///请求查询行情响应
	void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//响应查询合约
	void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//拷贝合约信息
	void CopyInstrumentInfo(CThostFtdcInstrumentField *dst, CThostFtdcInstrumentField *src);

	///合约交易状态通知
	void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus);

	//查询报单
	void QryOrder();

	//响应查询报单;
	void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


	//查询保证金率
	void QryInstrumentMarginRate();

	///请求查询合约保证金率响应
	void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


	//查询手续费
	void QryInstrumentCommissionRate();

	///请求查询合约手续费率响应
	void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//查询投资者
	void QryInvestor();

	//查询投资者响应
	void OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	/// 拷贝持仓明细数据
	void CopyPositionDetailData(CThostFtdcInvestorPositionDetailField *dst, CThostFtdcInvestorPositionDetailField *src);

	//查询投资者持仓
	void QryInvestorPosition();

	//查询投资者持仓明细
	void QryInvestorPositionDetail();

	// 请求查询投资者持仓明细响应
	void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//请求查询投资者持仓响应
	void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//查询账号资金
	void QryTradingAccount();

	//查询账号资金响应
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//查询成交单
	void QryTrade();

	//复制交易回报
	void CopyTradeInfo(CThostFtdcTradeField *dst, CThostFtdcTradeField *src);

	//复制订单回报
	void CopyOrderInfo(CThostFtdcOrderField *dst, CThostFtdcOrderField *src);

	//查询成交单响应
	void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    //下单
	int OrderInsert(User *user, CThostFtdcInputOrderField *pInputOrder, Strategy *stg);

	//下单响应
	void OnRtnOrder(CThostFtdcOrderField *pOrder);

	//成交通知
	void OnRtnTrade(CThostFtdcTradeField *pTrade);

	//下单错误响应
	void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

	///报单录入请求响应
	void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//撤单
	int OrderAction(char *ExchangeID, char *OrderRef, char *OrderSysID);

	//撤单错误响应
	void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//撤单错误
	void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);
	
	///用户口令更新请求响应
    void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate,
                                 CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    //登陆是否报错
    bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo, string source = "");

	//得到BrokerID
	string getBrokerID();
	char *getCharBrokerID();

	//得到UserID
	string getUserID();
	char *getCharUserID();

	//得到Password
	string getPassword();
	char *getCharPassword();

	//得到requestID
	int getRequestID();

	//得到交易日期
	char *getCharTradingDate();

	//释放api
	void Release();

	//设置isConfirmSettlement
	void setIsConfirmSettlement(bool isConfirmSettlement);

	//得到isConfirmSettlement
	bool getIsConfirmSettlement();

	//得到OrderRef

	//frontid
	void setFrontID(int FrontID);
	int getFrontID();

	//sessionid
	void setSessionID(int SessionID);
	int getSessionID();

	/// 得到strategy_list
	list<Strategy *> *getListStrategy();

	/// 设置strategy_list
	void setListStrategy(list<Strategy *> *l_strategys);

	list<CThostFtdcInstrumentField *> * getL_Instruments_Info();

	void setL_Instruments_Info(list<CThostFtdcInstrumentField *> *l_instruments_info);

	string getTradingDay();

	list<CThostFtdcTradeField *> *getL_query_trade();
	list<CThostFtdcOrderField *> *getL_query_order();

	void setCtpManager(CTP_Manager *ctp_m);
	CTP_Manager *getCtpManager();

private:
	CTP_Manager *ctp_m;
    CThostFtdcTraderApi *tdapi;
    CThostFtdcReqUserLoginField *loginField;
    CThostFtdcReqAuthenticateField *authField;
	bool isLogged;
	bool isFirstTimeLogged;
	bool isConfirmSettlement;
	bool isFirstQryTrade;
	bool isFirstQryOrder;
	int loginRequestID;
	string BrokerID;
	string UserID;
	string Password;
	string trading_day;
	char * c_BrokerID;
	char * c_UserID;
	char * c_Password;
	char * c_TradingDay;
	sem_t connect_sem;
	sem_t login_sem;
	sem_t logout_sem;
	sem_t sem_ReqQrySettlementInfoConfirm;
	sem_t sem_ReqQrySettlementInfo;
	sem_t sem_ReqSettlementInfoConfirm;
	//list<CThostFtdcTraderApi *> l_api;
	//list<User *> l_user;
	//list<User *>::iterator iter_user;
	map <string, int> m_user_requestid;
	map <int, CThostFtdcTraderApi *> m_requestid_api;
	User *current_user;
	int FrontID;
	int SessionID;
	list<Strategy *> *l_strategys;
	list<CThostFtdcTradeField *> *l_query_trade;
	list<CThostFtdcOrderField *> *l_query_order;
	list<CThostFtdcInstrumentField *> *l_instruments_info;
};

#endif //QUANT_CTP_TRADE_TDSPI_H
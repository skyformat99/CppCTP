//
// Created by quant on 6/7/16.
//

#ifndef QUANT_CTP_MDSPI_H
#define QUANT_CTP_MDSPI_H

#include <map>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <list>
#include "SgitFtdcMdApi.h"
#include "SgitFtdcUserApiStruct.h"
#include "User.h"
#include "Strategy.h"
#include "CTP_Manager.h"
#include "Utils.h"


using std::map;
using std::string;

class CTP_Manager;
class Strategy;

class MdSpi :public CSgitFtdcMdSpi{

public:
    MdSpi(CSgitFtdcMdApi *mdapi);
	//建立连接
	void Connect(char *frontAddress);
    //建立连接时触发
    void OnFrontConnected();
	//等待线程结束
	void Join();
	//增加毫秒
	void timeraddMS(struct timeval *a, int ms);
	//协程控制
	//int controlTimeOut(sem_t *t, int timeout = 5000);
	//登录
	void Login(char *BrokerID, char *UserID, char *Password);
    ///登录请求响应
    void OnRspUserLogin(CSgitFtdcRspUserLoginField *pRspUserLogin, CSgitFtdcRspInfoField *pRspInfo,
                        int nRequestID, bool bIsLast);
	//登出
	void Logout(char *BrokerID, char *UserID);
    ///登出请求响应
    void OnRspUserLogout(CSgitFtdcUserLogoutField *pUserLogout, CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///订阅行情
	void SubMarket(list<string> *l_instrument);

	///行情就绪
	void Ready();

	///取消订阅行情
	void UnSubMarket(list<string> *l_instrument);

    ///订阅行情应答
    void OnRspSubMarketData(CSgitFtdcSpecificInstrumentField *pSpecificInstrument, CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///取消订阅行情应答
    void OnRspUnSubMarketData(CSgitFtdcSpecificInstrumentField *pSpecificInstrument, CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///深度行情通知
    void OnRtnDepthMarketData(CSgitFtdcDepthMarketDataField *pDepthMarketData);
	void CopyTickData(CSgitFtdcDepthMarketDataField *dst, CSgitFtdcDepthMarketDataField *src);

	//通信断开
	void OnFrontDisconnected(int nReason);

	//返回数据是否报错
	bool IsErrorRspInfo(CSgitFtdcRspInfoField *pRspInfo);

	//订阅行情
	void SubMarketData(char *ppInstrumentID[], int nCount);

	//取消订阅行情
	void UnSubscribeMarketData(char *ppInstrumentID[], int nCount);

	//得到BrokerID
	string getBrokerID();

	//得到UserID
	string getUserID();

	//得到Password
	string getPassword();

	//添加strategy
	void addStrategyToList(Strategy *stg);

	/// 得到strategy_list
	list<Strategy *> *getListStrategy();

	/// 设置strategy_list
	void setListStrategy(list<Strategy *> *l_strategys);

	void setCtpManager(CTP_Manager *ctp_m);
	CTP_Manager *getCtpManager();

private:
    CSgitFtdcMdApi *mdapi;
    CSgitFtdcReqUserLoginField *loginField;
	CSgitFtdcUserLogoutField *logoutField;
    int loginRequestID;
	bool isLogged;
	bool isFirstTimeLogged;
	char **ppInstrumentID;
	int nCount;
	string BrokerID;
	string UserID;
	string Password;
	/*sem_t connect_sem;
	sem_t login_sem;
	sem_t logout_sem;
	sem_t submarket_sem;
	sem_t unsubmarket_sem;*/
	list<Strategy *> *l_strategys;
	CTP_Manager *ctp_m;
	CSgitFtdcDepthMarketDataField *last_tick_data;
};
#endif //QUANT_CTP_MDSPI_H

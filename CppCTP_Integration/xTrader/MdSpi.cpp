//
// Created by quant on 6/7/16.
//
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable, std::cv_status
#include "MdSpi.h"

using namespace std;

struct timespec outtime = {3, 0};
std::condition_variable md_cv;
std::mutex md_mtx;

#define MD_RSP_TIMEOUT	5

//初始化构造函数
MdSpi::MdSpi(CSgitFtdcMdApi *mdapi) {
	this->last_tick_data = new CSgitFtdcDepthMarketDataField();
	/*sem_init(&connect_sem, 0, 0);
	sem_init(&login_sem, 0, 0);
	sem_init(&logout_sem, 0, 0);
	sem_init(&submarket_sem, 0, 0);
	sem_init(&unsubmarket_sem, 0, 0);*/
	this->isLogged = false;
	this->isFirstTimeLogged = true;
    this->mdapi = mdapi;
	//注册事件处理对象
	mdapi->RegisterSpi(this);
    this->loginRequestID = 10;
}

//等待线程结束
void MdSpi::Join() {
	USER_PRINT("MdSpi::Join()")
	this->mdapi->Join();
}

////协程控制
//int MdSpi::controlTimeOut(sem_t *t, int timeout) {
//	/*协程开始*/
//	struct timeval now;
//	struct timespec outtime;
//	gettimeofday(&now, NULL);
//	//cout << now.tv_sec << " " << (now.tv_usec) << "\n";
//	timeraddMS(&now, timeout);
//	outtime.tv_sec = now.tv_sec;
//	outtime.tv_nsec = now.tv_usec * 1000;
//	//cout << outtime.tv_sec << " " << (outtime.tv_nsec) << "\n";
//	int ret = sem_timedwait(t, &outtime);
//	int value;
//	sem_getvalue(t, &value);
//	//cout << "value = " << value << endl;
//	//cout << "ret = " << ret << endl;
//	/*协程结束*/
//	return ret;
//}

//增加毫秒
void MdSpi::timeraddMS(struct timeval *now_time, int ms) {
	now_time->tv_usec += ms * 1000;
	if (now_time->tv_usec >= 1000000) {
		now_time->tv_sec += now_time->tv_usec / 1000000;
		now_time->tv_usec %= 1000000;
	}
}

//连接
void MdSpi::Connect(char *frontAddress) {
	// 行情订阅模式
	this->mdapi->SubscribeMarketTopic(Sgit_TERT_QUICK);
	// 前置地址
	this->mdapi->RegisterFront(frontAddress); //24H
	// 开启log
	this->mdapi->Init(true);
	//int ret = this->controlTimeOut(&connect_sem);

	//等待回调
	std::unique_lock<std::mutex> md_lck(md_mtx);
	while (md_cv.wait_for(md_lck, std::chrono::seconds(MD_RSP_TIMEOUT)) == std::cv_status::timeout) {
		std::cout << "MdSpi::Connect() 行情连接等待超时" << std::endl;
		return;
	}
}

//响应连接
void MdSpi::OnFrontConnected() {
	//std::unique_lock <std::mutex> lck(md_mtx);
	

	if (!this->isFirstTimeLogged) // 非第一次登陆
	{
		Utils::printRedColor("MdSpi::OnFrontConnected() 断线重连...");
		this->ctp_m->getXtsLogger()->info("MdSpi::OnFrontConnected() 断线重连...");
		this->Login(const_cast<char *>(this->BrokerID.c_str()), const_cast<char *>(this->UserID.c_str()), const_cast<char *>(this->Password.c_str()));
	}
	else { // 第一次登陆
		md_cv.notify_one();
	}

	/*const char *BrokerID = this->BrokerID.c_str();
	char *r_BrokerID = new char[strlen(BrokerID) + 1];
	strcpy(r_BrokerID, BrokerID);

	const char *UserID = this->UserID.c_str();
	char *r_UserID = new char[strlen(UserID) + 1];
	strcpy(r_UserID, BrokerID);

	const char *Password = this->Password.c_str();
	char *r_Password = new char[strlen(Password) + 1];
	strcpy(r_Password, BrokerID);*/

	//this->Login(r_BrokerID, r_UserID, r_Password);
	//this->Login("9999", "058176", "669822");
}

//通信断开
void MdSpi::OnFrontDisconnected(int nReason) {
	this->isFirstTimeLogged = false;
	if (this->ctp_m)
	{
		Utils::printRedColor("MdSpi::OnFrontDisconnected() 断线!");
		this->ctp_m->sendMarketOffLineMessage(1);
		this->ctp_m->getXtsLogger()->info("MdSpi::OnFrontDisconnected() 断线原因 = {}", nReason);
		this->ctp_m->getXtsLogger()->flush();
	}
	
}

//登录
void MdSpi::Login(char *BrokerID, char *UserID, char *Password) {
	
	if (!this->isFirstTimeLogged)
	{
		this->ctp_m->getXtsLogger()->info("MdSpi::Login() 自动登录...");
	}

	this->BrokerID = BrokerID;
	this->UserID = UserID;
	this->Password = Password;
	loginField = new CSgitFtdcReqUserLoginField();
	strcpy(loginField->BrokerID, BrokerID);
	strcpy(loginField->UserID, UserID);
	strcpy(loginField->Password, Password);
	this->mdapi->ReqUserLogin(loginField, this->loginRequestID);
	
	/*int ret = this->controlTimeOut(&login_sem);
	if (ret == -1) {
	USER_PRINT("MdSpi::Login TimeOut!")
	}*/
	//等待回调
	std::unique_lock<std::mutex> md_lck(md_mtx);
	while (md_cv.wait_for(md_lck, std::chrono::seconds(MD_RSP_TIMEOUT)) == std::cv_status::timeout) {
		this->ctp_m->getXtsLogger()->info("MdSpi::Connect() 行情登录等待超时");
		delete loginField;
		loginField = NULL;
		return;
	}

	delete loginField;
	loginField = NULL;
}

//响应登录
void MdSpi::OnRspUserLogin(CSgitFtdcRspUserLoginField *pRspUserLogin, CSgitFtdcRspInfoField *pRspInfo,
                           int nRequestID, bool bIsLast) {
	USER_PRINT("MdSpi::OnRspUserLogin");
	USER_PRINT(bIsLast);
	std::cout << "MdSpi::OnRspUserLogin()" << std::endl;
	if (bIsLast && !(this->IsErrorRspInfo(pRspInfo))) {
		///交易日
		cout << "\t*交易日" << pRspUserLogin->TradingDay << ", ";
		///登录成功时间
		cout << "登录成功时间" << pRspUserLogin->LoginTime << ", ";
		///经纪公司代码
		cout << "经纪公司代码" << pRspUserLogin->BrokerID << ", ";
		///用户代码
		cout << "用户代码" << pRspUserLogin->UserID << ", ";
		///交易系统名称
		cout << "交易系统名称" << pRspUserLogin->SystemName << ", ";
		///前置编号
		cout << "前置编号" << pRspUserLogin->FrontID << ", ";
		///会话编号
		cout << "会话编号" << pRspUserLogin->SessionID << ", ";
		///最大报单引用
		cout << "最大报单引用" << pRspUserLogin->MaxOrderRef << ", ";
		///上期所时间
		cout << "上期所时间" << pRspUserLogin->SHFETime << ", ";
		///大商所时间
		cout << "大商所时间" << pRspUserLogin->DCETime << ", ";
		///郑商所时间
		cout << "郑商所时间" << pRspUserLogin->CZCETime << ", ";
		///中金所时间
		cout << "中金所时间" << pRspUserLogin->FFEXTime << ", ";
		///能源中心时间
		//cout << "能源中心时间" << pRspUserLogin->INETime << ", ";
		this->isLogged = true;
		string s_trading_day = this->mdapi->GetTradingDay();
		this->ctp_m->getXtsLogger()->info("MdSpi::OnRspUserLogin() TradingDay = {}", s_trading_day);
		
		//this->ctp_m->setTradingDay(s_trading_day);
		this->ctp_m->setTradingDay("20170712");
		this->ctp_m->setMdLogin(true);
		/*sem_post(&login_sem);
		if (this->isFirstTimeLogged == false) {
		sem_init(&submarket_sem, 0, 1);
		this->SubMarketData(this->ppInstrumentID, this->nCount);
		}*/

		if (!this->isFirstTimeLogged)
		{
			sleep(1);
			// 断线重连登录后自动订阅行情
			this->SubMarket(this->ctp_m->getL_Instrument());
			// 通知客户端
			this->ctp_m->sendMarketOffLineMessage(0);
		}

	}
	//释放
	md_cv.notify_one();
}

//返回数据是否报错
bool MdSpi::IsErrorRspInfo(CSgitFtdcRspInfoField *pRspInfo) {
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cerr << "MdSpi::IsErrorRspInfo() ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	return bResult;
}

//登出
void MdSpi::Logout(char *BrokerID, char *UserID) {
	USER_PRINT("MdSpi::Logout")
	logoutField = new CSgitFtdcUserLogoutField();
	strcpy(logoutField->BrokerID, BrokerID);
	strcpy(logoutField->UserID, UserID);
	this->mdapi->ReqUserLogout(logoutField, this->loginRequestID);
	/*int ret = this->controlTimeOut(&logout_sem);
	if (ret == -1) {
		USER_PRINT("MdSpi::Logout TimeOut!");
	}*/
}

//响应登出
void MdSpi::OnRspUserLogout(CSgitFtdcUserLogoutField *pUserLogout, CSgitFtdcRspInfoField *pRspInfo,
                            int nRequestID, bool bIsLast) {
	USER_PRINT("MdSpi::OnRspUserLogout")
	if (bIsLast && !(this->IsErrorRspInfo(pRspInfo))) {
		USER_PRINT("OnRspUserLogout");
		this->isLogged = false;
		//sem_post(&logout_sem);
	}
}

//订阅行情
void MdSpi::SubMarketData(char *ppInstrumentID[], int nCount) {
	USER_PRINT("MdSpi::SubMarketData");
	if (this->isLogged) {
		USER_PRINT("SubMarketData");
		this->ppInstrumentID = ppInstrumentID;
		this->nCount = nCount;
		
		//this->mdapi->SubscribeMarketData(ppInstrumentID, nCount);

		// 兼容飞鼠API
		for (int i = 0; i < nCount; i++)
		{
			CSgitSubQuotField instruments;
			memset(&instruments, 0, sizeof(CSgitSubQuotField));
			strcpy(instruments.ContractID, ppInstrumentID[i]);

			this->mdapi->SubQuot(&instruments);
		}

		/*int ret = this->controlTimeOut(&submarket_sem);
		if (ret == -1) {
			USER_PRINT("MdSpi::SubMarketData TimeOut!");
		}*/
	} else {
		USER_PRINT("Please Login First!");
	}
}

///订阅行情
void MdSpi::SubMarket(list<string> *l_instrument) {
	list<string>::iterator itor;
	char **instrumentID = new char *[l_instrument->size()];
	int size = l_instrument->size();
	int i = 0;
	const char *charResult;
	for (itor = l_instrument->begin(), i = 0; itor != l_instrument->end(); itor++, i++) {
		USER_PRINT(*itor);
		charResult = (*itor).c_str();
		instrumentID[i] = new char[strlen(charResult) + 1];
		strcpy(instrumentID[i], charResult);
	}
	
	//this->mdapi->SubscribeMarketData(instrumentID, size);

	// 兼容飞鼠API订阅行情
	for (int i = 0; i < size; i++)
	{
		CSgitSubQuotField instruments;
		memset(&instruments, 0, sizeof(CSgitSubQuotField));
		strcpy(instruments.ContractID, instrumentID[i]);

		this->mdapi->SubQuot(&instruments);
	}
}

///行情就绪
void MdSpi::Ready() {
	this->mdapi->Ready();
}

///取消订阅行情
void MdSpi::UnSubMarket(list<string> *l_instrument) {
	list<string>::iterator itor;
	char **instrumentID = new char *[l_instrument->size()];
	int size = l_instrument->size();
	int i = 0;
	const char *charResult;
	for (itor = l_instrument->begin(), i = 0; itor != l_instrument->end(); itor++, i++) {
		//cout << *itor << endl;
		charResult = (*itor).c_str();
		instrumentID[i] = new char[strlen(charResult) + 1];
		strcpy(instrumentID[i], charResult);
	}

	// 兼容飞鼠API不支持退订合约
	//this->mdapi->UnSubscribeMarketData(instrumentID, size);

	// 析构字符串数组
	for (i = 0; i < size; i++) {
		delete[]instrumentID[i];
	}
	delete[]instrumentID;

	// 取消订阅列表里清空
	for (itor = l_instrument->begin(); itor != l_instrument->end();) {
		(*itor).clear();
		itor = l_instrument->erase(itor);
	}
}

//订阅行情应答
void MdSpi::OnRspSubMarketData(CSgitFtdcSpecificInstrumentField *pSpecificInstrument, CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		if (pSpecificInstrument) {
			Utils::printGreenColorWithKV("MdSpi::OnRspSubMarketData() 订阅行情合约代码", pSpecificInstrument->InstrumentID);
			this->ctp_m->getXtsLogger()->info("MdSpi::OnRspSubMarketData() 订阅行情合约代码 = {}", pSpecificInstrument->InstrumentID);
		}
	}
}

//取消订阅行情
void MdSpi::UnSubscribeMarketData(char *ppInstrumentID[], int nCount) {
	USER_PRINT("MdSpi::UnSubscribeMarketData");
	if (this->isLogged) {
		
		//兼容飞鼠API(不支持退订)
		//this->mdapi->UnSubscribeMarketData(ppInstrumentID, nCount);
		/*int ret = this->controlTimeOut(&unsubmarket_sem);
		if (ret == -1) {
			USER_PRINT("MdSpi::UnSubscribeMarketData TimeOut!");
		}*/
	}
	else {
		USER_PRINT("Please Login First!");
	}
}

//得到BrokerID
string MdSpi::getBrokerID() {
	return this->BrokerID;
}

//得到UserID
string MdSpi::getUserID() {
	return this->UserID;
}

//得到Password
string MdSpi::getPassword() {
	return this->Password;
}

//添加strategy
void MdSpi::addStrategyToList(Strategy *stg) {
	this->l_strategys->push_back(stg);
}

/// 得到strategy_list
list<Strategy *> * MdSpi::getListStrategy() {
	return this->l_strategys;
}

/// 设置strategy_list
void MdSpi::setListStrategy(list<Strategy *> *l_strategys) {
	this->l_strategys = l_strategys;
}

void MdSpi::setCtpManager(CTP_Manager *ctp_m) {
	this->ctp_m = ctp_m;
}
CTP_Manager * MdSpi::getCtpManager() {
	return this->ctp_m;
}

//取消订阅行情应答
void MdSpi::OnRspUnSubMarketData(CSgitFtdcSpecificInstrumentField *pSpecificInstrument, CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (bIsLast && !(this->IsErrorRspInfo(pRspInfo))) {
		this->ctp_m->getXtsLogger()->info("MdSpi::OnRspUnSubMarketData() 取消合约代码 = {} 取消应答信息 = {}", pSpecificInstrument->InstrumentID, pRspInfo->ErrorMsg);
		//sem_post(&unsubmarket_sem);
	}
}




//深度行情接收
void MdSpi::OnRtnDepthMarketData(CSgitFtdcDepthMarketDataField *pDepthMarketData) {
	//cout << "===========================================" << endl;
    //cout << "深度行情" << ", ";
	cout << "交易日:" << pDepthMarketData->TradingDay << ", " << "合约代码:" << pDepthMarketData->InstrumentID << ", " << "最新价:" << pDepthMarketData->LastPrice << ", " << "持仓量:" << pDepthMarketData->OpenInterest << endl;
    //<< "上次结算价:" << pDepthMarketData->presettlementprice << endl
 //   //<< "昨收盘:" << pDepthMarketData->PreClosePrice << endl
 //   //<< "数量:" << pDepthMarketData->Volume << endl
 //   //<< "昨持仓量:" << pDepthMarketData->PreOpenInterest << endl
	//<< "最后修改时间" << pDepthMarketData->UpdateTime << ", "
	//<< "最后修改毫秒" << pDepthMarketData->UpdateMillisec << endl;
 //   //<< "申买价一：" << pDepthMarketData->BidPrice1 << endl
 //   //<< "申买量一:" << pDepthMarketData->BidVolume1 << endl
 //   //<< "申卖价一:" << pDepthMarketData->AskPrice1 << endl
 //   //<< "申卖量一:" << pDepthMarketData->AskVolume1 << endl
 //   //<< "今收盘价:" << pDepthMarketData->ClosePrice << endl
 //   //<< "当日均价:" << pDepthMarketData->AveragePrice << endl
 //   //<< "本次结算价格:" << pDepthMarketData->SettlementPrice << endl
 //   //<< "成交金额:" << pDepthMarketData->Turnover << endl
    
	/*std::cout << "MdSpi::OnRtnDepthMarketData()" << std::endl;
	std::cout << "\t this->l_strategys->size() = " << this->l_strategys->size() << std::endl;*/
	//int count = 0;

	//if (!strcmp(this->last_tick_data->InstrumentID, pDepthMarketData->InstrumentID) &&
	//	!strcmp(this->last_tick_data->UpdateTime, pDepthMarketData->UpdateTime) &&
	//	(this->last_tick_data->UpdateMillisec == pDepthMarketData->UpdateMillisec)) {
	//	std::cout << "MdSpi::OnRtnDepthMarketData() 重复tick推送" << std::endl;
		//cout << "\t交易日:" << pDepthMarketData->TradingDay
			//<< ", 合约代码:" << pDepthMarketData->InstrumentID
	//		<< ", 最新价:" << pDepthMarketData->LastPrice
	//		<< ", 持仓量:" << pDepthMarketData->OpenInterest
	//		//<< ", 上次结算价:" << pDepthMarketData->PreSettlementPrice 
	//		//<< ", 昨收盘:" << pDepthMarketData->PreClosePrice 
	//		<< ", 数量:" << pDepthMarketData->Volume
	//		//<< ", 昨持仓量:" << pDepthMarketData->PreOpenInterest
	//		<< ", 最后修改时间:" << pDepthMarketData->UpdateTime
	//		<< ", 最后修改毫秒:" << pDepthMarketData->UpdateMillisec
	//		<< ", 申买价一：" << pDepthMarketData->BidPrice1
	//		<< ", 申买量一:" << pDepthMarketData->BidVolume1
	//		<< ", 申卖价一:" << pDepthMarketData->AskPrice1
	//		<< ", 申卖量一:" << pDepthMarketData->AskVolume1
	//		//<< ", 今收盘价:" << pDepthMarketData->ClosePrice
	//		//<< ", 当日均价:" << pDepthMarketData->AveragePrice
	//		//<< ", 本次结算价格:" << pDepthMarketData->SettlementPrice
			//<< ", 成交金额:" << pDepthMarketData->Turnover << endl;
	//	return;
	//}
	//else {
	//	this->CopyTickData(this->last_tick_data, pDepthMarketData);
	//	list<Strategy *>::iterator itor;
	//	for (itor = this->l_strategys->begin(); itor != this->l_strategys->end(); itor++) {
	//		USER_PRINT(((*itor)));
	//		//std::cout << "\tcount = " << count++ << std::endl;
	//		(*itor)->OnRtnDepthMarketData(pDepthMarketData);
	//	}
	//}

	list<Strategy *>::iterator itor;

	// 当有其他地方调用策略列表,阻塞,信号量P操作
	sem_wait((this->ctp_m->getSem_strategy_handler()));

	for (itor = this->l_strategys->begin(); itor != this->l_strategys->end(); itor++) {
		USER_PRINT(((*itor)));
		//std::cout << "\tcount = " << count++ << std::endl;
		//std::cout << "UserID = " << (*itor)->getStgUserId() << ", StrategyID = " << (*itor)->getStgStrategyId() << std::endl;
		(*itor)->OnRtnDepthMarketData(pDepthMarketData);
	}

	// 释放信号量,信号量V操作
	sem_post((this->ctp_m->getSem_strategy_handler()));

	//cout << "===========================================" << endl;
}

void MdSpi::CopyTickData(CSgitFtdcDepthMarketDataField *dst, CSgitFtdcDepthMarketDataField *src) {
	///交易日
	strcpy(dst->TradingDay, src->TradingDay);
	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);
	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);
	///合约在交易所的代码
	strcpy(dst->ExchangeInstID, src->ExchangeInstID);
	///最新价
	dst->LastPrice = src->LastPrice;
	///上次结算价
	dst->PreSettlementPrice = src->PreSettlementPrice;
	///昨收盘
	dst->PreClosePrice = src->PreClosePrice;
	///昨持仓量
	dst->PreOpenInterest = src->PreOpenInterest;
	///今开盘
	dst->OpenPrice = src->OpenPrice;
	///最高价
	dst->HighestPrice = src->HighestPrice;
	///最低价
	dst->LowestPrice = src->LowestPrice;
	///数量
	dst->Volume = src->Volume;
	///成交金额
	dst->Turnover = src->Turnover;
	///持仓量
	dst->OpenInterest = src->OpenInterest;
	///今收盘
	dst->ClosePrice = src->ClosePrice;
	///本次结算价
	dst->SettlementPrice = src->SettlementPrice;
	///涨停板价
	dst->UpperLimitPrice = src->UpperLimitPrice;
	///跌停板价
	dst->LowerLimitPrice = src->LowerLimitPrice;
	///昨虚实度
	dst->PreDelta = src->PreDelta;
	///今虚实度
	dst->CurrDelta = src->CurrDelta;
	///最后修改时间
	strcpy(dst->UpdateTime, src->UpdateTime);
	///最后修改毫秒
	dst->UpdateMillisec = src->UpdateMillisec;
	///申买价一
	dst->BidPrice1 = src->BidPrice1;
	///申买量一
	dst->BidVolume1 = src->BidVolume1;
	///申卖价一
	dst->AskPrice1 = src->AskPrice1;
	///申卖量一
	dst->AskVolume1 = src->AskVolume1;
	///申买价二
	dst->BidPrice2 = src->BidPrice2;
	///申买量二
	dst->BidVolume2 = src->BidVolume2;
	///申卖价二
	dst->AskPrice2 = src->AskPrice2;
	///申卖量二
	dst->AskVolume2 = src->AskVolume2;
	///申买价三
	dst->BidPrice3 = src->BidPrice3;
	///申买量三
	dst->BidVolume3 = src->BidVolume3;
	///申卖价三
	dst->AskPrice3 = src->AskPrice3;
	///申卖量三
	dst->AskVolume3 = src->AskVolume3;
	///申买价四
	dst->BidPrice4 = src->BidPrice4;
	///申买量四
	dst->BidVolume4 = src->BidVolume4;
	///申卖价四
	dst->AskPrice4 = src->AskPrice4;
	///申卖量四
	dst->AskVolume4 = src->AskVolume4;
	///申买价五
	dst->BidPrice5 = src->BidPrice5;
	///申买量五
	dst->BidVolume5 = src->BidVolume5;
	///申卖价五
	dst->AskPrice5 = src->AskPrice5;
	///申卖量五
	dst->AskVolume5 = src->AskVolume5;
	///当日均价
	dst->AveragePrice = src->AveragePrice;
	///业务日期
	//strcpy(dst->ActionDay, src->ActionDay);
}



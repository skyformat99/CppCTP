#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable, std::cv_status


#include "TdSpi.h"
#include "Utils.h"
#include "User.h"
#include "Debug.h"
#include "Session.h"

//转码数组
char codeDst[90] = { 0 };
std::condition_variable cv;
std::mutex mtx;
std::mutex position_add_mtx;
#define RSP_TIMEOUT	120



//协程控制
int TdSpi::controlTimeOut(sem_t *t, int timeout) {
	/*协程开始*/
	//struct timeval now;
	//struct timespec outtime;
	//gettimeofday(&now, NULL);
	////std::cout << now.tv_sec << " " << (now.tv_usec) << "\n";
	//timeraddMS(&now, timeout);
	//outtime.tv_sec = now.tv_sec;
	//outtime.tv_nsec = now.tv_usec * 1000;
	//std::cout << outtime.tv_sec << " " << (outtime.tv_nsec) << "\n";
	//int ret = sem_timedwait(t, &outtime);
	int ret = 0;
	int value = 0;
	//sem_getvalue(t, &value);
	//std::cout << "value = " << value << endl;
	//std::cout << "ret = " << ret << endl;
	/*协程结束*/
	return ret;
}

//增加毫秒
void TdSpi::timeraddMS(struct timeval *now_time, int ms) {
	now_time->tv_usec += ms * 1000;
	if (now_time->tv_usec >= 1000000) {
		now_time->tv_sec += now_time->tv_usec / 1000000;
		now_time->tv_usec %= 1000000;
	}
}

//构造函数
TdSpi::TdSpi() {
	USER_PRINT("TdSpi::TdSpi");
	/*int flag = Utils::CreateFolder(flowpath.c_str());
	if (flag != 0) {
		USER_PRINT("Can not create folder");
	}
	tdapi = CThostFtdcTraderApi::CreateFtdcTraderApi(flowpath.c_str());*/
	/*sem_init(&connect_sem, 0, 1);
	sem_init(&login_sem, 0, 1);
	sem_init(&logout_sem, 0, 1);
	sem_init(&sem_ReqQrySettlementInfoConfirm, 0, 1);
	sem_init(&sem_ReqQrySettlementInfo, 0, 1);
	sem_init(&sem_ReqSettlementInfoConfirm, 0, 1);*/
	this->l_instruments_info = new list<CThostFtdcInstrumentField *>();
	this->l_strategys = NULL;
	this->isFirstQryTrade = true;
	this->isFirstQryOrder = true;
	this->l_query_trade = new list<CThostFtdcTradeField *>();
	this->l_query_order = new list<CThostFtdcOrderField *>();
}

//增加api
void TdSpi::addApi(User *user, string flowpath) {
	USER_PRINT("TdSpi::addApi");
	USER_PRINT(flowpath);
	

	int flag = Utils::CreateFolder(flowpath.c_str());
	if (flag != 0) {
		USER_PRINT("Can not create folder");
	} else {
		this->tdapi = CThostFtdcTraderApi::CreateFtdcTraderApi(flowpath.c_str());
		cout << this->tdapi << endl;
		USER_PRINT("API ADD TO user Already!");
		//l_api.push_back(tdapi);
	}
}

//建立连接
void TdSpi::Connect(User *user, bool init_flag) {
	USER_PRINT("TdSpi::Connect");
	USER_PRINT(const_cast<char *>(user->getFrontAddress().c_str()));
	this->tdapi = user->getUserTradeAPI();
	cout << "TdSpi::Connect()" << std::endl;
	cout << "\t已创建连接对象 = " << this->tdapi << std::endl;
	this->tdapi->RegisterFront(const_cast<char *>(user->getFrontAddress().c_str()));
	//注册事件处理对象
	this->tdapi->RegisterSpi(user->getUserTradeSPI());
	//订阅共有流
	this->tdapi->SubscribePublicTopic(THOST_TERT_QUICK);
	/************************************************************************/
	/* 根据读取数据库数据决定订阅私有流是否需要重新传输                                                                     */
	/************************************************************************/
	//订阅私有流
	if (init_flag) //系统正常初始化,从上次退出继续初始化
	{
		std::cout << "\t启动模式 = THOST_TERT_RESUME" << std::endl;
		this->tdapi->SubscribePrivateTopic(THOST_TERT_RESUME);
	} 
	else // 系统非正常退出,重新传送所有数据
	{
		std::cout << "\t启动模式 = THOST_TERT_RESTART" << std::endl;
		this->tdapi->SubscribePrivateTopic(THOST_TERT_RESTART);
	}

	this->tdapi->Init();

	//等待回调
	std::unique_lock<std::mutex> lck(mtx);
	while (cv.wait_for(lck, std::chrono::seconds(RSP_TIMEOUT)) == std::cv_status::timeout) {
		std::cout << "TdSpi::Connect()" << std::endl;
		std::cout << "\t连接等待超时" << std::endl;
		user->setIsConnected(false);
		return;
	}

}

//当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
void TdSpi::OnFrontConnected() {
	USER_PRINT("TdSpi::OnFrontConnected");
	//if (this->isFirstTimeLogged) {
	//	//sem_post(&connect_sem);
	//	//this->Login("0187", "86001525", "206029");
	//}
	//else {
	//	//sem_init(&login_sem, 0, 1);
	//	
	//	//this->Login(this->c_BrokerID, this->c_UserID, this->c_Password);
	//}
	cv.notify_one();
}

///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
///@param nReason 错误原因
///        0x1001 网络读失败
///        0x1002 网络写失败
///        0x2001 接收心跳超时
///        0x2002 发送心跳失败
///        0x2003 收到错误报文
void TdSpi::OnFrontDisconnected(int nReason) {
	std::cout << "TdSpi::OnFrontDisconnected()" << std::endl;
	std::cout << "\t断线原因 = " << nReason << std::endl;
	//std::cout << "\t当前用户 = " << this-> << std::endl;
	this->ctp_m->sendTradeOffLineMessage(this->current_user->getUserID());
}

//登录
void TdSpi::Login(User *user) {
	USER_PRINT("TdSpi::Login");
	this->current_user = user;

	this->BrokerID = user->getBrokerID();
	this->UserID = user->getUserID();
	this->Password = user->getPassword();

	//this->l_BrokerID.push_back(BrokerID);
	//this->l_UserID.push_back(UserID);
	//this->l_Password.push_back(Password);

	loginField = new CThostFtdcReqUserLoginField();
	strcpy(loginField->BrokerID, user->getBrokerID().c_str());
	strcpy(loginField->UserID, user->getUserID().c_str());
	strcpy(loginField->Password, user->getPassword().c_str());
	this->tdapi->ReqUserLogin(loginField, user->getRequestID());

	/*int ret = this->controlTimeOut(&login_sem);

	if (ret == -1) {
		USER_PRINT("TdSpi::Login TimeOut!");
	}*/

	//等待登陆回调
	std::unique_lock<std::mutex> lck(mtx);
	while (cv.wait_for(lck, std::chrono::seconds(RSP_TIMEOUT)) == std::cv_status::timeout) {
		std::cout << "TdSpi::Connect()" << std::endl;
		std::cout << "\t登陆等待超时" << std::endl;
		user->setIsLogged(false);
		return;
	}

	delete loginField;
}

///登录请求响应
void TdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                           CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	USER_PRINT("TdSpi::OnRspUserLogin")
	USER_PRINT(bIsLast)
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		//sem_post(&login_sem);
		std::cout << "=================================================================================" << endl;
		std::cout << "||TdAPI 交易日:" << this->tdapi->GetTradingDay() << ", ";
		///交易日
		//std::cout << "CThostFtdcRspUserLoginField 交易日:" << pRspUserLogin->TradingDay << ", ";
		///登录成功时间
		std::cout << "登录成功时间:" << pRspUserLogin->LoginTime << ", ";
		///经纪公司代码
		std::cout << "经纪公司代码:" << pRspUserLogin->BrokerID << ", ";
		///用户代码
		std::cout << "用户代码:" << pRspUserLogin->UserID << endl;
		///交易系统名称
		std::cout << "||交易系统名称:" << pRspUserLogin->SystemName << ", ";
		///前置编号
		std::cout << "前置编号:" << pRspUserLogin->FrontID << ", ";
		this->FrontID = pRspUserLogin->FrontID;
		///会话编号
		std::cout << "会话编号:" << pRspUserLogin->SessionID << ", ";
		this->SessionID = pRspUserLogin->SessionID;
		///最大报单引用
		std::cout << "最大报单引用:" << pRspUserLogin->MaxOrderRef << ", ";
		///上期所时间
		std::cout << "上期所时间:" << pRspUserLogin->SHFETime << endl;
		///大商所时间
		std::cout << "||大商所时间:" << pRspUserLogin->DCETime << ", ";
		///郑商所时间
		std::cout << "郑商所时间:" << pRspUserLogin->CZCETime << ", ";
		///中金所时间
		std::cout << "中金所时间" << pRspUserLogin->FFEXTime << ", ";
		///能源中心时间
		std::cout << "能源中心时间" << pRspUserLogin->INETime << endl;
		string s_trading_day = this->tdapi->GetTradingDay();
		std::cout << "=================================================================================" << endl;
		
		this->current_user->setBrokerID(pRspUserLogin->BrokerID);
		this->current_user->setUserID(pRspUserLogin->UserID);
		this->current_user->setRequestID(nRequestID);
		this->current_user->setIsConfirmSettlement(false);
		this->current_user->setUserTradeAPI(this->tdapi);
		this->current_user->setUserTradeSPI(this);
		this->current_user->setTradingDay(s_trading_day);

		this->setIsConfirmSettlement(false);

		/************************************************************************/
		/* 系统登录的session_id暂时不用维护,通过order_ref来判断是否属于本系统发单                                                                     */
		/************************************************************************/
		//Session *sid = new Session(this->current_user->getUserID(), this->SessionID, this->FrontID, s_trading_day);
		//this->current_user->getL_Sessions()->push_back(sid);
		//this->current_user->getDBManager()->CreateSession(sid);

		USER_PRINT(this->current_user);
		USER_PRINT(this->current_user->getUserID());
	}
	else {
		std::cout << "TdSpi::OnRspUserLogin()" << std::endl;
		std::cout << "\t登陆出错!" << std::endl;
		this->current_user->setIsLoggedError(true);
		return;
	}
	//释放
	cv.notify_one();
}

//查询交易结算确认
void TdSpi::QrySettlementInfoConfirm(User *user) {
	USER_PRINT("TdSpi::QrySettlementInfoConfirm")

	CThostFtdcQrySettlementInfoConfirmField *qrySettlementField = new CThostFtdcQrySettlementInfoConfirmField();

	strcpy(qrySettlementField->BrokerID, user->getBrokerID().c_str());
	strcpy(qrySettlementField->InvestorID, user->getUserID().c_str());

	sleep(1);
	this->tdapi->ReqQrySettlementInfoConfirm(qrySettlementField, user->getRequestID());

	/*int ret = this->controlTimeOut(&sem_ReqQrySettlementInfoConfirm);
	if (ret == -1) {
	USER_PRINT("TdSpi::QrySettlementInfoConfirm TimeOut!")
	}*/


	delete qrySettlementField;
}


//请求查询结算信息确认响应
void TdSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQrySettlementInfoConfirm");
	USER_PRINT(bIsLast)
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		if (this->current_user->getRequestID() == nRequestID) {
			//sem_post(&sem_ReqQrySettlementInfoConfirm);
			if ((!pSettlementInfoConfirm)) { //如果未确认过，pSettlementInfoConfirm为空
				USER_PRINT("pSettlementInfoConfirm is null");
				USER_PRINT(this->isConfirmSettlement);
				if (this->isConfirmSettlement) {
					USER_PRINT("Already Confirm!");
					std::cout << "|==确认结算信息==|" << endl;
					std::cout << "|经纪公司" << this->current_user->getBrokerID() << "|" << endl;
					std::cout << "|结算客户" << this->current_user->getUserID() << "|" << endl;
					std::cout << "|确认日期" << this->getCharTradingDate() << "|" << endl;
					std::cout << "|交易时间" << this->tdapi->GetTradingDay() << "|" << endl;
					std::cout << "|================|" << endl;
				} else {
					sleep(1);
					this->QrySettlementInfo(this->current_user);
				}
			} else {
				USER_PRINT("今天已经确认结算!");
				std::cout << "|==确认结算信息==|" << endl;
				std::cout << "|经纪公司代码" << pSettlementInfoConfirm->BrokerID << "|" << endl;
				std::cout << "|投资者代码" << pSettlementInfoConfirm->InvestorID << "|" << endl;
				std::cout << "|确认日期" << pSettlementInfoConfirm->ConfirmDate << "|" << endl;
				std::cout << "|确认时间" << pSettlementInfoConfirm->ConfirmTime << "|" << endl;
				std::cout << "|交易时间" << this->tdapi->GetTradingDay() << "|" << endl;
				std::cout << "|================|" << endl;
				//std::chrono::milliseconds sleepDuration(15 * 1000);
				//sleep(1);
			}
	}
	}
}

//查询结算信息
void TdSpi::QrySettlementInfo(User *user) {
	USER_PRINT("TdSpi::QrySettlementInfo")
	std::cout << "broker ID = " << user->getBrokerID() << endl;
	std::cout << "InvestorID = " << user->getUserID() << endl;

	CThostFtdcQrySettlementInfoField *pQrySettlementInfo = new CThostFtdcQrySettlementInfoField();
	strcpy(pQrySettlementInfo->BrokerID, user->getBrokerID().c_str());
	strcpy(pQrySettlementInfo->InvestorID, user->getUserID().c_str());

	strcpy(pQrySettlementInfo->TradingDay, "");

	sleep(1);
	this->tdapi->ReqQrySettlementInfo(pQrySettlementInfo, user->getRequestID());
	USER_PRINT("after ReqQrySettlementInfo1");

	/*int ret = this->controlTimeOut(&sem_ReqQrySettlementInfo);
	if (ret == -1) {
		USER_PRINT("TdSpi::QrySettlementInfo TimeOut!")
	}*/

	delete pQrySettlementInfo;
	USER_PRINT("after ReqQrySettlementInfo2");
}

//请求查询投资者结算结果响应
void TdSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo,
                                   CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQrySettlementInfo");
	std::cout << "bIsLast = " << bIsLast;
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		if (this->current_user->getRequestID() == nRequestID) {
			//sem_post(&sem_ReqQrySettlementInfo);
			if (pSettlementInfo) {
				///交易日
				std::cout << "交易日:" << pSettlementInfo->TradingDay << endl;
				///结算编号
				std::cout << "结算编号:" << pSettlementInfo->SettlementID << endl;
				///经纪公司代码
				std::cout << "经纪公司代码:" << pSettlementInfo->BrokerID << endl;
				///投资者代码
				std::cout << "投资者代码:" << pSettlementInfo->InvestorID << endl;
				///序号
				std::cout << "序号:" << pSettlementInfo->SequenceNo << endl;
				///消息正文
				std::cout << "消息正文:" << pSettlementInfo->Content << endl;
			}

			if (bIsLast) {
				//确认投资者结算结果
				this->ConfirmSettlementInfo(this->current_user);
			}
		}
		
	}
	
}

//确认结算结果
void TdSpi::ConfirmSettlementInfo(User *user) {
	USER_PRINT("TdSpi::ConfirmSettlementInfo");
	CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm = new CThostFtdcSettlementInfoConfirmField();

	strcpy(pSettlementInfoConfirm->BrokerID, user->getBrokerID().c_str());
	strcpy(pSettlementInfoConfirm->InvestorID, user->getUserID().c_str());
	USER_PRINT(this->tdapi->GetTradingDay());
	strcpy(pSettlementInfoConfirm->ConfirmDate, this->tdapi->GetTradingDay());

	sleep(1);
	this->tdapi->ReqSettlementInfoConfirm(pSettlementInfoConfirm, user->getRequestID());

	int ret = this->controlTimeOut(&sem_ReqSettlementInfoConfirm);
	if (ret == -1) {
		USER_PRINT("TdSpi::ConfirmSettlementInfo")
	}
	delete pSettlementInfoConfirm;
}

//投资者结算结果确认响应
void TdSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspSettlementInfoConfirm");
	USER_PRINT(bIsLast);
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		//sem_post(&sem_ReqSettlementInfoConfirm);
		if (this->current_user->getRequestID() == nRequestID) {
			if (pSettlementInfoConfirm) {
				///经纪公司代码
				std::cout << "经纪公司代码" << pSettlementInfoConfirm->BrokerID << endl;
				///投资者代码
				std::cout << "投资者代码" << pSettlementInfoConfirm->InvestorID << endl;
				///确认日期
				std::cout << "确认日期" << pSettlementInfoConfirm->ConfirmDate << endl;
				///确认时间
				std::cout << "确认时间" << pSettlementInfoConfirm->ConfirmTime << endl;
				string today = this->tdapi->GetTradingDay();
				string confirm_date = pSettlementInfoConfirm->ConfirmDate;
				if (today == confirm_date) {
					USER_PRINT("today_date == confirm_date");
					this->setIsConfirmSettlement(true);
				}
				else {
					USER_PRINT("today_date != confirm_date");
					this->setIsConfirmSettlement(false);
					this->current_user->setIsConfirmSettlement(false);
				}
			}

			if (bIsLast) {
				this->QrySettlementInfoConfirm(this->current_user);
			}
		}
	}
	
}

//查询交易所
void TdSpi::QryExchange() {
	USER_PRINT("TdSpi::QryExchange");
	CThostFtdcQryExchangeField *pQryExchange = new CThostFtdcQryExchangeField();
	strcpy(pQryExchange->ExchangeID, "");
	this->tdapi->ReqQryExchange(pQryExchange, this->loginRequestID);
	delete pQryExchange;
}

//响应查询交易所
void TdSpi::OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQryExchange")
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		///交易所代码
		std::cout << "交易所代码" << pExchange->ExchangeID << endl;
		///交易所名称
		std::cout << "交易所名称" << pExchange->ExchangeName << endl;
		///交易所属性
		std::cout << "交易所属性" << pExchange->ExchangeProperty << endl;
	}
}

//查询合约
void TdSpi::QryInstrument(string exchangeid, string instrumentid) {
	USER_PRINT("TdSpi::QryInstrument Begin")
	CThostFtdcQryInstrumentField *pQryInstrument = new CThostFtdcQryInstrumentField();
	strcpy(pQryInstrument->ExchangeID, exchangeid.c_str());
	strcpy(pQryInstrument->InstrumentID, instrumentid.c_str());
	sleep(1);
	this->tdapi->ReqQryInstrument(pQryInstrument, this->loginRequestID + 1);
	USER_PRINT("TdSpi::QryInstrument End")
	delete pQryInstrument;
}

//查询行情
void TdSpi::QryDepthMarketData(string instrumentid) {
	CThostFtdcQryDepthMarketDataField *pQryDepthMarketData = new CThostFtdcQryDepthMarketDataField();

	strcpy(pQryDepthMarketData->InstrumentID, instrumentid.c_str());

	this->tdapi->ReqQryDepthMarketData(pQryDepthMarketData, this->current_user->getRequestID());
	delete pQryDepthMarketData;
}

///请求查询行情响应
void TdSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQryDepthMarketData");
	if (!IsErrorRspInfo(pRspInfo)) {
		if (pDepthMarketData) {
			std::cout << "=================================================================================" << endl;
			///交易日
			cout << "交易日:" << pDepthMarketData->TradingDay << ", ";
			///合约代码
			cout << "合约代码:" << pDepthMarketData->InstrumentID << ", ";
			///交易所代码
			cout << "交易所代码:" << pDepthMarketData->ExchangeID << ", ";
			///合约在交易所的代码
			cout << "合约在交易所的代码:" << pDepthMarketData->ExchangeInstID << ", ";
			///最新价
			cout << "最新价:" << pDepthMarketData->LastPrice << endl;
			///上次结算价
			cout << "上次结算价:" << pDepthMarketData->PreSettlementPrice << ", ";
			///昨收盘
			cout << "昨收盘:" << pDepthMarketData->PreClosePrice << ", ";
			///昨持仓量
			cout << "昨持仓量:" << pDepthMarketData->PreOpenInterest << ", ";
			///今开盘
			cout << "今开盘:" << pDepthMarketData->OpenPrice << ", ";
			///最高价
			cout << "最高价:" << pDepthMarketData->HighestPrice << endl;
			///最低价
			cout << "最低价:" << pDepthMarketData->LowestPrice << ", ";
			///数量
			cout << "数量:" << pDepthMarketData->Volume << ", ";
			///成交金额
			cout << "成交金额:" << pDepthMarketData->Turnover << ", ";
			///持仓量
			cout << "持仓量:" << pDepthMarketData->OpenInterest << ", ";
			///今收盘
			cout << "今收盘:" << pDepthMarketData->ClosePrice << endl;
			///本次结算价
			cout << "本次结算价:" << pDepthMarketData->SettlementPrice << ", ";
			///涨停板价
			cout << "涨停板价:" << pDepthMarketData->UpperLimitPrice << ", ";
			///跌停板价
			cout << "跌停板价:" << pDepthMarketData->LowerLimitPrice << ", ";
			///昨虚实度
			cout << "昨虚实度:" << pDepthMarketData->PreDelta << ", ";
			///今虚实度
			cout << "今虚实度:" << pDepthMarketData->CurrDelta << endl;
			///最后修改时间
			cout << "最后修改时间:" << pDepthMarketData->UpdateTime << ", ";
			///最后修改毫秒
			cout << "最后修改毫秒:" << pDepthMarketData->UpdateMillisec << ", ";
			///申买价一
			cout << "申买价一:" << pDepthMarketData->BidPrice1 << ", ";
			///申买量一
			cout << "申买量一:" << pDepthMarketData->BidVolume1 << ", ";
			///申卖价一
			cout << "申卖价一:" << pDepthMarketData->AskPrice1 << endl;
			///申卖量一
			cout << "申卖量一:" << pDepthMarketData->AskVolume1 << ", ";
			///申买价二
			cout << "申买价二:" << pDepthMarketData->BidPrice2 << ", ";
			///申买量二
			cout << "申买量二:" << pDepthMarketData->BidVolume2 << ", ";
			///申卖价二
			cout << "申卖价二:" << pDepthMarketData->AskPrice2 << ", ";
			///申卖量二
			cout << "申卖量二:" << pDepthMarketData->AskVolume2 << endl;
			///申买价三
			cout << "申买价三:" << pDepthMarketData->BidPrice3 << ", ";
			///申买量三
			cout << "申买量三:" << pDepthMarketData->BidVolume3 << ", ";
			///申卖价三
			cout << "申卖价三:" << pDepthMarketData->AskPrice3 << ", ";
			///申卖量三
			cout << "申卖量三:" << pDepthMarketData->AskVolume3 << ", ";
			///申买价四
			cout << "申买价四:" << pDepthMarketData->BidPrice4 << endl;
			///申买量四
			cout << "申买量四:" << pDepthMarketData->BidVolume4 << ", ";
			///申卖价四
			cout << "申卖价四:" << pDepthMarketData->AskPrice4 << ", ";
			///申卖量四
			cout << "申卖量四:" << pDepthMarketData->AskVolume4 << ", ";
			///申买价五
			cout << "申买价五:" << pDepthMarketData->BidPrice5 << ", ";
			///申买量五
			cout << "申买量五:" << pDepthMarketData->BidVolume5 << endl;
			///申卖价五
			cout << "申卖价五:" << pDepthMarketData->AskPrice5 << ", ";
			///申卖量五
			cout << "申卖量五:" << pDepthMarketData->AskVolume5 << ", ";
			///当日均价
			cout << "当日均价:" << pDepthMarketData->AveragePrice << ", ";
			///业务日期
			cout << "业务日期:" << pDepthMarketData->ActionDay << endl;
			std::cout << "=================================================================================" << endl;
		}
	}
}

//响应查询合约
void TdSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	//USER_PRINT("TdSpi::OnRspQryInstrument")
	//std::cout << "isLast" << bIsLast << endl;
	if ((!this->IsErrorRspInfo(pRspInfo))) {
		if (pInstrument) {
			/// 初始化的时候，必须保证l_instruments_info为空
			/*if (this->l_instruments_info->size() > 0) {
			list<CThostFtdcInstrumentField *>::iterator instrument_itor;
			for (instrument_itor = l_instruments_info->begin(); instrument_itor != l_instruments_info->end();) {
			instrument_itor = l_instruments_info->erase(instrument_itor);
			}
			}*/

			///合约代码
			///std::cout << "合约代码:" << pInstrument->InstrumentID << ", ";
			///交易所代码
			///std::cout << "交易所代码:" << pInstrument->ExchangeID << ", ";
			///合约名称
			///std::cout << "合约名称:" << pInstrument->InstrumentName << ", ";
			///合约在交易所的代码
			///std::cout << "合约在交易所的代码:" << pInstrument->ExchangeInstID << ", ";
			///产品代码
			///std::cout << "产品代码:" << pInstrument->ProductID << ", ";
			///产品类型
			///std::cout << "产品类型:" << pInstrument->ProductClass << endl;
			/*///交割年份
			std::cout << "交割年份" << pInstrument->DeliveryYear << endl;
			///交割月
			std::cout << "交割月" << pInstrument->DeliveryMonth << endl;
			///市价单最大下单量
			std::cout << "市价单最大下单量" << pInstrument->MaxMarketOrderVolume << endl;
			///市价单最小下单量
			std::cout << "市价单最小下单量" << pInstrument->MinMarketOrderVolume << endl;
			///限价单最大下单量
			std::cout << "限价单最大下单量" << pInstrument->MaxLimitOrderVolume << endl;
			///限价单最小下单量
			std::cout << "限价单最小下单量" << pInstrument->MinLimitOrderVolume << endl;
			///合约数量乘数
			std::cout << "合约数量乘数" << pInstrument->VolumeMultiple << endl;
			///最小变动价位
			std::cout << "最小变动价位" << pInstrument->PriceTick << endl;
			///创建日
			std::cout << "创建日" << pInstrument->CreateDate << endl;
			///上市日
			std::cout << "上市日" << pInstrument->OpenDate << endl;
			///到期日
			std::cout << "到期日" << pInstrument->ExpireDate << endl;
			///开始交割日
			std::cout << "开始交割日" << pInstrument->StartDelivDate << endl;
			///结束交割日
			std::cout << "结束交割日" << pInstrument->EndDelivDate << endl;
			///合约生命周期状态
			std::cout << "合约生命周期状态" << pInstrument->InstLifePhase << endl;
			///当前是否交易
			std::cout << "当前是否交易" << pInstrument->IsTrading << endl;
			///持仓类型
			std::cout << "持仓类型" << pInstrument->PositionType << endl;
			///持仓日期类型
			std::cout << "持仓日期类型" << pInstrument->PositionDateType << endl;
			///多头保证金率
			std::cout << "多头保证金率" << pInstrument->LongMarginRatio << endl;
			///空头保证金率
			std::cout << "空头保证金率" << pInstrument->ShortMarginRatio << endl;
			///是否使用大额单边保证金算法
			std::cout << "是否使用大额单边保证金算法" << pInstrument->MaxMarginSideAlgorithm << endl;
			///基础商品代码
			std::cout << "基础商品代码" << pInstrument->UnderlyingInstrID << endl;
			///执行价
			std::cout << "执行价" << pInstrument->StrikePrice << endl;
			///期权类型
			std::cout << "期权类型" << pInstrument->OptionsType << endl;
			///合约基础商品乘数
			std::cout << "合约基础商品乘数" << pInstrument->UnderlyingMultiple << endl;
			///组合类型
			std::cout << "组合类型" << pInstrument->CombinationType << endl;*/

			CThostFtdcInstrumentField *pInstrument_tmp = new CThostFtdcInstrumentField();
			/*memset(pInstrument_tmp, 0x00, sizeof(pInstrument_tmp));
			memcpy(pInstrument_tmp, pInstrument, sizeof(CThostFtdcInstrumentField));*/

			this->CopyInstrumentInfo(pInstrument_tmp, pInstrument);

			/*///合约代码
			std::cout << "合约代码_tmp:" << pInstrument_tmp->InstrumentID << ", ";
			///交易所代码
			std::cout << "交易所代码_tmp:" << pInstrument_tmp->ExchangeID << ", ";
			///合约名称
			std::cout << "合约名称_tmp:" << pInstrument_tmp->InstrumentName << ", ";
			///合约在交易所的代码
			std::cout << "合约在交易所的代码_tmp:" << pInstrument_tmp->ExchangeInstID << ", ";
			///产品代码
			std::cout << "产品代码_tmp:" << pInstrument_tmp->ProductID << ", ";
			///产品类型
			std::cout << "产品类型_tmp:" << pInstrument_tmp->ProductClass << endl;*/

			this->l_instruments_info->push_back(pInstrument_tmp);
		}
	}
}

//拷贝合约信息
void TdSpi::CopyInstrumentInfo(CThostFtdcInstrumentField *dst, CThostFtdcInstrumentField *src) {
	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);

	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);

	///合约名称
	TThostFtdcInstrumentNameType	InstrumentName;
	strcpy(dst->InstrumentName, src->InstrumentName);

	///合约在交易所的代码
	strcpy(dst->ExchangeInstID, src->ExchangeInstID);

	///产品代码
	strcpy(dst->ProductID, src->ProductID);

	///产品类型
	dst->ProductClass = src->ProductClass;

	///交割年份
	dst->DeliveryYear = src->DeliveryYear;

	///交割月
	dst->DeliveryMonth = src->DeliveryMonth;

	///市价单最大下单量
	dst->MaxMarketOrderVolume = src->MaxMarketOrderVolume;

	///市价单最小下单量
	dst->MinMarketOrderVolume = src->MinMarketOrderVolume;

	///限价单最大下单量
	dst->MaxLimitOrderVolume = src->MaxLimitOrderVolume;

	///限价单最小下单量
	dst->MinLimitOrderVolume = src->MinLimitOrderVolume;

	///合约数量乘数
	dst->VolumeMultiple = src->VolumeMultiple;

	///最小变动价位
	dst->PriceTick = src->PriceTick;

	///创建日
	strcpy(dst->CreateDate, src->CreateDate);

	///上市日
	strcpy(dst->OpenDate, src->OpenDate);

	///到期日
	strcpy(dst->ExpireDate, src->ExpireDate);

	///开始交割日
	strcpy(dst->StartDelivDate, src->StartDelivDate);

	///结束交割日
	strcpy(dst->EndDelivDate, src->EndDelivDate);

	///合约生命周期状态
	dst->InstLifePhase = src->InstLifePhase;

	///当前是否交易
	dst->IsTrading = src->IsTrading;

	///持仓类型
	dst->PositionType = src->PositionType;

	///持仓日期类型
	dst->PositionDateType = src->PositionDateType;

	///多头保证金率
	dst->LongMarginRatio = src->LongMarginRatio;

	///空头保证金率
	dst->ShortMarginRatio = src->ShortMarginRatio;

	///是否使用大额单边保证金算法
	dst->MaxMarginSideAlgorithm = src->MaxMarginSideAlgorithm;

	///基础商品代码
	strcpy(dst->UnderlyingInstrID, src->UnderlyingInstrID);

	///执行价
	dst->StrikePrice = src->StrikePrice;

	///期权类型
	dst->OptionsType = src->OptionsType;

	///合约基础商品乘数
	dst->UnderlyingMultiple = src->UnderlyingMultiple;

	///组合类型
	dst->CombinationType = src->CombinationType;
}

///合约交易状态通知
void TdSpi::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus) {
	//USER_PRINT("TdSpi::OnRtnInstrumentStatus");
	//if (pInstrumentStatus) {
		/*std::cout << "|==================== " << endl;
		///交易所代码
		std::cout << "|交易所代码         = " << pInstrumentStatus->ExchangeID << endl;
		///合约在交易所的代码
		std::cout << "|合约在交易所的代码 = " << pInstrumentStatus->ExchangeInstID << endl;
		///结算组代码
		std::cout << "|结算组代码         = " << pInstrumentStatus->SettlementGroupID << endl;
		///合约代码
		std::cout << "|合约代码           = " << pInstrumentStatus->InstrumentID << endl;
		///合约交易状态
		std::cout << "|合约交易状态       = " << pInstrumentStatus->InstrumentStatus << endl;
		///交易阶段编号
		std::cout << "|交易阶段编号       = " << pInstrumentStatus->TradingSegmentSN << endl;
		///进入本状态时间
		std::cout << "|进入本状态时间     = " << pInstrumentStatus->EnterTime << endl;
		///进入本状态原因
		std::cout << "|进入本状态原因     = " << pInstrumentStatus->EnterReason << endl;
		std::cout << "|==================== " << endl;*/

		//string status_time(pInstrumentStatus->EnterTime);
		//string real_status_time = this->getTradingDay() + status_time;
		//string localtime = this->getTradingDay() + "15:00:00";
		//if (Utils::compareTradingDaySeconds(real_status_time.c_str(), localtime.c_str())) { // 时间相等进行收盘工作
		//	USER_PRINT("收盘了...策略进行保存");
		//	std::cout << "TdSpi::OnRtnInstrumentStatus() 收盘了开始策略保存" << std::endl;
		//	this->current_user->getCTP_Manager()->saveStrategy();
		//}
		
	//}
}

//查询报单
void TdSpi::QryOrder() {
	USER_PRINT("TdSpi::QryOrder");
	CThostFtdcQryOrderField *pQryOrder = new CThostFtdcQryOrderField();
	//strcpy(pQryOrder->BrokerID, const_cast<char *>(this->getBrokerID().c_str()));
	//strcpy(pQryOrder->InvestorID, const_cast<char *>(this->getUserID().c_str()));
	int error_no = this->tdapi->ReqQryOrder(pQryOrder, this->getRequestID());
	//std::cout << "error_no = " << error_no << endl;
	delete pQryOrder;

	
	std::unique_lock<std::mutex> lck(mtx);
	while (cv.wait_for(lck, std::chrono::seconds(RSP_TIMEOUT)) == std::cv_status::timeout) {
		std::cout << "TdSpi::QryOrder()" << std::endl;
		std::cout << "\t等待超时" << std::endl;
		return;
	}

}

//响应查询报单;
void TdSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQryOrder");
	//list<Session *>::iterator sid_itor;

	if (!this->IsErrorRspInfo(pRspInfo)) {

		if (this->isFirstQryOrder == false) //如果新一轮接收，清空list列表
		{
			if (this->l_query_order->size() > 0) {
				list<CThostFtdcOrderField *>::iterator itor;
				for (itor = this->l_query_order->begin(); itor != this->l_query_order->end();) {
					delete (*itor);
					itor = l_query_order->erase(itor);
				}
			}
			this->isFirstQryOrder == true;
		}

		if (pOrder) {
			string temp(pOrder->OrderRef);
			string result = temp.substr(0, 1);
			if (temp.length() == 12 && result == "1") {
				//std::cout << "=================================================================================" << endl;
				/////经纪公司代码
				//std::cout << "||经纪公司代码:" << pOrder->BrokerID << ", ";
				/////投资者代码
				//std::cout << "投资者代码:" << pOrder->InvestorID << ", ";
				/////合约代码
				//std::cout << "合约代码:" << pOrder->InstrumentID << ", ";
				/////报单引用
				//std::cout << "报单引用:" << pOrder->OrderRef << ", ";
				/////用户代码
				//std::cout << "用户代码:" << pOrder->UserID << endl;
				/////报单价格条件
				//std::cout << "||报单价格条件:" << pOrder->OrderPriceType << ", ";
				/////买卖方向
				//std::cout << "买卖方向:" << pOrder->Direction << ", ";
				/////组合开平标志
				//std::cout << "组合开平标志:" << pOrder->CombOffsetFlag << ", ";
				/////组合投机套保标志
				//std::cout << "组合投机套保标志:" << pOrder->CombHedgeFlag << ", ";
				/////价格
				//std::cout << "价格:" << pOrder->LimitPrice << endl;
				/////数量
				//std::cout << "||数量:" << pOrder->VolumeTotalOriginal << ", ";
				/////有效期类型
				//std::cout << "有效期类型:" << pOrder->TimeCondition << ", ";
				/////GTD日期
				////std::cout << "GTD日期:" << pOrder->GTDDate << ", ";
				/////成交量类型
				//std::cout << "成交量类型:" << pOrder->VolumeCondition << ", ";
				/////最小成交量
				//std::cout << "最小成交量:" << pOrder->MinVolume << endl;
				/////触发条件
				//std::cout << "||触发条件:" << pOrder->ContingentCondition << ", ";
				/////止损价
				//std::cout << "止损价:" << pOrder->StopPrice << ", ";
				/////强平原因
				//std::cout << "强平原因:" << pOrder->ForceCloseReason << ", ";
				/////自动挂起标志
				//std::cout << "自动挂起标志:" << pOrder->IsAutoSuspend << ", ";
				/////业务单元
				//std::cout << "业务单元:" << pOrder->BusinessUnit << endl;
				/////请求编号
				//std::cout << "||请求编号:" << pOrder->RequestID << ", ";
				/////本地报单编号
				//std::cout << "本地报单编号:" << pOrder->OrderLocalID << ", ";
				/////交易所代码
				//std::cout << "交易所代码:" << pOrder->ExchangeID << ", ";
				/////会员代码
				//std::cout << "会员代码:" << pOrder->ParticipantID << ", ";
				/////客户代码
				//std::cout << "客户代码:" << pOrder->ClientID << endl;
				/////合约在交易所的代码
				//std::cout << "||合约在交易所的代码:" << pOrder->ExchangeInstID << ", ";
				/////交易所交易员代码
				//std::cout << "交易所交易员代码:" << pOrder->TraderID << ", ";
				/////安装编号
				//std::cout << "安装编号:" << pOrder->InstallID << ", ";
				/////报单提交状态
				//std::cout << "报单提交状态:" << pOrder->OrderSubmitStatus << ", ";
				/////报单提示序号
				//std::cout << "报单提示序号:" << pOrder->NotifySequence << endl;
				/////交易日
				//std::cout << "||交易日:" << pOrder->TradingDay << ", ";
				/////结算编号
				//std::cout << "结算编号:" << pOrder->SettlementID << ", ";
				/////报单编号
				//std::cout << "报单编号:" << pOrder->OrderSysID << ", ";
				/////报单来源
				//std::cout << "报单来源:" << pOrder->OrderSource << ", ";
				/////报单状态
				//std::cout << "报单状态:" << pOrder->OrderStatus << endl;
				/////报单类型
				//std::cout << "||报单类型:" << pOrder->OrderType << ", ";
				/////今成交数量
				//std::cout << "今成交数量:" << pOrder->VolumeTraded << ", ";
				/////剩余数量
				//std::cout << "剩余数量:" << pOrder->VolumeTotal << ", ";
				/////报单日期
				//std::cout << "报单日期:" << pOrder->InsertDate << ", ";
				/////委托时间
				//std::cout << "委托时间:" << pOrder->InsertTime << endl;
				/////激活时间
				//std::cout << "||激活时间:" << pOrder->ActiveTime << ", ";
				/////挂起时间
				//std::cout << "挂起时间:" << pOrder->SuspendTime << ", ";
				/////最后修改时间
				//std::cout << "最后修改时间:" << pOrder->UpdateTime << ", ";
				/////撤销时间
				//std::cout << "撤销时间:" << pOrder->CancelTime << ", ";
				/////最后修改交易所交易员代码
				//std::cout << "最后修改交易所交易员代码:" << pOrder->ActiveTraderID << endl;
				/////结算会员编号
				//std::cout << "||结算会员编号:" << pOrder->ClearingPartID << ", ";
				/////序号
				//std::cout << "序号:" << pOrder->SequenceNo << ", ";
				/////前置编号
				//std::cout << "前置编号:" << pOrder->FrontID << ", ";
				/////会话编号
				//std::cout << "会话编号:" << pOrder->SessionID << ", ";
				/////用户端产品信息
				//std::cout << "用户端产品信息:" << pOrder->UserProductInfo << endl;
				/////状态信息

				//codeDst[90] = { 0 };
				//Utils::Gb2312ToUtf8(codeDst, 90, pOrder->StatusMsg, strlen(pOrder->StatusMsg)); // Gb2312ToUtf8
				//std::cout << "||状态信息:" << codeDst << ", ";
				/////用户强评标志
				//std::cout << "用户强评标志:" << pOrder->UserForceClose << ", ";
				/////操作用户代码
				//std::cout << "操作用户代码:" << pOrder->ActiveUserID << ", ";
				/////经纪公司报单编号
				//std::cout << "经纪公司报单编号:" << pOrder->BrokerOrderSeq << ", ";
				/////相关报单
				//std::cout << "相关报单:" << pOrder->RelativeOrderSysID << endl;
				/////郑商所成交数量
				//std::cout << "||郑商所成交数量:" << pOrder->ZCETotalTradedVolume << ", ";
				/////互换单标志
				//std::cout << "互换单标志:" << pOrder->IsSwapOrder << endl;
				//std::cout << "=================================================================================" << endl;

				CThostFtdcOrderField *pOrder_new = new CThostFtdcOrderField();
				this->CopyOrderInfo(pOrder_new, pOrder);

				this->l_query_order->push_back(pOrder_new);

				if (bIsLast == true) {
					this->isFirstQryOrder == false;
				}
			}
		} else {
			std::cout << "TdSpi::OnRspQryOrder()" << std::endl;
			std::cout << "\t当前账户 = " << this->current_user->getUserID() << std::endl;
			std::cout << "\t报单回报为空!" << std::endl;
			std::cout << "\t是否最后一条 = " << bIsLast << std::endl;
			
		}
		
	}
	cv.notify_one();

}

//查询保证金率
void TdSpi::QryInstrumentMarginRate() {
	CThostFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate = new CThostFtdcQryInstrumentMarginRateField();

	this->tdapi->ReqQryInstrumentMarginRate(pQryInstrumentMarginRate, this->getRequestID());
	delete pQryInstrumentMarginRate;
}

///请求查询合约保证金率响应
void TdSpi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQryInstrumentMarginRate");
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		if (pInstrumentMarginRate) {
			std::cout << "=================================================================================" << endl;
			///合约代码
			std::cout << "||合约代码:" << pInstrumentMarginRate->InstrumentID << ", ";
			///投资者范围
			std::cout << "投资者范围:" << pInstrumentMarginRate->InvestorRange << ", ";
			///经纪公司代码
			std::cout << "经纪公司代码:" << pInstrumentMarginRate->BrokerID << ", ";
			///投资者代码
			std::cout << "投资者代码:" << pInstrumentMarginRate->InvestorID << ", ";
			///投机套保标志
			std::cout << "投机套保标志:" << pInstrumentMarginRate->HedgeFlag << endl;
			///多头保证金率
			std::cout << "||多头保证金率:" << pInstrumentMarginRate->LongMarginRatioByMoney << ", ";
			///多头保证金费
			std::cout << "多头保证金费:" << pInstrumentMarginRate->LongMarginRatioByVolume << ", ";
			///空头保证金率
			std::cout << "空头保证金率:" << pInstrumentMarginRate->ShortMarginRatioByMoney << ", ";
			///空头保证金费
			std::cout << "空头保证金费:" << pInstrumentMarginRate->ShortMarginRatioByVolume << ", ";
			///是否相对交易所收取
			std::cout << "是否相对交易所收取:" << pInstrumentMarginRate->IsRelative << endl;
			std::cout << "=================================================================================" << endl;
		}
	}
}


//查询手续费
void TdSpi::QryInstrumentCommissionRate() {
	CThostFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate = new CThostFtdcQryInstrumentCommissionRateField();
	this->tdapi->ReqQryInstrumentCommissionRate(pQryInstrumentCommissionRate, this->getRequestID());
	delete pQryInstrumentCommissionRate;
}

///请求查询合约手续费率响应
void TdSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQryInstrumentCommissionRate");
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		if (pInstrumentCommissionRate) {
			///合约代码
			std::cout << "合约代码:" << pInstrumentCommissionRate->InstrumentID << endl;
			///投资者范围
			std::cout << "投资者范围:" << pInstrumentCommissionRate->InvestorRange << endl;
			///经纪公司代码
			std::cout << "经纪公司代码:" << pInstrumentCommissionRate->BrokerID << endl;
			///投资者代码
			std::cout << "投资者代码:" << pInstrumentCommissionRate->InvestorID << endl;
			///开仓手续费率
			std::cout << "开仓手续费率:" << pInstrumentCommissionRate->OpenRatioByMoney << endl;
			///开仓手续费
			std::cout << "开仓手续费:" << pInstrumentCommissionRate->OpenRatioByVolume << endl;
			///平仓手续费率
			std::cout << "平仓手续费率:" << pInstrumentCommissionRate->CloseRatioByMoney << endl;
			///平仓手续费
			std::cout << "平仓手续费:" << pInstrumentCommissionRate->CloseRatioByVolume << endl;
			///平今手续费率
			std::cout << "平今手续费率:" << pInstrumentCommissionRate->CloseTodayRatioByMoney << endl;
			///平今手续费
			std::cout << "平今手续费:" << pInstrumentCommissionRate->CloseTodayRatioByVolume << endl;
		}
	}
}

//查询投资者
void TdSpi::QryInvestor() {
	USER_PRINT("TdSpi::QryInvestor");
	CThostFtdcQryInvestorField *pQryInvestor = new CThostFtdcQryInvestorField();
	this->tdapi->ReqQryInvestor(pQryInvestor, this->getRequestID());
	delete pQryInvestor;
}



//查询投资者响应
void TdSpi::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQryInvestor");
	if (!this->IsErrorRspInfo(pRspInfo)) {
		if (pInvestor) {
			std::cout << "=================================================================================" << endl;
			///投资者代码
			std::cout << "||投资者代码:" << pInvestor->InvestorID << ", ";
			///经纪公司代码
			std::cout << "经纪公司代码:" << pInvestor->BrokerID << ", ";
			///投资者分组代码
			std::cout << "投资者分组代码:" << pInvestor->InvestorGroupID << ", ";
			///投资者名称
			std::cout << "投资者名称:" << pInvestor->InvestorName << ", ";
			///证件类型
			std::cout << "证件类型:" << pInvestor->IdentifiedCardType << endl;
			///证件号码
			std::cout << "||证件号码:" << pInvestor->IdentifiedCardNo << ", ";
			///是否活跃
			std::cout << "是否活跃:" << pInvestor->IsActive << ", ";
			///联系电话
			std::cout << "联系电话:" << pInvestor->Telephone << ", ";
			///通讯地址
			std::cout << "通讯地址:" << pInvestor->Address << ", ";
			///开户日期
			std::cout << "开户日期:" << pInvestor->OpenDate << endl;
			///手机
			std::cout << "||手机:" << pInvestor->Mobile << ", ";
			///手续费率模板代码
			std::cout << "手续费率模板代码:" << pInvestor->CommModelID << ", ";
			///保证金率模板代码
			std::cout << "保证金率模板代码:" << pInvestor->MarginModelID << endl;
			std::cout << "=================================================================================" << endl;
		}
	}
}

/// 拷贝持仓明细数据
void TdSpi::CopyPositionDetailData(CThostFtdcInvestorPositionDetailField *dst, CThostFtdcInvestorPositionDetailField *src) {
	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);
	///经纪公司代码
	strcpy(dst->BrokerID, src->BrokerID);
	///投资者代码
	strcpy(dst->InvestorID, src->InvestorID);
	///投机套保标志
	dst->HedgeFlag = src->HedgeFlag;
	///买卖
	dst->Direction = src->Direction;
	///开仓日期
	strcpy(dst->OpenDate, src->OpenDate);
	///成交编号
	strcpy(dst->TradeID, src->TradeID);
	///数量
	dst->Volume = src->Volume;
	///开仓价
	dst->OpenPrice = src->OpenPrice;
	///交易日
	strcpy(dst->TradingDay, src->TradingDay);
	///结算编号
	dst->SettlementID = src->SettlementID;
	///成交类型
	dst->TradeType = src->TradeType;
	///组合合约代码
	strcpy(dst->CombInstrumentID, src->CombInstrumentID);
	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);
	///逐日盯市平仓盈亏
	dst->CloseProfitByDate = src->CloseProfitByDate;
	///逐笔对冲平仓盈亏
	dst->CloseProfitByTrade = src->CloseProfitByTrade;
	///逐日盯市持仓盈亏
	dst->PositionProfitByDate = src->PositionProfitByDate;
	///逐笔对冲持仓盈亏
	dst->PositionProfitByTrade = src->PositionProfitByTrade;
	///投资者保证金
	dst->Margin = src->Margin;
	///交易所保证金
	dst->ExchMargin = src->ExchMargin;
	///保证金率
	dst->MarginRateByMoney = src->MarginRateByMoney;
	///保证金率(按手数)
	dst->MarginRateByVolume = src->MarginRateByVolume;
	///昨结算价
	dst->LastSettlementPrice = src->LastSettlementPrice;
	///结算价
	dst->SettlementPrice = src->SettlementPrice;
	///平仓量
	dst->CloseVolume = src->CloseVolume;
	///平仓金额
	dst->CloseAmount = src->CloseAmount;
}

//查询投资者持仓
void TdSpi::QryInvestorPosition() {
	USER_PRINT("TdSpi::QryInvestorPosition");
	CThostFtdcQryInvestorPositionField *pQryInvestorPosition = new CThostFtdcQryInvestorPositionField();
	this->tdapi->ReqQryInvestorPosition(pQryInvestorPosition, this->getRequestID());
	delete pQryInvestorPosition;
}

//查询投资者持仓明细
void TdSpi::QryInvestorPositionDetail() {
	USER_PRINT("TdSpi::QryInvestorPositionDetail");
	CThostFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail = new CThostFtdcQryInvestorPositionDetailField();
	this->tdapi->ReqQryInvestorPositionDetail(pQryInvestorPositionDetail, this->getRequestID());
	delete pQryInvestorPositionDetail;
	//等待回调
	std::unique_lock<std::mutex> lck(mtx);
	while (cv.wait_for(lck, std::chrono::seconds(RSP_TIMEOUT)) == std::cv_status::timeout) {
		std::cout << "TdSpi::QryInvestorPositionDetail()" << std::endl;
		std::cout << "\t查询持仓明细等待超时, 当前期货账户 = " << this->current_user->getUserID() << std::endl;
		return;
	}
}

///请求查询投资者持仓明细响应
void TdSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "TdSpi::OnRspQryInvestorPositionDetail() bIsLast = " << bIsLast << ", 当前期货账户 = " << this->current_user->getUserID() << " 查询投资者持仓明细 In Thread = " << std::this_thread::get_id() << std::endl;
	USER_PRINT("TdSpi::OnRspQryInvestorPositionDetail");
	if (!this->IsErrorRspInfo(pRspInfo)) {
		if (pInvestorPositionDetail) {
			cout << "=================================================================================" << endl;
			///合约代码
			cout << "||合约代码:" << pInvestorPositionDetail->InstrumentID << ", ";
			///经纪公司代码
			cout << "经纪公司代码:" << pInvestorPositionDetail->BrokerID << ", ";
			///投资者代码
			cout << "投资者代码:" << pInvestorPositionDetail->InvestorID << ", ";
			///投机套保标志
			cout << "投机套保标志:" << pInvestorPositionDetail->HedgeFlag << ", ";
			///买卖
			cout << "买卖:" << pInvestorPositionDetail->Direction << endl;
			///开仓日期
			cout << "||开仓日期:" << pInvestorPositionDetail->OpenDate << ", ";
			///成交编号
			cout << "成交编号:" << pInvestorPositionDetail->TradeID << ", ";
			///数量
			cout << "数量:" << pInvestorPositionDetail->Volume << ", ";
			///开仓价
			cout << "开仓价:" << pInvestorPositionDetail->OpenPrice << ", ";
			///交易日
			cout << "交易日:" << pInvestorPositionDetail->TradingDay << endl;
			///结算编号
			cout << "||结算编号:" << pInvestorPositionDetail->SettlementID << ", ";
			///成交类型
			cout << "成交类型:" << pInvestorPositionDetail->TradeType << ", ";
			///组合合约代码
			cout << "组合合约代码:" << pInvestorPositionDetail->CombInstrumentID << ", ";
			///交易所代码
			cout << "交易所代码:" << pInvestorPositionDetail->ExchangeID << ", ";
			///逐日盯市平仓盈亏
			cout << "逐日盯市平仓盈亏:" << pInvestorPositionDetail->CloseProfitByDate << ", ";
			///逐笔对冲平仓盈亏
			cout << "逐笔对冲平仓盈亏:" << pInvestorPositionDetail->CloseProfitByTrade << endl;
			///逐日盯市持仓盈亏
			cout << "||逐日盯市持仓盈亏:" << pInvestorPositionDetail->PositionProfitByDate << ", ";
			///逐笔对冲持仓盈亏
			cout << "逐笔对冲持仓盈亏:" << pInvestorPositionDetail->PositionProfitByTrade << ", ";
			///投资者保证金
			cout << "投资者保证金:" << pInvestorPositionDetail->Margin << ", ";
			///交易所保证金
			cout << "交易所保证金:" << pInvestorPositionDetail->ExchMargin << ", ";
			///保证金率
			cout << "保证金率:" << pInvestorPositionDetail->MarginRateByMoney << endl;
			///保证金率(按手数)
			cout << "||保证金率(按手数):" << pInvestorPositionDetail->MarginRateByVolume << ", ";
			///昨结算价
			cout << "昨结算价:" << pInvestorPositionDetail->LastSettlementPrice << ", ";
			///结算价
			cout << "结算价:" << pInvestorPositionDetail->SettlementPrice << ", ";
			///平仓量
			cout << "平仓量:" << pInvestorPositionDetail->CloseVolume << ", ";
			///平仓金额
			cout << "平仓金额:" << pInvestorPositionDetail->CloseAmount << endl;
			cout << "=================================================================================" << endl;
		
			CThostFtdcInvestorPositionDetailField *tmp = new CThostFtdcInvestorPositionDetailField();
			this->CopyPositionDetailData(tmp, pInvestorPositionDetail);

			/*将持仓明细添加到对应user统计列表里*/
			this->current_user->addL_Position_Detail_From_CTP(tmp);
		}
	}

	if (bIsLast) {
		cv.notify_one();
	}
}

//请求查询投资者持仓响应
void TdSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "TdSpi::OnRspQryInvestorPosition() bIsLast = " << bIsLast << ", 当前期货账户 = " << this->current_user->getUserID() << " 查询投资者持仓 In Thread = " << std::this_thread::get_id() << std::endl;
	USER_PRINT("TdSpi::OnRspQryInvestorPosition");
	if (!this->IsErrorRspInfo(pRspInfo)) {
		if (pInvestorPosition) {
			std::cout << "=================================================================================" << endl;
			///合约代码
			std::cout << "||合约代码:" << pInvestorPosition->InstrumentID << ", ";
			///经纪公司代码
			std::cout << "经纪公司代码:" << pInvestorPosition->BrokerID << ", ";
			///投资者代码
			std::cout << "投资者代码:" << pInvestorPosition->InvestorID << ", ";
			///持仓多空方向
			std::cout << "持仓多空方向:" << pInvestorPosition->PosiDirection << ", ";
			///投机套保标志
			std::cout << "投机套保标志:" << pInvestorPosition->HedgeFlag << endl;
			///持仓日期
			std::cout << "||持仓日期:" << pInvestorPosition->PositionDate << ", ";
			///上日持仓
			std::cout << "上日持仓:" << pInvestorPosition->YdPosition << ", ";
			///今日持仓
			std::cout << "今日持仓:" << pInvestorPosition->Position << ", ";
			///多头冻结
			std::cout << "多头冻结:" << pInvestorPosition->LongFrozen << ", ";
			///空头冻结
			std::cout << "空头冻结:" << pInvestorPosition->ShortFrozen << endl;
			///开仓冻结金额
			std::cout << "||开仓冻结金额:" << pInvestorPosition->LongFrozenAmount << ", ";
			///开仓冻结金额
			std::cout << "开仓冻结金额:" << pInvestorPosition->ShortFrozenAmount << ", ";
			///开仓量
			std::cout << "开仓量:" << pInvestorPosition->OpenVolume << ", ";
			///平仓量
			std::cout << "平仓量:" << pInvestorPosition->CloseVolume << ", ";
			///开仓金额
			std::cout << "开仓金额:" << pInvestorPosition->OpenAmount << endl;
			///平仓金额
			std::cout << "||平仓金额:" << pInvestorPosition->CloseAmount << ", ";
			///持仓成本
			std::cout << "持仓成本:" << pInvestorPosition->PositionCost << ", ";
			///上次占用的保证金
			std::cout << "上次占用的保证金:" << pInvestorPosition->PreMargin << ", ";
			///占用的保证金
			std::cout << "占用的保证金:" << pInvestorPosition->UseMargin << ", ";
			///冻结的保证金
			std::cout << "冻结的保证金:" << pInvestorPosition->FrozenMargin << endl;
			///冻结的资金
			std::cout << "||冻结的资金:" << pInvestorPosition->FrozenCash << ", ";
			///冻结的手续费
			std::cout << "冻结的手续费:" << pInvestorPosition->FrozenCommission << ", ";
			///资金差额
			std::cout << "资金差额:" << pInvestorPosition->CashIn << ", ";
			///手续费
			std::cout << "手续费:" << pInvestorPosition->Commission << ", ";
			///平仓盈亏
			std::cout << "平仓盈亏:" << pInvestorPosition->CloseProfit << endl;
			///持仓盈亏
			std::cout << "||持仓盈亏:" << pInvestorPosition->PositionProfit << ", ";
			///上次结算价
			std::cout << "上次结算价:" << pInvestorPosition->PreSettlementPrice << ", ";
			///本次结算价
			std::cout << "本次结算价:" << pInvestorPosition->SettlementPrice << ", ";
			///交易日
			std::cout << "交易日:" << pInvestorPosition->TradingDay << ", ";
			///结算编号
			std::cout << "结算编号:" << pInvestorPosition->SettlementID << endl;
			///开仓成本
			std::cout << "||开仓成本:" << pInvestorPosition->OpenCost << ", ";
			///交易所保证金
			std::cout << "交易所保证金:" << pInvestorPosition->ExchangeMargin << ", ";
			///组合成交形成的持仓
			std::cout << "组合成交形成的持仓:" << pInvestorPosition->CombPosition << ", ";
			///组合多头冻结
			std::cout << "组合多头冻结:" << pInvestorPosition->CombLongFrozen << ", ";
			///组合空头冻结
			std::cout << "组合空头冻结:" << pInvestorPosition->CombShortFrozen << endl;
			///逐日盯市平仓盈亏
			std::cout << "||逐日盯市平仓盈亏:" << pInvestorPosition->CloseProfitByDate << ", ";
			///逐笔对冲平仓盈亏
			std::cout << "逐笔对冲平仓盈亏:" << pInvestorPosition->CloseProfitByTrade << ", ";
			///今日持仓
			std::cout << "今日持仓:" << pInvestorPosition->TodayPosition << ", ";
			///保证金率
			std::cout << "保证金率:" << pInvestorPosition->MarginRateByMoney << ", ";
			///保证金率(按手数)
			std::cout << "保证金率(按手数):" << pInvestorPosition->MarginRateByVolume << endl;
			///执行冻结
			std::cout << "||执行冻结:" << pInvestorPosition->StrikeFrozen << ", ";
			///执行冻结金额
			std::cout << "执行冻结金额:" << pInvestorPosition->StrikeFrozenAmount << ", ";
			///放弃执行冻结
			std::cout << "放弃执行冻结:" << pInvestorPosition->AbandonFrozen << endl;
			std::cout << "=================================================================================" << endl;
			this->current_user->DB_OnRspQryInvestorPosition(this->current_user->GetPositionConn(), pInvestorPosition);
		}
	}
}

//查询账号资金
void TdSpi::QryTradingAccount() {
	CThostFtdcQryTradingAccountField *pQryTradingAccount = new CThostFtdcQryTradingAccountField();
	this->tdapi->ReqQryTradingAccount(pQryTradingAccount, this->getRequestID());
	delete pQryTradingAccount;
}

//查询账号资金响应
void TdSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQryTradingAccount");
	if (!this->IsErrorRspInfo(pRspInfo)) {
		if (pTradingAccount) {
			std::cout << "=================================================================================" << endl;
			///经纪公司代码
			std::cout << "||经纪公司代码:" << pTradingAccount->BrokerID << ", ";
			///投资者帐号
			std::cout << "投资者帐号:" << pTradingAccount->AccountID << ", ";
			///上次质押金额
			std::cout << "上次质押金额:" << pTradingAccount->PreMortgage << ", ";
			///上次信用额度
			std::cout << "上次信用额度:" << pTradingAccount->PreCredit << ", ";
			///上次存款额
			std::cout << "上次存款额:" << pTradingAccount->PreDeposit << endl;
			///上次结算准备金
			std::cout << "||上次结算准备金:" << pTradingAccount->PreBalance << ", ";
			///上次占用的保证金
			std::cout << "上次占用的保证金:" << pTradingAccount->PreMargin << ", ";
			///利息基数
			std::cout << "利息基数:" << pTradingAccount->InterestBase << ", ";
			///利息收入
			std::cout << "利息收入:" << pTradingAccount->Interest << ", ";
			///入金金额
			std::cout << "入金金额:" << pTradingAccount->Deposit << endl;
			///出金金额
			std::cout << "||出金金额:" << pTradingAccount->Withdraw << ", ";
			///冻结的保证金
			std::cout << "冻结的保证金:" << pTradingAccount->FrozenMargin << ", ";
			///冻结的资金
			std::cout << "冻结的资金:" << pTradingAccount->FrozenCash << ", ";
			///冻结的手续费
			std::cout << "冻结的手续费:" << pTradingAccount->FrozenCommission << ", ";
			///当前保证金总额
			std::cout << "当前保证金总额:" << pTradingAccount->CurrMargin << endl;
			///资金差额
			std::cout << "||资金差额:" << pTradingAccount->CashIn << ", ";
			///手续费
			std::cout << "手续费:" << pTradingAccount->Commission << ", ";
			///平仓盈亏
			std::cout << "平仓盈亏:" << pTradingAccount->CloseProfit << ", ";
			///持仓盈亏
			std::cout << "持仓盈亏:" << pTradingAccount->PositionProfit << ", ";
			///期货结算准备金
			std::cout << "期货结算准备金:" << pTradingAccount->Balance << endl;
			///可用资金
			std::cout << "||可用资金:" << pTradingAccount->Available << ", ";
			///可取资金
			std::cout << "可取资金:" << pTradingAccount->WithdrawQuota << ", ";
			///基本准备金
			std::cout << "基本准备金:" << pTradingAccount->Reserve << ", ";
			///交易日
			std::cout << "交易日:" << pTradingAccount->TradingDay << ", ";
			///结算编号
			std::cout << "结算编号:" << pTradingAccount->SettlementID << endl;
			///信用额度
			std::cout << "||信用额度:" << pTradingAccount->Credit << ", ";
			///质押金额
			std::cout << "质押金额:" << pTradingAccount->Mortgage << ", ";
			///交易所保证金
			std::cout << "交易所保证金:" << pTradingAccount->ExchangeMargin << ", ";
			///投资者交割保证金
			std::cout << "投资者交割保证金:" << pTradingAccount->DeliveryMargin << ", ";
			///交易所交割保证金
			std::cout << "交易所交割保证金:" << pTradingAccount->ExchangeDeliveryMargin << endl;
			///保底期货结算准备金
			std::cout << "||保底期货结算准备金:" << pTradingAccount->ReserveBalance << ", ";
			///币种代码
			std::cout << "币种代码:" << pTradingAccount->CurrencyID << ", ";
			///上次货币质入金额
			std::cout << "上次货币质入金额:" << pTradingAccount->PreFundMortgageIn << ", ";
			///上次货币质出金额
			std::cout << "上次货币质出金额:" << pTradingAccount->PreFundMortgageOut << ", ";
			///货币质入金额
			std::cout << "货币质入金额:" << pTradingAccount->FundMortgageIn << endl;
			///货币质出金额
			std::cout << "||货币质出金额:" << pTradingAccount->FundMortgageOut << ", ";
			///货币质押余额
			std::cout << "货币质押余额:" << pTradingAccount->FundMortgageAvailable << ", ";
			///可质押货币金额
			std::cout << "可质押货币金额:" << pTradingAccount->MortgageableFund << ", ";
			///特殊产品占用保证金
			std::cout << "特殊产品占用保证金:" << pTradingAccount->SpecProductMargin << ", ";
			///特殊产品冻结保证金
			std::cout << "特殊产品冻结保证金:" << pTradingAccount->SpecProductFrozenMargin << endl;
			///特殊产品手续费
			std::cout << "||特殊产品手续费:" << pTradingAccount->SpecProductCommission << ", ";
			///特殊产品冻结手续费
			std::cout << "特殊产品冻结手续费:" << pTradingAccount->SpecProductFrozenCommission << ", ";
			///特殊产品持仓盈亏
			std::cout << "特殊产品持仓盈亏:" << pTradingAccount->SpecProductPositionProfit << ", ";
			///特殊产品平仓盈亏
			std::cout << "特殊产品平仓盈亏:" << pTradingAccount->SpecProductCloseProfit << ", ";
			///根据持仓盈亏算法计算的特殊产品持仓盈亏
			std::cout << "根据持仓盈亏算法计算的特殊产品持仓盈亏:" << pTradingAccount->SpecProductPositionProfitByAlg << endl;
			///特殊产品交易所保证金
			std::cout << "||特殊产品交易所保证金:" << pTradingAccount->SpecProductExchangeMargin << endl;
			std::cout << "=================================================================================" << endl;
		}
	}
}

//查询成交单
void TdSpi::QryTrade() {
	USER_PRINT("TdSpi::QryTrade");
	CThostFtdcQryTradeField *pQryTrade = new CThostFtdcQryTradeField();
	this->tdapi->ReqQryTrade(pQryTrade, this->getRequestID());
	sleep(1);
	delete pQryTrade;
}

void TdSpi::CopyTradeInfo(CThostFtdcTradeField *dst, CThostFtdcTradeField *src) {
	///经纪公司代码
	strcpy(dst->BrokerID, src->BrokerID);
	///投资者代码
	strcpy(dst->InvestorID, src->InvestorID);
	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);
	///报单引用
	strcpy(dst->OrderRef, src->OrderRef);
	///用户代码
	strcpy(dst->UserID, src->UserID);
	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);
	///成交编号
	strcpy(dst->TradeID, src->TradeID);
	///买卖方向
	dst->Direction = src->Direction;
	///报单编号
	strcpy(dst->OrderSysID, src->OrderSysID);
	///会员代码
	strcpy(dst->ParticipantID, src->ParticipantID);
	///客户代码
	strcpy(dst->ClientID, src->ClientID);
	///交易角色
	dst->TradingRole = src->TradingRole;
	///合约在交易所的代码
	strcpy(dst->ExchangeInstID, src->ExchangeInstID);
	///开平标志
	dst->OffsetFlag = src->OffsetFlag;
	///投机套保标志
	dst->HedgeFlag = src->HedgeFlag;
	///价格
	dst->Price = src->Price;
	///数量
	dst->Volume = src->Volume;
	///成交时期
	strcpy(dst->TradeDate, src->TradeDate);
	///成交时间
	strcpy(dst->TradeTime, src->TradeTime);
	///成交类型
	dst->TradeType = src->TradeType;
	///成交价来源
	dst->PriceSource = src->PriceSource;
	///交易所交易员代码
	strcpy(dst->TraderID, src->TraderID);
	///本地报单编号
	strcpy(dst->OrderLocalID, src->OrderLocalID);
	///结算会员编号
	strcpy(dst->ClearingPartID, src->ClearingPartID);
	///业务单元
	strcpy(dst->BusinessUnit, src->BusinessUnit);
	///序号
	dst->SequenceNo = src->SequenceNo;
	///交易日
	strcpy(dst->TradingDay, src->TradingDay);
	///结算编号
	dst->SettlementID = src->SettlementID;
	///经纪公司报单编号
	dst->BrokerOrderSeq = src->BrokerOrderSeq;
	///成交来源
	dst->TradeSource = src->TradeSource;
}


//复制订单回报
void TdSpi::CopyOrderInfo(CThostFtdcOrderField *dst, CThostFtdcOrderField *src) {
	///经纪公司代码
	strcpy(dst->BrokerID, src->BrokerID);

	///投资者代码
	strcpy(dst->InvestorID, src->InvestorID);

	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);

	///报单引用
	strcpy(dst->OrderRef, src->OrderRef);

	///用户代码
	strcpy(dst->UserID, src->UserID);

	///报单价格条件
	dst->OrderPriceType = src->OrderPriceType;

	///买卖方向
	dst->Direction = src->Direction;

	///组合开平标志
	strcpy(dst->CombOffsetFlag, src->CombOffsetFlag);

	///组合投机套保标志
	strcpy(dst->CombHedgeFlag, src->CombHedgeFlag);

	///价格
	dst->LimitPrice = src->LimitPrice;

	///数量
	dst->VolumeTotalOriginal = src->VolumeTotalOriginal;

	///有效期类型
	dst->TimeCondition = src->TimeCondition;

	///GTD日期
	strcpy(dst->GTDDate, src->GTDDate);

	///成交量类型
	dst->VolumeCondition = src->VolumeCondition;

	///最小成交量
	dst->MinVolume = src->MinVolume;

	///触发条件
	dst->ContingentCondition = src->ContingentCondition;

	///止损价
	dst->StopPrice = src->StopPrice;

	///强平原因
	dst->ForceCloseReason = src->ForceCloseReason;

	///自动挂起标志
	dst->IsAutoSuspend = src->IsAutoSuspend;

	///业务单元
	strcpy(dst->BusinessUnit, src->BusinessUnit);

	///请求编号
	dst->RequestID = src->RequestID;

	///本地报单编号
	strcpy(dst->OrderLocalID, src->OrderLocalID);

	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);

	///会员代码
	strcpy(dst->ParticipantID, src->ParticipantID);

	///客户代码
	strcpy(dst->ClientID, src->ClientID);

	///合约在交易所的代码
	strcpy(dst->ExchangeInstID, src->ExchangeInstID);

	///交易所交易员代码
	strcpy(dst->TraderID, src->TraderID);

	///安装编号
	dst->InstallID = src->InstallID;

	///报单提交状态
	dst->OrderSubmitStatus = src->OrderSubmitStatus;

	///报单提示序号
	dst->NotifySequence = src->NotifySequence;

	///交易日
	strcpy(dst->TradingDay, src->TradingDay);

	///结算编号
	dst->SettlementID = src->SettlementID;

	///报单编号
	strcpy(dst->OrderSysID, src->OrderSysID);

	///报单来源
	dst->OrderSource = src->OrderSource;

	///报单状态
	dst->OrderStatus = src->OrderStatus;

	///报单类型
	dst->OrderType = src->OrderType;

	///今成交数量
	dst->VolumeTraded = src->VolumeTraded;

	///剩余数量
	dst->VolumeTotal = src->VolumeTotal;

	///报单日期
	strcpy(dst->InsertDate, src->InsertDate);

	///委托时间
	strcpy(dst->InsertTime, src->InsertTime);

	///激活时间
	strcpy(dst->ActiveTime, src->ActiveTime);

	///挂起时间
	strcpy(dst->SuspendTime, src->SuspendTime);

	///最后修改时间
	strcpy(dst->UpdateTime, src->UpdateTime);

	///撤销时间
	strcpy(dst->CancelTime, src->CancelTime);

	///最后修改交易所交易员代码
	strcpy(dst->ActiveTraderID, src->ActiveTraderID);

	///结算会员编号
	strcpy(dst->ClearingPartID, src->ClearingPartID);

	///序号
	dst->SequenceNo = src->SequenceNo;

	///前置编号
	dst->FrontID = src->FrontID;

	///会话编号
	dst->SessionID = src->SessionID;

	///用户端产品信息
	strcpy(dst->UserProductInfo, src->UserProductInfo);

	///状态信息
	strcpy(dst->StatusMsg, src->StatusMsg);

	///用户强评标志
	dst->UserForceClose = src->UserForceClose;

	///操作用户代码
	strcpy(dst->ActiveUserID, src->ActiveUserID);

	///经纪公司报单编号
	dst->BrokerOrderSeq = src->BrokerOrderSeq;

	///相关报单
	strcpy(dst->RelativeOrderSysID, src->RelativeOrderSysID);

	///郑商所成交数量
	dst->ZCETotalTradedVolume = src->ZCETotalTradedVolume;

	///互换单标志
	dst->IsSwapOrder = src->IsSwapOrder;

	///营业部编号
	strcpy(dst->BranchID, src->BranchID);

	///投资单元代码
	strcpy(dst->InvestUnitID, src->InvestUnitID);

	///资金账号
	strcpy(dst->AccountID, src->AccountID);

	///币种代码
	strcpy(dst->CurrencyID, src->CurrencyID);

	///IP地址
	strcpy(dst->IPAddress, src->IPAddress);

	///Mac地址
	strcpy(dst->MacAddress, src->MacAddress);
}

//查询成交单响应
void TdSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspQryTrade");
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		if (this->isFirstQryTrade == false) { //如果新一轮的接收,清空接收list列表
			//清空
			if (this->l_query_trade->size() > 0) {
				list<CThostFtdcTradeField *>::iterator Itor;
				for (Itor = l_query_trade->begin(); Itor != l_query_trade->end();) {
					delete (*Itor);
					Itor = l_query_trade->erase(Itor);
				}
			}

			//清空操作后置true
			this->isFirstQryTrade == true;
		}
		if (pTrade) {
			std::cout << "TdSpi.cpp 接收查询交易是否为最后 = " << bIsLast << std::endl;
			std::cout << "=================================================================================" << endl;
			///经纪公司代码
			std::cout << "||经纪公司代码:" << pTrade->BrokerID << ",";
			///投资者代码
			std::cout << "投资者代码:" << pTrade->InvestorID << ",";
			///合约代码
			std::cout << "合约代码:" << pTrade->InstrumentID << ",";
			///报单引用
			std::cout << "报单引用:" << pTrade->OrderRef << ",";
			///用户代码
			std::cout << "用户代码:" << pTrade->UserID << endl;
			///交易所代码
			std::cout << "||交易所代码:" << pTrade->ExchangeID << ",";
			///成交编号
			std::cout << "成交编号:" << pTrade->TradeID << ",";
			///买卖方向
			std::cout << "买卖方向:" << pTrade->Direction << ",";
			///报单编号
			std::cout << "报单编号:" << pTrade->OrderSysID << ",";
			///会员代码
			std::cout << "会员代码:" << pTrade->ParticipantID << endl;
			///客户代码
			std::cout << "||客户代码:" << pTrade->ClientID << ",";
			///交易角色
			std::cout << "交易角色:" << pTrade->TradingRole << ",";
			///合约在交易所的代码
			std::cout << "合约在交易所的代码:" << pTrade->ExchangeInstID << ",";
			///开平标志
			std::cout << "开平标志:" << pTrade->OffsetFlag << ",";
			///投机套保标志
			std::cout << "投机套保标志:" << pTrade->HedgeFlag << endl;
			///价格
			std::cout << "||价格:" << pTrade->Price << ",";
			///数量
			std::cout << "数量:" << pTrade->Volume << ",";
			///成交时期
			std::cout << "成交时期:" << pTrade->TradeDate << ",";
			///成交时间
			std::cout << "成交时间:" << pTrade->TradeTime << ",";
			///成交类型
			std::cout << "成交类型:" << pTrade->TradeType << endl;
			///成交价来源
			std::cout << "||成交价来源:" << pTrade->PriceSource << ",";
			///交易所交易员代码
			std::cout << "交易所交易员代码:" << pTrade->TraderID << ",";
			///本地报单编号
			std::cout << "本地报单编号:" << pTrade->OrderLocalID << ",";
			///结算会员编号
			std::cout << "结算会员编号:" << pTrade->ClearingPartID << ",";
			///业务单元
			std::cout << "业务单元:" << pTrade->BusinessUnit << endl;
			///序号
			std::cout << "||序号:" << pTrade->SequenceNo << ",";
			///交易日
			std::cout << "交易日:" << pTrade->TradingDay << ",";
			///结算编号
			std::cout << "结算编号:" << pTrade->SettlementID << ",";
			///经纪公司报单编号
			std::cout << "经纪公司报单编号:" << pTrade->BrokerOrderSeq << ",";
			///成交来源
			std::cout << "成交来源:" << pTrade->TradeSource << endl;

			CThostFtdcTradeField *pTrade_new = new CThostFtdcTradeField();
			this->CopyTradeInfo(pTrade_new, pTrade);
			

			this->l_query_trade->push_back(pTrade_new);

			std::cout << "=================================================================================" << endl;
			if (bIsLast == true) {
				this->isFirstQryTrade == false;
			}
		}
		
	}
}

//下单
void TdSpi::OrderInsert(User *user, CThostFtdcInputOrderField *pInputOrder) {
	///经纪公司代码
	strcpy(pInputOrder->BrokerID, user->getBrokerID().c_str());

	///投资者代码
	strcpy(pInputOrder->InvestorID, user->getUserID().c_str());

	///合约代码
	//string heyue = "cu1609";
	//strcpy(pInputOrder->InstrumentID, "cu1609");
	//std::cout << "instrument ID c_str()" << InstrumentID.c_str() << endl;

	//std::strcpy(pInputOrder->InstrumentID, InstrumentID);

	//memcpy(pInputOrder->InstrumentID, InstrumentID.c_str(), InstrumentID.size() + 1);

	///报单引用
	//strcpy(pInputOrder->OrderRef, OrderRef.c_str());

	///用户代码
	//strcpy(pInputOrder->UserID, this->getUserID().c_str());

	///报单价格条件
	//TThostFtdcOrderPriceTypeType	OrderPriceType; //char 任意价 '1'
	pInputOrder->OrderPriceType = THOST_FTDC_OPT_LimitPrice;

	///买卖方向
	//TThostFtdcDirectionType	Direction; //char 0买1卖
	//pInputOrder->Direction = Direction;

	///组合开平标志
	//TThostFtdcCombOffsetFlagType	CombOffsetFlag; //char s[5]
	//strcpy(pInputOrder->CombOffsetFlag, CombOffsetFlag); //组合开平标志 开0平1强平2平今3平昨4
	//pInputOrder->CombOffsetFlag[0] = CombOffsetFlag;

	///组合投机套保标志
	//TThostFtdcCombHedgeFlagType	CombHedgeFlag; //char s[5]
	//strcpy(pInputOrder->CombHedgeFlag, "1"); //"1"投机, "2"套利, "3"套保
	pInputOrder->CombHedgeFlag[0] = '1';

	///价格
	//TThostFtdcPriceType	LimitPrice; //double
	//pInputOrder->LimitPrice = Price;

	///数量
	//TThostFtdcVolumeType	VolumeTotalOriginal; //int
	//pInputOrder->VolumeTotalOriginal = Volume;

	///有效期类型
	//TThostFtdcTimeConditionType	TimeCondition; //char 当日有效：'3'
	pInputOrder->TimeCondition = THOST_FTDC_TC_GFD; //当日有效

	///GTD日期
	//TThostFtdcDateType	GTDDate; //char s[9]
	///成交量类型
	//TThostFtdcVolumeConditionType	VolumeCondition; //char 任何数量 '1'
	pInputOrder->VolumeCondition = THOST_FTDC_VC_AV; //任何数量 '1'

	///最小成交量
	//TThostFtdcVolumeType	MinVolume; //int
	pInputOrder->MinVolume = 1;

	///触发条件
	//TThostFtdcContingentConditionType	ContingentCondition; //char 立即 '1'
	pInputOrder->ContingentCondition = THOST_FTDC_CC_Immediately;

	///止损价
	//TThostFtdcPriceType	StopPrice; //double 置为0
	//pInputOrder->StopPrice = 0;

	///强平原因
	//TThostFtdcForceCloseReasonType	ForceCloseReason; //char 非强平 '0'
	pInputOrder->ForceCloseReason = THOST_FTDC_FCC_NotForceClose;

	///自动挂起标志
	//TThostFtdcBoolType	IsAutoSuspend; //bool false
	pInputOrder->IsAutoSuspend = 0;
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit; // char s[21]

	///请求编号
	//TThostFtdcRequestIDType	RequestID; //int
	//pInputOrder->RequestID = this->getRequestID();

	///用户强评标志
	//TThostFtdcBoolType	UserForceClose; //int
	pInputOrder->UserForceClose = 0;

	///互换单标志
	//TThostFtdcBoolType	IsSwapOrder; //bool false
	///交易所代码
	//TThostFtdcExchangeIDType	ExchangeID;
	///投资单元代码
	//TThostFtdcInvestUnitIDType	InvestUnitID;
	///资金账号
	//TThostFtdcAccountIDType	AccountID;
	///币种代码
	//TThostFtdcCurrencyIDType	CurrencyID;
	///交易编码
	//TThostFtdcClientIDType	ClientID;
	///IP地址
	//TThostFtdcIPAddressType	IPAddress;
	///Mac地址
	//TThostFtdcMacAddressType	MacAddress;
	//sleep(1);

	this->tdapi->ReqOrderInsert(pInputOrder, 1);
	USER_PRINT(this->current_user);
	USER_PRINT(user);
	//this->current_user->DB_OrderInsert(this->current_user->GetOrderConn(), pInputOrder);

	// 存储报单参数
	//user->DB_OrderInsert(user->GetOrderConn(), pInputOrder);

	string orderref = string(pInputOrder->OrderRef);
	USER_PRINT(pInputOrder->OrderRef);
	USER_PRINT(orderref);
	// 更新报单引用基准
	user->DB_UpdateOrderRef(orderref.substr(0, 10));
	//delete pInputOrder;
}

///报单录入请求响应
void TdSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspOrderInsert");
	if ((this->IsErrorRspInfo(pRspInfo))) {
		if (pInputOrder) {

			std::cout << "TdSpi::OnRspOrderInsert()" << endl;
			std::cout << "=================================================================================" << endl;
			///经纪公司代码
			std::cout << "经纪公司代码:" << pInputOrder->BrokerID << ", ";
			///投资者代码
			std::cout << "投资者代码:" << pInputOrder->InvestorID << ", ";
			///合约代码
			std::cout << "合约代码:" << pInputOrder->InstrumentID << ", ";
			///报单引用
			std::cout << "报单引用:" << pInputOrder->OrderRef << ", ";
			///用户代码
			std::cout << "用户代码:" << pInputOrder->UserID << endl;
			///报单价格条件
			std::cout << "报单价格条件:" << pInputOrder->OrderPriceType << ", ";
			///买卖方向
			std::cout << "买卖方向:" << pInputOrder->Direction << ", ";
			///组合开平标志
			std::cout << "组合开平标志:" << pInputOrder->CombOffsetFlag << ", ";
			///组合投机套保标志
			std::cout << "组合投机套保标志:" << pInputOrder->CombHedgeFlag << ", ";
			///价格
			std::cout << "价格:" << pInputOrder->LimitPrice << endl;
			///数量
			std::cout << "数量:" << pInputOrder->VolumeTotalOriginal << ", ";
			///有效期类型
			std::cout << "有效期类型:" << pInputOrder->TimeCondition << ", ";
			///GTD日期
			std::cout << "GTD日期:" << pInputOrder->GTDDate << ", ";
			///成交量类型
			std::cout << "成交量类型:" << pInputOrder->VolumeCondition << ", ";
			///最小成交量
			std::cout << "最小成交量:" << pInputOrder->MinVolume << endl;
			///触发条件
			std::cout << "触发条件:" << pInputOrder->ContingentCondition << ", ";
			///止损价
			std::cout << "止损价:" << pInputOrder->StopPrice << ", ";
			///强平原因
			std::cout << "强平原因:" << pInputOrder->ForceCloseReason << ", ";
			///自动挂起标志
			std::cout << "自动挂起标志:" << pInputOrder->IsAutoSuspend << ", ";
			///业务单元
			std::cout << "业务单元:" << pInputOrder->BusinessUnit << endl;
			///请求编号
			std::cout << "请求编号:" << pInputOrder->RequestID << ", ";
			///用户强评标志
			std::cout << "用户强评标志:" << pInputOrder->UserForceClose << ", ";
			///互换单标志
			std::cout << "互换单标志:" << pInputOrder->IsSwapOrder << endl;
			std::cout << "=================================================================================" << endl;
			this->current_user->DB_OnRspOrderInsert(this->current_user->GetOrderConn(), pInputOrder);

			string temp(pInputOrder->OrderRef);
			string result = temp.substr(0, 1);
			int len_order_ref = temp.length();
			string strategyid = temp.substr(len_order_ref - 2, 2);
			if (temp.length() == 12 && result == "1") {

				list<Strategy *>::iterator itor;
				for (itor = this->l_strategys->begin(); itor != this->l_strategys->end(); itor++) {
					if ((*itor)->getStgStrategyId() == strategyid) {
						(*itor)->OnRspOrderInsert(pInputOrder);
						//break;
					}
				}

			}
		}
	}
}

//下单响应
void TdSpi::OnRtnOrder(CThostFtdcOrderField *pOrder) {
	USER_PRINT("TdSpi::OnRtnOrder");
	//list<Session *>::iterator sid_itor;
	if (pOrder) {
		/*std::cout << "pOrder->SessionID = " << pOrder->SessionID << endl;
		std::cout << "this->SessionID = " << this->SessionID << endl;
		std::cout << "pOrder->FrontID = " << pOrder->FrontID << endl;
		std::cout << "this->FrontID = " << this->FrontID << endl;*/

		///// sessionid判断
		//for (sid_itor = this->current_user->getL_Sessions()->begin(); sid_itor != this->current_user->getL_Sessions()->end(); sid_itor++) {
		//	USER_PRINT((*sid_itor)->getSessionID());
		//	if (pOrder->SessionID == (*sid_itor)->getSessionID()) {
		std::cout << "TdSpi::OnRtnOrder()" << std::endl;
		//std::cout << "\t=================================================================================" << endl;
		///经纪公司代码
		//std::cout << "\t||经纪公司代码:" << pOrder->BrokerID << ", ";
		///投资者代码
		//std::cout << "\t投资者代码:" << pOrder->InvestorID << ", ";
		///合约代码
		//std::cout << "\t合约代码:" << pOrder->InstrumentID << ",       ";
		///报单引用
		//std::cout << "\t报单引用:" << pOrder->OrderRef << ", ";
		///用户代码
		//std::cout << "\t用户代码:" << pOrder->UserID << endl;
		///报单价格条件
		//std::cout << "\t||报单价格条件:" << pOrder->OrderPriceType << ",    ";
		///买卖方向
		//std::cout << "\t\t\t买卖方向:" << pOrder->Direction << ",        ";
		///组合开平标志
		//std::cout << "\t组合开平标志:" << pOrder->CombOffsetFlag << ",        ";
		///组合投机套保标志
		//std::cout << "\t组合投机套保标志:" << pOrder->CombHedgeFlag << ",    ";
		///价格
		//std::cout << "\t价格:" << pOrder->LimitPrice << endl;
		///数量
		//std::cout << "\t||数量:" << pOrder->VolumeTotalOriginal << ",            ";
		///有效期类型
		//std::cout << "\t有效期类型:" << pOrder->TimeCondition << ", ";
		///GTD日期
		//std::cout << "\tGTD日期:" << pOrder->GTDDate << ", ";
		///成交量类型
		//std::cout << "\t成交量类型:" << pOrder->VolumeCondition << ", ";
		///最小成交量
		//std::cout << "\t最小成交量:" << pOrder->MinVolume << endl;
		///触发条件
		//std::cout << "\t||触发条件:" << pOrder->ContingentCondition << ", ";
		///止损价
		//std::cout << "\t止损价:" << pOrder->StopPrice << ", ";
		///强平原因
		//std::cout << "\t强平原因:" << pOrder->ForceCloseReason << ", ";
		///自动挂起标志
		//std::cout << "\t自动挂起标志:" << pOrder->IsAutoSuspend << ", ";
		///业务单元
		//std::cout << "\t业务单元:" << pOrder->BusinessUnit << endl;
		///请求编号
		//std::cout << "\t||请求编号:" << pOrder->RequestID << ", ";
		///本地报单编号
		//std::cout << "\t本地报单编号:" << pOrder->OrderLocalID << ", ";
		///交易所代码
		//std::cout << "\t交易所代码:" << pOrder->ExchangeID << ", ";
		///会员代码
		//std::cout << "\t会员代码:" << pOrder->ParticipantID << ", ";
		///客户代码
		//std::cout << "\t客户代码:" << pOrder->ClientID << endl;
		///合约在交易所的代码
		//std::cout << "\t||合约在交易所的代码:" << pOrder->ExchangeInstID << ", ";
		///交易所交易员代码
		//std::cout << "\t交易所交易员代码:" << pOrder->TraderID << ", ";
		///安装编号
		//std::cout << "\t安装编号:" << pOrder->InstallID << ", ";
		///报单提交状态
		//std::cout << "\t报单提交状态:" << pOrder->OrderSubmitStatus << ", ";
		///报单提示序号
		//std::cout << "\t报单提示序号:" << pOrder->NotifySequence << endl;
		///交易日
		//std::cout << "\t\t\t交易日:" << pOrder->TradingDay << ",   ";
		///结算编号
		//std::cout << "\t结算编号:" << pOrder->SettlementID << ", ";
		///报单编号
		//std::cout << "\t\t报单编号:" << pOrder->OrderSysID << ", ";
		/////报单来源
		//std::cout << "\t报单来源:" << pOrder->OrderSource << ", ";
		/////报单状态
		//std::cout << "\t报单状态:" << pOrder->OrderStatus << endl;
		/////报单类型
		//std::cout << "\t||报单类型:" << pOrder->OrderType << ", ";
		/////今成交数量
		//std::cout << "\t今成交数量:" << pOrder->VolumeTraded << ", ";
		/////剩余数量
		//std::cout << "\t剩余数量:" << pOrder->VolumeTotal << ", ";
		/////报单日期
		//std::cout << "\t报单日期:" << pOrder->InsertDate << ", ";
		/////委托时间
		//std::cout << "\t委托时间:" << pOrder->InsertTime << endl;
		/////激活时间
		//std::cout << "\t||激活时间:" << pOrder->ActiveTime << ", ";
		/////挂起时间
		//std::cout << "\t挂起时间:" << pOrder->SuspendTime << ", ";
		/////最后修改时间
		//std::cout << "\t最后修改时间:" << pOrder->UpdateTime << ", ";
		/////撤销时间
		//std::cout << "\t撤销时间:" << pOrder->CancelTime << ", ";
		/////最后修改交易所交易员代码
		//std::cout << "\t最后修改交易所交易员代码:" << pOrder->ActiveTraderID << endl;
		/////结算会员编号
		//std::cout << "\t||结算会员编号:" << pOrder->ClearingPartID << ", ";
		/////序号
		//std::cout << "\t序号:" << pOrder->SequenceNo << ", ";
		/////前置编号
		//std::cout << "\t前置编号:" << pOrder->FrontID << ", ";
		/////会话编号
		//std::cout << "\t会话编号:" << pOrder->SessionID << ", ";
		/////用户端产品信息
		//std::cout << "\t用户端产品信息:" << pOrder->UserProductInfo << endl;
		/////状态信息
		//codeDst[90] = { 0 };
		//Utils::Gb2312ToUtf8(codeDst, 90, pOrder->StatusMsg, strlen(pOrder->StatusMsg)); // Gb2312ToUtf8
		//std::cout << "\t||状态信息:" << codeDst << ", ";
		/////用户强评标志
		//std::cout << "\t用户强评标志:" << pOrder->UserForceClose << ", ";
		/////操作用户代码
		//std::cout << "\t操作用户代码:" << pOrder->ActiveUserID << ", ";
		/////经纪公司报单编号
		//std::cout << "\t经纪公司报单编号:" << pOrder->BrokerOrderSeq << ", ";
		/////相关报单
		//std::cout << "\t相关报单:" << pOrder->RelativeOrderSysID << endl;
		/////郑商所成交数量
		//std::cout << "\t||郑商所成交数量:" << pOrder->ZCETotalTradedVolume << ", ";
		/////互换单标志
		//std::cout << "\t互换单标志:" << pOrder->IsSwapOrder << endl;
		//std::cout << "\t=================================================================================" << endl;

		//		this->current_user->DB_OnRtnOrder(this->current_user->GetOrderConn(), pOrder);
		//		//delete[] codeDst;

		//		list<Strategy *>::iterator itor;
		//		for (itor = this->l_strategys->begin(); itor != this->l_strategys->end(); itor++) {
		//			(*itor)->OnRtnOrder(pOrder);
		//		}
		//	}
		//}

		string temp(pOrder->OrderRef);
		std::cout << "\t报单引用 = " << temp << std::endl;
		int len_order_ref = temp.length();
		
		string result = temp.substr(0, 1);
		//std::cout << "\tafter substr temp = " << temp << std::endl;
		string strategyid = "";
		
		if (len_order_ref == 12 && result == "1") { // 通过本交易系统发出去的order长度12,首位字符为1

			this->current_user->DB_OnRtnOrder(this->current_user->GetOrderConn(), pOrder);
			//delete[] codeDst;
			strategyid = temp.substr(len_order_ref - 2, 2);
			std::cout << "\t策略编号 = " << strategyid << std::endl;
			list<Strategy *>::iterator itor;
			for (itor = this->l_strategys->begin(); itor != this->l_strategys->end(); itor++) {
				if ((*itor)->getStgStrategyId() == strategyid) {
					(*itor)->OnRtnOrder(pOrder);
					//break;
				}
			}
		}

	} else {
		USER_PRINT("OnRtnOrder no pOrder");
	}
}

//成交通知
void TdSpi::OnRtnTrade(CThostFtdcTradeField *pTrade) {
	USER_PRINT("TdSpi::OnRtnTrade");
	if (pTrade) {
		std::cout << "TdSpi::OnRtnTrade()" << std::endl;
		//std::cout << "===========================TdSpi::OnRtnTrade()===================================" << std::endl;
		/////经纪公司代码
		//cout << "||经纪公司代码:" << pTrade->BrokerID << ", ";
		/////投资者代码
		//cout << "投资者代码:" << pTrade->InvestorID << ", ";
		/////合约代码
		//cout << "合约代码:" << pTrade->InstrumentID << ", ";
		/////报单引用
		//cout << "报单引用:" << pTrade->OrderRef << ", ";
		/////用户代码
		//cout << "用户代码:" << pTrade->UserID << endl;
		/////交易所代码
		//cout << "||交易所代码:" << pTrade->ExchangeID << ", ";
		/////成交编号
		//cout << "成交编号:" << pTrade->TradeID << ", ";
		/////买卖方向
		//cout << "买卖方向:" << pTrade->Direction << ", ";
		/////报单编号
		//cout << "报单编号:" << pTrade->OrderSysID << ", ";
		/////会员代码
		//cout << "会员代码:" << pTrade->ParticipantID << endl;
		/////客户代码
		//cout << "||客户代码:" << pTrade->ClientID << ", ";
		/////交易角色
		//cout << "交易角色:" << pTrade->TradingRole << ", ";
		/////合约在交易所的代码
		//cout << "合约在交易所的代码:" << pTrade->ExchangeInstID << ", ";
		/////开平标志
		//cout << "开平标志:" << pTrade->OffsetFlag << ", ";
		/////投机套保标志
		//cout << "投机套保标志:" << pTrade->HedgeFlag << endl;
		/////价格
		//cout << "||价格:" << pTrade->Price << ", ";
		/////数量(本次成交数量，特指本次交易)
		//cout << "数量:" << pTrade->Volume << ", "; //本批成交量
		/////成交时期
		//cout << "成交时期:" << pTrade->TradeDate << ", ";
		/////成交时间
		//cout << "成交时间:" << pTrade->TradeTime << ", ";
		/////成交类型
		//cout << "成交类型:" << pTrade->TradeType << endl;
		/////成交价来源
		//cout << "||成交价来源:" << pTrade->PriceSource << ", ";
		/////交易所交易员代码
		//cout << "交易所交易员代码:" << pTrade->TraderID << ", ";
		/////本地报单编号
		//cout << "本地报单编号:" << pTrade->OrderLocalID << ", ";
		/////结算会员编号
		//cout << "结算会员编号:" << pTrade->ClearingPartID << endl;
		/////业务单元
		//cout << "||业务单元:" << pTrade->BusinessUnit << ", ";
		/////序号
		//cout << "序号:" << pTrade->SequenceNo << ", ";
		/////交易日
		//cout << "交易日:" << pTrade->TradingDay << ", ";
		/////结算编号
		//cout << "结算编号:" << pTrade->SettlementID << endl;
		/////经纪公司报单编号
		//cout << "||经纪公司报单编号:" << pTrade->BrokerOrderSeq << ", ";
		/////成交来源
		//cout << "成交来源:" << pTrade->TradeSource << endl;
		//cout << "=================================================================================" << endl;

		//this->current_user->DB_OnRtnTrade(this->current_user->GetTradeConn(), pTrade);

		
		string temp(pTrade->OrderRef);
		std::cout << "\t报单引用 = " << temp << std::endl;
		int len_order_ref = temp.length();
		//std::cout << "\t报单引用长度 = " << len_order_ref << std::endl;

		string result = temp.substr(0, 1);
		//std::cout << "\tafter substr temp = " << temp << std::endl;
		string strategyid = "";

		if (len_order_ref == 12 && result == "1") { // 通过本交易系统发出去的order长度12,首位字符为1

			//this->current_user->DB_OnRtnOrder(this->current_user->GetOrderConn(), pOrder);
			//delete[] codeDst;
			strategyid = temp.substr(len_order_ref - 2, 2);
			std::cout << "\t策略编号 = " << strategyid << std::endl;
			list<Strategy *>::iterator itor;
			for (itor = this->l_strategys->begin(); itor != this->l_strategys->end(); itor++) {
				if ((*itor)->getStgStrategyId() == strategyid) {
					(*itor)->OnRtnTrade(pTrade);
					//break;
				}
			}
		}
	}
}

//下单错误响应
void TdSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) {
	USER_PRINT("TdSpi::OnErrRtnOrderInsert");
	if ((this->IsErrorRspInfo(pRspInfo))) {
		if (pInputOrder) {
			std::cout << "TdSpi::OnErrRtnOrderInsert()" << endl;
			this->current_user->DB_OnErrRtnOrderInsert(this->current_user->GetOrderConn(), pInputOrder);
			string temp(pInputOrder->OrderRef);
			string result = temp.substr(0, 1);
			int len_order_ref = temp.length();
			string strategyid = temp.substr(len_order_ref - 2, 2);
			if (temp.length() == 12 && result == "1") {

				list<Strategy *>::iterator itor;
				for (itor = this->l_strategys->begin(); itor != this->l_strategys->end(); itor++) {
					if ((*itor)->getStgStrategyId() == strategyid) {
						(*itor)->OnRspOrderInsert(pInputOrder);
						//break;
					}
				}

			}
		}
	}
}

//撤单
void TdSpi::OrderAction(char *ExchangeID, char *OrderRef, char *OrderSysID) {
	CThostFtdcInputOrderActionField *pOrderAction = new CThostFtdcInputOrderActionField();
	strcpy(pOrderAction->BrokerID, this->getBrokerID().c_str());
	strcpy(pOrderAction->InvestorID, this->getUserID().c_str());
	//strcpy(f->InstrumentID, "a1501");
	strcpy(pOrderAction->ExchangeID, ExchangeID);
	strcpy(pOrderAction->OrderRef, OrderRef);		//设置报单引用
	strcpy(pOrderAction->OrderSysID, OrderSysID);
	pOrderAction->ActionFlag = THOST_FTDC_AF_Delete; //删除
	this->tdapi->ReqOrderAction(pOrderAction, this->getRequestID());
	this->current_user->DB_OrderAction(this->current_user->GetOrderConn(), pOrderAction);
	delete pOrderAction;
}

//撤单错误响应
void TdSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("TdSpi::OnRspOrderAction");
	if ((this->IsErrorRspInfo(pRspInfo))) {
		if (pInputOrderAction) {
			this->current_user->DB_OnRspOrderAction(this->current_user->GetOrderConn(), pInputOrderAction);

			string temp(pInputOrderAction->OrderRef);
			string result = temp.substr(0, 1);
			int len_order_ref = temp.length();
			string strategyid = temp.substr(len_order_ref - 2, 2);

			if (temp.length() == 12 && result == "1") { // 通过本交易系统发出去的order长度12,首位字符为1
				//this->current_user->DB_OnRtnOrder(this->current_user->GetOrderConn(), pInputOrderAction);
				//delete[] codeDst;

				list<Strategy *>::iterator itor;
				for (itor = this->l_strategys->begin(); itor != this->l_strategys->end(); itor++) {
					if ((*itor)->getStgStrategyId() == strategyid) {
						(*itor)->OnRspOrderAction(pInputOrderAction);
						//break;
					}
				}
			}
		}
	}
}

//撤单错误
void TdSpi::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) {
	USER_PRINT("TdSpi::OnErrRtnOrderAction");
	if ((this->IsErrorRspInfo(pRspInfo))) {
		if (pOrderAction) {
			this->current_user->DB_OnErrRtnOrderAction(this->current_user->GetOrderConn(), pOrderAction);


			string temp(pOrderAction->OrderRef);
			string result = temp.substr(0, 1);
			int len_order_ref = temp.length();
			string strategyid = temp.substr(len_order_ref - 2, 2);

			if (temp.length() == 12 && result == "1") { // 通过本交易系统发出去的order长度12,首位字符为1
				//this->current_user->DB_OnRtnOrder(this->current_user->GetOrderConn(), pInputOrderAction);
				//delete[] codeDst;

				list<Strategy *>::iterator itor;
				for (itor = this->l_strategys->begin(); itor != this->l_strategys->end(); itor++) {
					if ((*itor)->getStgStrategyId() == strategyid) {
						(*itor)->OnErrRtnOrderAction(pOrderAction);
						//break;
					}
				}
			}
		}
	}
}


//登出
void TdSpi::Logout(char *BrokerID, char *UserID) {
	USER_PRINT("TdSpi::Logout");
	CThostFtdcUserLogoutField *pUserLogout = new CThostFtdcUserLogoutField();
	strcpy(pUserLogout->BrokerID, BrokerID);
	strcpy(pUserLogout->UserID, UserID);
	this->tdapi->ReqUserLogout(pUserLogout, this->loginRequestID);
	int ret = this->controlTimeOut(&logout_sem);
	if (ret == -1) {
		USER_PRINT("TdSpi::Logout TimeOut!")
	}
}

///登出请求响应
void TdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
                            CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (bIsLast && !(this->IsErrorRspInfo(pRspInfo))) {
		USER_PRINT("TdSpi::OnRspUserLogout")
		sem_post(&logout_sem);
	}
}

//等待线程结束
void TdSpi::Join() {
	USER_PRINT("TdSpi::Join()");
	this->tdapi->Join();
}



///用户口令更新请求响应
void TdSpi::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate,
                                    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    std::cout << "回调用户口令更新请求响应OnRspUserPasswordUpdate" << endl;
    if (pRspInfo->ErrorID == 0){
        std::cout << "更改成功 " << endl
        << "旧密码为:" << pUserPasswordUpdate->OldPassword << endl
        << "新密码为:" << pUserPasswordUpdate->NewPassword << endl;
    }
    else{
        std::cout << pRspInfo->ErrorID << ends << pRspInfo->ErrorMsg << endl;
    }
}

bool TdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult) {
		codeDst[90] = { 0 };
		Utils::Gb2312ToUtf8(codeDst, 90, pRspInfo->ErrorMsg, strlen(pRspInfo->ErrorMsg)); // Gb2312ToUtf8
		cerr << "TdSpi::IsErrorRspInfo() ErrorID = " << pRspInfo->ErrorID << ", ErrorMsg=" << codeDst << endl;
	}
	return bResult;
}

//得到BrokerID
string TdSpi::getBrokerID() {
	return this->BrokerID;
}

char * TdSpi::getCharBrokerID() {

	return this->c_BrokerID;
}


//得到UserID
string TdSpi::getUserID() {
	return this->UserID;
}

char * TdSpi::getCharUserID() {
	return this->c_UserID;
}

//得到Password
string TdSpi::getPassword() {
	return this->Password;
}

char * TdSpi::getCharPassword() {
	return this->c_Password;
}

//得到requestID
int TdSpi::getRequestID() {
	return this->loginRequestID;
}

//得到交易日期
char * TdSpi::getCharTradingDate() {
	return (const_cast<char *> (this->tdapi->GetTradingDay()));
}

//设置isConfirmSettlement
void TdSpi::setIsConfirmSettlement(bool isConfirmSettlement) {
	this->isConfirmSettlement = isConfirmSettlement;
}

//得到isConfirmSettlement
bool TdSpi::getIsConfirmSettlement() {
	return this->isConfirmSettlement;
}

//frontid
void TdSpi::setFrontID(int FrontID) {
	this->FrontID = FrontID;
}

int TdSpi::getFrontID() {
	return this->FrontID;
}

//sessionid
void TdSpi::setSessionID(int SessionID) {
	this->SessionID = SessionID;
}

int TdSpi::getSessionID() {
	return this->SessionID;
}

/// 得到strategy_list
list<Strategy *> * TdSpi::getListStrategy() {
	return this->l_strategys;
}

/// 设置strategy_list
void TdSpi::setListStrategy(list<Strategy *> *l_strategys) {
	this->l_strategys = l_strategys;
}

list<CThostFtdcInstrumentField *> * TdSpi::getL_Instruments_Info() {
	return this->l_instruments_info;
}

void TdSpi::setL_Instruments_Info(list<CThostFtdcInstrumentField *> *l_instruments_info) {
	this->l_instruments_info = l_instruments_info;
}

string TdSpi::getTradingDay() {
	string str_day;
	if (this->tdapi) {
		str_day = this->tdapi->GetTradingDay();
		
	}
	else
	{
		str_day = "";
	}
	return str_day;
}

list<CThostFtdcTradeField *> * TdSpi::getL_query_trade() {
	return this->l_query_trade;
}

list<CThostFtdcOrderField *> * TdSpi::getL_query_order() {
	return this->l_query_order;
}

void TdSpi::setCtpManager(CTP_Manager *ctp_m) {
	this->ctp_m = ctp_m;
}

CTP_Manager * TdSpi::getCtpManager() {
	return this->ctp_m;
}

//释放api
void TdSpi::Release() {
	USER_PRINT("TdSpi::Release");
	//this->tdapi->Release();
	if (this->tdapi) {
		this->tdapi->RegisterSpi(NULL);
		this->tdapi->Release();
		this->tdapi = NULL;
	}
}
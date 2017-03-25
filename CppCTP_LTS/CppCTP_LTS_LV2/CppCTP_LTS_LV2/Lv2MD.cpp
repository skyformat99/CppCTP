#include <iostream>
#include <string>
#include "Lv2MD.h"
using namespace std;
_LTS_NS_BEGIN_

Lv2MD::Lv2MD() {

}

Lv2MD::Lv2MD(char *front_address, char *broker_id, char *user_id, char *password, const char *pszFlowPath) {
	
	std::cout<<"Lv2MD::Lv2MD()"<<endl;
	plv2MD = CSecurityFtdcL2MDUserApi::CreateFtdcL2MDUserApi();

	strcpy_s(LoginInfo.BrokerID, broker_id);
	strcpy_s(LoginInfo.UserID, user_id);
	strcpy_s(LoginInfo.Password, password);

	plv2MD->RegisterSpi(this);
	plv2MD->RegisterFront(front_address);
	plv2MD->Init();
}

Lv2MD::~Lv2MD()
{
	plv2MD->Release();
}

//行情初始化回调
void Lv2MD::OnFrontConnected() {
	std::cout << "Lv2MD::OnFrontConnected!" << std::endl;
	this->Login();
}

//登陆
void Lv2MD::Login() {
	std::cout << "Lv2MD::Login()" << std::endl;
	int ReqCode = 0;
	ReqCode = plv2MD->ReqUserLogin(&LoginInfo, 1);
	// 请求出错
	if (ReqCode != 0) {
		std::cout << "\tMdClass::Login() ReqUserLogin Failed!" << std::endl;
	}
}

///订阅行情
void Lv2MD::SubMarket(list<string> list_instrumentid, char* pExchageID) {
	std::cout << "MdClass::SubMarketData()" << std::endl;
	list<string>::iterator itor;
	char **instrumentID = new char *[list_instrumentid.size()];
	int size = list_instrumentid.size();
	int i = 0;
	const char *charResult;
	for (itor = list_instrumentid.begin(), i = 0; itor != list_instrumentid.end(); itor++, i++) {
		charResult = (*itor).c_str();
		instrumentID[i] = new char[strlen(charResult) + 1];
		strcpy(instrumentID[i], charResult);
	}
	this->plv2MD->SubscribeL2MarketData(instrumentID, size, pExchageID);
}

///取消订阅行情
void Lv2MD::UnSubMarketData(list<string> list_instrumentid, char* pExchageID) {
	std::cout << "MdClass::SubMarketData()" << std::endl;
	list<string>::iterator itor;
	char **instrumentID = new char *[list_instrumentid.size()];
	int size = list_instrumentid.size();
	int i = 0;
	const char *charResult;
	for (itor = list_instrumentid.begin(), i = 0; itor != list_instrumentid.end(); itor++, i++) {
		charResult = (*itor).c_str();
		instrumentID[i] = new char[strlen(charResult) + 1];
		strcpy(instrumentID[i], charResult);
	}
	this->plv2MD->UnSubscribeL2MarketData(instrumentID, size, pExchageID);
}

void Lv2MD::OnFrontDisconnected(int nReason) {
	std::cout << "Lv2MD::OnFrontDisconnected()" << std::endl;
}
		
///心跳超时警告。当长时间未收到报文时，该方法被调用。
///@param nTimeLapse 距离上次接收报文的时间
void Lv2MD::OnHeartBeatWarning(int nTimeLapse) {
	std::cout << "Lv2MD::OnHeartBeatWarning()" << std::endl;
}
	 
///错误应答
void Lv2MD::OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
}

///登录请求响应
void Lv2MD::OnRspUserLogin(CSecurityFtdcUserLoginField *pUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "Lv2MD::OnRspUserLogin()" << std::endl;
	if (!IsErrorRspInfo(pRspInfo) && pUserLogin) {
		std::cout << "\tLv2MD::OnRspUserLogin() Success!" << std::endl;
	}
	else {
		std::cout << "\tLv2MD::OnRspUserLogin() Failed!" << std::endl;
	}
}

///登出请求响应
void Lv2MD::OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "Lv2MD::OnRspUserLogout()" << std::endl;
}

///订阅Level2行情应答
void Lv2MD::OnRspSubL2MarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "Lv2MD::OnRspSubL2MarketData()" << std::endl;
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument) {
		std::cout << "\t[" << pSpecificInstrument->InstrumentID << "] 订阅 成功!" << std::endl;
	}
}

///取消订阅Level2行情应答
void Lv2MD::OnRspUnSubL2MarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "MdClass::OnRspUnSubL2MarketData()" << std::endl;
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument) {
		std::cout << "\t[" << pSpecificInstrument->InstrumentID << "] 取消订阅 成功!" << std::endl;
	}
}

///订阅Level2指数行情应答
void Lv2MD::OnRspSubL2Index(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "Lv2MD::OnRspSubL2Index()" << std::endl;
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument) {
		std::cout << "\t[" << pSpecificInstrument->InstrumentID << "] 订阅 成功!" << std::endl;
	}
}

///取消订阅Level2指数行情应答
void Lv2MD::OnRspUnSubL2Index(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "MdClass::OnRspUnSubL2Index()" << std::endl;
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument) {
		std::cout << "\t[" << pSpecificInstrument->InstrumentID << "] 取消订阅 成功!" << std::endl;
	}
}

///Level2行情通知
void Lv2MD::OnRtnL2MarketData(CSecurityFtdcL2MarketDataField *pL2MarketData) {
	std::cout << "Lv2MD::OnRtnL2MarketData()" << std::endl;
	if (pL2MarketData) {
		///交易日
		std::cout << "\t交易日 = " << pL2MarketData->TradingDay << std::endl;
		///时间戳
		std::cout << "\t时间戳 = " << pL2MarketData->TimeStamp << std::endl;
		///交易所代码
		std::cout << "\t交易所代码 = " << pL2MarketData->ExchangeID << std::endl;
		///合约代码
		std::cout << "\t合约代码 = " << pL2MarketData->InstrumentID << std::endl;
		
	}
}

///Level2指数行情通知
void Lv2MD::OnRtnL2Index(CSecurityFtdcL2IndexField *pL2Index) {
	std::cout << "Lv2MD::OnRtnL2Index()" << std::endl;
	if (pL2Index) {
		///交易日
		std::cout << "\t交易日 = " << pL2Index->TradingDay << std::endl;
		///行情时间（秒）
		std::cout << "\t行情时间（秒） = " << pL2Index->TimeStamp << std::endl;
		///交易所代码
		std::cout << "\t交易所代码 = " << pL2Index->ExchangeID << std::endl;
		///指数代码
		std::cout << "\t指数代码 = " << pL2Index->InstrumentID << std::endl;
	}
}

///订阅逐笔委托及成交应答
void Lv2MD::OnRspSubL2OrderAndTrade(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

}

///取消订阅逐笔委托及成交应答
void Lv2MD::OnRspUnSubL2OrderAndTrade(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	
}

///Level2委托通知
void Lv2MD::OnRtnL2Order(CSecurityFtdcL2OrderField *pL2Order) {
	
}

///Level2成交通知
void Lv2MD::OnRtnL2Trade(CSecurityFtdcL2TradeField *pL2Trade) {

}

///通知清理SSE买卖一队列中数量为0的报单
void Lv2MD::OnNtfCheckOrderList(TSecurityFtdcInstrumentIDType InstrumentID, TSecurityFtdcFunctionCodeType FunctionCode) {
	
}

//返回数据是否报错
bool Lv2MD::IsErrorRspInfo(CSecurityFtdcRspInfoField *pRspInfo) {
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult) {
		cerr << "\t--->>> ErrorID = " << pRspInfo->ErrorID << ", ErrorMsg = " << pRspInfo->ErrorMsg << endl;
	}
	return bResult;
}
_LTS_NS_END_
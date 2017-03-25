
#include <string>
#include <iostream>
#include "Include/MdClass.h"
using namespace std;
using std::cout;
using std::endl;

MdClass::MdClass(char *front_address, char *broker_id, char *user_id, char *password, const char *pszFlowPath)
{
	//行情API初始化
	pMD = CSecurityFtdcMdApi::CreateFtdcMdApi(pszFlowPath);

	//创建行情api
	pMD->RegisterSpi(this);

	//注册行情接口
	pMD->RegisterFront(front_address);

	//拷贝登陆信息
	strcpy_s(LoginInfo.BrokerID, broker_id);
	strcpy_s(LoginInfo.UserID, user_id);
	strcpy_s(LoginInfo.Password, password);

	//cout<<"注册行情前置机地址:"<<FRONT_ADDR_MD<<endl;
	pMD->Init();
}


void MdClass::OnFrontConnected()
{
	std::cout << "MdClass::OnFrontConnected!" << std::endl;
	this->Login();
}

void MdClass::OnFrontDisconnected(int nReason)
{
	std::cout << "MdClass::OnFrontDisconnected()" << endl;
}

//登陆
void MdClass::Login() {
	std::cout << "MdClass::Login()" << std::endl;
	int ReqCode = 0;
	ReqCode = pMD->ReqUserLogin(&LoginInfo, 1);
	// 请求出错
	if (ReqCode != 0) {
		std::cout << "\tMdClass::Login() ReqUserLogin Failed!" << std::endl;
	}
}

//心跳
void MdClass::OnHeartBeatWarning(int nTimeLapse) {

}

//订阅行情
void MdClass::SubMarketData(list<string> list_instrumentid, char* pExchageID) {
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
	this->pMD->SubscribeMarketData(instrumentID, size, pExchageID);
}

//取消订阅行情
void MdClass::UnSubMarketData(list<string> list_instrumentid, char* pExchageID) {
	std::cout << "MdClass::UnSubMarketData()" << std::endl;
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
	this->pMD->UnSubscribeMarketData(instrumentID, size, pExchageID);
}

void MdClass::OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "MdClass::OnRspError()" << std::endl;
}

// 登陆回调
void MdClass::OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "MdClass::OnRspUserLogin()" << std::endl;
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {
		std::cout << "\tMdClass::OnRspUserLogin() Success!" << std::endl;
	}
	else {
		std::cout << "\tMdClass::OnRspUserLogin() Failed!" << std::endl;
	}
}


void MdClass::OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "MdClass::OnRspUserLogout()" << std::endl;
}

void MdClass::OnRspSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "MdClass::OnRspSubMarketData()" << std::endl;
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument) {
		std::cout << "\t[" << pSpecificInstrument->InstrumentID << "] 订阅 成功!" << std::endl;
	}
}

void MdClass::OnRspUnSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "MdClass::OnRspUnSubMarketData()" << std::endl;
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument) {
		std::cout << "\t[" << pSpecificInstrument->InstrumentID << "] 取消订阅 成功!" << std::endl;
	}
}




void MdClass::OnRtnDepthMarketData(CSecurityFtdcDepthMarketDataField *pDepthMarketData)
{
	std::cout << "行情信息:" << std::endl;
	if (pDepthMarketData != NULL) {
		///交易日
		std::cout << "\t交易日 = " << pDepthMarketData->TradingDay << std::endl;
		///合约代码
		std::cout << "\t合约代码 = " << pDepthMarketData->InstrumentID << std::endl;
		///交易所代码
		std::cout << "\t交易所代码 = " << pDepthMarketData->ExchangeID << std::endl;
		///合约在交易所的代码
		std::cout << "\t合约在交易所的代码 = " << pDepthMarketData->ExchangeInstID << std::endl;
		///最新价
		std::cout << "\t最新价 = " << pDepthMarketData->LastPrice << std::endl;
		//最后修改时间
		std::cout << "\t最后修改时间 = " << pDepthMarketData->UpdateTime << std::endl;
		///最后修改毫秒
		std::cout << "\t最后修改毫秒 = " << pDepthMarketData->UpdateMillisec << std::endl;
	}
}

//返回数据是否报错
bool MdClass::IsErrorRspInfo(CSecurityFtdcRspInfoField *pRspInfo) {
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult) {
		cerr << "\t--->>> ErrorID = " << pRspInfo->ErrorID << ", ErrorMsg = " << pRspInfo->ErrorMsg << endl;
	}
	return bResult;
}

MdClass::~MdClass() {
	pMD->Release();
}
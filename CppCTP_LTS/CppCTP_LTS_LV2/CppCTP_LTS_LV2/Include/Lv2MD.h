#include "SecurityFtdcL2MDUserApi.h"
#include <list>
using namespace std;
using std::list;

_LTS_NS_BEGIN_
class Lv2MD:public CSecurityFtdcL2MDUserSpi
{
public:
	Lv2MD();

	Lv2MD(char *front_address, char *broker_id, char *user_id, char *password, const char *pszFlowPath = "");

	void Login();

	///订阅行情
	void SubMarket(list<string> list_instrumentid, char* pExchageID);

	///取消订阅行情
	void UnSubMarketData(list<string> list_instrumentid, char* pExchageID);

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void OnFrontConnected();
	
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	void OnFrontDisconnected(int nReason);
		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	void OnHeartBeatWarning(int nTimeLapse);
	 
	 ///错误应答
	void OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///登录请求响应
	void OnRspUserLogin(CSecurityFtdcUserLoginField *pUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///登出请求响应
	void OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///订阅Level2行情应答
	void OnRspSubL2MarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///取消订阅Level2行情应答
	void OnRspUnSubL2MarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///订阅Level2指数行情应答
	void OnRspSubL2Index(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///取消订阅Level2指数行情应答
	void OnRspUnSubL2Index(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Level2行情通知
	void OnRtnL2MarketData(CSecurityFtdcL2MarketDataField *pL2MarketData);

	///Level2指数行情通知
	void OnRtnL2Index(CSecurityFtdcL2IndexField *pL2Index);

	///订阅逐笔委托及成交应答
	void OnRspSubL2OrderAndTrade(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///取消订阅逐笔委托及成交应答
	void OnRspUnSubL2OrderAndTrade(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Level2委托通知
	void OnRtnL2Order(CSecurityFtdcL2OrderField *pL2Order);

	///Level2成交通知
	void OnRtnL2Trade(CSecurityFtdcL2TradeField *pL2Trade);

	///通知清理SSE买卖一队列中数量为0的报单
	void OnNtfCheckOrderList(TSecurityFtdcInstrumentIDType InstrumentID, TSecurityFtdcFunctionCodeType FunctionCode);

	bool IsErrorRspInfo(CSecurityFtdcRspInfoField *pRspInfo);

	 ~Lv2MD();
private:
	CSecurityFtdcL2MDUserApi* plv2MD;
	CSecurityFtdcUserLoginField LoginInfo;
};
_LTS_NS_END_
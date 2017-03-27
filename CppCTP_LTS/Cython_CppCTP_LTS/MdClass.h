#include <list>
#include "SecurityFtdcMdApi.h"

using namespace std;
using std::list;

class MdClass : public CSecurityFtdcMdSpi
{
public:

	// 构造函数带front_id, broker_id, user_id, password
	MdClass(char *front_address, char *broker_id, char *user_id, char *password, const char *pszFlowPath = "");

	//登陆
	void Login();
	
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
	
	 //订阅行情
	 void SubMarketData(list<string> list_instrumentid, char* pExchageID);

	 //取消订阅行情
	 void UnSubMarketData(list<string> list_instrumentid, char* pExchageID);

	///错误应答
	 void OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///登录请求响应
	 void OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///登出请求响应
	 void OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///订阅行情应答
	 void OnRspSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///取消订阅行情应答
	 void OnRspUnSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///深度行情通知
	 void OnRtnDepthMarketData(CSecurityFtdcDepthMarketDataField *pDepthMarketData);

	 //返回数据是否报错
	 bool IsErrorRspInfo(CSecurityFtdcRspInfoField *pRspInfo);

	 ~MdClass();

    CSecurityFtdcMdApi* pMD;
    CSecurityFtdcReqUserLoginField LoginInfo;
};
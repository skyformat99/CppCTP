/////////////////////////////////////////////////////////////////////////
///@system 新一代交易所系统
///@company Alex Capital Manager Co.LTD
///@file ThostFtdcTraderApi.h
///@brief 定义了客户端接口
///@history 
///20060106	赵鸿昊		创建该文件
/////////////////////////////////////////////////////////////////////////

#if !defined(THOST_FTDCTRADERAPI_H)
#define THOST_FTDCTRADERAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ThostFtdcUserApiStruct.h"

#if defined(ISLIB) && defined(WIN32)
#ifdef LIB_TRADER_API_EXPORT
#define TRADER_API_EXPORT __declspec(dllexport)
#else
#define TRADER_API_EXPORT __declspec(dllimport)
#endif
#else
#define TRADER_API_EXPORT 
#endif

class CThostFtdcTraderSpi
{
public:
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected(){};
	
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason){};
		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	virtual void OnHeartBeatWarning(int nTimeLapse){};
	
	///客户端认证响应
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	

	///数据输出请求
	virtual void OnRspDataDump(CThostFtdcSettlementRefField *pSettlementRef, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///用户口令更新请求响应
	virtual void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///资金账户口令更新请求响应
	virtual void OnRspTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///查询最大报单数量响应
	virtual void OnRspQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求增加交易员响应
	virtual void OnRspInsTrader(CThostFtdcTraderField *pTrader, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求修改交易员响应
	virtual void OnRspUpdTrader(CThostFtdcTraderField *pTrader, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求增加投资者响应
	virtual void OnRspInsInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求修改投资者响应
	virtual void OnRspUpdInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求增加交易编码响应
	virtual void OnRspInsTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求修改交易编码响应
	virtual void OnRspUpdTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求删除交易编码响应
	virtual void OnRspDelTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询报单响应
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询成交响应
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者持仓响应
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者响应
	virtual void OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询交易编码响应
	virtual void OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询合约保证金率响应
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询合约手续费率响应
	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询交易员响应
	virtual void OnRspQryTrader(CThostFtdcTraderField *pTrader, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询交易所响应
	virtual void OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询行情响应
	virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者结算结果响应
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者持仓明细响应
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询客户通知响应
	virtual void OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询结算信息确认响应
	virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///查询保证金监管系统经纪公司资金账户密钥响应
	virtual void OnRspQryCFMMCTradingAccountKey(CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询仓单折抵信息响应
	virtual void OnRspQryEWarrantOffset(CThostFtdcEWarrantOffsetField *pEWarrantOffset, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询银期签约关系响应
	virtual void OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder) {};

	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade) {};

	///报单录入错误回报
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) {};

	///报单操作错误回报
	virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) {};

	///合约交易状态通知
	virtual void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus) {};

	///交易通知
	virtual void OnRtnTradingNotice(CThostFtdcTradingNoticeInfoField *pTradingNoticeInfo) {};

	///请求查询签约银行响应
	virtual void OnRspQryContractBank(CThostFtdcContractBankField *pContractBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询交易通知响应
	virtual void OnRspQryTradingNotice(CThostFtdcTradingNoticeField *pTradingNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询经纪公司交易参数响应
	virtual void OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询经纪公司交易算法响应
	virtual void OnRspQryBrokerTradingAlgos(CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询交易策略响应
	virtual void OnRspQryTradingStrategy(CThostFtdcTradingStrategyField *pTradingStrategy, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求增加交易策略响应
	virtual void OnRspInsTradingStrategy(CThostFtdcTradingStrategyField *pTradingStrategy, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求修改交易策略响应
	virtual void OnRspUpdTradingStrategy(CThostFtdcTradingStrategyField *pTradingStrategy, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求删除交易策略响应
	virtual void OnRspDelTradingStrategy(CThostFtdcTradingStrategyField *pTradingStrategy, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求设置投资者持仓响应
	virtual void OnRspSetInvestorPosition(CThostFtdcTradingStrategyField *pTradingStrategy, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者风控参数响应
	virtual void OnRspQryInvestorRiskParam(CThostFtdcInvestorRiskParamField *pInvestorRiskParam, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求增加投资者风控参数响应
	virtual void OnRspInsInvestorRiskParam(CThostFtdcInvestorRiskParamField *pInvestorRiskParam, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求修改投资者风控参数响应
	virtual void OnRspUpdInvestorRiskParam(CThostFtdcInvestorRiskParamField *pInvestorRiskParam, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求删除投资者风控参数响应
	virtual void OnRspDelInvestorRiskParam(CThostFtdcInvestorRiskParamField *pInvestorRiskParam, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求增加投资者撤单计数器响应
	virtual void OnRspInsInvestorOrderActionCount(CThostFtdcInvestorOrderActionCountField *pInvestorOrderActionCount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求修改投资者撤单计数器响应
	virtual void OnRspUpdInvestorOrderActionCount(CThostFtdcInvestorOrderActionCountField *pInvestorOrderActionCount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求删除投资者撤单计数器响应
	virtual void OnRspDelInvestorOrderActionCount(CThostFtdcInvestorOrderActionCountField *pInvestorOrderActionCount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者撤单计数器响应
	virtual void OnRspQryInvestorOrderActionCount(CThostFtdcInvestorOrderActionCountField *pInvestorOrderActionCount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求提交函数应答
	virtual void OnRspSubmitFunction(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求提交公式应答
	virtual void OnRspSubmitFormula(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求编译代码应答
	virtual void OnRspCompileCode(CThostFtdcCompileCodeField *pCompileCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求执行公式应答
	virtual void OnRspExecuteFormula(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
};

class TRADER_API_EXPORT CThostFtdcTraderApi
{
public:
	///创建TraderApi
	///@param pszFlowPath 存贮订阅信息文件的目录，默认为当前目录
	///@return 创建出的UserApi
	//modify for udp marketdata
	static CThostFtdcTraderApi *CreateFtdcTraderApi(const char *pszFlowPath = "", const bool bIsUsingUdp=false);
	
	///删除接口对象本身
	///@remark 不再使用本接口对象时,调用该函数删除接口对象
	virtual void Release() = 0;
	
	///初始化
	///@remark 初始化运行环境,只有调用后,接口才开始工作
	virtual void Init() = 0;
	
	///等待接口线程结束运行
	///@return 线程退出代码
	virtual int Join() = 0;
	
	///获取当前交易日
	///@retrun 获取到的交易日
	///@remark 只有登录成功后,才能得到正确的交易日
	virtual const char *GetTradingDay() = 0;
	
	///注册前置机网络地址
	///@param pszFrontAddress：前置机网络地址。
	///@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:17001”。 
	///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”17001”代表服务器端口号。
	virtual void RegisterFront(char *pszFrontAddress) = 0;
	
	///注册回调接口
	///@param pSpi 派生自回调接口类的实例
	virtual void RegisterSpi(CThostFtdcTraderSpi *pSpi) = 0;
	
	///订阅私有流。
	///@param nResumeType 私有流重传方式  
	///        THOST_TERT_RESTART:从本交易日开始重传
	///        THOST_TERT_RESUME:从上次收到的续传
	///        THOST_TERT_QUICK:只传送登录后私有流的内容
	///@remark 该方法要在Init方法前调用。若不调用则不会收到私有流的数据。
	virtual void SubscribePrivateTopic(THOST_TE_RESUME_TYPE nResumeType) = 0;
	
	///订阅公共流。
	///@param nResumeType 公共流重传方式  
	///        THOST_TERT_RESTART:从本交易日开始重传
	///        THOST_TERT_RESUME:从上次收到的续传
	///        THOST_TERT_QUICK:只传送登录后公共流的内容
	///@remark 该方法要在Init方法前调用。若不调用则不会收到公共流的数据。
	virtual void SubscribePublicTopic(THOST_TE_RESUME_TYPE nResumeType) = 0;

	///客户端认证请求
	virtual int ReqAuthenticate(CThostFtdcReqAuthenticateField *pReqAuthenticateField, int nRequestID) = 0;

	///用户登录请求
	virtual int ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID) = 0;
	

	///数据输出请求
	virtual int ReqDataDump(CThostFtdcSettlementRefField *pSettlementRef, int nRequestID) = 0;

	///登出请求
	virtual int ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID) = 0;

	///用户口令更新请求
	virtual int ReqUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID) = 0;

	///资金账户口令更新请求
	virtual int ReqTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, int nRequestID) = 0;

	///报单录入请求
	virtual int ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID) = 0;

	///报单操作请求
	virtual int ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID) = 0;

	///查询最大报单数量请求
	virtual int ReqQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, int nRequestID) = 0;

	///投资者结算结果确认
	virtual int ReqSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int nRequestID) = 0;

	///请求增加交易员
	virtual int ReqInsTrader(CThostFtdcTraderField *pTrader, int nRequestID) = 0;

	///请求修改交易员
	virtual int ReqUpdTrader(CThostFtdcTraderField *pTrader, int nRequestID) = 0;

	///请求增加投资者
	virtual int ReqInsInvestor(CThostFtdcInvestorField *pInvestor, int nRequestID) = 0;

	///请求修改投资者
	virtual int ReqUpdInvestor(CThostFtdcInvestorField *pInvestor, int nRequestID) = 0;

	///请求增加交易编码
	virtual int ReqInsTradingCode(CThostFtdcTradingCodeField *pTradingCode, int nRequestID) = 0;

	///请求修改交易编码
	virtual int ReqUpdTradingCode(CThostFtdcTradingCodeField *pTradingCode, int nRequestID) = 0;

	///请求删除交易编码
	virtual int ReqDelTradingCode(CThostFtdcTradingCodeField *pTradingCode, int nRequestID) = 0;

	///请求查询报单
	virtual int ReqQryOrder(CThostFtdcQryOrderField *pQryOrder, int nRequestID) = 0;

	///请求查询成交
	virtual int ReqQryTrade(CThostFtdcQryTradeField *pQryTrade, int nRequestID) = 0;

	///请求查询投资者持仓
	virtual int ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID) = 0;

	///请求查询资金账户
	virtual int ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID) = 0;

	///请求查询投资者
	virtual int ReqQryInvestor(CThostFtdcQryInvestorField *pQryInvestor, int nRequestID) = 0;

	///请求查询交易编码
	virtual int ReqQryTradingCode(CThostFtdcQryTradingCodeField *pQryTradingCode, int nRequestID) = 0;

	///请求查询合约保证金率
	virtual int ReqQryInstrumentMarginRate(CThostFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate, int nRequestID) = 0;

	///请求查询合约手续费率
	virtual int ReqQryInstrumentCommissionRate(CThostFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate, int nRequestID) = 0;

	///请求查询交易员
	virtual int ReqQryTrader(CThostFtdcQryTraderField *pQryTrader, int nRequestID) = 0;

	///请求查询交易所
	virtual int ReqQryExchange(CThostFtdcQryExchangeField *pQryExchange, int nRequestID) = 0;

	///请求查询合约
	virtual int ReqQryInstrument(CThostFtdcQryInstrumentField *pQryInstrument, int nRequestID) = 0;

	///请求查询行情
	virtual int ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField *pQryDepthMarketData, int nRequestID) = 0;

	///请求查询投资者结算结果
	virtual int ReqQrySettlementInfo(CThostFtdcQrySettlementInfoField *pQrySettlementInfo, int nRequestID) = 0;

	///请求查询投资者持仓明细
	virtual int ReqQryInvestorPositionDetail(CThostFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail, int nRequestID) = 0;

	///请求查询客户通知
	virtual int ReqQryNotice(CThostFtdcQryNoticeField *pQryNotice, int nRequestID) = 0;

	///请求查询结算信息确认
	virtual int ReqQrySettlementInfoConfirm(CThostFtdcQrySettlementInfoConfirmField *pQrySettlementInfoConfirm, int nRequestID) = 0;

	///请求查询保证金监管系统经纪公司资金账户密钥
	virtual int ReqQryCFMMCTradingAccountKey(CThostFtdcQryCFMMCTradingAccountKeyField *pQryCFMMCTradingAccountKey, int nRequestID) = 0;

	///请求查询仓单折抵信息
	virtual int ReqQryEWarrantOffset(CThostFtdcQryEWarrantOffsetField *pQryEWarrantOffset, int nRequestID) = 0;

	///请求查询银期签约关系
	virtual int ReqQryAccountregister(CThostFtdcQryAccountregisterField *pQryAccountregister, int nRequestID) = 0;

	///请求查询签约银行
	virtual int ReqQryContractBank(CThostFtdcQryContractBankField *pQryContractBank, int nRequestID) = 0;

	///请求查询交易通知
	virtual int ReqQryTradingNotice(CThostFtdcQryTradingNoticeField *pQryTradingNotice, int nRequestID) = 0;

	///请求查询经纪公司交易参数
	virtual int ReqQryBrokerTradingParams(CThostFtdcQryBrokerTradingParamsField *pQryBrokerTradingParams, int nRequestID) = 0;

	///请求查询经纪公司交易算法
	virtual int ReqQryBrokerTradingAlgos(CThostFtdcQryBrokerTradingAlgosField *pQryBrokerTradingAlgos, int nRequestID) = 0;

	///请求查询交易策略
	virtual int ReqQryTradingStrategy(CThostFtdcQryTradingStrategyField *pQryTradingStrategy, int nRequestID) = 0;

	///请求增加交易策略
	virtual int ReqInsTradingStrategy(CThostFtdcTradingStrategyField *pTradingStrategy, int nRequestID) = 0;

	///请求修改交易策略
	virtual int ReqUpdTradingStrategy(CThostFtdcTradingStrategyField *pTradingStrategy, int nRequestID) = 0;

	///请求删除交易策略
	virtual int ReqDelTradingStrategy(CThostFtdcTradingStrategyField *pTradingStrategy, int nRequestID) = 0;

	///请求设置投资者持仓
	virtual int ReqSetInvestorPosition(CThostFtdcTradingStrategyField *pTradingStrategy, int nRequestID) = 0;

	///请求查询投资者风控参数
	virtual int ReqQryInvestorRiskParam(CThostFtdcQryInvestorRiskParamField *pQryInvestorRiskParam, int nRequestID) = 0;

	///请求增加投资者风控参数
	virtual int ReqInsInvestorRiskParam(CThostFtdcInvestorRiskParamField *pInvestorRiskParam, int nRequestID) = 0;

	///请求修改投资者风控参数
	virtual int ReqUpdInvestorRiskParam(CThostFtdcInvestorRiskParamField *pInvestorRiskParam, int nRequestID) = 0;

	///请求删除投资者风控参数
	virtual int ReqDelInvestorRiskParam(CThostFtdcInvestorRiskParamField *pInvestorRiskParam, int nRequestID) = 0;

	///请求增加投资者撤单计数器
	virtual int ReqInsInvestorOrderActionCount(CThostFtdcInvestorOrderActionCountField *pInvestorOrderActionCount, int nRequestID) = 0;

	///请求修改投资者撤单计数器
	virtual int ReqUpdInvestorOrderActionCount(CThostFtdcInvestorOrderActionCountField *pInvestorOrderActionCount, int nRequestID) = 0;

	///请求删除投资者撤单计数器
	virtual int ReqDelInvestorOrderActionCount(CThostFtdcInvestorOrderActionCountField *pInvestorOrderActionCount, int nRequestID) = 0;

	///请求查询投资者撤单计数器
	virtual int ReqQryInvestorOrderActionCount(CThostFtdcQryInvestorOrderActionCountField *pQryInvestorOrderActionCount, int nRequestID) = 0;

	///请求提交函数
	virtual int ReqSubmitFunction(CThostFtdcSubmitFunctionField *pSubmitFunction, int nRequestID) = 0;

	///请求提交公式
	virtual int ReqSubmitFormula(CThostFtdcSubmitFormulaField *pSubmitFormula, int nRequestID) = 0;

	///请求编译代码
	virtual int ReqCompileCode(CThostFtdcCompileCodeField *pCompileCode, int nRequestID) = 0;

	///请求执行公式
	virtual int ReqExecuteFormula(CThostFtdcExecuteFormulaField *pExecuteFormula, int nRequestID) = 0;
protected:
	~CThostFtdcTraderApi(){};
};

#endif

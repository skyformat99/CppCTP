#ifndef QUANT_CTP_ORDER_H
#define QUANT_CTP_ORDER_H

#include <string>

using namespace std;
using std::string;

class Order {
public:
	Order();
	~Order();

	string getBrokerID();
	void setBrokerID(string brokerID);
	string getInvestorID();
	void setInvestorID(string investorID);
	string getInstrumentID();
	void setInstrumentID(string instrumentID);
	string getOrderRef();
	void setOrderRef(string orderRef);
	string getUserID();
	void setUserID(string userID);
	char getOrderPriceType();
	void setOrderPriceType(char orderPriceType);
	char getDirection();
	void setDirection(char direction);
	string getCombOffsetFlag();
	void setCombOffsetFlag(string combOffsetFlag);
	string getCombHedgeFlag();
	void setCombHedgeFlag(string combHedgeFlag);
	double getLimitPrice();
	void setLimitPrice(double limitPrice);
	int getVolumeTotalOriginal();
	void setVolumeTotalOriginal(int volumeTotalOriginal);
	char getTimeCondition();
	void setTimeCondition(char timeCondition);
	string getGTDDate();
	void setGTDDate(string gTDDate);
	char getVolumeCondition();
	void setVolumeCondition(char volumeCondition);
	int getMinVolume();
	void setMinVolume(int minVolume);
	char getContingentCondition();
	void setContingentCondition(char contingentCondition);
	double getStopPrice();
	void setStopPrice(double stopPrice);
	char getForceCloseReason();
	void setForceCloseReason(char forceCloseReason);
	int getIsAutoSuspend();
	void setIsAutoSuspend(int isAutoSuspend);
	string getBusinessUnit();
	void setBusinessUnit(string businessUnit);
	int getRequestID();
	void setRequestID(int requestID);
	string getOrderLocalID();
	void setOrderLocalID(string orderLocalID);
	string getExchangeID();
	void setExchangeID(string exchangeID);
	string getParticipantID();
	void setParticipantID(string participantID);
	string getClientID();
	void setClientID(string clientID);
	string getExchangeInstID();
	void setExchangeInstID(string exchangeInstID);
	string getTraderID();
	void setTraderID(string traderID);
	int getInstallID();
	void setInstallID(int installID);
	char getOrderSubmitStatus();
	void setOrderSubmitStatus(char orderSubmitStatus);
	int getNotifySequence();
	void setNotifySequence(int notifySequence);
	string getTradingDay();
	void setTradingDay(string tradingDay);
	int getSettlementID();
	void setSettlementID(int settlementID);
	string getOrderSysID();
	void setOrderSysID(string orderSysID);
	char getOrderSource();
	void setOrderSource(char orderSource);
	char getOrderStatus();
	void setOrderStatus(char orderStatus);
	char getOrderType();
	void setOrderType(char orderType);
	int getVolumeTraded();
	void setVolumeTraded(int volumeTraded);
	int getVolumeTotal();
	void setVolumeTotal(int volumeTotal);
	string getInsertDate();
	void setInsertDate(string insertDate);
	string getInsertTime();
	void setInsertTime(string insertTime);
	string getActiveTime();
	void setActiveTime(string activeTime);
	string getSuspendTime();
	void setSuspendTime(string suspendTime);
	string getUpdateTime();
	void setUpdateTime(string updateTime);
	string getCancelTime();
	void setCancelTime(string cancelTime);
	string getActiveTraderID();
	void setActiveTraderID(string activeTraderID);
	string getClearingPartID();
	void setClearingPartID(string clearingPartID);
	int getSequenceNo();
	void setSequenceNo(int sequenceNo);
	int getFrontID();
	void setFrontID(int frontID);
	int getSessionID();
	void setSessionID(int sessionID);
	string getUserProductInfo();
	void setUserProductInfo(string userProductInfo);
	string getStatusMsg();
	void setStatusMsg(string statusMsg);
	int getUserForceClose();
	void setUserForceClose(int userForceClose);
	string getActiveUserID();
	void setActiveUserID(string activeUserID);
	int getBrokerOrderSeq();
	void setBrokerOrderSeq(int brokerOrderSeq);
	string getRelativeOrderSysID();
	void setRelativeOrderSysID(string relativeOrderSysID);
	int getZCETotalTradedVolume();
	void setZCETotalTradedVolume(int zCETotalTradedVolume);
	int getIsSwapOrder();
	void setIsSwapOrder(int isSwapOrder);
	string getBranchID();
	void setBranchID(string branchID);
	string getInvestUnitID();
	void setInvestUnitID(string investUnitID);
	string getAccountID();
	void setAccountID(string accountID);
	string getCurrencyID();
	void setCurrencyID(string currencyID);
	string getIPAddress();
	void setIPAddress(string iPAddress);
	string getMacAddress();
	void setMacAddress(string macAddress);
	string getSendOrderTime();
	void setSendOrderTime(string sendOrderTime);
	string getSendOrderMicrosecond();
	void setSendOrderMicrosecond(string sendOrderMicrosecond);
	string getCtpRtnOrderTime();
	void setCtpRtnOrderTime(string ctpRtnOrderTime);
	string getCtpRtnOrderMicrosecond();
	void setCtpRtnOrderMicrosecond(string ctpRtnOrderMicrosecond);
	string getExchRtnOrderTime();
	void setExchRtnOrderTime(string exchRtnOrderTime);
	string getExchRtnOrderMicrosecond();
	void setExchRtnOrderMicrosecond(string exchRtnOrderMicrosecond);
	string getOperatorID();
	void setOperatorID(string operatorID);
	string getStrategyID();
	void setStrategyID(string strategyID);

private:
	///经纪公司代码
	string BrokerID;
	///投资者代码
	string InvestorID;
	///合约代码
	string InstrumentID;
	///报单引用
	string OrderRef;
	///用户代码
	string	UserID;
	///报单价格条件
	char OrderPriceType;
	///买卖方向
	char Direction;
	///组合开平标志
	string	CombOffsetFlag;
	///组合投机套保标志
	string	CombHedgeFlag;
	///价格
	double	LimitPrice;
	///数量
	int VolumeTotalOriginal;
	///有效期类型
	char TimeCondition;
	///GTD日期
	string GTDDate;
	///成交量类型
	char VolumeCondition;
	///最小成交量
	int MinVolume;
	///触发条件
	char ContingentCondition;
	///止损价
	double StopPrice;
	///强平原因
	char ForceCloseReason;
	///自动挂起标志
	int IsAutoSuspend;
	///业务单元
	string BusinessUnit;
	///请求编号
	int	RequestID;
	///本地报单编号
	string OrderLocalID;
	///交易所代码
	string ExchangeID;
	///会员代码
	string ParticipantID;
	///客户代码
	string ClientID;
	///合约在交易所的代码
	string ExchangeInstID;
	///交易所交易员代码
	string TraderID;
	///安装编号
	int InstallID;
	///报单提交状态
	char OrderSubmitStatus;
	///报单提示序号
	int NotifySequence;
	///交易日
	string TradingDay;
	///结算编号
	int SettlementID;
	///报单编号
	string OrderSysID;
	///报单来源
	char OrderSource;
	///报单状态
	char OrderStatus;
	///报单类型
	char OrderType;
	///今成交数量
	int VolumeTraded;
	///剩余数量
	int VolumeTotal;
	///报单日期
	string InsertDate;
	///委托时间
	string InsertTime;
	///激活时间
	string ActiveTime;
	///挂起时间
	string SuspendTime;
	///最后修改时间
	string UpdateTime;
	///撤销时间
	string CancelTime;
	///最后修改交易所交易员代码
	string ActiveTraderID;
	///结算会员编号
	string ClearingPartID;
	///序号
	int SequenceNo;
	///前置编号
	int FrontID;
	///会话编号
	int SessionID;
	///用户端产品信息
	string UserProductInfo;
	///状态信息
	string StatusMsg;
	///用户强评标志
	int UserForceClose;
	///操作用户代码
	string ActiveUserID;
	///经纪公司报单编号
	int BrokerOrderSeq;
	///相关报单
	string RelativeOrderSysID;
	///郑商所成交数量
	int ZCETotalTradedVolume;
	///互换单标志
	int IsSwapOrder;
	///营业部编号
	string BranchID;
	///投资单元代码
	string InvestUnitID;
	///资金账号
	string AccountID;
	///币种代码
	string CurrencyID;
	///IP地址
	string IPAddress;
	///Mac地址
	string MacAddress;
	///发送order时间
	string SendOrderTime;
	///发送order微秒
	string SendOrderMicrosecond;
	///CTP响应时间
	string CtpRtnOrderTime;
	///CTP响应时间微秒
	string CtpRtnOrderMicrosecond;
	///交易所响应时间
	string ExchRtnOrderTime;
	///交易所响应时间微秒
	string ExchRtnOrderMicrosecond;
	///客户端账号
	string OperatorID;
	///策略号
	string StrategyID;
};

#endif
#include "Order.h"
#include <string>
using std::string;

string Order::getBrokerID() {
	return BrokerID;
}
void Order::setBrokerID(string brokerID) {
	BrokerID = brokerID;
}
string Order::getInvestorID() {
	return InvestorID;
}
void Order::setInvestorID(string investorID) {
	InvestorID = investorID;
}
string Order::getInstrumentID() {
	return InstrumentID;
}
void Order::setInstrumentID(string instrumentID) {
	InstrumentID = instrumentID;
}
string Order::getOrderRef() {
	return OrderRef;
}
void Order::setOrderRef(string orderRef) {
	OrderRef = orderRef;
}
string Order::getUserID() {
	return UserID;
}
void Order::setUserID(string userID) {
	UserID = userID;
}
char Order::getOrderPriceType() {
	return OrderPriceType;
}
void Order::setOrderPriceType(char orderPriceType) {
	OrderPriceType = orderPriceType;
}
char Order::getDirection() {
	return Direction;
}
void Order::setDirection(char direction) {
	Direction = direction;
}
string Order::getCombOffsetFlag() {
	return CombOffsetFlag;
}
void Order::setCombOffsetFlag(string combOffsetFlag) {
	CombOffsetFlag = combOffsetFlag;
}
string Order::getCombHedgeFlag() {
	return CombHedgeFlag;
}
void Order::setCombHedgeFlag(string combHedgeFlag) {
	CombHedgeFlag = combHedgeFlag;
}
double Order::getLimitPrice() {
	return LimitPrice;
}
void Order::setLimitPrice(double limitPrice) {
	LimitPrice = limitPrice;
}
int Order::getVolumeTotalOriginal() {
	return VolumeTotalOriginal;
}
void Order::setVolumeTotalOriginal(int volumeTotalOriginal) {
	VolumeTotalOriginal = volumeTotalOriginal;
}
char Order::getTimeCondition() {
	return TimeCondition;
}
void Order::setTimeCondition(char timeCondition) {
	TimeCondition = timeCondition;
}
string Order::getGTDDate() {
	return GTDDate;
}
void Order::setGTDDate(string gTDDate) {
	GTDDate = gTDDate;
}
char Order::getVolumeCondition() {
	return VolumeCondition;
}
void Order::setVolumeCondition(char volumeCondition) {
	VolumeCondition = volumeCondition;
}
int Order::getMinVolume() {
	return MinVolume;
}
void Order::setMinVolume(int minVolume) {
	MinVolume = minVolume;
}
char Order::getContingentCondition() {
	return ContingentCondition;
}
void Order::setContingentCondition(char contingentCondition) {
	ContingentCondition = contingentCondition;
}
double Order::getStopPrice() {
	return StopPrice;
}
void Order::setStopPrice(double stopPrice) {
	StopPrice = stopPrice;
}
char Order::getForceCloseReason() {
	return ForceCloseReason;
}
void Order::setForceCloseReason(char forceCloseReason) {
	ForceCloseReason = forceCloseReason;
}
int Order::getIsAutoSuspend() {
	return IsAutoSuspend;
}
void Order::setIsAutoSuspend(int isAutoSuspend) {
	IsAutoSuspend = isAutoSuspend;
}
string Order::getBusinessUnit() {
	return BusinessUnit;
}
void Order::setBusinessUnit(string businessUnit) {
	BusinessUnit = businessUnit;
}
int Order::getRequestID() {
	return RequestID;
}
void Order::setRequestID(int requestID) {
	RequestID = requestID;
}
string Order::getOrderLocalID() {
	return OrderLocalID;
}
void Order::setOrderLocalID(string orderLocalID) {
	OrderLocalID = orderLocalID;
}
string Order::getExchangeID() {
	return ExchangeID;
}
void Order::setExchangeID(string exchangeID) {
	ExchangeID = exchangeID;
}
string Order::getParticipantID() {
	return ParticipantID;
}
void Order::setParticipantID(string participantID) {
	ParticipantID = participantID;
}
string Order::getClientID() {
	return ClientID;
}
void Order::setClientID(string clientID) {
	ClientID = clientID;
}
string Order::getExchangeInstID() {
	return ExchangeInstID;
}
void Order::setExchangeInstID(string exchangeInstID) {
	ExchangeInstID = exchangeInstID;
}
string Order::getTraderID() {
	return TraderID;
}
void Order::setTraderID(string traderID) {
	TraderID = traderID;
}
int Order::getInstallID() {
	return InstallID;
}
void Order::setInstallID(int installID) {
	InstallID = installID;
}
char Order::getOrderSubmitStatus() {
	return OrderSubmitStatus;
}
void Order::setOrderSubmitStatus(char orderSubmitStatus) {
	OrderSubmitStatus = orderSubmitStatus;
}
int Order::getNotifySequence() {
	return NotifySequence;
}
void Order::setNotifySequence(int notifySequence) {
	NotifySequence = notifySequence;
}
string Order::getTradingDay() {
	return TradingDay;
}
void Order::setTradingDay(string tradingDay) {
	TradingDay = tradingDay;
}
int Order::getSettlementID() {
	return SettlementID;
}
void Order::setSettlementID(int settlementID) {
	SettlementID = settlementID;
}
string Order::getOrderSysID() {
	return OrderSysID;
}
void Order::setOrderSysID(string orderSysID) {
	OrderSysID = orderSysID;
}
char Order::getOrderSource() {
	return OrderSource;
}
void Order::setOrderSource(char orderSource) {
	OrderSource = orderSource;
}
char Order::getOrderStatus() {
	return OrderStatus;
}
void Order::setOrderStatus(char orderStatus) {
	OrderStatus = orderStatus;
}
char Order::getOrderType() {
	return OrderType;
}
void Order::setOrderType(char orderType) {
	OrderType = orderType;
}
int Order::getVolumeTraded() {
	return VolumeTraded;
}
void Order::setVolumeTraded(int volumeTraded) {
	VolumeTraded = volumeTraded;
}
int Order::getVolumeTotal() {
	return VolumeTotal;
}
void Order::setVolumeTotal(int volumeTotal) {
	VolumeTotal = volumeTotal;
}
string Order::getInsertDate() {
	return InsertDate;
}
void Order::setInsertDate(string insertDate) {
	InsertDate = insertDate;
}
string Order::getInsertTime() {
	return InsertTime;
}
void Order::setInsertTime(string insertTime) {
	InsertTime = insertTime;
}
string Order::getActiveTime() {
	return ActiveTime;
}
void Order::setActiveTime(string activeTime) {
	ActiveTime = activeTime;
}
string Order::getSuspendTime() {
	return SuspendTime;
}
void Order::setSuspendTime(string suspendTime) {
	SuspendTime = suspendTime;
}
string Order::getUpdateTime() {
	return UpdateTime;
}
void Order::setUpdateTime(string updateTime) {
	UpdateTime = updateTime;
}
string Order::getCancelTime() {
	return CancelTime;
}
void Order::setCancelTime(string cancelTime) {
	CancelTime = cancelTime;
}
string Order::getActiveTraderID() {
	return ActiveTraderID;
}
void Order::setActiveTraderID(string activeTraderID) {
	ActiveTraderID = activeTraderID;
}
string Order::getClearingPartID() {
	return ClearingPartID;
}
void Order::setClearingPartID(string clearingPartID) {
	ClearingPartID = clearingPartID;
}
int Order::getSequenceNo() {
	return SequenceNo;
}
void Order::setSequenceNo(int sequenceNo) {
	SequenceNo = sequenceNo;
}
int Order::getFrontID() {
	return FrontID;
}
void Order::setFrontID(int frontID) {
	FrontID = frontID;
}
int Order::getSessionID() {
	return SessionID;
}
void Order::setSessionID(int sessionID) {
	SessionID = sessionID;
}
string Order::getUserProductInfo() {
	return UserProductInfo;
}
void Order::setUserProductInfo(string userProductInfo) {
	UserProductInfo = userProductInfo;
}
string Order::getStatusMsg() {
	return StatusMsg;
}
void Order::setStatusMsg(string statusMsg) {
	StatusMsg = statusMsg;
}
int Order::getUserForceClose() {
	return UserForceClose;
}
void Order::setUserForceClose(int userForceClose) {
	UserForceClose = userForceClose;
}
string Order::getActiveUserID() {
	return ActiveUserID;
}
void Order::setActiveUserID(string activeUserID) {
	ActiveUserID = activeUserID;
}
int Order::getBrokerOrderSeq() {
	return BrokerOrderSeq;
}
void Order::setBrokerOrderSeq(int brokerOrderSeq) {
	BrokerOrderSeq = brokerOrderSeq;
}
string Order::getRelativeOrderSysID() {
	return RelativeOrderSysID;
}
void Order::setRelativeOrderSysID(string relativeOrderSysID) {
	RelativeOrderSysID = relativeOrderSysID;
}
int Order::getZCETotalTradedVolume() {
	return ZCETotalTradedVolume;
}
void Order::setZCETotalTradedVolume(int zCETotalTradedVolume) {
	ZCETotalTradedVolume = zCETotalTradedVolume;
}
int Order::getIsSwapOrder() {
	return IsSwapOrder;
}
void Order::setIsSwapOrder(int isSwapOrder) {
	IsSwapOrder = isSwapOrder;
}
string Order::getBranchID() {
	return BranchID;
}
void Order::setBranchID(string branchID) {
	BranchID = branchID;
}
string Order::getInvestUnitID() {
	return InvestUnitID;
}
void Order::setInvestUnitID(string investUnitID) {
	InvestUnitID = investUnitID;
}
string Order::getAccountID() {
	return AccountID;
}
void Order::setAccountID(string accountID) {
	AccountID = accountID;
}
string Order::getCurrencyID() {
	return CurrencyID;
}
void Order::setCurrencyID(string currencyID) {
	CurrencyID = currencyID;
}
string Order::getIPAddress() {
	return IPAddress;
}
void Order::setIPAddress(string iPAddress) {
	IPAddress = iPAddress;
}
string Order::getMacAddress() {
	return MacAddress;
}
void Order::setMacAddress(string macAddress) {
	MacAddress = macAddress;
}
string Order::getSendOrderTime() {
	return SendOrderTime;
}
void Order::setSendOrderTime(string sendOrderTime) {
	SendOrderTime = sendOrderTime;
}
string Order::getSendOrderMicrosecond() {
	return SendOrderMicrosecond;
}
void Order::setSendOrderMicrosecond(string sendOrderMicrosecond) {
	SendOrderMicrosecond = sendOrderMicrosecond;
}
string Order::getCtpRtnOrderTime() {
	return CtpRtnOrderTime;
}
void Order::setCtpRtnOrderTime(string ctpRtnOrderTime) {
	CtpRtnOrderTime = ctpRtnOrderTime;
}
string Order::getCtpRtnOrderMicrosecond() {
	return CtpRtnOrderMicrosecond;
}
void Order::setCtpRtnOrderMicrosecond(string ctpRtnOrderMicrosecond) {
	CtpRtnOrderMicrosecond = ctpRtnOrderMicrosecond;
}
string Order::getExchRtnOrderTime() {
	return ExchRtnOrderTime;
}
void Order::setExchRtnOrderTime(string exchRtnOrderTime) {
	ExchRtnOrderTime = exchRtnOrderTime;
}
string Order::getExchRtnOrderMicrosecond() {
	return ExchRtnOrderMicrosecond;
}
void Order::setExchRtnOrderMicrosecond(string exchRtnOrderMicrosecond) {
	ExchRtnOrderMicrosecond = exchRtnOrderMicrosecond;
}
string Order::getOperatorID() {
	return OperatorID;
}
void Order::setOperatorID(string operatorID) {
	OperatorID = operatorID;
}
string Order::getStrategyID() {
	return StrategyID;
}
void Order::setStrategyID(string strategyID) {
	StrategyID = strategyID;
}
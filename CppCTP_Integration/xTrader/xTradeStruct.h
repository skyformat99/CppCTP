#ifndef QUANT_XTRADESTRUCT_H
#define QUANT_XTRADESTRUCT_H
#include <iostream>
#include "SgitFtdcUserApiDataType.h"

using namespace std;
using std::string;

///报单
struct USER_CSgitFtdcOrderField
{
	///经纪公司代码
	TSgitFtdcBrokerIDType	BrokerID;
	///投资者代码
	TSgitFtdcInvestorIDType	InvestorID;
	///合约代码
	TSgitFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TSgitFtdcOrderRefType	OrderRef;
	/*///策略编号
	string StrategyID;*/
	///策略编号
	char StrategyID[3];
	///用户代码
	TSgitFtdcUserIDType	UserID;
	///报单价格条件
	TSgitFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TSgitFtdcDirectionType	Direction;
	///组合开平标志
	TSgitFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TSgitFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
	TSgitFtdcPriceType	LimitPrice;
	///数量
	TSgitFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
	TSgitFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TSgitFtdcDateType	GTDDate;
	///成交量类型
	TSgitFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TSgitFtdcVolumeType	MinVolume;
	///触发条件
	TSgitFtdcContingentConditionType	ContingentCondition;
	///止损价
	TSgitFtdcPriceType	StopPrice;
	///强平原因
	TSgitFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TSgitFtdcBoolType	IsAutoSuspend;
	///业务单元
	TSgitFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	TSgitFtdcRequestIDType	RequestID;
	///本地报单编号
	TSgitFtdcOrderLocalIDType	OrderLocalID;
	///交易所代码
	TSgitFtdcExchangeIDType	ExchangeID;
	///会员代码
	TSgitFtdcParticipantIDType	ParticipantID;
	///客户代码
	TSgitFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///交易所交易员代码
	TSgitFtdcTraderIDType	TraderID;
	///安装编号
	TSgitFtdcInstallIDType	InstallID;
	///报单提交状态
	TSgitFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///报单提示序号
	TSgitFtdcSequenceNoType	NotifySequence;
	///交易日
	TSgitFtdcDateType	TradingDay;
	///记录保存日期
	TSgitFtdcDateType	TradingDayRecord;
	///结算编号
	TSgitFtdcSettlementIDType	SettlementID;
	///报单编号
	TSgitFtdcOrderSysIDType	OrderSysID;
	///报单来源
	TSgitFtdcOrderSourceType	OrderSource;
	///报单状态
	TSgitFtdcOrderStatusType	OrderStatus;
	///报单类型
	TSgitFtdcOrderTypeType	OrderType;
	///今成交数量
	TSgitFtdcVolumeType	VolumeTraded;
	///本次成交量
	TSgitFtdcVolumeType	VolumeTradedBatch;
	///剩余数量
	TSgitFtdcVolumeType	VolumeTotal;
	///报单日期
	TSgitFtdcDateType	InsertDate;
	///委托时间
	TSgitFtdcTimeType	InsertTime;
	///激活时间
	TSgitFtdcTimeType	ActiveTime;
	///挂起时间
	TSgitFtdcTimeType	SuspendTime;
	///最后修改时间
	TSgitFtdcTimeType	UpdateTime;
	///撤销时间
	TSgitFtdcTimeType	CancelTime;
	///最后修改交易所交易员代码
	TSgitFtdcTraderIDType	ActiveTraderID;
	///结算会员编号
	TSgitFtdcParticipantIDType	ClearingPartID;
	///序号
	TSgitFtdcSequenceNoType	SequenceNo;
	///前置编号
	TSgitFtdcFrontIDType	FrontID;
	///会话编号
	TSgitFtdcSessionIDType	SessionID;
	///用户端产品信息
	TSgitFtdcProductInfoType	UserProductInfo;
	///状态信息
	TSgitFtdcErrorMsgType	StatusMsg;
	///用户强评标志
	TSgitFtdcBoolType	UserForceClose;
	///操作用户代码
	TSgitFtdcUserIDType	ActiveUserID;
	///经纪公司报单编号
	TSgitFtdcSequenceNoType	BrokerOrderSeq;
	///相关报单
	TSgitFtdcOrderSysIDType	RelativeOrderSysID;
	///郑商所成交数量
	TSgitFtdcVolumeType	ZCETotalTradedVolume;
	///互换单标志
	TSgitFtdcBoolType	IsSwapOrder;
	/////营业部编号
	//TSgitFtdcBranchIDType	BranchID;
	/////投资单元代码
	//TSgitFtdcInvestUnitIDType	InvestUnitID;
	///资金账号
	TSgitFtdcAccountIDType	AccountID;
	///币种代码
	TSgitFtdcCurrencyIDType	CurrencyID;
	///IP地址
	TSgitFtdcIPAddressType	IPAddress;
	///Mac地址
	TSgitFtdcMacAddressType	MacAddress;

	//USER_CSgitFtdcOrderField(){}
	//~USER_CSgitFtdcOrderField(){}

};

///深度行情
struct THREAD_CSgitFtdcDepthMarketDataField
{
	///交易日
	TSgitFtdcDateType	TradingDay;
	///合约代码
	TSgitFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TSgitFtdcExchangeIDType	ExchangeID;
	///合约在交易所的代码
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///最新价
	TSgitFtdcPriceType	LastPrice;
	///上次结算价
	TSgitFtdcPriceType	PreSettlementPrice;
	///昨收盘
	TSgitFtdcPriceType	PreClosePrice;
	///昨持仓量
	TSgitFtdcLargeVolumeType	PreOpenInterest;
	///今开盘
	TSgitFtdcPriceType	OpenPrice;
	///最高价
	TSgitFtdcPriceType	HighestPrice;
	///最低价
	TSgitFtdcPriceType	LowestPrice;
	///数量
	TSgitFtdcVolumeType	Volume;
	///成交金额
	TSgitFtdcMoneyType	Turnover;
	///持仓量
	TSgitFtdcLargeVolumeType	OpenInterest;
	///今收盘
	TSgitFtdcPriceType	ClosePrice;
	///本次结算价
	TSgitFtdcPriceType	SettlementPrice;
	///涨停板价
	TSgitFtdcPriceType	UpperLimitPrice;
	///跌停板价
	TSgitFtdcPriceType	LowerLimitPrice;
	///昨虚实度
	TSgitFtdcRatioType	PreDelta;
	///今虚实度
	TSgitFtdcRatioType	CurrDelta;
	///最后修改时间
	TSgitFtdcTimeType	UpdateTime;
	///最后修改毫秒
	TSgitFtdcMillisecType	UpdateMillisec;
	///申买价一
	TSgitFtdcPriceType	BidPrice1;
	///申买量一
	TSgitFtdcVolumeType	BidVolume1;
	///申卖价一
	TSgitFtdcPriceType	AskPrice1;
	///申卖量一
	TSgitFtdcVolumeType	AskVolume1;
	///申买价二
	TSgitFtdcPriceType	BidPrice2;
	///申买量二
	TSgitFtdcVolumeType	BidVolume2;
	///申卖价二
	TSgitFtdcPriceType	AskPrice2;
	///申卖量二
	TSgitFtdcVolumeType	AskVolume2;
	///申买价三
	TSgitFtdcPriceType	BidPrice3;
	///申买量三
	TSgitFtdcVolumeType	BidVolume3;
	///申卖价三
	TSgitFtdcPriceType	AskPrice3;
	///申卖量三
	TSgitFtdcVolumeType	AskVolume3;
	///申买价四
	TSgitFtdcPriceType	BidPrice4;
	///申买量四
	TSgitFtdcVolumeType	BidVolume4;
	///申卖价四
	TSgitFtdcPriceType	AskPrice4;
	///申卖量四
	TSgitFtdcVolumeType	AskVolume4;
	///申买价五
	TSgitFtdcPriceType	BidPrice5;
	///申买量五
	TSgitFtdcVolumeType	BidVolume5;
	///申卖价五
	TSgitFtdcPriceType	AskPrice5;
	///申卖量五
	TSgitFtdcVolumeType	AskVolume5;
	///当日均价
	TSgitFtdcPriceType	AveragePrice;
	///业务日期
	TSgitFtdcDateType	ActionDay;
	///是否是最后一个元素
	int IsLastElement;
};


///报单
struct THREAD_CSgitFtdcOrderField
{
	///经纪公司代码
	TSgitFtdcBrokerIDType	BrokerID;
	///投资者代码
	TSgitFtdcInvestorIDType	InvestorID;
	///合约代码
	TSgitFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TSgitFtdcOrderRefType	OrderRef;
	///用户代码
	TSgitFtdcUserIDType	UserID;
	///报单价格条件
	TSgitFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TSgitFtdcDirectionType	Direction;
	///组合开平标志
	TSgitFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TSgitFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
	TSgitFtdcPriceType	LimitPrice;
	///数量
	TSgitFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
	TSgitFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TSgitFtdcDateType	GTDDate;
	///成交量类型
	TSgitFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TSgitFtdcVolumeType	MinVolume;
	///触发条件
	TSgitFtdcContingentConditionType	ContingentCondition;
	///止损价
	TSgitFtdcPriceType	StopPrice;
	///强平原因
	TSgitFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TSgitFtdcBoolType	IsAutoSuspend;
	///业务单元
	TSgitFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	TSgitFtdcRequestIDType	RequestID;
	///本地报单编号
	TSgitFtdcOrderLocalIDType	OrderLocalID;
	///交易所代码
	TSgitFtdcExchangeIDType	ExchangeID;
	///会员代码
	TSgitFtdcParticipantIDType	ParticipantID;
	///客户代码
	TSgitFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///交易所交易员代码
	TSgitFtdcTraderIDType	TraderID;
	///安装编号
	TSgitFtdcInstallIDType	InstallID;
	///报单提交状态
	TSgitFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///报单提示序号
	TSgitFtdcSequenceNoType	NotifySequence;
	///交易日
	TSgitFtdcDateType	TradingDay;
	///记录保存日期
	TSgitFtdcDateType	TradingDayRecord;
	///结算编号
	TSgitFtdcSettlementIDType	SettlementID;
	///报单编号
	TSgitFtdcOrderSysIDType	OrderSysID;
	///报单来源
	TSgitFtdcOrderSourceType	OrderSource;
	///报单状态
	TSgitFtdcOrderStatusType	OrderStatus;
	///报单类型
	TSgitFtdcOrderTypeType	OrderType;
	///今成交数量
	TSgitFtdcVolumeType	VolumeTraded;
	///剩余数量
	TSgitFtdcVolumeType	VolumeTotal;
	///报单日期
	TSgitFtdcDateType	InsertDate;
	///委托时间
	TSgitFtdcTimeType	InsertTime;
	///激活时间
	TSgitFtdcTimeType	ActiveTime;
	///挂起时间
	TSgitFtdcTimeType	SuspendTime;
	///最后修改时间
	TSgitFtdcTimeType	UpdateTime;
	///撤销时间
	TSgitFtdcTimeType	CancelTime;
	///最后修改交易所交易员代码
	TSgitFtdcTraderIDType	ActiveTraderID;
	///结算会员编号
	TSgitFtdcParticipantIDType	ClearingPartID;
	///序号
	TSgitFtdcSequenceNoType	SequenceNo;
	///前置编号
	TSgitFtdcFrontIDType	FrontID;
	///会话编号
	TSgitFtdcSessionIDType	SessionID;
	///用户端产品信息
	TSgitFtdcProductInfoType	UserProductInfo;
	///状态信息
	TSgitFtdcErrorMsgType	StatusMsg;
	///用户强评标志
	TSgitFtdcBoolType	UserForceClose;
	///操作用户代码
	TSgitFtdcUserIDType	ActiveUserID;
	///经纪公司报单编号
	TSgitFtdcSequenceNoType	BrokerOrderSeq;
	///相关报单
	TSgitFtdcOrderSysIDType	RelativeOrderSysID;
	///郑商所成交数量
	TSgitFtdcVolumeType	ZCETotalTradedVolume;
	///互换单标志
	TSgitFtdcBoolType	IsSwapOrder;
	/////营业部编号
	//TSgitFtdcBranchIDType	BranchID;
	/////投资单元代码
	//TSgitFtdcInvestUnitIDType	InvestUnitID;
	///资金账号
	TSgitFtdcAccountIDType	AccountID;
	///币种代码
	TSgitFtdcCurrencyIDType	CurrencyID;
	///IP地址
	TSgitFtdcIPAddressType	IPAddress;
	///Mac地址
	TSgitFtdcMacAddressType	MacAddress;
	///是否是最后一个元素
	int IsLastElement;
};

///成交
struct THREAD_CSgitFtdcTradeField
{
	///经纪公司代码
	TSgitFtdcBrokerIDType	BrokerID;
	///投资者代码
	TSgitFtdcInvestorIDType	InvestorID;
	///合约代码
	TSgitFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TSgitFtdcOrderRefType	OrderRef;

	///策略编号
	char StrategyID[3];

	///用户代码
	TSgitFtdcUserIDType	UserID;
	///交易所代码
	TSgitFtdcExchangeIDType	ExchangeID;
	///成交编号
	TSgitFtdcTradeIDType	TradeID;
	///买卖方向
	TSgitFtdcDirectionType	Direction;
	///报单编号
	TSgitFtdcOrderSysIDType	OrderSysID;
	///会员代码
	TSgitFtdcParticipantIDType	ParticipantID;
	///客户代码
	TSgitFtdcClientIDType	ClientID;
	///交易角色
	TSgitFtdcTradingRoleType	TradingRole;
	///合约在交易所的代码
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///开平标志
	TSgitFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TSgitFtdcHedgeFlagType	HedgeFlag;
	///价格
	TSgitFtdcPriceType	Price;
	///数量
	TSgitFtdcVolumeType	Volume;
	///成交时期
	TSgitFtdcDateType	TradeDate;
	///成交时间
	TSgitFtdcTimeType	TradeTime;
	///成交类型
	TSgitFtdcTradeTypeType	TradeType;
	///成交价来源
	TSgitFtdcPriceSourceType	PriceSource;
	///交易所交易员代码
	TSgitFtdcTraderIDType	TraderID;
	///本地报单编号
	TSgitFtdcOrderLocalIDType	OrderLocalID;
	///结算会员编号
	TSgitFtdcParticipantIDType	ClearingPartID;
	///业务单元
	TSgitFtdcBusinessUnitType	BusinessUnit;
	///序号
	TSgitFtdcSequenceNoType	SequenceNo;
	///交易日
	TSgitFtdcDateType	TradingDay;
	///记录保存日期
	TSgitFtdcDateType	TradingDayRecord;
	///结算编号
	TSgitFtdcSettlementIDType	SettlementID;
	///经纪公司报单编号
	TSgitFtdcSequenceNoType	BrokerOrderSeq;
	///成交来源
	TSgitFtdcTradeSourceType	TradeSource;
	///是否是最后一个元素
	int IsLastElement;
};

///输入报单操作
struct THREAD_CSgitFtdcInputOrderActionField
{
	///经纪公司代码
	TSgitFtdcBrokerIDType	BrokerID;
	///投资者代码
	TSgitFtdcInvestorIDType	InvestorID;
	///报单操作引用
	TSgitFtdcOrderActionRefType	OrderActionRef;
	///报单引用
	TSgitFtdcOrderRefType	OrderRef;
	///请求编号
	TSgitFtdcRequestIDType	RequestID;
	///前置编号
	TSgitFtdcFrontIDType	FrontID;
	///会话编号
	TSgitFtdcSessionIDType	SessionID;
	///交易所代码
	TSgitFtdcExchangeIDType	ExchangeID;
	///报单编号
	TSgitFtdcOrderSysIDType	OrderSysID;
	///操作标志
	TSgitFtdcActionFlagType	ActionFlag;
	///价格
	TSgitFtdcPriceType	LimitPrice;
	///数量变化
	TSgitFtdcVolumeType	VolumeChange;
	///用户代码
	TSgitFtdcUserIDType	UserID;
	///合约代码
	TSgitFtdcInstrumentIDType	InstrumentID;
	///是否是最后一个元素
	int IsLastElement;
};

///成交
struct USER_CSgitFtdcTradeField
{
	///经纪公司代码
	TSgitFtdcBrokerIDType	BrokerID;
	///投资者代码
	TSgitFtdcInvestorIDType	InvestorID;
	///合约代码
	TSgitFtdcInstrumentIDType	InstrumentID;
	///策略编号
	char StrategyID[3];
	///报单引用
	TSgitFtdcOrderRefType	OrderRef;
	///用户代码
	TSgitFtdcUserIDType	UserID;
	///交易所代码
	TSgitFtdcExchangeIDType	ExchangeID;
	///成交编号
	TSgitFtdcTradeIDType	TradeID;
	///买卖方向
	TSgitFtdcDirectionType	Direction;
	///报单编号
	TSgitFtdcOrderSysIDType	OrderSysID;
	///会员代码
	TSgitFtdcParticipantIDType	ParticipantID;
	///客户代码
	TSgitFtdcClientIDType	ClientID;
	///交易角色
	TSgitFtdcTradingRoleType	TradingRole;
	///合约在交易所的代码
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///开平标志
	TSgitFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TSgitFtdcHedgeFlagType	HedgeFlag;
	///价格
	TSgitFtdcPriceType	Price;
	///数量
	TSgitFtdcVolumeType	Volume;
	///成交时期
	TSgitFtdcDateType	TradeDate;
	///成交时间
	TSgitFtdcTimeType	TradeTime;
	///成交类型
	TSgitFtdcTradeTypeType	TradeType;
	///成交价来源
	TSgitFtdcPriceSourceType	PriceSource;
	///交易所交易员代码
	TSgitFtdcTraderIDType	TraderID;
	///本地报单编号
	TSgitFtdcOrderLocalIDType	OrderLocalID;
	///结算会员编号
	TSgitFtdcParticipantIDType	ClearingPartID;
	///业务单元
	TSgitFtdcBusinessUnitType	BusinessUnit;
	///序号
	TSgitFtdcSequenceNoType	SequenceNo;
	///交易日
	TSgitFtdcDateType	TradingDay;
	///记录保存日期
	TSgitFtdcDateType	TradingDayRecord;
	///结算编号
	TSgitFtdcSettlementIDType	SettlementID;
	///经纪公司报单编号
	TSgitFtdcSequenceNoType	BrokerOrderSeq;
	///成交来源
	TSgitFtdcTradeSourceType	TradeSource;
};

struct USER_INSTRUMENT_POSITION
{
	/// 合约代码
	TSgitFtdcInstrumentIDType	InstrumentID;
	/// 今买
	int Buy_Today;
	/// 昨买
	int Buy_Yesterday;
	/// 今卖
	int Sell_Today;
	/// 昨卖
	int Sell_Yesterday;
	/// 判断结果
	bool Is_Same;
};

#endif
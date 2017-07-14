#ifndef QUANT_XTRADESTRUCT_H
#define QUANT_XTRADESTRUCT_H
#include <iostream>
#include "SgitFtdcUserApiDataType.h"

using namespace std;
using std::string;

///����
struct USER_CSgitFtdcOrderField
{
	///���͹�˾����
	TSgitFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSgitFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSgitFtdcInstrumentIDType	InstrumentID;
	///��������
	TSgitFtdcOrderRefType	OrderRef;
	/*///���Ա��
	string StrategyID;*/
	///���Ա��
	char StrategyID[3];
	///�û�����
	TSgitFtdcUserIDType	UserID;
	///�����۸�����
	TSgitFtdcOrderPriceTypeType	OrderPriceType;
	///��������
	TSgitFtdcDirectionType	Direction;
	///��Ͽ�ƽ��־
	TSgitFtdcCombOffsetFlagType	CombOffsetFlag;
	///���Ͷ���ױ���־
	TSgitFtdcCombHedgeFlagType	CombHedgeFlag;
	///�۸�
	TSgitFtdcPriceType	LimitPrice;
	///����
	TSgitFtdcVolumeType	VolumeTotalOriginal;
	///��Ч������
	TSgitFtdcTimeConditionType	TimeCondition;
	///GTD����
	TSgitFtdcDateType	GTDDate;
	///�ɽ�������
	TSgitFtdcVolumeConditionType	VolumeCondition;
	///��С�ɽ���
	TSgitFtdcVolumeType	MinVolume;
	///��������
	TSgitFtdcContingentConditionType	ContingentCondition;
	///ֹ���
	TSgitFtdcPriceType	StopPrice;
	///ǿƽԭ��
	TSgitFtdcForceCloseReasonType	ForceCloseReason;
	///�Զ������־
	TSgitFtdcBoolType	IsAutoSuspend;
	///ҵ��Ԫ
	TSgitFtdcBusinessUnitType	BusinessUnit;
	///������
	TSgitFtdcRequestIDType	RequestID;
	///���ر������
	TSgitFtdcOrderLocalIDType	OrderLocalID;
	///����������
	TSgitFtdcExchangeIDType	ExchangeID;
	///��Ա����
	TSgitFtdcParticipantIDType	ParticipantID;
	///�ͻ�����
	TSgitFtdcClientIDType	ClientID;
	///��Լ�ڽ������Ĵ���
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///����������Ա����
	TSgitFtdcTraderIDType	TraderID;
	///��װ���
	TSgitFtdcInstallIDType	InstallID;
	///�����ύ״̬
	TSgitFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///������ʾ���
	TSgitFtdcSequenceNoType	NotifySequence;
	///������
	TSgitFtdcDateType	TradingDay;
	///��¼��������
	TSgitFtdcDateType	TradingDayRecord;
	///������
	TSgitFtdcSettlementIDType	SettlementID;
	///�������
	TSgitFtdcOrderSysIDType	OrderSysID;
	///������Դ
	TSgitFtdcOrderSourceType	OrderSource;
	///����״̬
	TSgitFtdcOrderStatusType	OrderStatus;
	///��������
	TSgitFtdcOrderTypeType	OrderType;
	///��ɽ�����
	TSgitFtdcVolumeType	VolumeTraded;
	///���γɽ���
	TSgitFtdcVolumeType	VolumeTradedBatch;
	///ʣ������
	TSgitFtdcVolumeType	VolumeTotal;
	///��������
	TSgitFtdcDateType	InsertDate;
	///ί��ʱ��
	TSgitFtdcTimeType	InsertTime;
	///����ʱ��
	TSgitFtdcTimeType	ActiveTime;
	///����ʱ��
	TSgitFtdcTimeType	SuspendTime;
	///����޸�ʱ��
	TSgitFtdcTimeType	UpdateTime;
	///����ʱ��
	TSgitFtdcTimeType	CancelTime;
	///����޸Ľ���������Ա����
	TSgitFtdcTraderIDType	ActiveTraderID;
	///�����Ա���
	TSgitFtdcParticipantIDType	ClearingPartID;
	///���
	TSgitFtdcSequenceNoType	SequenceNo;
	///ǰ�ñ��
	TSgitFtdcFrontIDType	FrontID;
	///�Ự���
	TSgitFtdcSessionIDType	SessionID;
	///�û��˲�Ʒ��Ϣ
	TSgitFtdcProductInfoType	UserProductInfo;
	///״̬��Ϣ
	TSgitFtdcErrorMsgType	StatusMsg;
	///�û�ǿ����־
	TSgitFtdcBoolType	UserForceClose;
	///�����û�����
	TSgitFtdcUserIDType	ActiveUserID;
	///���͹�˾�������
	TSgitFtdcSequenceNoType	BrokerOrderSeq;
	///��ر���
	TSgitFtdcOrderSysIDType	RelativeOrderSysID;
	///֣�����ɽ�����
	TSgitFtdcVolumeType	ZCETotalTradedVolume;
	///��������־
	TSgitFtdcBoolType	IsSwapOrder;
	/////Ӫҵ�����
	//TSgitFtdcBranchIDType	BranchID;
	/////Ͷ�ʵ�Ԫ����
	//TSgitFtdcInvestUnitIDType	InvestUnitID;
	///�ʽ��˺�
	TSgitFtdcAccountIDType	AccountID;
	///���ִ���
	TSgitFtdcCurrencyIDType	CurrencyID;
	///IP��ַ
	TSgitFtdcIPAddressType	IPAddress;
	///Mac��ַ
	TSgitFtdcMacAddressType	MacAddress;

	//USER_CSgitFtdcOrderField(){}
	//~USER_CSgitFtdcOrderField(){}

};

///�������
struct THREAD_CSgitFtdcDepthMarketDataField
{
	///������
	TSgitFtdcDateType	TradingDay;
	///��Լ����
	TSgitFtdcInstrumentIDType	InstrumentID;
	///����������
	TSgitFtdcExchangeIDType	ExchangeID;
	///��Լ�ڽ������Ĵ���
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///���¼�
	TSgitFtdcPriceType	LastPrice;
	///�ϴν����
	TSgitFtdcPriceType	PreSettlementPrice;
	///������
	TSgitFtdcPriceType	PreClosePrice;
	///��ֲ���
	TSgitFtdcLargeVolumeType	PreOpenInterest;
	///����
	TSgitFtdcPriceType	OpenPrice;
	///��߼�
	TSgitFtdcPriceType	HighestPrice;
	///��ͼ�
	TSgitFtdcPriceType	LowestPrice;
	///����
	TSgitFtdcVolumeType	Volume;
	///�ɽ����
	TSgitFtdcMoneyType	Turnover;
	///�ֲ���
	TSgitFtdcLargeVolumeType	OpenInterest;
	///������
	TSgitFtdcPriceType	ClosePrice;
	///���ν����
	TSgitFtdcPriceType	SettlementPrice;
	///��ͣ���
	TSgitFtdcPriceType	UpperLimitPrice;
	///��ͣ���
	TSgitFtdcPriceType	LowerLimitPrice;
	///����ʵ��
	TSgitFtdcRatioType	PreDelta;
	///����ʵ��
	TSgitFtdcRatioType	CurrDelta;
	///����޸�ʱ��
	TSgitFtdcTimeType	UpdateTime;
	///����޸ĺ���
	TSgitFtdcMillisecType	UpdateMillisec;
	///�����һ
	TSgitFtdcPriceType	BidPrice1;
	///������һ
	TSgitFtdcVolumeType	BidVolume1;
	///������һ
	TSgitFtdcPriceType	AskPrice1;
	///������һ
	TSgitFtdcVolumeType	AskVolume1;
	///����۶�
	TSgitFtdcPriceType	BidPrice2;
	///��������
	TSgitFtdcVolumeType	BidVolume2;
	///�����۶�
	TSgitFtdcPriceType	AskPrice2;
	///��������
	TSgitFtdcVolumeType	AskVolume2;
	///�������
	TSgitFtdcPriceType	BidPrice3;
	///��������
	TSgitFtdcVolumeType	BidVolume3;
	///��������
	TSgitFtdcPriceType	AskPrice3;
	///��������
	TSgitFtdcVolumeType	AskVolume3;
	///�������
	TSgitFtdcPriceType	BidPrice4;
	///��������
	TSgitFtdcVolumeType	BidVolume4;
	///��������
	TSgitFtdcPriceType	AskPrice4;
	///��������
	TSgitFtdcVolumeType	AskVolume4;
	///�������
	TSgitFtdcPriceType	BidPrice5;
	///��������
	TSgitFtdcVolumeType	BidVolume5;
	///��������
	TSgitFtdcPriceType	AskPrice5;
	///��������
	TSgitFtdcVolumeType	AskVolume5;
	///���վ���
	TSgitFtdcPriceType	AveragePrice;
	///ҵ������
	TSgitFtdcDateType	ActionDay;
	///�Ƿ������һ��Ԫ��
	int IsLastElement;
};


///����
struct THREAD_CSgitFtdcOrderField
{
	///���͹�˾����
	TSgitFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSgitFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSgitFtdcInstrumentIDType	InstrumentID;
	///��������
	TSgitFtdcOrderRefType	OrderRef;
	///�û�����
	TSgitFtdcUserIDType	UserID;
	///�����۸�����
	TSgitFtdcOrderPriceTypeType	OrderPriceType;
	///��������
	TSgitFtdcDirectionType	Direction;
	///��Ͽ�ƽ��־
	TSgitFtdcCombOffsetFlagType	CombOffsetFlag;
	///���Ͷ���ױ���־
	TSgitFtdcCombHedgeFlagType	CombHedgeFlag;
	///�۸�
	TSgitFtdcPriceType	LimitPrice;
	///����
	TSgitFtdcVolumeType	VolumeTotalOriginal;
	///��Ч������
	TSgitFtdcTimeConditionType	TimeCondition;
	///GTD����
	TSgitFtdcDateType	GTDDate;
	///�ɽ�������
	TSgitFtdcVolumeConditionType	VolumeCondition;
	///��С�ɽ���
	TSgitFtdcVolumeType	MinVolume;
	///��������
	TSgitFtdcContingentConditionType	ContingentCondition;
	///ֹ���
	TSgitFtdcPriceType	StopPrice;
	///ǿƽԭ��
	TSgitFtdcForceCloseReasonType	ForceCloseReason;
	///�Զ������־
	TSgitFtdcBoolType	IsAutoSuspend;
	///ҵ��Ԫ
	TSgitFtdcBusinessUnitType	BusinessUnit;
	///������
	TSgitFtdcRequestIDType	RequestID;
	///���ر������
	TSgitFtdcOrderLocalIDType	OrderLocalID;
	///����������
	TSgitFtdcExchangeIDType	ExchangeID;
	///��Ա����
	TSgitFtdcParticipantIDType	ParticipantID;
	///�ͻ�����
	TSgitFtdcClientIDType	ClientID;
	///��Լ�ڽ������Ĵ���
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///����������Ա����
	TSgitFtdcTraderIDType	TraderID;
	///��װ���
	TSgitFtdcInstallIDType	InstallID;
	///�����ύ״̬
	TSgitFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///������ʾ���
	TSgitFtdcSequenceNoType	NotifySequence;
	///������
	TSgitFtdcDateType	TradingDay;
	///��¼��������
	TSgitFtdcDateType	TradingDayRecord;
	///������
	TSgitFtdcSettlementIDType	SettlementID;
	///�������
	TSgitFtdcOrderSysIDType	OrderSysID;
	///������Դ
	TSgitFtdcOrderSourceType	OrderSource;
	///����״̬
	TSgitFtdcOrderStatusType	OrderStatus;
	///��������
	TSgitFtdcOrderTypeType	OrderType;
	///��ɽ�����
	TSgitFtdcVolumeType	VolumeTraded;
	///ʣ������
	TSgitFtdcVolumeType	VolumeTotal;
	///��������
	TSgitFtdcDateType	InsertDate;
	///ί��ʱ��
	TSgitFtdcTimeType	InsertTime;
	///����ʱ��
	TSgitFtdcTimeType	ActiveTime;
	///����ʱ��
	TSgitFtdcTimeType	SuspendTime;
	///����޸�ʱ��
	TSgitFtdcTimeType	UpdateTime;
	///����ʱ��
	TSgitFtdcTimeType	CancelTime;
	///����޸Ľ���������Ա����
	TSgitFtdcTraderIDType	ActiveTraderID;
	///�����Ա���
	TSgitFtdcParticipantIDType	ClearingPartID;
	///���
	TSgitFtdcSequenceNoType	SequenceNo;
	///ǰ�ñ��
	TSgitFtdcFrontIDType	FrontID;
	///�Ự���
	TSgitFtdcSessionIDType	SessionID;
	///�û��˲�Ʒ��Ϣ
	TSgitFtdcProductInfoType	UserProductInfo;
	///״̬��Ϣ
	TSgitFtdcErrorMsgType	StatusMsg;
	///�û�ǿ����־
	TSgitFtdcBoolType	UserForceClose;
	///�����û�����
	TSgitFtdcUserIDType	ActiveUserID;
	///���͹�˾�������
	TSgitFtdcSequenceNoType	BrokerOrderSeq;
	///��ر���
	TSgitFtdcOrderSysIDType	RelativeOrderSysID;
	///֣�����ɽ�����
	TSgitFtdcVolumeType	ZCETotalTradedVolume;
	///��������־
	TSgitFtdcBoolType	IsSwapOrder;
	/////Ӫҵ�����
	//TSgitFtdcBranchIDType	BranchID;
	/////Ͷ�ʵ�Ԫ����
	//TSgitFtdcInvestUnitIDType	InvestUnitID;
	///�ʽ��˺�
	TSgitFtdcAccountIDType	AccountID;
	///���ִ���
	TSgitFtdcCurrencyIDType	CurrencyID;
	///IP��ַ
	TSgitFtdcIPAddressType	IPAddress;
	///Mac��ַ
	TSgitFtdcMacAddressType	MacAddress;
	///�Ƿ������һ��Ԫ��
	int IsLastElement;
};

///�ɽ�
struct THREAD_CSgitFtdcTradeField
{
	///���͹�˾����
	TSgitFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSgitFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSgitFtdcInstrumentIDType	InstrumentID;
	///��������
	TSgitFtdcOrderRefType	OrderRef;

	///���Ա��
	char StrategyID[3];

	///�û�����
	TSgitFtdcUserIDType	UserID;
	///����������
	TSgitFtdcExchangeIDType	ExchangeID;
	///�ɽ����
	TSgitFtdcTradeIDType	TradeID;
	///��������
	TSgitFtdcDirectionType	Direction;
	///�������
	TSgitFtdcOrderSysIDType	OrderSysID;
	///��Ա����
	TSgitFtdcParticipantIDType	ParticipantID;
	///�ͻ�����
	TSgitFtdcClientIDType	ClientID;
	///���׽�ɫ
	TSgitFtdcTradingRoleType	TradingRole;
	///��Լ�ڽ������Ĵ���
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///��ƽ��־
	TSgitFtdcOffsetFlagType	OffsetFlag;
	///Ͷ���ױ���־
	TSgitFtdcHedgeFlagType	HedgeFlag;
	///�۸�
	TSgitFtdcPriceType	Price;
	///����
	TSgitFtdcVolumeType	Volume;
	///�ɽ�ʱ��
	TSgitFtdcDateType	TradeDate;
	///�ɽ�ʱ��
	TSgitFtdcTimeType	TradeTime;
	///�ɽ�����
	TSgitFtdcTradeTypeType	TradeType;
	///�ɽ�����Դ
	TSgitFtdcPriceSourceType	PriceSource;
	///����������Ա����
	TSgitFtdcTraderIDType	TraderID;
	///���ر������
	TSgitFtdcOrderLocalIDType	OrderLocalID;
	///�����Ա���
	TSgitFtdcParticipantIDType	ClearingPartID;
	///ҵ��Ԫ
	TSgitFtdcBusinessUnitType	BusinessUnit;
	///���
	TSgitFtdcSequenceNoType	SequenceNo;
	///������
	TSgitFtdcDateType	TradingDay;
	///��¼��������
	TSgitFtdcDateType	TradingDayRecord;
	///������
	TSgitFtdcSettlementIDType	SettlementID;
	///���͹�˾�������
	TSgitFtdcSequenceNoType	BrokerOrderSeq;
	///�ɽ���Դ
	TSgitFtdcTradeSourceType	TradeSource;
	///�Ƿ������һ��Ԫ��
	int IsLastElement;
};

///���뱨������
struct THREAD_CSgitFtdcInputOrderActionField
{
	///���͹�˾����
	TSgitFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSgitFtdcInvestorIDType	InvestorID;
	///������������
	TSgitFtdcOrderActionRefType	OrderActionRef;
	///��������
	TSgitFtdcOrderRefType	OrderRef;
	///������
	TSgitFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	TSgitFtdcFrontIDType	FrontID;
	///�Ự���
	TSgitFtdcSessionIDType	SessionID;
	///����������
	TSgitFtdcExchangeIDType	ExchangeID;
	///�������
	TSgitFtdcOrderSysIDType	OrderSysID;
	///������־
	TSgitFtdcActionFlagType	ActionFlag;
	///�۸�
	TSgitFtdcPriceType	LimitPrice;
	///�����仯
	TSgitFtdcVolumeType	VolumeChange;
	///�û�����
	TSgitFtdcUserIDType	UserID;
	///��Լ����
	TSgitFtdcInstrumentIDType	InstrumentID;
	///�Ƿ������һ��Ԫ��
	int IsLastElement;
};

///�ɽ�
struct USER_CSgitFtdcTradeField
{
	///���͹�˾����
	TSgitFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSgitFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSgitFtdcInstrumentIDType	InstrumentID;
	///���Ա��
	char StrategyID[3];
	///��������
	TSgitFtdcOrderRefType	OrderRef;
	///�û�����
	TSgitFtdcUserIDType	UserID;
	///����������
	TSgitFtdcExchangeIDType	ExchangeID;
	///�ɽ����
	TSgitFtdcTradeIDType	TradeID;
	///��������
	TSgitFtdcDirectionType	Direction;
	///�������
	TSgitFtdcOrderSysIDType	OrderSysID;
	///��Ա����
	TSgitFtdcParticipantIDType	ParticipantID;
	///�ͻ�����
	TSgitFtdcClientIDType	ClientID;
	///���׽�ɫ
	TSgitFtdcTradingRoleType	TradingRole;
	///��Լ�ڽ������Ĵ���
	TSgitFtdcExchangeInstIDType	ExchangeInstID;
	///��ƽ��־
	TSgitFtdcOffsetFlagType	OffsetFlag;
	///Ͷ���ױ���־
	TSgitFtdcHedgeFlagType	HedgeFlag;
	///�۸�
	TSgitFtdcPriceType	Price;
	///����
	TSgitFtdcVolumeType	Volume;
	///�ɽ�ʱ��
	TSgitFtdcDateType	TradeDate;
	///�ɽ�ʱ��
	TSgitFtdcTimeType	TradeTime;
	///�ɽ�����
	TSgitFtdcTradeTypeType	TradeType;
	///�ɽ�����Դ
	TSgitFtdcPriceSourceType	PriceSource;
	///����������Ա����
	TSgitFtdcTraderIDType	TraderID;
	///���ر������
	TSgitFtdcOrderLocalIDType	OrderLocalID;
	///�����Ա���
	TSgitFtdcParticipantIDType	ClearingPartID;
	///ҵ��Ԫ
	TSgitFtdcBusinessUnitType	BusinessUnit;
	///���
	TSgitFtdcSequenceNoType	SequenceNo;
	///������
	TSgitFtdcDateType	TradingDay;
	///��¼��������
	TSgitFtdcDateType	TradingDayRecord;
	///������
	TSgitFtdcSettlementIDType	SettlementID;
	///���͹�˾�������
	TSgitFtdcSequenceNoType	BrokerOrderSeq;
	///�ɽ���Դ
	TSgitFtdcTradeSourceType	TradeSource;
};

struct USER_INSTRUMENT_POSITION
{
	/// ��Լ����
	TSgitFtdcInstrumentIDType	InstrumentID;
	/// ����
	int Buy_Today;
	/// ����
	int Buy_Yesterday;
	/// ����
	int Sell_Today;
	/// ����
	int Sell_Yesterday;
	/// �жϽ��
	bool Is_Same;
};

#endif
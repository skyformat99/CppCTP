#include <algorithm>
#include <sstream>
#include <mutex>
#include "Strategy.h"

std::mutex tick_mtx; // locks access to counter
std::mutex update_status_mtx; // locks access to counter
std::mutex select_order_algorithm_mtx; // locks access to counter

Strategy::Strategy(User *stg_user) {

	this->on_off = 0;						//开关					
	this->stg_only_close = 0;				//只能平仓
	this->sell_open_on_off = 1;				//卖开-开关
	this->buy_close_on_off = 1;				//买平-开关
	this->buy_open_on_off = 1;				//买开-开关
	this->sell_close_on_off = 1;			//卖平-开关
	this->stg_a_limit_price_shift = 0;
	this->stg_b_limit_price_shift = 0;
	this->stg_is_active = true;				//默认策略均为激活状态
	this->stg_trading_day = "";
	this->stg_pending_a_open = 0;			//A开仓挂单
	this->stg_select_order_algorithm_flag = false; //默认允许选择下单算法锁为false，可以选择
	this->stg_lock_order_ref = "";

	this->stg_user = stg_user;					// 默认用户为空

	this->l_instruments = new list<string>();

	this->stg_list_order_pending = new list<CThostFtdcOrderField *>();
	this->stg_list_position_detail = new list<CThostFtdcTradeField *>();
	this->stg_list_position_detail_from_order = new list<USER_CThostFtdcOrderField *>();

	this->stg_a_order_insert_args = new CThostFtdcInputOrderField();
	this->stg_b_order_insert_args = new CThostFtdcInputOrderField();

	stg_instrument_A_tick = new CThostFtdcDepthMarketDataField();	// A合约tick（第一腿）
	stg_instrument_B_tick = new CThostFtdcDepthMarketDataField();	// B合约tick（第二腿）
	stg_instrument_A_tick_last = new CThostFtdcDepthMarketDataField();	// A合约tick（第一腿）交易前最后一次
	stg_instrument_B_tick_last = new CThostFtdcDepthMarketDataField();	// B合约tick（第二腿）交易前最后一次

	this->stg_trade_tasking = false;
	init_finished = false;

	this->l_query_order = new list<USER_CThostFtdcOrderField *>();
}



/// 初始化
void Strategy::InitStrategy() {
	
}

/// 拷贝结构体CThostFtdcDepthMarketDataField
void Strategy::CopyTickData(CThostFtdcDepthMarketDataField *dst, CThostFtdcDepthMarketDataField *src) {
	///交易日
	strcpy(dst->TradingDay, src->TradingDay);
	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);
	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);
	///合约在交易所的代码
	strcpy(dst->ExchangeInstID, src->ExchangeInstID);
	///最新价
	dst->LastPrice = src->LastPrice;
	///上次结算价
	dst->PreSettlementPrice = src->PreSettlementPrice;
	///昨收盘
	dst->PreClosePrice = src->PreClosePrice;
	///昨持仓量
	dst->PreOpenInterest = src->PreOpenInterest;
	///今开盘
	dst->OpenPrice = src->OpenPrice;
	///最高价
	dst->HighestPrice = src->HighestPrice;
	///最低价
	dst->LowestPrice = src->LowestPrice;
	///数量
	dst->Volume = src->Volume;
	///成交金额
	dst->Turnover = src->Turnover;
	///持仓量
	dst->OpenInterest = src->OpenInterest;
	///今收盘
	dst->ClosePrice = src->ClosePrice;
	///本次结算价
	dst->SettlementPrice = src->SettlementPrice;
	///涨停板价
	dst->UpperLimitPrice = src->UpperLimitPrice;
	///跌停板价
	dst->LowerLimitPrice = src->LowerLimitPrice;
	///昨虚实度
	dst->PreDelta = src->PreDelta;
	///今虚实度
	dst->CurrDelta = src->CurrDelta;
	///最后修改时间
	strcpy(dst->UpdateTime, src->UpdateTime);
	///最后修改毫秒
	dst->UpdateMillisec = src->UpdateMillisec;
	///申买价一
	dst->BidPrice1 = src->BidPrice1;
	///申买量一
	dst->BidVolume1 = src->BidVolume1;
	///申卖价一
	dst->AskPrice1 = src->AskPrice1;
	///申卖量一
	dst->AskVolume1 = src->AskVolume1;
	///申买价二
	dst->BidPrice2 = src->BidPrice2;
	///申买量二
	dst->BidVolume2 = src->BidVolume2;
	///申卖价二
	dst->AskPrice2 = src->AskPrice2;
	///申卖量二
	dst->AskVolume2 = src->AskVolume2;
	///申买价三
	dst->BidPrice3 = src->BidPrice3;
	///申买量三
	dst->BidVolume3 = src->BidVolume3;
	///申卖价三
	dst->AskPrice3 = src->AskPrice3;
	///申卖量三
	dst->AskVolume3 = src->AskVolume3;
	///申买价四
	dst->BidPrice4 = src->BidPrice4;
	///申买量四
	dst->BidVolume4 = src->BidVolume4;
	///申卖价四
	dst->AskPrice4 = src->AskPrice4;
	///申卖量四
	dst->AskVolume4 = src->AskVolume4;
	///申买价五
	dst->BidPrice5 = src->BidPrice5;
	///申买量五
	dst->BidVolume5 = src->BidVolume5;
	///申卖价五
	dst->AskPrice5 = src->AskPrice5;
	///申卖量五
	dst->AskVolume5 = src->AskVolume5;
	///当日均价
	dst->AveragePrice = src->AveragePrice;
	///业务日期
	strcpy(dst->ActionDay, src->ActionDay);
}

/// 拷贝结构体CThostFtdcOrderField
void Strategy::CopyOrderData(CThostFtdcOrderField *dst, CThostFtdcOrderField *src) {
	
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

void Strategy::CopyOrderDataToNew(USER_CThostFtdcOrderField *dst, CThostFtdcOrderField *src) {

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

/// 拷贝结构体USER_CThostFtdcOrderField
void Strategy::CopyNewOrderData(USER_CThostFtdcOrderField *dst, USER_CThostFtdcOrderField *src) {

	///经纪公司代码
	strcpy(dst->BrokerID, src->BrokerID);

	///投资者代码
	strcpy(dst->InvestorID, src->InvestorID);

	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);

	///报单引用
	strcpy(dst->OrderRef, src->OrderRef);

	///策略ID
	strcpy(dst->StrategyID, src->StrategyID);

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

	///本次成交量
	dst->VolumeTradedBatch = src->VolumeTradedBatch;

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

/// 拷贝结构体CThostFtdcTradeField
void Strategy::CopyTradeData(CThostFtdcTradeField *dst, CThostFtdcTradeField *src) {
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

/// 设置开关
int Strategy::getOn_Off() {
	return this->on_off;
}

void Strategy::setOn_Off(int on_off) {
	this->on_off = on_off;
}

void Strategy::setStgSellOpenOnOff(int sell_open_on_off) {
	this->sell_open_on_off = sell_open_on_off;
}

int Strategy::getStgSellOpenOnOff() {
	return this->sell_open_on_off;
}

void Strategy::setStgBuyCloseOnOff(int buy_close_on_off) {
	this->buy_close_on_off = buy_close_on_off;
}

int Strategy::getStgBuyCloseOnOff() {
	return this->buy_close_on_off;
}

void Strategy::setStgBuyOpenOnOff(int buy_open_on_off) {
	this->buy_open_on_off = buy_open_on_off;
}

int Strategy::getStgBuyOpenOnOff() {
	return this->buy_open_on_off;
}

void Strategy::setStgSellCloseOnOff(int sell_close_on_off) {
	this->sell_close_on_off = sell_close_on_off;
}

int Strategy::getStgSellCloseOnOff() {
	return this->sell_close_on_off;
}

/// 更新交易状态
void Strategy::update_task_status() {
	//update_status_mtx.lock();
	USER_PRINT("Strategy::update_task_status");
	USER_PRINT(this->stg_trade_tasking);

	/*std::cout << "Strategy::update_task_status() A今买 = " << stg_position_a_buy_today << std::endl;
	std::cout << "Strategy::update_task_status() B今卖 = " << stg_position_b_sell_today << std::endl;
	std::cout << "Strategy::update_task_status() A昨买 = " << stg_position_a_buy_yesterday << std::endl;
	std::cout << "Strategy::update_task_status() B昨卖 = " << stg_position_b_sell_yesterday << std::endl;
	std::cout << "Strategy::update_task_status() A今卖 = " << stg_position_a_sell_today << std::endl;
	std::cout << "Strategy::update_task_status() B今买 = " << stg_position_b_buy_today << std::endl;
	std::cout << "Strategy::update_task_status() A昨卖 = " << stg_position_a_sell_yesterday << std::endl;
	std::cout << "Strategy::update_task_status() B昨买 = " << stg_position_b_buy_yesterday << std::endl;*/

	std::cout << "Strategy::update_task_status():" << std::endl;
	std::cout << "\t(this->stg_position_a_buy_today == this->stg_position_b_sell_today)(" << this->stg_position_a_buy_today << ", " << this->stg_position_b_sell_today << ")" << std::endl;
	std::cout << "\t(this->stg_position_a_buy_yesterday == this->stg_position_b_sell_yesterday)(" << this->stg_position_a_buy_yesterday << ", " << this->stg_position_b_sell_yesterday << ")" << std::endl;
	std::cout << "\t(this->stg_position_a_sell_today == this->stg_position_b_buy_today)(" << this->stg_position_a_sell_today << ", " << this->stg_position_b_buy_today << ")" << std::endl;
	std::cout << "\t(this->stg_position_a_sell_yesterday == this->stg_position_b_buy_yesterday)(" << this->stg_position_a_sell_yesterday << ", " << this->stg_position_b_buy_yesterday << ")" << std::endl;
	std::cout << "\t(this->stg_list_order_pending->size() == 0)(" << this->stg_list_order_pending->size() << ", " << 0 << ")" << std::endl;
	

	if ((this->stg_position_a_buy_today == this->stg_position_b_sell_today)
		&& (this->stg_position_a_buy_yesterday == this->stg_position_b_sell_yesterday)
		&& (this->stg_position_a_sell_today == this->stg_position_b_buy_today)
		&& (this->stg_position_a_sell_yesterday == this->stg_position_b_buy_yesterday)
		&& (this->stg_list_order_pending->size() == 0)) {
		/*this->printStrategyInfo("更新交易状态");
		this->printStrategyInfoPosition();*/
		this->stg_trade_tasking = false;
	}
	else
	{
		//this->printStrategyInfo("Strategy::update_task_status() 更新交易状态");
		/*std::cout << "stg_position_a_buy_today = " << stg_position_a_buy_today << std::endl;
		std::cout << "stg_position_b_sell_today = " << stg_position_b_sell_today << std::endl;
		std::cout << "stg_position_a_buy_yesterday = " << stg_position_a_buy_yesterday << std::endl;
		std::cout << "stg_position_b_sell_yesterday = " << stg_position_b_sell_yesterday << std::endl;
		std::cout << "stg_position_a_sell_today = " << stg_position_a_sell_today << std::endl;
		std::cout << "stg_position_b_buy_today = " << stg_position_b_buy_today << std::endl;
		std::cout << "stg_position_a_sell_yesterday = " << stg_position_a_sell_yesterday << std::endl;
		std::cout << "stg_position_b_buy_yesterday = " << stg_position_b_buy_yesterday << std::endl;
		std::cout << "this->stg_list_order_pending->size() = " << this->stg_list_order_pending->size() << std::endl;*/

		this->stg_trade_tasking = true;
	}
	//std::cout << "After update this.trade_tasking = " << this->stg_trade_tasking << endl;
	std::cout << "\t挂单列表长度 = " << this->stg_list_order_pending->size() << std::endl;
	std::cout << "\t任务执行状态 = " << this->stg_trade_tasking << std::endl;
	//update_status_mtx.unlock();
	USER_PRINT(this->stg_trade_tasking);
}

/// 更新tick锁
void Strategy::update_tick_lock_status(USER_CThostFtdcOrderField *pOrder) {
	std::cout << "Strategy::update_tick_lock_status():" << std::endl;

	bool flag = false; // 默认关闭tick锁

	if (pOrder->OrderStatus == '0') { // 全部成交
	}
	else if (pOrder->OrderStatus == '1') { // 部分成交还在队列中

	}
	else if (pOrder->OrderStatus == '2') { // 部分成交不在队列中

	}
	else if (pOrder->OrderStatus == '3') { // 未成交还在队列中
	
	}
	else if (pOrder->OrderStatus == '4') { // 未成交不在队列中

	}
	else if (pOrder->OrderStatus == '5') { // 撤单
	
	}
	else if (pOrder->OrderStatus == 'a') { // 未知
	
	}
	else if (pOrder->OrderStatus == 'b') { // 尚未触发

	}
	else if (pOrder->OrderStatus == 'c') { // 已触发

	}
	else {
		// 当报单状态不在以上之中,依然打开锁
		flag = true;
	}

	std::cout << "\t tick flag = " << flag << std::endl;
	this->setStgSelectOrderAlgorithmFlag("Strategy::update_tick_lock_status()", flag); // tick锁
}

/// 交易模型
void Strategy::setStgTradeModel(string stg_trade_model) {
	this->stg_trade_model = stg_trade_model;
}

string Strategy::getStgTradeModel() {
	return this->stg_trade_model;
}

string Strategy::getStgOrderAlgorithm() {
	return this->stg_order_algorithm;
}		// 下单算法

void Strategy::setStgOrderAlgorithm(string stg_order_algorithm) {
	this->stg_order_algorithm = stg_order_algorithm;
}

double Strategy::getStgHoldProfit() {
	return this->stg_hold_profit;
}				// 持仓盈亏

void Strategy::setStgHoldProfit(double stg_hold_profit) {
	this->stg_hold_profit = stg_hold_profit;
}

double Strategy::getStgCloseProfit() {
	return this->stg_close_profit;
}			// 平仓盈亏

void Strategy::setStgCloseProfit(double stg_close_profit) {
	this->stg_close_profit = stg_close_profit;
}

double Strategy::getStgCommission() {
	return this->stg_commission;
}				// 手续费

void Strategy::setStgCommisstion(double stg_commission) {
	this->stg_commission = stg_commission;
}

int Strategy::getStgPosition() {
	return this->stg_position;
}					// 总持仓

void Strategy::setStgPosition(int stg_position) {
	this->stg_position = stg_position;
}

int Strategy::getStgPositionBuy() {
	return this->stg_position_buy;
}				// 买持仓

void Strategy::setStgPositionBuy(int stg_position_buy) {
	this->stg_position_buy = stg_position_buy;
}

int Strategy::getStgPositionSell() {
	return this->stg_position_sell;
}				// 卖持仓

int Strategy::setStgPositionSell(int stg_position_sell) {
	this->stg_position_sell = stg_position_sell;
}

int Strategy::getStgTradeVolume() {
	return this->stg_trade_volume;
}			// 成交量

int Strategy::setStgTradeVolume(int stg_trade_volume) {
	this->stg_trade_volume = stg_trade_volume;
}

double Strategy::getStgAmount() {
	return this->stg_amount;
}					// 成交金额

void Strategy::setStgAmount(double stg_amount) {
	this->stg_amount = stg_amount;
}

double Strategy::getStgAverageShift() { // 平均滑点
	return this->stg_average_shift;
}
	

void Strategy::setStgAverageShift(double stg_average_shift) {
	this->stg_average_shift = stg_average_shift;
}

void Strategy::setInit_Finished(bool init_finished) {
	this->init_finished = init_finished;
}

bool Strategy::getInit_Finished() {
	return this->init_finished;
}


// 初始化今仓
void Strategy::init_today_position() {
	USER_PRINT("init_today_position");
	this->stg_user->getUserTradeSPI()->QryTrade();
}

void Strategy::setL_query_trade(list<CThostFtdcTradeField *> *l_query_trade) {
	USER_PRINT("Strategy::setL_query_trade");
	this->l_query_trade = l_query_trade;
	// 遍历进行今仓初始化
	//std::cout << "l_query_trade->size() = " << l_query_trade->size()  << std::endl;
	if (l_query_trade->size() > 0) {
		list<CThostFtdcTradeField *>::iterator Itor;
		for (Itor = l_query_trade->begin(); Itor != l_query_trade->end(); Itor++) {
			if (!strcmp((*Itor)->InstrumentID, this->stg_instrument_id_A.c_str())) {
				if ((*Itor)->OffsetFlag == '0') { // A开仓成交回报
					if ((*Itor)->Direction == '0') { /*A买开仓*/
						this->stg_position_a_buy_today += (*Itor)->Volume;
					}
					else if ((*Itor)->Direction == '1') { /*A卖开仓*/
						this->stg_position_a_sell_today += (*Itor)->Volume;
					}
				}
				else if ((*Itor)->OffsetFlag == '3') { // A平今成交回报
					if ((*Itor)->Direction == '0') { /*A买平今*/
						this->stg_position_a_sell_today -= (*Itor)->Volume;
					}
					else if ((*Itor)->Direction == '1') { /*A卖平今*/
						this->stg_position_a_buy_today -= (*Itor)->Volume;
					}
				}
				else if ((*Itor)->OffsetFlag == '4') { // A平昨成交回报
					if ((*Itor)->Direction == '0') { /*A买平昨*/
						this->stg_position_a_sell_yesterday -= (*Itor)->Volume;
					}
					else if ((*Itor)->Direction == '1') { /*A卖平昨*/
						this->stg_position_a_buy_yesterday -= (*Itor)->Volume;
					}
				}
				this->stg_position_a_buy = this->stg_position_a_buy_today + this->stg_position_a_buy_yesterday;
				this->stg_position_a_sell = this->stg_position_a_sell_today + this->stg_position_a_sell_yesterday;
			}
			else if (strcmp((*Itor)->InstrumentID, this->stg_instrument_id_B.c_str())) {
				if ((*Itor)->OffsetFlag == '0') { // B开仓成交回报
					if ((*Itor)->Direction == '0') { /*B买开仓*/
						this->stg_position_b_buy_today += (*Itor)->Volume;
					}
					else if ((*Itor)->Direction == '1') { /*B卖开仓*/
						this->stg_position_b_sell_today += (*Itor)->Volume;
					}
				}
				else if ((*Itor)->OffsetFlag == '3') { // B平今成交回报
					if ((*Itor)->Direction == '0') { /*B买平今*/
						this->stg_position_b_sell_today -= (*Itor)->Volume;
					}
					else if ((*Itor)->Direction == '1') { /*B卖平今*/
						this->stg_position_b_buy_today -= (*Itor)->Volume;
					}
				}
				else if ((*Itor)->OffsetFlag == '4') { // B平昨成交回报
					if ((*Itor)->Direction == '0') { /*B买平昨*/
						this->stg_position_b_sell_yesterday -= (*Itor)->Volume;
						//std::cout << "Strategy::setL_query_trade 1051 stg_position_b_sell_yesterday = " << this->stg_position_b_sell_yesterday << std::endl;
					}
					else if ((*Itor)->Direction == '1') { /*B卖平昨*/
						this->stg_position_b_buy_yesterday -= (*Itor)->Volume;
					}
				}
				this->stg_position_b_buy = this->stg_position_b_buy_today + this->stg_position_b_buy_yesterday;
				this->stg_position_b_sell = this->stg_position_b_sell_today + this->stg_position_b_sell_yesterday;
			}

			/*std::cout << "==============================================" << std::endl;
			std::cout << "A合约 = " << this->stg_instrument_id_A << std::endl;
			std::cout << "A合约今买 = " << this->stg_position_a_buy_today << ", "
				<< "A合约昨买 = " << this->stg_position_a_buy_yesterday << ", "
				<< "A合约总买 = " << this->stg_position_a_buy << ", "
				<< "A合约今卖 = " << this->stg_position_a_sell_today << ", "
				<< "A合约昨卖 = " << this->stg_position_a_buy_yesterday << ", "
				<< "A合约总卖 = " << this->stg_position_a_sell << std::endl;

			std::cout << "==============================================" << std::endl;
			std::cout << "B合约 = " << this->stg_instrument_id_B << std::endl;
			std::cout << "B合约今买 = " << this->stg_position_b_buy_today << ", "
				<< "B合约昨买 = " << this->stg_position_b_buy_yesterday << ", "
				<< "B合约总买 = " << this->stg_position_b_buy << ", "
				<< "B合约今卖 = " << this->stg_position_b_sell_today << ", "
				<< "B合约昨卖 = " << this->stg_position_b_buy_yesterday << ", "
				<< "B合约总卖 = " << this->stg_position_b_sell << std::endl;*/

			USER_PRINT("A合约今买");
			USER_PRINT(this->stg_position_a_buy_today);
			USER_PRINT("A合约昨买");
			USER_PRINT(this->stg_position_a_buy_yesterday);
			USER_PRINT("A合约总买");
			USER_PRINT(this->stg_position_a_buy);
			USER_PRINT("A合约今卖");
			USER_PRINT(this->stg_position_a_sell_today);
			USER_PRINT("A合约昨卖");
			USER_PRINT(this->stg_position_a_sell_yesterday);
			USER_PRINT("A合约总卖");
			USER_PRINT(this->stg_position_a_sell);

			USER_PRINT("B合约今买");
			USER_PRINT(this->stg_position_b_buy_today);
			USER_PRINT("B合约昨买");
			USER_PRINT(this->stg_position_b_buy_yesterday);
			USER_PRINT("B合约总买");
			USER_PRINT(this->stg_position_b_buy);
			USER_PRINT("B合约今卖");
			USER_PRINT(this->stg_position_b_sell_today);
			USER_PRINT("B合约昨卖");
			USER_PRINT(this->stg_position_b_sell_yesterday);
			USER_PRINT("B合约总卖");
			USER_PRINT(this->stg_position_b_sell);

		}
	}
}

void Strategy::addOrderToListQueryOrder(CThostFtdcOrderField *order) {
	USER_PRINT("Strategy::addOrderToListQueryOrder IN");
	USER_CThostFtdcOrderField *new_order = new USER_CThostFtdcOrderField();
	memset(new_order, 0, sizeof(USER_CThostFtdcOrderField));
	this->CopyOrderDataToNew(new_order, order);
	this->stg_user->add_instrument_id_action_counter(order);
	this->update_pending_order_list(order);
	this->add_VolumeTradedBatch(order, new_order);
	this->update_position_detail(new_order);
	//this->l_query_order->push_back(new_order);
	USER_PRINT("Strategy::addOrderToListQueryOrder OUT");
}

void Strategy::setL_query_order(list<CThostFtdcOrderField *> *l_query_order) {

}

void Strategy::add_position_detail(PositionDetail *posd) {
	USER_PRINT("Strategy::add_position_detail");
	USER_CThostFtdcOrderField *new_order = new USER_CThostFtdcOrderField();
	memset(new_order, 0, sizeof(USER_CThostFtdcOrderField));
	this->CopyPositionData(posd, new_order);
	this->stg_list_position_detail_from_order->push_back(new_order);
}

void Strategy::CopyPositionData(PositionDetail *posd, USER_CThostFtdcOrderField *order) {
	USER_PRINT("Strategy::CopyPositionData");
	strcpy(order->InstrumentID, posd->getInstrumentID().c_str());
	strcpy(order->OrderRef, posd->getOrderRef().c_str());
	strcpy(order->UserID, posd->getUserID().c_str());
	order->Direction = posd->getDirection();
	order->CombOffsetFlag[0] = posd->getCombOffsetFlag().c_str()[0];
	order->CombHedgeFlag[0] = posd->getCombHedgeFlag().c_str()[0];
	order->LimitPrice = posd->getLimitPrice();
	order->VolumeTotalOriginal = posd->getVolumeTotalOriginal();
	strcpy(order->TradingDay, posd->getTradingDay().c_str());
	order->OrderStatus = posd->getOrderStatus();
	order->VolumeTraded = posd->getVolumeTraded();
	order->VolumeTotal = posd->getVolumeTotal();
	strcpy(order->InsertDate, posd->getInsertDate().c_str());
	strcpy(order->InsertTime, posd->getInsertTime().c_str());
	//order->StrategyID = posd->getStrategyID();
	strcpy(order->StrategyID, posd->getStrategyID().c_str());
	order->VolumeTradedBatch = posd->getVolumeTradedBatch();
}

void Strategy::printStrategyInfo(string message) {
	time_t tt = system_clock::to_time_t(system_clock::now()); 
	std::string nowt(std::ctime(&tt)); 
	/*std::cout << "====策略状态信息====" << std::endl;
	std::cout << "时间:" << nowt.substr(0, nowt.length() - 1) << std::endl;
	std::cout << "期货账户:" << this->stg_user_id << std::endl;
	std::cout << "策略编号:" << this->stg_strategy_id << std::endl;
	std::cout << "调试信息:" << message << std::endl;*/
}

void Strategy::printStrategyInfoPosition() {
	/*std::cout << "A合约今买 = " << this->stg_position_a_buy_today << ", "
		<< "A合约昨买 = " << this->stg_position_a_buy_yesterday << ", "
		<< "A合约总买 = " << this->stg_position_a_buy << ", "
		<< "A合约今卖 = " << this->stg_position_a_sell_today << ", "
		<< "A合约昨卖 = " << this->stg_position_a_buy_yesterday << ", "
		<< "A合约总卖 = " << this->stg_position_a_sell << std::endl;

	std::cout << "B合约今买 = " << this->stg_position_b_buy_today << ", "
		<< "B合约昨买 = " << this->stg_position_b_buy_yesterday << ", "
		<< "B合约总买 = " << this->stg_position_b_buy << ", "
		<< "B合约今卖 = " << this->stg_position_b_sell_today << ", "
		<< "B合约昨卖 = " << this->stg_position_b_buy_yesterday << ", "
		<< "B合约总卖 = " << this->stg_position_b_sell << std::endl;*/
}

// 获取持仓明细
list<USER_CThostFtdcOrderField *> * Strategy::getStg_List_Position_Detail_From_Order() {
	return this->stg_list_position_detail_from_order;
}

bool Strategy::CompareTickData(CThostFtdcDepthMarketDataField *last_tick_data, CThostFtdcDepthMarketDataField *pDepthMarketData) {
	if (!strcmp(last_tick_data->InstrumentID, pDepthMarketData->InstrumentID) &&
		!strcmp(last_tick_data->UpdateTime, pDepthMarketData->UpdateTime) &&
		(last_tick_data->UpdateMillisec == pDepthMarketData->UpdateMillisec)) {
		std::cout << "Strategy::CompareTickData() 重复tick" << std::endl;
		std::cout << "\t交易日:" << pDepthMarketData->TradingDay
			<< ", 合约代码:" << pDepthMarketData->InstrumentID
			<< ", 最新价:" << pDepthMarketData->LastPrice
			<< ", 持仓量:" << pDepthMarketData->OpenInterest
			//<< ", 上次结算价:" << pDepthMarketData->PreSettlementPrice 
			//<< ", 昨收盘:" << pDepthMarketData->PreClosePrice 
			<< ", 数量:" << pDepthMarketData->Volume
			//<< ", 昨持仓量:" << pDepthMarketData->PreOpenInterest
			<< ", 最后修改时间:" << pDepthMarketData->UpdateTime
			<< ", 最后修改毫秒:" << pDepthMarketData->UpdateMillisec
			<< ", 申买价一：" << pDepthMarketData->BidPrice1
			<< ", 申买量一:" << pDepthMarketData->BidVolume1
			<< ", 申卖价一:" << pDepthMarketData->AskPrice1
			<< ", 申卖量一:" << pDepthMarketData->AskVolume1
			//<< ", 今收盘价:" << pDepthMarketData->ClosePrice
			//<< ", 当日均价:" << pDepthMarketData->AveragePrice
			//<< ", 本次结算价格:" << pDepthMarketData->SettlementPrice
			<< ", 成交金额:" << pDepthMarketData->Turnover << std::endl;
		return true;
	}
	else
	{
		return false;
	}
}

void Strategy::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
	
	//tick_mtx.lock();

	// Get Own Instrument
	USER_PRINT(this);
	USER_PRINT("Strategy::OnRtnDepthMarketData IN");
	USER_PRINT(this->getStgInstrumentIdA());
	USER_PRINT(this->getStgInstrumentIdB());
	USER_PRINT(pDepthMarketData->InstrumentID);

	if (!strcmp(pDepthMarketData->InstrumentID, this->getStgInstrumentIdA().c_str())) {
		USER_PRINT("stg_instrument_A_tick ask_volume bid_volume");
		this->CopyTickData(stg_instrument_A_tick, pDepthMarketData);
		/*std::cout << "stg_instrument_A_tick = " << stg_instrument_A_tick->InstrumentID << endl;
		std::cout << "stg_instrument_A_tick->AskVolume1 = " << stg_instrument_A_tick->AskVolume1 << endl;
		std::cout << "stg_instrument_A_tick->BidVolume1 = " << stg_instrument_A_tick->BidVolume1 << endl;
		std::cout << "stg_instrument_A_tick->AskPrice1 = " << stg_instrument_A_tick->AskPrice1 << endl;
		std::cout << "stg_instrument_A_tick->BidPrice1 = " << stg_instrument_A_tick->BidPrice1 << endl;*/
		USER_PRINT(stg_instrument_A_tick->InstrumentID);
		USER_PRINT(stg_instrument_A_tick->AskVolume1);
		USER_PRINT(stg_instrument_A_tick->BidVolume1);
		USER_PRINT(stg_instrument_A_tick->AskPrice1);
		USER_PRINT(stg_instrument_A_tick->BidPrice1);
	}
	else if (!strcmp(pDepthMarketData->InstrumentID, this->getStgInstrumentIdB().c_str()))
	{
		USER_PRINT("stg_instrument_B_tick ask_volume bid_volume");
		this->CopyTickData(stg_instrument_B_tick, pDepthMarketData);
		/*std::cout << "stg_instrument_B_tick = " << stg_instrument_B_tick->InstrumentID << endl;
		std::cout << "stg_instrument_B_tick->AskVolume1 = " << stg_instrument_B_tick->AskVolume1 << endl;
		std::cout << "stg_instrument_B_tick->BidVolume1 = " << stg_instrument_B_tick->BidVolume1 << endl;
		std::cout << "stg_instrument_B_tick->AskPrice1 = " << stg_instrument_B_tick->AskPrice1 << endl;
		std::cout << "stg_instrument_B_tick->BidPrice1 = " << stg_instrument_B_tick->BidPrice1 << endl;*/
		USER_PRINT(stg_instrument_B_tick->InstrumentID);
		USER_PRINT(stg_instrument_B_tick->AskVolume1);
		USER_PRINT(stg_instrument_B_tick->BidVolume1);
		USER_PRINT(stg_instrument_B_tick->AskPrice1);
		USER_PRINT(stg_instrument_B_tick->BidPrice1);
	}
	USER_PRINT(this->stg_trade_tasking);
	/// 如果有交易任务,进入交易任务执行
	if (this->stg_trade_tasking) {
		//this->printStrategyInfo("Strategy::OnRtnDepthMarketData() 有交易任务,进入交易任务执行");
		/*std::cout << "stg_position_a_buy_today = " << stg_position_a_buy_today << std::endl;
		std::cout << "stg_position_b_sell_today = " << stg_position_b_sell_today << std::endl;
		std::cout << "stg_position_a_buy_yesterday = " << stg_position_a_buy_yesterday << std::endl;
		std::cout << "stg_position_b_sell_yesterday = " << stg_position_b_sell_yesterday << std::endl;
		std::cout << "stg_position_a_sell_today = " << stg_position_a_sell_today << std::endl;
		std::cout << "stg_position_b_buy_today = " << stg_position_b_buy_today << std::endl;
		std::cout << "stg_position_a_sell_yesterday = " << stg_position_a_sell_yesterday << std::endl;
		std::cout << "stg_position_b_buy_yesterday = " << stg_position_b_buy_yesterday << std::endl;
		std::cout << "挂单列表长度 = " << this->stg_list_order_pending->size() << std::endl;*/
		std::cout << "Strategy::OnRtnDepthMarketData():" << std::endl;
		std::cout << "\t(有交易任务,进入交易任务执行)" << std::endl;
		std::cout << "\t(stg_trade_tasking):(" << this->stg_trade_tasking << ")" << std::endl;
		this->Exec_OnTickComing(pDepthMarketData);
	}
	else { /// 如果没有交易任务，那么选择开始新的交易任务
		//if (!stg_select_order_algorithm_flag) {
		//	this->setStgSelectOrderAlgorithmFlag("Strategy::OnRtnDepthMarketData()", true); // 开启下单锁
		//	this->Select_Order_Algorithm(this->getStgOrderAlgorithm());
		//	/*std::cout << "Strategy::OnRtnDepthMarketData():" << std::endl;
		//	std::cout << "\t(开启tick锁 stg_select_order_algorithm_flag):(" << this->stg_select_order_algorithm_flag << ")" << std::endl;*/
		//}
		//else
		//{
		//	/*std::cout << "Strategy::OnRtnDepthMarketData():" << std::endl;
		//	std::cout << "\t(tick锁已开 stg_select_order_algorithm_flag):(" << this->stg_select_order_algorithm_flag << ")" << std::endl;*/
		//}

		//一把锁测试
		this->Select_Order_Algorithm(this->getStgOrderAlgorithm());

		
	}
	USER_PRINT("Strategy::OnRtnDepthMarketData OUT");
	//tick_mtx.unlock();
}

//选择下单算法
void Strategy::Select_Order_Algorithm(string stg_order_algorithm) {
	USER_PRINT("Strategy::Select_Order_Algorithm");
	USER_PRINT(this->stg_trade_tasking);
	USER_PRINT(this->stg_list_order_pending);
	USER_PRINT(this->stg_list_order_pending->size());
	USER_PRINT(this->stg_position_a_sell);
	USER_PRINT(this->stg_position_b_buy);
	USER_PRINT(this->stg_position_a_buy);
	USER_PRINT(this->stg_position_b_sell);

	////如果正在交易,直接返回0
	//if (this->stg_trade_tasking) {
	//	USER_PRINT("正在交易,返回");
	//	//this->printStrategyInfo("正在交易,返回");
	//	return;
	//}
	//如果有挂单,返回0
	if (this->stg_list_order_pending->size() > 0) {
		USER_PRINT("有挂单,返回");
		USER_PRINT(this->stg_list_order_pending->size());
		list<CThostFtdcOrderField *>::iterator itor;
		for (itor = this->stg_list_order_pending->begin(); itor != this->stg_list_order_pending->end(); itor++) {
			USER_PRINT((*itor)->InstrumentID);
		}
		//this->printStrategyInfo("有挂单,返回");
		std::cout << "Strategy::Select_Order_Algorithm():" << std::endl;
		std::cout << "\t(有挂单,返回)" << std::endl;
		this->setStgSelectOrderAlgorithmFlag("Strategy::OnRtnDepthMarketData()_1", false); // 关闭下单锁
		return;
	}

	if (!((this->stg_position_a_sell == this->stg_position_b_buy) && 
		(this->stg_position_a_buy == this->stg_position_b_sell))) {
		USER_PRINT("有撇腿,返回");
		//this->printStrategyInfo("有撇腿,返回");
		// 有撇腿
		std::cout << "Strategy::Select_Order_Algorithm():" << std::endl;
		std::cout << "\t(有撇腿)" << std::endl;
		this->setStgSelectOrderAlgorithmFlag("Strategy::OnRtnDepthMarketData()_2", false); // 关闭下单锁
		return;
	}

	if (stg_order_algorithm == ALGORITHM_ONE) { //下单算法1
		this->Order_Algorithm_One();
	}
	else if (stg_order_algorithm == ALGORITHM_TWO) { // 下单算法2
		this->Order_Algorithm_Two();
	}
	else if (stg_order_algorithm == ALGORITHM_THREE) { // 下单算法3
		
		this->Order_Algorithm_Three();
	}
	else {
		//std::cout << "Select_Order_Algorithm has no algorithm for you!" << endl;
	}
}

//下单算法1
void Strategy::Order_Algorithm_One() {
	USER_PRINT("Order_Algorithm_One");
	// 计算盘口价差，量
	if ((this->stg_instrument_A_tick->AskPrice1 != 0) &&
		(this->stg_instrument_A_tick->BidPrice1 != 0) &&
		(this->stg_instrument_A_tick->AskVolume1 != 0) &&
		(this->stg_instrument_A_tick->BidVolume1 != 0) &&
		(this->stg_instrument_B_tick->AskPrice1 != 0) &&
		(this->stg_instrument_B_tick->BidPrice1 != 0) &&
		(this->stg_instrument_B_tick->AskVolume1 != 0) &&
		(this->stg_instrument_B_tick->BidVolume1 != 0))
	{
		//std::cout << "计算多头" << endl;
		//std::cout << "A_tick->BidPrice1 = " << this->stg_instrument_A_tick->BidPrice1 << endl;
		//std::cout << "B_tick->AskPrice1 = " << this->stg_instrument_B_tick->AskPrice1 << endl;
		USER_PRINT("计算多头");
		USER_PRINT(this->stg_instrument_A_tick->BidPrice1);
		USER_PRINT(this->stg_instrument_B_tick->AskPrice1);
		//市场多头价差
		this->stg_spread_long = this->stg_instrument_A_tick->BidPrice1 - 
								this->stg_instrument_B_tick->AskPrice1;

		//std::cout << "A_tick->BidVolume1 = " << this->stg_instrument_A_tick->BidVolume1 << endl;
		//std::cout << "B_tick->AskVolume1 = " << this->stg_instrument_B_tick->AskVolume1 << endl;
		USER_PRINT(this->stg_instrument_A_tick->BidVolume1);
		USER_PRINT(this->stg_instrument_B_tick->AskVolume1);

		//市场多头价差挂单量
		this->stg_spread_long_volume = std::min(this->stg_instrument_A_tick->BidVolume1,
			this->stg_instrument_B_tick->AskVolume1);

		//std::cout << "stg_spread_long_volume = " << this->stg_spread_long_volume << endl;
		USER_PRINT(this->stg_spread_long_volume);

		//std::cout << "计算空头" << endl;
		//std::cout << "A_tick->AskPrice1 = " << this->stg_instrument_A_tick->AskPrice1 << endl;
		//std::cout << "B_tick->BidPrice1 = " << this->stg_instrument_B_tick->BidPrice1 << endl;
		USER_PRINT("计算空头");
		USER_PRINT(this->stg_instrument_A_tick->AskPrice1);
		USER_PRINT(this->stg_instrument_B_tick->BidPrice1);
		// 市场空头价差
		this->stg_spread_short = this->stg_instrument_A_tick->AskPrice1 -
			this->stg_instrument_B_tick->BidPrice1;


		//std::cout << "A_tick->AskVolume1 = " << this->stg_instrument_A_tick->AskVolume1 << endl;
		//std::cout << "B_tick->BidVolume1 = " << this->stg_instrument_B_tick->BidVolume1 << endl;

		USER_PRINT(this->stg_instrument_A_tick->AskVolume1);
		USER_PRINT(this->stg_instrument_B_tick->BidVolume1);

		// 市场空头价差挂单量
		this->stg_spread_short_volume = std::min(this->stg_instrument_A_tick->AskVolume1,
			this->stg_instrument_B_tick->BidVolume1);

		//std::cout << "stg_spread_short_volume = " << this->stg_spread_short_volume << endl;
		USER_PRINT(this->stg_spread_short_volume);
	} 
	else
	{
		//this->printStrategyInfo("策略跳过异常行情");
		this->setStgSelectOrderAlgorithmFlag("Strategy::Order_Algorithm_One()_1", false);
		return;
	}

	/*std::cout << "测试是否是stg_user问题" << std::endl;
	std::cout << "this->stg_user = " << this->stg_user << std::endl;
	std::cout << "this->stg_user->getCTP_Manager() = " << this->stg_user->getCTP_Manager() << std::endl;
	std::cout << "策略开关,期货账户开关,总开关" << std::endl;*/
	/*if (this->stg_user) {
		USER_PRINT(this->stg_user);
		}
		else {
		USER_PRINT("CRASH!!!");
		}*/
	USER_PRINT(this->stg_user->getCTP_Manager()->getOn_Off());
	//为了测试需要打开开关
	//this->stg_user->getCTP_Manager()->setOn_Off(1);
	USER_PRINT(this->getStgStrategyId())
	USER_PRINT(this->stg_user->GetTrader()->getOn_Off());
	USER_PRINT(this->stg_user->getOn_Off());
	USER_PRINT(this->getOn_Off());


	/// 策略开关，期货账户开关，总开关
	if (!((this->getOn_Off()) && (this->stg_user->getOn_Off()) && (this->stg_user->GetTrader()->getOn_Off()))) {
		USER_PRINT("请检查开关状态!");
		//this->printStrategyInfo("请检查开关状态!");
		/*std::cout << "策略开关 = " << this->getOn_Off() << std::endl;
		std::cout << "期货账户开关 = " << this->stg_user->getOn_Off() << std::endl;
		std::cout << "总开关 = " << this->stg_user->GetTrader()->getOn_Off() << std::endl;*/
		this->setStgSelectOrderAlgorithmFlag("Strategy::Order_Algorithm_One()_2", false);
		return;
	}

	//std::cout << "策略开关,期货账户开关,总开关22222" << std::endl;
	USER_PRINT("**********************");
	USER_PRINT("价差多头卖平");
	USER_PRINT(this->sell_close_on_off);
	USER_PRINT(this->stg_spread_long);
	USER_PRINT(this->stg_sell_close);
	USER_PRINT(this->stg_position_a_sell);
	USER_PRINT(this->stg_position_b_buy);
	USER_PRINT(this->stg_position_a_sell);
	USER_PRINT("*满足条件：");
	USER_PRINT(this->sell_close_on_off);
	USER_PRINT((this->stg_spread_long >= this->stg_sell_close));
	USER_PRINT((this->stg_position_a_sell == this->stg_position_b_buy));
	USER_PRINT((this->stg_position_a_sell > 0));
	USER_PRINT("**********************");

	USER_PRINT("######################");
	USER_PRINT("价差空头买平");
	USER_PRINT(this->buy_close_on_off);
	USER_PRINT(this->stg_position_a_sell);
	USER_PRINT(this->stg_position_b_buy);
	USER_PRINT(this->stg_position_a_sell);
	USER_PRINT("#满足条件：");
	USER_PRINT(this->buy_close_on_off);
	USER_PRINT((this->stg_position_a_sell == this->stg_position_b_buy));
	USER_PRINT((this->stg_position_a_sell > 0));
	USER_PRINT((this->stg_spread_short <= this->stg_buy_close));
	USER_PRINT("######################");

	USER_PRINT("$$$$$$$$$$$$$$$$$$$$$$");
	USER_PRINT("价差多头卖开");
	USER_PRINT(this->sell_open_on_off);
	USER_PRINT(this->stg_position_a_buy);
	USER_PRINT(this->stg_position_a_sell);
	USER_PRINT(this->stg_lots);
	USER_PRINT("$满足条件：");
	USER_PRINT(this->sell_open_on_off);
	USER_PRINT(((this->stg_position_a_buy + this->stg_position_a_sell) < this->stg_lots));
	USER_PRINT("$$$$$$$$$$$$$$$$$$$$$$");

	USER_PRINT("&&&&&&&&&&&&&&&&&&&&&&");
	USER_PRINT("价差空头买开");
	USER_PRINT(this->buy_open_on_off);
	USER_PRINT(this->stg_position_a_buy);
	USER_PRINT(this->stg_position_a_sell);
	USER_PRINT(this->stg_lots);
	USER_PRINT("&满足条件：");
	USER_PRINT(this->buy_open_on_off);
	USER_PRINT(((this->stg_position_a_buy + this->stg_position_a_sell) < this->stg_lots));
	USER_PRINT("&&&&&&&&&&&&&&&&&&&&&&");
	
	/*this->printStrategyInfo("价差卖开参数");
	std::cout << "this->stg_spread_long = " << this->stg_spread_long << std::endl;
	std::cout << "this->stg_sell_open = " << this->stg_sell_open << std::endl;
	std::cout << "this->stg_spread_shift = " << this->stg_spread_shift << std::endl;
	std::cout << "this->stg_a_price_tick = " << this->stg_a_price_tick << std::endl;
	std::cout << "(this->stg_spread_long >= (this->stg_sell_open + this->stg_spread_shift * this->stg_a_price_tick)) = " << (this->stg_spread_long >= (this->stg_sell_open + this->stg_spread_shift * this->stg_a_price_tick)) << std::endl;*/


	/*this->printStrategyInfo("价差卖平参数");
	std::cout << "this->sell_close_on_off = " << this->sell_close_on_off << std::endl;
	std::cout << "this->stg_position_a_sell = " << this->stg_position_a_sell << std::endl;
	std::cout << "this->stg_position_b_buy = " << this->stg_position_b_buy << std::endl;
	std::cout << "this->stg_position_a_buy = " << this->stg_position_a_buy << std::endl;
	std::cout << "this->stg_spread_long = " << this->stg_spread_long << std::endl;
	std::cout << "this->stg_sell_close = " << this->stg_sell_close << std::endl;
	std::cout << "this->stg_spread_shift = " << this->stg_spread_shift << std::endl;
	std::cout << "this->stg_a_price_tick = " << this->stg_a_price_tick << std::endl;
	std::cout << "(this->stg_spread_long >= (this->stg_sell_close + this->stg_spread_shift * this->stg_a_price_tick)) = " << (this->stg_spread_long >= (this->stg_sell_close + this->stg_spread_shift * this->stg_a_price_tick)) << std::endl;*/

	/// 价差卖平(b)
	if ((this->sell_close_on_off) &&
		(this->stg_position_a_sell == this->stg_position_b_buy) &&
		(this->stg_position_a_buy > 0) &&
		(this->stg_spread_long >= (this->stg_sell_close + this->stg_spread_shift * this->stg_a_price_tick))) {
		
		//this->stg_trade_tasking = true;
		this->printStrategyInfo("价差卖平");
		//this->update_task_status();

		/// 市场多头价差大于触发参数， AB持仓量相等且大于0
		std::cout << "Strategy::Order_Algorithm_One()" << std::endl;
		std::cout << "\t策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差卖平" << endl;
		

		/*std::cout << "user_id = " << this->stg_user_id << ", "
			<< "strategy_id = " << this->stg_strategy_id << ", "
			<< "instrument_A = " << this->stg_instrument_id_A << ", "
			<< "instrument_B = " << this->stg_instrument_id_B << ", "
			<< "spread long = " << this->stg_spread_long << ", "
			<< "spread long volume = " << this->stg_spread_long_volume << ", "
			<< "spread short = " << this->stg_spread_short << ", "
			<< "spread short volume = " << this->stg_spread_short_volume << endl;*/

		/// 满足交易任务之前的一个tick
		this->CopyTickData(stg_instrument_A_tick_last, stg_instrument_A_tick);
		this->CopyTickData(stg_instrument_B_tick_last, stg_instrument_B_tick);

		int order_volume = 0;

		//this->printStrategyInfo("计算发单手数");

		/// 优先平昨仓
		/// 报单手数：盘口挂单量、每份发单手数、持仓量
		if (this->stg_position_a_buy_yesterday > 0) {
			
			//std::cout << "this->stg_spread_long_volume = " << this->stg_spread_long_volume << std::endl;
			//std::cout << "this->stg_position_a_buy_yesterday = " << this->stg_position_a_buy_yesterday << std::endl;
			order_volume = this->getMinNum(this->stg_spread_long_volume, this->stg_lots_batch, this->stg_position_a_buy_yesterday);
			//std::cout << "order_volume = " << order_volume << std::endl;
			this->stg_a_order_insert_args->CombOffsetFlag[0] = '4'; /// 平昨
			this->stg_b_order_insert_args->CombOffsetFlag[0] = '4'; /// 平昨
		}
		else if ((this->stg_position_a_buy_yesterday == 0) && (this->stg_position_a_buy_today > 0)) {
			//std::cout << "this->stg_spread_short_volume = " << this->stg_spread_short_volume << std::endl;
			//std::cout << "this->stg_lots_batch = " << this->stg_lots_batch << std::endl;
			//std::cout << "this->stg_position_a_buy_today = " << this->stg_position_a_buy_today << std::endl;
			order_volume = this->getMinNum(this->stg_spread_short_volume, this->stg_lots_batch, this->stg_position_a_buy_today);
			//std::cout << "order_volume = " << order_volume << std::endl;
			this->stg_a_order_insert_args->CombOffsetFlag[0] = '3'; /// 平今
			this->stg_b_order_insert_args->CombOffsetFlag[0] = '3'; /// 平今
		}
		if ((order_volume <= 0)) {
			//std::cout << "发单手数错误值 = " << order_volume << endl;
			this->stg_trade_tasking = false;
			return;
		} else {
			//std::cout << "发单手数 = " << order_volume << endl;
		}

		this->stg_order_ref_a = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_a;
		this->stg_lock_order_ref = this->stg_order_ref_a;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_a);

		/// A合约报单参数，全部确定
		// 报单引用
		strcpy(this->stg_a_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_a_order_insert_args->InstrumentID, this->stg_instrument_id_A.c_str());
		// 限价
		this->stg_a_order_insert_args->LimitPrice = this->stg_instrument_A_tick->BidPrice1 - this->stg_a_limit_price_shift * this->stg_a_price_tick;
		//std::cout << "this->stg_a_order_insert_args->LimitPrice = " << this->stg_a_order_insert_args->LimitPrice << std::endl;
		//std::cout << "this->stg_instrument_A_tick->BidPrice1 = " << this->stg_instrument_A_tick->BidPrice1 << std::endl;
		//std::cout << "this->stg_a_limit_price_shift = " << this->stg_a_limit_price_shift << std::endl;
		//std::cout << "this->stg_a_price_tick = " << this->stg_a_price_tick << std::endl;
		// 数量
		this->stg_a_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_a_order_insert_args->Direction = '1'; // 0买 1卖
		// 组合开平标志
		/// this->stg_a_order_insert_args->CombOffsetFlag[0] = '0';
		// 组合投机套保标志
		this->stg_a_order_insert_args->CombHedgeFlag[0] = '1'; // 1投机 2套利 3保值


		// B合约报单参数,部分确定
		// 报单引用
		//strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_b_order_insert_args->InstrumentID, this->stg_instrument_id_B.c_str());
		// 限价
		this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->AskPrice1 + this->stg_b_limit_price_shift * this->stg_b_price_tick;
		//std::cout << "this->stg_b_order_insert_args->LimitPrice = " << this->stg_b_order_insert_args->LimitPrice << std::endl;
		//std::cout << "this->stg_instrument_B_tick->AskPrice1 = " << this->stg_instrument_B_tick->AskPrice1 << std::endl;
		//std::cout << "this->stg_b_limit_price_shift = " << this->stg_b_limit_price_shift << std::endl;
		//std::cout << "this->stg_b_price_tick = " << this->stg_b_price_tick << std::endl;
		// 数量
		//this->stg_b_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_b_order_insert_args->Direction = '0'; // 0买 1卖
		// 组合开平标志
		/// this->stg_b_order_insert_args->CombOffsetFlag[0] = '0'; //组合开平标志0开仓 上期所3平今、4平昨，其他交易所1平仓
		// 组合投机套保标志
		this->stg_b_order_insert_args->CombHedgeFlag[0] = '1'; // 1投机 2套利 3保值

		/// 执行下单任务
		this->Exec_OrderInsert(this->stg_a_order_insert_args);
		

	}
	/// 价差买平(f)
	else if ((this->buy_close_on_off) && 
		(this->stg_position_a_sell == this->stg_position_b_buy) &&
		(this->stg_position_a_sell > 0) && 
		(this->stg_spread_short <= (this->stg_buy_close - this->stg_spread_shift * this->stg_a_price_tick))) {

		//this->stg_trade_tasking = true;
		this->printStrategyInfo("价差买平");
		//this->update_task_status();

		/// 市场空头价差小于等于触发参数， AB持仓量相等且大于0
		//std::cout << "策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差买平" << endl;
		std::cout << "Strategy::Order_Algorithm_One()" << std::endl;
		std::cout << "\t策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差买平" << endl;

		/*std::cout << "user_id = " << this->stg_user_id << ", "
			<< "strategy_id = " << this->stg_strategy_id << ", "
			<< "instrument_A = " << this->stg_instrument_id_A << ", "
			<< "instrument_B = " << this->stg_instrument_id_B << ", "
			<< "spread long = " << this->stg_spread_long << ", "
			<< "spread long volume = " << this->stg_spread_long_volume << ", "
			<< "spread short = " << this->stg_spread_short << ", "
			<< "spread short volume = " << this->stg_spread_short_volume << ", "
			<< "stg_lots_batch = " << this->stg_lots_batch << ", "
			<< "stg_position_a_sell_yesterday = " << this->stg_position_a_sell_yesterday << endl;*/

		/// 满足交易任务之前的一个tick
		this->CopyTickData(stg_instrument_A_tick_last, stg_instrument_A_tick);
		this->CopyTickData(stg_instrument_B_tick_last, stg_instrument_B_tick);

		//USER_PRINT("stg_instrument_A_tick_last");
		//USER_PRINT(stg_instrument_A_tick_last);
		//USER_PRINT("stg_instrument_B_tick_last");
		//USER_PRINT(stg_instrument_B_tick_last);

		int order_volume = 0;

		//this->printStrategyInfo("计算发单手数");
		/// 优先平昨仓
		/// 报单手数：盘口挂单量、每份发单手数、持仓量
		if (this->stg_position_a_sell_yesterday > 0) {
			//std::cout << "this->stg_spread_short_volume = " << this->stg_spread_short_volume << std::endl;
			//std::cout << "this->stg_lots_batch = " << this->stg_lots_batch << std::endl;
			//std::cout << "this->stg_position_a_sell_yesterday = " << this->stg_position_a_sell_yesterday << std::endl;
			order_volume = this->getMinNum(this->stg_spread_short_volume, this->stg_lots_batch, this->stg_position_a_sell_yesterday);
			//std::cout << "order_volume = " << order_volume << std::endl;
			this->stg_a_order_insert_args->CombOffsetFlag[0] = '4'; /// 平昨
			this->stg_b_order_insert_args->CombOffsetFlag[0] = '4'; /// 平昨
		}
		else if ((this->stg_position_a_sell_yesterday == 0) && (this->stg_position_a_sell_today > 0)) {
			//std::cout << "this->stg_spread_short_volume = " << this->stg_spread_short_volume << std::endl;
			//std::cout << "this->stg_lots_batch = " << this->stg_lots_batch << std::endl;
			//std::cout << "this->stg_position_a_sell_today = " << this->stg_position_a_sell_today << std::endl;
			order_volume = this->getMinNum(this->stg_spread_short_volume, this->stg_lots_batch, this->stg_position_a_sell_today);
			//std::cout << "order_volume = " << order_volume << std::endl;
			this->stg_a_order_insert_args->CombOffsetFlag[0] = '3'; /// 平今
			this->stg_b_order_insert_args->CombOffsetFlag[0] = '3'; /// 平今
		}
		if ((order_volume <= 0)) {
			//std::cout << "发单手数错误值 = " << order_volume << endl;
			this->stg_trade_tasking = false;
			return;
		} else {
			//std::cout << "发单手数 = " << order_volume << endl;
		}

		this->stg_order_ref_a = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_a;
		this->stg_lock_order_ref = this->stg_order_ref_a;

		//USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_a);

		/// A合约报单参数，全部确定
		// 报单引用
		strcpy(this->stg_a_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_a_order_insert_args->InstrumentID, this->stg_instrument_id_A.c_str());
		// 限价
		this->stg_a_order_insert_args->LimitPrice = this->stg_instrument_A_tick->AskPrice1 + this->stg_a_limit_price_shift * this->stg_a_price_tick;
		
		//std::cout << "this->stg_instrument_A_tick->AskPrice1 = " << this->stg_instrument_A_tick->AskPrice1 << std::endl;
		//std::cout << "this->stg_spread_shift = " << this->stg_spread_shift << std::endl;
		//std::cout << "this->stg_a_price_tick = " << this->stg_a_price_tick << std::endl;
		//std::cout << "this->stg_a_order_insert_args->LimitPrice = " << this->stg_a_order_insert_args->LimitPrice << std::endl;
		
		// 数量
		this->stg_a_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_a_order_insert_args->Direction = '0';
		// 组合开平标志
		/// this->stg_a_order_insert_args->CombOffsetFlag[0] = '0';
		// 组合投机套保标志
		this->stg_a_order_insert_args->CombHedgeFlag[0] = '1'; /// 1投机 2套利 3保值


		// B合约报单参数,部分确定
		// 报单引用
		//strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_b_order_insert_args->InstrumentID, this->stg_instrument_id_B.c_str());
		// 限价
		this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->BidPrice1 - this->stg_b_limit_price_shift * this->stg_b_price_tick;
		// 数量
		//this->stg_b_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_b_order_insert_args->Direction = '1';
		// 组合开平标志
		/// this->stg_b_order_insert_args->CombOffsetFlag[0] = '0'; //组合开平标志0开仓 上期所3平今、4平昨，其他交易所1平仓
		// 组合投机套保标志
		this->stg_b_order_insert_args->CombHedgeFlag[0] = '1';

		/// 执行下单任务
		this->Exec_OrderInsert(this->stg_a_order_insert_args);
		
	}

	/// 价差卖开(f)
	else if ((this->sell_open_on_off) && 
		((this->stg_position_a_buy + this->stg_position_a_sell + this->stg_pending_a_open) < this->stg_lots) &&
		(this->stg_spread_long >= (this->stg_sell_open + this->stg_spread_shift * this->stg_a_price_tick))) {
		
		//this->stg_trade_tasking = true;
		this->printStrategyInfo("价差卖开");
		//this->update_task_status();

		/** 市场多头价差大于触发参数
		A合约买持仓加B合约买小于总仓位**/

		//std::cout << "策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差卖开" << endl;
		std::cout << "Strategy::Order_Algorithm_One()" << std::endl;
		std::cout << "\t策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差卖开" << endl;
		
		/*std::cout << "user_id = " << this->stg_user_id << ", "
			<< "strategy_id = " << this->stg_strategy_id << ", "
			<< "instrument_A = " << this->stg_instrument_id_A << ", "
			<< "instrument_B = " << this->stg_instrument_id_B << ", "
			<< "spread long = " << this->stg_spread_long << ", "
			<< "spread long volume = " << this->stg_spread_long_volume << ", "
			<< "spread short = " << this->stg_spread_short << ", "
			<< "spread short volume = " << this->stg_spread_short_volume << ", "
			<< "stg_lots_batch = " << this->stg_lots_batch << ", "
			<< "stg_position_a_buy = " << this->stg_position_a_buy << ", "
			<< "stg_position_a_sell = " << this->stg_position_a_sell << ", "
			<< "stg_lots = " << this->stg_lots << endl;*/

		//cout << "\t交易日:" << stg_instrument_A_tick->TradingDay 
		//	<< ", 合约代码:" << stg_instrument_A_tick->InstrumentID 
		//	<< ", 最新价:" << stg_instrument_A_tick->LastPrice 
		//	<< ", 持仓量:" << stg_instrument_A_tick->OpenInterest 
		//	//<< ", 上次结算价:" << stg_instrument_A_tick->PreSettlementPrice 
		//	//<< ", 昨收盘:" << stg_instrument_A_tick->PreClosePrice 
		//	<< ", 数量:" << stg_instrument_A_tick->Volume 
		//	//<< ", 昨持仓量:" << stg_instrument_A_tick->PreOpenInterest
		//	<< ", 最后修改时间:" << stg_instrument_A_tick->UpdateTime
		//	<< ", 最后修改毫秒:" << stg_instrument_A_tick->UpdateMillisec
		//	<< ", 申买价一：" << stg_instrument_A_tick->BidPrice1 
		//	<< ", 申买量一:" << stg_instrument_A_tick->BidVolume1 
		//	<< ", 申卖价一:" << stg_instrument_A_tick->AskPrice1 
		//	<< ", 申卖量一:" << stg_instrument_A_tick->AskVolume1 
		//	//<< ", 今收盘价:" << stg_instrument_A_tick->ClosePrice
		//	//<< ", 当日均价:" << stg_instrument_A_tick->AveragePrice
		//	//<< ", 本次结算价格:" << stg_instrument_A_tick->SettlementPrice
		//	<< ", 成交金额:" << stg_instrument_A_tick->Turnover << endl;

		if (this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) || this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick)) {
			return;
		}

		/// 满足交易任务之前的tick
		this->CopyTickData(stg_instrument_A_tick_last, stg_instrument_A_tick);
		this->CopyTickData(stg_instrument_B_tick_last, stg_instrument_B_tick);

		//USER_PRINT("stg_instrument_A_tick_last");
		//USER_PRINT(stg_instrument_A_tick_last);
		//USER_PRINT("stg_instrument_B_tick_last");
		//USER_PRINT(stg_instrument_B_tick_last);

		/*this->printStrategyInfo("计算发单手数");
		std::cout << "this->stg_spread_long_volume = " << this->stg_spread_long_volume << std::endl;
		std::cout << "this->stg_lots_batch = " << this->stg_lots_batch << std::endl;
		std::cout << "this->stg_lots = " << this->stg_lots << std::endl;
		std::cout << "this->stg_position_a_buy = " << this->stg_position_a_buy << std::endl;
		std::cout << "this->stg_position_b_buy = " << this->stg_position_b_buy << std::endl;*/
		/// 报单手数：盘口挂单量,每份发单手数,剩余可开仓手数中取最小值
		int order_volume = this->getMinNum(this->stg_spread_long_volume, this->stg_lots_batch, this->stg_lots - (this->stg_position_a_buy + this->stg_position_b_buy));
		//std::cout << "order_volume = " << order_volume << std::endl;


		if (order_volume <= 0) {
			//std::cout << "发单手数错误值 = " << order_volume << endl;
			this->stg_trade_tasking = false;
			return;
		} else {
			//std::cout << "发单手数 = " << order_volume << endl;
		}

		this->stg_order_ref_a = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_a;
		this->stg_lock_order_ref = this->stg_order_ref_a;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_a);

		// A合约报单参数,全部确定
		// 报单引用
		std::strcpy(this->stg_a_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_a_order_insert_args->InstrumentID, this->stg_instrument_id_A.c_str());
		// 限价
		this->stg_a_order_insert_args->LimitPrice = this->stg_instrument_A_tick->BidPrice1 - this->stg_a_limit_price_shift * this->stg_a_price_tick;
		
		//std::cout << "this->stg_instrument_A_tick->BidPrice1 = " << this->stg_instrument_A_tick->BidPrice1 << std::endl;
		//std::cout << "this->stg_a_limit_price_shift = " << this->stg_a_limit_price_shift << std::endl;
		//std::cout << "this->stg_a_price_tick = " << this->stg_a_price_tick << std::endl;
		//std::cout << "this->stg_a_order_insert_args->LimitPrice = " << this->stg_a_order_insert_args->LimitPrice << std::endl;

		// 数量
		this->stg_a_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_a_order_insert_args->Direction = '1'; // 0买 1卖
		// 组合开平标志
		this->stg_a_order_insert_args->CombOffsetFlag[0] = '0';  /// 0开仓 1平仓 3平今 4平昨
		// 组合投机套保标志
		this->stg_a_order_insert_args->CombHedgeFlag[0] = '1'; /// 1投机 2套利 3保值


		// B合约报单参数,部分确定
		// 报单引用
		//strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_b_order_insert_args->InstrumentID, this->stg_instrument_id_B.c_str());
		// 限价
		this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->AskPrice1 + this->stg_b_limit_price_shift * this->stg_b_price_tick;
		//std::cout << "this->stg_instrument_B_tick->AskPrice1 = " << this->stg_instrument_B_tick->AskPrice1 << std::endl;
		//std::cout << "this->stg_b_limit_price_shift = " << this->stg_b_limit_price_shift << std::endl;
		//std::cout << "this->stg_b_price_tick = " << this->stg_b_price_tick << std::endl;
		//std::cout << "this->stg_b_order_insert_args->LimitPrice = " << this->stg_b_order_insert_args->LimitPrice << std::endl;
		// 数量
		//this->stg_b_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_b_order_insert_args->Direction = '0';
		// 组合开平标志
		this->stg_b_order_insert_args->CombOffsetFlag[0] = '0'; //组合开平标志0开仓 上期所3平今、4平昨，其他交易所1平仓
		// 组合投机套保标志
		this->stg_b_order_insert_args->CombHedgeFlag[0] = '1';

		/// 执行下单任务
		this->Exec_OrderInsert(this->stg_a_order_insert_args);
		
	}

	/// 价差买开
	else if ((this->buy_open_on_off) &&
		((this->stg_position_a_buy + this->stg_position_a_sell + this->stg_pending_a_open) < this->stg_lots) &&
		((this->stg_spread_short <= (this->stg_buy_open - this->stg_spread_shift * this->stg_a_price_tick)))) {

		//this->stg_trade_tasking = true;
		this->printStrategyInfo("价差买开");
		//this->update_task_status();

		//std::cout << "策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差买开" << endl;
		std::cout << "Strategy::Order_Algorithm_One()" << std::endl;
		std::cout << "\t策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差买开" << endl;

		/*std::cout << "user_id = " << this->stg_user_id << ", "
			<< "strategy_id = " << this->stg_strategy_id << ", "
			<< "instrument_A = " << this->stg_instrument_id_A << ", "
			<< "instrument_B = " << this->stg_instrument_id_B << ", "
			<< "spread long = " << this->stg_spread_long << ", "
			<< "spread long volume = " << this->stg_spread_long_volume << ", "
			<< "spread short = " << this->stg_spread_short << ", "
			<< "spread short volume = " << this->stg_spread_short_volume << endl;*/

		if (this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) || this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick)) {
			return;
		}

		/// 满足交易任务之前的tick
		this->CopyTickData(stg_instrument_A_tick_last, stg_instrument_A_tick);
		this->CopyTickData(stg_instrument_B_tick_last, stg_instrument_B_tick);

		/*this->printStrategyInfo("计算发单手数");
		std::cout << "this->stg_spread_long_volume = " << this->stg_spread_long_volume << std::endl;
		std::cout << "this->stg_lots_batch = " << this->stg_lots_batch << std::endl;
		std::cout << "this->stg_lots = " << this->stg_lots << std::endl;
		std::cout << "this->stg_position_a_buy = " << this->stg_position_a_buy << std::endl;
		std::cout << "this->stg_position_a_sell = " << this->stg_position_a_sell << std::endl;
		std::cout << "this->stg_position_b_buy = " << this->stg_position_b_buy << std::endl;*/

		/// 报单手数：盘口挂单量,每份发单手数,剩余可开仓手数中取最小值
		int order_volume = this->getMinNum(this->stg_spread_long_volume, this->stg_lots_batch, this->stg_lots - (this->stg_position_a_buy + this->stg_position_b_buy));
		//std::cout << "order_volume = " << order_volume << std::endl;


		if (order_volume <= 0) {
			//std::cout << "发单手数错误值 = " << order_volume << endl;
			this->stg_trade_tasking = false;
			return;
		} else {
			//std::cout << "发单手数 = " << order_volume << endl;
		}

		this->stg_order_ref_a = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_a;
		this->stg_lock_order_ref = this->stg_order_ref_a;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_a);

		// A合约报单参数,全部确定
		// 报单引用
		std::strcpy(this->stg_a_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_a_order_insert_args->InstrumentID, this->stg_instrument_id_A.c_str());
		// 限价
		this->stg_a_order_insert_args->LimitPrice = this->stg_instrument_A_tick->AskPrice1 + this->stg_a_limit_price_shift * this->stg_a_price_tick;
		//std::cout << "this->stg_a_order_insert_args->LimitPrice = " << this->stg_a_order_insert_args->LimitPrice << std::endl;
		//std::cout << "this->stg_instrument_A_tick->AskPrice1 = " << this->stg_instrument_A_tick->AskPrice1 << std::endl;
		//std::cout << "this->stg_a_limit_price_shift = " << this->stg_a_limit_price_shift << std::endl;
		//std::cout << "this->stg_a_price_tick = " << this->stg_a_price_tick << std::endl;

		// 数量
		this->stg_a_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_a_order_insert_args->Direction = '0'; // 0买 1卖
		// 组合开平标志
		this->stg_a_order_insert_args->CombOffsetFlag[0] = '0';  /// 0开仓 1平仓 3平今 4平昨
		// 组合投机套保标志
		this->stg_a_order_insert_args->CombHedgeFlag[0] = '1'; /// 1投机 2套利 3保值


		// B合约报单参数,部分确定
		// 报单引用
		//strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_b_order_insert_args->InstrumentID, this->stg_instrument_id_B.c_str());
		// 限价
		this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->BidPrice1 - this->stg_b_limit_price_shift * this->stg_b_price_tick;
		//std::cout << "this->stg_b_order_insert_args->LimitPrice = " << this->stg_b_order_insert_args->LimitPrice << std::endl;
		//std::cout << "this->stg_instrument_B_tick->BidPrice1 = " << this->stg_instrument_B_tick->BidPrice1 << std::endl;
		//std::cout << "this->stg_b_limit_price_shift = " << this->stg_b_limit_price_shift << std::endl;
		//std::cout << "this->stg_b_price_tick = " << this->stg_b_price_tick << std::endl;
		// 数量
		//this->stg_b_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_b_order_insert_args->Direction = '1'; // 0买 1卖
		// 组合开平标志
		this->stg_b_order_insert_args->CombOffsetFlag[0] = '0'; //组合开平标志0开仓 上期所3平今、4平昨，其他交易所1平仓
		// 组合投机套保标志
		this->stg_b_order_insert_args->CombHedgeFlag[0] = '1'; /// 1投机 2套利 3保值

		/// 执行下单任务
		this->Exec_OrderInsert(this->stg_a_order_insert_args);
		
	}
}
//下单算法2
void Strategy::Order_Algorithm_Two() {

}
//下单算法3
void Strategy::Order_Algorithm_Three() {

}

/// 生成报单引用
string Strategy::Generate_Order_Ref() {

	/*std::stringstream strstream;
	std::string number;
	strstream << (this->stg_order_ref_base + 1L);
	strstream >> number;
	string order_ref_base = number + this->stg_strategy_id;*/
	
	//USER_PRINT("Strategy::Generate_Order_Ref()");
	//USER_PRINT("this->stg_user");
	//USER_PRINT(this->stg_user);
	USER_PRINT(this->stg_user->getStgOrderRefBase());
	this->stg_user->setStgOrderRefBase(this->stg_user->getStgOrderRefBase() + 1);
	this->stg_order_ref_base = this->stg_user->getStgOrderRefBase(); // 更新基准数
	USER_PRINT(this->stg_order_ref_base);
	string order_ref_base = std::to_string(this->stg_order_ref_base) + this->stg_strategy_id;
	USER_PRINT(order_ref_base);

	//USER_PRINT("Generate_Order_Ref");
	//USER_PRINT(order_ref_base);

	//stringstream strValue;
	//strValue << order_ref_base;
	//strValue << this->stg_order_ref_base; // 更新
	return order_ref_base;
}


// 报单
void Strategy::Exec_OrderInsert(CThostFtdcInputOrderField *insert_order) {
	/*std::cout << "====报单参数====" << std::endl;
	std::cout << "OrderRef = " << insert_order->OrderRef << std::endl;
	std::cout << "InstrumentID = " << insert_order->InstrumentID << std::endl;
	std::cout << "LimitPrice = " << insert_order->LimitPrice << std::endl;
	std::cout << "VolumeTotalOriginal = " << insert_order->VolumeTotalOriginal << std::endl;
	std::cout << "Direction = " << insert_order->Direction << std::endl;
	std::cout << "CombOffsetFlag = " << insert_order->CombOffsetFlag[0] << std::endl;
	std::cout << "CombHedgeFlag = " << insert_order->CombHedgeFlag[0] << std::endl;*/

	//下单操作
	this->stg_user->getUserTradeSPI()->OrderInsert(this->stg_user, insert_order);
}

// 报单录入请求
void Strategy::Exec_OnRspOrderInsert() {
	USER_PRINT("Exec_OnRspOrderInsert()");
	this->setStgSelectOrderAlgorithmFlag("Strategy::Exec_OnRspOrderInsert()", false);
}

// 报单操作请求响应
void Strategy::Exec_OnRspOrderAction() {
	USER_PRINT("Exec_OnRspOrderAction()");
}

// 报单回报
void Strategy::Exec_OnRtnOrder(CThostFtdcOrderField *pOrder) {
	USER_PRINT("Exec_OnRtnOrder");
	USER_PRINT(pOrder->InstrumentID);
	USER_PRINT(pOrder->VolumeTraded);

	// 添加字段,本次成交量
	USER_CThostFtdcOrderField *order_new = new USER_CThostFtdcOrderField();
	memset(order_new, 0x00, sizeof(USER_CThostFtdcOrderField));
	// 添加字段,本次成交量
	USER_CThostFtdcOrderField *order_new_tmp = new USER_CThostFtdcOrderField();
	memset(order_new_tmp, 0x00, sizeof(USER_CThostFtdcOrderField));
	USER_PRINT(order_new);

	// 添加本次成交字段VolumeTradedBatch
	this->add_VolumeTradedBatch(pOrder, order_new);

	this->CopyNewOrderData(order_new_tmp, order_new);
	
	// 更新挂单列表，持仓信息
	this->update_pending_order_list(pOrder);

	//std::auto_ptr<USER_CThostFtdcOrderField> order_new(new USER_CThostFtdcOrderField);

	// 更新持仓明细列表
	this->update_position_detail(order_new_tmp);

	// 更新持仓变量
	this->update_position(order_new);

	// 更新标志位
	this->update_task_status();

	// 更新tick锁
	this->update_tick_lock_status(order_new);

	delete order_new;
	delete order_new_tmp;

	/// A成交回报,B发送等量的报单
	if ((!strcmp(pOrder->InstrumentID, this->stg_instrument_id_A.c_str())) && ((pOrder->OrderStatus == '0') || (pOrder->OrderStatus == '1')) && (strlen(pOrder->OrderSysID) != 0)) { //只有全部成交或者部分成交还在队列中

		if (this->stg_list_order_pending->size() == 0) { // 无挂单
			//std::cout << "A无挂单" << std::endl;
			this->stg_b_order_insert_args->VolumeTotalOriginal = pOrder->VolumeTraded;
		}
		else { // 有挂单
			//std::cout << "A有挂单" << std::endl;
			bool b_fined = false;
			list<CThostFtdcOrderField *>::iterator Itor;
			for (Itor = this->stg_list_order_pending->begin(); Itor != this->stg_list_order_pending->end(); Itor++) {
				if (!strcmp((*Itor)->OrderRef, pOrder->OrderRef)) { //报单引用相等
					/*std::cout << "挂单列表找到A合约" << std::endl;
					std::cout << "pOrder->VolumeTraded = " << pOrder->VolumeTraded << endl;
					std::cout << "(*Itor)->VolumeTraded = " << (*Itor)->VolumeTraded << endl;*/

					this->stg_b_order_insert_args->VolumeTotalOriginal = pOrder->VolumeTraded - (*Itor)->VolumeTraded; // B发单量等于本次回报A的成交量
					b_fined = true;
					break;
				}
			}
			if (!b_fined) { // 无挂单，但是属于分批成交, 第一批
				//std::cout << "无挂单，但是属于分批成交, 第一批" << std::endl;
				//std::cout << "pOrder->VolumeTraded = " << pOrder->VolumeTraded << endl;
				this->stg_b_order_insert_args->VolumeTotalOriginal = pOrder->VolumeTraded;
			}
		}

		this->stg_order_ref_b = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_b;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_b);
		
		strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_b.c_str());

		//this->stg_user->getUserTradeSPI()->OrderInsert(this->stg_user, this->stg_b_order_insert_args);
		this->Exec_OrderInsert(this->stg_b_order_insert_args);
	}
	/// B成交回报
	else if ((!strcmp(pOrder->InstrumentID, this->stg_instrument_id_B.c_str())) && (pOrder->OrderStatus == '0' || pOrder->OrderStatus == '1') && (strlen(pOrder->OrderSysID) != 0)) { // 全部成交或者部分成交

	}
	/// B撤单回报，启动B重新发单一定成交策略
	else if ((!strcmp(pOrder->InstrumentID, this->stg_instrument_id_B.c_str())) && (pOrder->OrderStatus == '5') && (strlen(pOrder->OrderSysID) != 0)) {
		this->stg_order_ref_b = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_b;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_b);

		if (pOrder->Direction == '0') {
			this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->AskPrice1;
		}
		else if (pOrder->Direction == '1') {
			this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->BidPrice1;
		}

		strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_b.c_str());
		this->stg_b_order_insert_args->VolumeTotalOriginal = pOrder->VolumeTotal;
		this->stg_b_order_insert_args->Direction = pOrder->Direction;
		this->stg_b_order_insert_args->CombOffsetFlag[0] = pOrder->CombOffsetFlag[0];
		this->stg_b_order_insert_args->CombHedgeFlag[0] = pOrder->CombHedgeFlag[0];

		//this->stg_user->getUserTradeSPI()->OrderInsert(this->stg_user, this->stg_b_order_insert_args);
		this->Exec_OrderInsert(this->stg_b_order_insert_args);
	}

}

// 成交回报
void Strategy::ExEc_OnRtnTrade(CThostFtdcTradeField *pTrade) {
	/*1:更新持仓明细*/
	/************************************************************************/
	/* 由于修改了持仓计算方式，故以下被注释                                                                     */
	/************************************************************************/
	//this->update_position_detail(pTrade); 
	//this->update_position(pTrade);
	//this->update_task_status();
}

// 报单录入错误回报
void Strategy::Exec_OnErrRtnOrderInsert() {
	USER_PRINT("Exec_OnErrRtnOrderInsert()");
	this->setStgSelectOrderAlgorithmFlag("Strategy::Exec_OnErrRtnOrderInsert()", false);
}

// 报单操作错误回报
void Strategy::Exec_OnErrRtnOrderAction() {
	USER_PRINT("Exec_OnErrRtnOrderAction()");
}

// 行情回调,执行交易任务
void Strategy::Exec_OnTickComing(CThostFtdcDepthMarketDataField *pDepthMarketData) {
	USER_PRINT("Exec_OnTickComing()");
	//std::cout << "行情回调" << std::endl;
	/*this->printStrategyInfo("行情回调打印挂单列表");
	std::cout << "行情挂单列表长度 = " << this->stg_list_order_pending->size() << std::endl;*/
	list<CThostFtdcOrderField *>::iterator Itor;
	for (Itor = this->stg_list_order_pending->begin(); Itor != this->stg_list_order_pending->end(); Itor++) {
		//this->printStrategyInfo("遍历挂单列表");
		/*std::cout << "遍历挂单:(*Itor)->InstrumentID = " << (*Itor)->InstrumentID << endl;
		std::cout << "遍历挂单:this->stg_instrument_id_A = " << this->stg_instrument_id_A << endl;
		std::cout << "遍历挂单:pDepthMarketData->InstrumentID = " << pDepthMarketData->InstrumentID << endl;*/
		/// A有挂单，判断是否需要撤单
		if (!strcmp((*Itor)->InstrumentID, this->stg_instrument_id_A.c_str())) {
			//this->printStrategyInfo("A有挂单");
			/// 通过A最新tick判断A合约是否需要撤单
			if (!strcmp(pDepthMarketData->InstrumentID, this->stg_instrument_id_A.c_str())) {
				/// A挂单方向为买
				if ((*Itor)->Direction == '0') {
					/*this->printStrategyInfo("A挂单方向为买，进入撤单判断");
					std::cout << "A_Tick 撤单判断:pDepthMarketData->BidPrice1 = " << pDepthMarketData->BidPrice1 << std::endl;
					std::cout << "A_Tick 撤单判断:(*Itor)->LimitPrice = " << (*Itor)->LimitPrice << std::endl;
					std::cout << "A_Tick 撤单判断:this->stg_a_wait_price_tick = " << this->stg_a_wait_price_tick << std::endl;
					std::cout << "A_Tick 撤单判断:this->stg_a_price_tick = " << this->stg_a_price_tick << std::endl;
					std::cout << "A_Tick 撤单判断:pDepthMarketData->BidPrice1 > ((*Itor)->LimitPrice + (this->stg_a_wait_price_tick * this->stg_a_price_tick)) = " << (pDepthMarketData->BidPrice1 > ((*Itor)->LimitPrice + (this->stg_a_wait_price_tick * this->stg_a_price_tick))) << std::endl;*/
					/// 挂单价格与盘口买一价比较，如果与盘口价格差距n个最小跳以上，撤单
					if (pDepthMarketData->BidPrice1 > ((*Itor)->LimitPrice + (this->stg_a_wait_price_tick * this->stg_a_price_tick))) {
						//std::cout << "A合约通过最新tick判断A合约买挂单符合撤单条件" << endl;
						/// A合约撤单
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
				else if ((*Itor)->Direction == '1') {
					/*this->printStrategyInfo("A挂单方向为卖，进入撤单判断");
					std::cout << "A_Tick 撤单判断:pDepthMarketData->AskPrice1 = " << pDepthMarketData->AskPrice1 << std::endl;
					std::cout << "A_Tick 撤单判断:(*Itor)->LimitPrice = " << (*Itor)->LimitPrice << std::endl;
					std::cout << "A_Tick 撤单判断:this->stg_a_wait_price_tick = " << this->stg_a_wait_price_tick << std::endl;
					std::cout << "A_Tick 撤单判断:this->stg_a_price_tick = " << this->stg_a_price_tick << std::endl;
					std::cout << "A_Tick 撤单判断:pDepthMarketData->AskPrice1 < ((*Itor)->LimitPrice - (this->stg_a_wait_price_tick * this->stg_a_price_tick)) = " << (pDepthMarketData->AskPrice1 < ((*Itor)->LimitPrice - (this->stg_a_wait_price_tick * this->stg_a_price_tick))) << std::endl;*/
					if (pDepthMarketData->AskPrice1 < ((*Itor)->LimitPrice - (this->stg_a_wait_price_tick * this->stg_a_price_tick))) {
						//std::cout << "Strategy::Exec_OnTickComing A挂单方向为卖，撤单" << std::endl;
						/// A合约撤单
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
			}

			//根据B的行情判断A是否需要撤单
			else if (!strcmp(pDepthMarketData->InstrumentID, this->stg_instrument_id_B.c_str())) {
				/// A挂单的买卖方向为买
				if ((*Itor)->Direction == '0') {
					/*this->printStrategyInfo("根据B的行情判断A是否需要撤单，A挂单方向为买，进入撤单判断");
					std::cout << "B_Tick 撤单判断:pDepthMarketData->BidPrice1" << pDepthMarketData->BidPrice1 << std::endl;
					std::cout << "B_Tick 撤单判断:this->stg_instrument_B_tick_last->BidPrice1" << this->stg_instrument_B_tick_last->BidPrice1 << std::endl;
					std::cout << "B_Tick 撤单判断:pDepthMarketData->BidPrice1 < this->stg_instrument_B_tick_last->BidPrice1 = " << (pDepthMarketData->BidPrice1 < this->stg_instrument_B_tick_last->BidPrice1) << std::endl;*/
					/// B最新tick的对手价如果与开仓信号触发时B的tick对手价发生不利变化则A撤单
					if (pDepthMarketData->BidPrice1 < (this->stg_instrument_B_tick_last->BidPrice1 - (this->stg_b_wait_price_tick * this->stg_b_price_tick))) {
						USER_PRINT("Strategy.trade_task() 通过B最新tick判断A合约买挂单符合撤单条件");
						/*this->printStrategyInfo("Strategy.trade_task() 通过B最新tick判断A合约买挂单符合撤单条件");
						std::cout << "B_Tick 符合撤单:pDepthMarketData->BidPrice1 = " << pDepthMarketData->BidPrice1 << std::endl;
						std::cout << "B_Tick 符合撤单:this->stg_instrument_B_tick_last->BidPrice1 = " << this->stg_instrument_B_tick_last->BidPrice1 << std::endl;
						std::cout << "B_Tick 符合撤单:this->stg_b_wait_price_tick = " << this->stg_b_wait_price_tick << std::endl;
						std::cout << "B_Tick 符合撤单:this->stg_b_price_tick = " << this->stg_b_price_tick << std::endl;*/
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
				/// A挂单的买卖方向为卖
				else if ((*Itor)->Direction == '1') {
					/*this->printStrategyInfo("根据B的行情判断A是否需要撤单，A挂单的方向为卖，进入撤单判断");
					std::cout << "B_Tick 撤单判断:pDepthMarketData->AskPrice1" << pDepthMarketData->AskPrice1 << std::endl;
					std::cout << "B_Tick 撤单判断:this->stg_instrument_B_tick_last->AskPrice1" << this->stg_instrument_B_tick_last->AskPrice1 << std::endl;
					std::cout << "B_Tick 撤单判断:pDepthMarketData->AskPrice1 > this->stg_instrument_B_tick_last->AskPrice1 = " << (pDepthMarketData->AskPrice1 > this->stg_instrument_B_tick_last->AskPrice1) << std::endl;*/
					/// B最新tick的对手价如果与开仓信号触发时B的tick对手价发生不利变化则A撤单
					if (pDepthMarketData->AskPrice1 > (this->stg_instrument_B_tick_last->AskPrice1 + (this->stg_b_wait_price_tick * this->stg_b_price_tick))) {
						USER_PRINT("Strategy.trade_task() 通过B最新tick判断A合约卖挂单符合撤单条件");
						/*this->printStrategyInfo("Strategy.trade_task()通过B最新tick判断A合约卖挂单符合撤单条件");
						std::cout << "B_Tick 符合撤单:pDepthMarketData->AskPrice1 = " << pDepthMarketData->AskPrice1 << std::endl;
						std::cout << "B_Tick 符合撤单:this->stg_instrument_B_tick_last->AskPrice1 = " << this->stg_instrument_B_tick_last->AskPrice1 << std::endl;
						std::cout << "B_Tick 符合撤单:this->stg_b_wait_price_tick = " << this->stg_b_wait_price_tick << std::endl;
						std::cout << "B_Tick 符合撤单:this->stg_b_price_tick = " << this->stg_b_price_tick << std::endl;*/
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
			}
		}

		/// B有挂单，判断是否需要撤单，并启动B合约一定成交策略
		if (!strcmp((*Itor)->InstrumentID, this->stg_instrument_id_B.c_str())) {
			/// 通过B最新tick判断B合约是否需要撤单
			if (!strcmp(pDepthMarketData->InstrumentID, this->stg_instrument_id_B.c_str())) {
				/// B挂单的买卖方向为买
				if ((*Itor)->Direction == '0') {
					/// 挂单价格与盘口买一价比较，如果与盘口价格差距n个最小跳以上，撤单
					if (pDepthMarketData->BidPrice1 >= ((*Itor)->LimitPrice + this->stg_b_wait_price_tick * this->stg_b_price_tick)) {
						USER_PRINT("通过B最新tick判断B合约买挂单符合撤单条件");
						//this->printStrategyInfo("通过B最新tick判断B合约买挂单符合撤单条件");
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
				/// B挂单的买卖方向为卖
				else if ((*Itor)->Direction == '1') {
					/// 挂单价格与盘口卖一价比较，如果与盘口价格差距n个最小跳以上，撤单
					if (pDepthMarketData->AskPrice1 <= ((*Itor)->LimitPrice - this->stg_b_wait_price_tick * this->stg_b_price_tick)) {
						USER_PRINT("通过B最新tick判断B合约卖挂单符合撤单条件");
						//this->printStrategyInfo("通过B最新tick判断B合约卖挂单符合撤单条件");
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
			}
		}
	}
}

/// 更新挂单list
void Strategy::update_pending_order_list(CThostFtdcOrderField *pOrder) {
	USER_PRINT("Strategy::update_pending_order_list");
	/************************************************************************/
	/* 此处pOrder应该谨慎操作                                                  */
	/************************************************************************/
	//std::cout << "pending_order_list size = "<< this->stg_list_order_pending->size() << std::endl;
	USER_PRINT(this->stg_list_order_pending->size());
	USER_PRINT(pOrder->OrderSysID);

	/************************************************************************/
	/* 
		order中的字段OrderStatus
		 0 全部成交
		 1 部分成交，订单还在交易所撮合队列中
		 3 未成交，订单还在交易所撮合队列中
		 5 已撤销
		 a 未知 - 订单已提交交易所，未从交易所收到确认信息
		                                                                     */
	/************************************************************************/

	//if (strlen(pOrder->OrderSysID) != 0) {
	//	//如果list为空,直接添加到挂单列表里
	//	if (this->stg_list_order_pending->size() == 0) {
	//		USER_PRINT("this->stg_list_order_pending->size() == 0");
	//		// 深复制对象
	//		CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
	//		memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
	//		this->CopyOrderData(pOrder_tmp, pOrder);

	//		this->stg_list_order_pending->push_back(pOrder_tmp);
	//		return;
	//	} else {
	//		USER_PRINT("this->stg_list_order_pending->size() > 0");
	//		/// 挂单列表不为空时
	//		list<CThostFtdcOrderField *>::iterator Itor;
	//		for (Itor = this->stg_list_order_pending->begin(); Itor != this->stg_list_order_pending->end();) {
	//			USER_PRINT((*Itor)->OrderRef);
	//			USER_PRINT(pOrder->OrderRef);
	//			if (!strcmp((*Itor)->OrderRef, pOrder->OrderRef)) { // 先前已经有orderref存在挂单编号里
	//				if (pOrder->OrderStatus == '0') { // 全部成交

	//					USER_PRINT("全部成交");
	//					delete (*Itor);
	//					Itor = this->stg_list_order_pending->erase(Itor); //移除

	//				} else if (pOrder->OrderStatus == '1') { // 部分成交还在队列中

	//					USER_PRINT("部分成交还在队列中");

	//					// 深复制对象
	//					/*CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
	//					memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
	//					this->CopyOrderData(pOrder_tmp, pOrder);

	//					*Itor = pOrder_tmp;*/
	//					this->CopyOrderData(*Itor, pOrder);


	//				} else if (pOrder->OrderStatus == '2') { // 部分成交不在队列中

	//					USER_PRINT("部分成交不在队列中");

	//				} else if (pOrder->OrderStatus == '3') { // 未成交还在队列中

	//					USER_PRINT("未成交还在队列中");
	//					// 深复制对象
	//					/*CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
	//					memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
	//					this->CopyOrderData(pOrder_tmp, pOrder);

	//					*Itor = pOrder_tmp;*/

	//					this->CopyOrderData(*Itor, pOrder);

	//				} else if (pOrder->OrderStatus == '4') { // 未成交不在队列中

	//					USER_PRINT("未成交不在队列中");


	//				} else if (pOrder->OrderStatus == '5') { // 撤单

	//					USER_PRINT("撤单");
	//					delete (*Itor);
	//					Itor = this->stg_list_order_pending->erase(Itor); //移除

	//				} else if (pOrder->OrderStatus == 'a') { // 未知

	//					USER_PRINT("未知");

	//				} else if (pOrder->OrderStatus == 'b') { // 尚未触发

	//					USER_PRINT("尚未触发");

	//				} else if (pOrder->OrderStatus == 'c') { // 已触发

	//					USER_PRINT("已触发");

	//				}

	//			} else {	// 没有orderref存在挂单编号里,第一次
	//				if (pOrder->OrderStatus == '0') { // 全部成交

	//				}
	//				else if (pOrder->OrderStatus == '1') { // 部分成交还在队列中
	//					USER_PRINT("没有orderref存在挂单列表,部分成交还在队列中");
	//					// 深复制对象
	//					CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
	//					memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
	//					this->CopyOrderData(pOrder_tmp, pOrder);

	//					this->stg_list_order_pending->push_back(pOrder_tmp);
	//				}
	//				else if (pOrder->OrderStatus == '2') { // 部分成交不在队列中

	//				}
	//				else if (pOrder->OrderStatus == '3') { // 未成交还在队列中
	//					USER_PRINT("没有orderref存在挂单列表,未成交还在队列中");
	//					// 深复制对象
	//					CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
	//					memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
	//					this->CopyOrderData(pOrder_tmp, pOrder);

	//					this->stg_list_order_pending->push_back(pOrder_tmp);
	//				}
	//				else if (pOrder->OrderStatus == '4') { // 未成交不在队列中

	//				}
	//				else if (pOrder->OrderStatus == '5') { // 撤单

	//				}
	//				else if (pOrder->OrderStatus == 'a') { // 未知

	//				}
	//				else if (pOrder->OrderStatus == 'b') { // 尚未触发

	//				}
	//				else if (pOrder->OrderStatus == 'c') { // 已触发

	//				}
	//			}
	//			Itor++;
	//		}
	//	}
	//}

	USER_PRINT(pOrder->OrderStatus);

	std::cout << "Strategy::update_pending_order_list()" << std::endl;
	std::cout << "\tthis->stg_pending_a_open = " << this->stg_pending_a_open << std::endl;


	if (strlen(pOrder->OrderSysID) != 0) { // 如果报单编号不为空，为交易所返回
		if (pOrder->OrderStatus == '0') { // 全部成交
			//std::cout << "更新挂单,全部成交" << std::endl;
			list<CThostFtdcOrderField *>::iterator itor;
			for (itor = this->stg_list_order_pending->begin(); itor != this->stg_list_order_pending->end();) {
				if (!strcmp((*itor)->OrderRef, pOrder->OrderRef)) {
					delete (*itor);
					itor = this->stg_list_order_pending->erase(itor); //移除
					break;
				}
				else {
					itor++;
				}
			}
		}
		else if (pOrder->OrderStatus == '1') { // 部分成交还在队列中
			//std::cout << "更新挂单,部分成交还在队列中" << std::endl;
			list<CThostFtdcOrderField *>::iterator itor;
			for (itor = this->stg_list_order_pending->begin(); itor != this->stg_list_order_pending->end();) {
				if (!strcmp((*itor)->OrderRef, pOrder->OrderRef)) {
					this->CopyOrderData(*itor, pOrder);
					break;
				}
				else {
					itor++;
				}
			}

		}
		else if (pOrder->OrderStatus == '3') { // 未成交还在队列中
			//std::cout << "更新挂单,未成交还在队列中" << std::endl;
			bool isExists = false;
			// 判断挂单列表是否存在
			list<CThostFtdcOrderField *>::iterator itor;
			for (itor = this->stg_list_order_pending->begin(); itor != this->stg_list_order_pending->end();) {
				if (!strcmp((*itor)->OrderRef, pOrder->OrderRef)) {
					// 存在置flag标志位
					isExists = true;
					//std::cout << "更新挂单,有挂单" << std::endl;
					this->CopyOrderData(*itor, pOrder);
					break;
				}
				else {
					itor++;
				}
			}
			// 如果不存在直接加入
			if (!isExists) {
				//std::cout << "更新挂单,无挂单" << std::endl;
				// 深复制对象
				CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
				memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
				this->CopyOrderData(pOrder_tmp, pOrder);

				this->stg_list_order_pending->push_back(pOrder_tmp);
			}
		}
		else if (pOrder->OrderStatus == '5') { // 撤单
			//std::cout << "更新挂单,撤单" << std::endl;
			list<CThostFtdcOrderField *>::iterator itor;
			for (itor = this->stg_list_order_pending->begin(); itor != this->stg_list_order_pending->end();) {
				if (!strcmp((*itor)->OrderRef, pOrder->OrderRef)) {
					delete (*itor);
					itor = this->stg_list_order_pending->erase(itor); //移除
					break;
				}
				else {
					itor++;
				}
			}
		}
		else if (pOrder->OrderStatus == 'a') { // 未知
			
		}

		// 遍历挂单列表，找出A合约开仓未成交的量
		list<CThostFtdcOrderField *>::iterator cal_itor;
		for (cal_itor = this->stg_list_order_pending->begin(); cal_itor != this->stg_list_order_pending->end(); cal_itor++) {
			// 对比InstrumentID
			if (!strcmp((*cal_itor)->InstrumentID, this->stg_instrument_id_A.c_str()) && ((*cal_itor)->CombOffsetFlag[0] == '0')) { // 查找A合约开仓
				this->stg_pending_a_open += (*cal_itor)->VolumeTotalOriginal - (*cal_itor)->VolumeTraded;
			}
		}

		//std::cout << "Strategy::update_pending_order_list()" << std::endl;
		std::cout << "\tthis->stg_pending_a_open = " << this->stg_pending_a_open << std::endl;
	}
}

/// 更新持仓量
void Strategy::update_position(CThostFtdcOrderField *pOrder) {
	USER_PRINT("update_position");

	if (!strcmp(pOrder->InstrumentID, this->stg_instrument_id_A.c_str())) { // A合约
		if (pOrder->CombOffsetFlag[0] == '0') { // A开仓成交回报
			if (pOrder->Direction == '0') { // A买开仓成交回报
				this->stg_position_a_buy_today += pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // A卖开仓成交回报
				this->stg_position_a_sell_today += pOrder->VolumeTraded;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '3') { // A平今成交回报
			if (pOrder->Direction == '0') { // A买平今成交回报
				this->stg_position_a_sell_today -= pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // A卖平今成交回报
				this->stg_position_a_buy_today -= pOrder->VolumeTraded;
			}
		} 
		else if (pOrder->CombOffsetFlag[0] == '4') { // A平昨成交回报
			if (pOrder->Direction == '0') { // A买平昨成交回报
				this->stg_position_a_sell_yesterday -= pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // A卖平昨成交回报
				this->stg_position_a_buy_yesterday -= pOrder->VolumeTraded;
			}
		}

		this->stg_position_a_buy = this->stg_position_a_buy_today + this->stg_position_a_buy_yesterday;
		this->stg_position_a_sell = this->stg_position_a_sell_today + this->stg_position_a_sell_yesterday;
	}
	else if (!strcmp(pOrder->InstrumentID, this->stg_instrument_id_B.c_str())) { // B合约
		if (pOrder->CombOffsetFlag[0] == '0') { // B开仓成交回报
			if (pOrder->Direction == '0') { // B买开仓成交回报
				this->stg_position_b_buy_today += pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // B卖开仓成交回报
				this->stg_position_b_sell_today += pOrder->VolumeTraded;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '3') { // B平今成交回报
			if (pOrder->Direction == '0') { // B买平今成交回报
				this->stg_position_b_sell_today -= pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // B卖平今成交回报
				this->stg_position_b_buy_today -= pOrder->VolumeTraded;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '4') { // B平昨成交回报
			if (pOrder->Direction == '0') { // B买平昨成交回报
				this->stg_position_b_sell_yesterday -= pOrder->VolumeTraded;
				//std::cout << "Strategy::update_position 2431 stg_position_b_sell_yesterday = " << this->stg_position_b_sell_yesterday << std::endl;
			}
			else if (pOrder->Direction == '1') { // B卖平昨成交回报
				this->stg_position_b_buy_yesterday -= pOrder->VolumeTraded;
			}
		}

		this->stg_position_b_buy = this->stg_position_b_buy_today + this->stg_position_b_buy_yesterday;
		this->stg_position_b_sell = this->stg_position_b_sell_today + this->stg_position_b_sell_yesterday;
	}
}

/// 更新持仓量(UserOrder)
void Strategy::update_position(USER_CThostFtdcOrderField *pOrder) {
	USER_PRINT("Strategy::update_position");
	USER_PRINT(pOrder->InstrumentID);
	USER_PRINT(this->stg_instrument_id_A);
	USER_PRINT(this->stg_instrument_id_B);

	this->printStrategyInfo("Strategy::update_position() 输出VolumeTradedBatch");
	

	// A成交
	if (!strcmp(pOrder->InstrumentID, this->stg_instrument_id_A.c_str())) {
		USER_PRINT(pOrder->CombOffsetFlag[0]);
		USER_PRINT(pOrder->Direction);
		USER_PRINT(pOrder->VolumeTradedBatch);
		if (pOrder->CombOffsetFlag[0] == '0') { // A开仓成交回报
			if (pOrder->Direction == '0') // A买开仓
			{
				this->stg_position_a_buy_today += pOrder->VolumeTradedBatch;
			}
			else if (pOrder->Direction == '1') { // A卖开仓
				this->stg_position_a_sell_today += pOrder->VolumeTradedBatch;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '3') { // A平今成交回报
			if (pOrder->Direction == '0') // A买平今成交回报
			{
				this->stg_position_a_sell_today -= pOrder->VolumeTradedBatch;
			}
			else if (pOrder->Direction == '1') { // A卖平今成交回报
				this->stg_position_a_buy_today -= pOrder->VolumeTradedBatch;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '4') { // A平昨成交回报
			if (pOrder->Direction == '0') // A买平昨成交回报
			{
				this->stg_position_a_sell_yesterday -= pOrder->VolumeTradedBatch;
			}
			else if (pOrder->Direction == '1') { // A卖平昨成交回报
				this->stg_position_a_buy_yesterday -= pOrder->VolumeTradedBatch;
			}
		}

		this->stg_position_a_buy = this->stg_position_a_buy_today + this->stg_position_a_buy_yesterday;
		this->stg_position_a_sell = this->stg_position_a_sell_today + this->stg_position_a_sell_yesterday;

	}
	// B成交
	else if (!strcmp(pOrder->InstrumentID, this->stg_instrument_id_B.c_str()))
	{
		USER_PRINT(pOrder->CombOffsetFlag[0]);
		USER_PRINT(pOrder->Direction);
		USER_PRINT(pOrder->VolumeTradedBatch);
		if (pOrder->CombOffsetFlag[0] == '0') { // B开仓成交回报
			if (pOrder->Direction == '0') // B买开仓
			{
				this->stg_position_b_buy_today += pOrder->VolumeTradedBatch;
			}
			else if (pOrder->Direction == '1') { // B卖开仓
				this->stg_position_b_sell_today += pOrder->VolumeTradedBatch;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '3') { // B平今成交回报
			if (pOrder->Direction == '0') // B买平今成交回报
			{
				this->stg_position_b_sell_today -= pOrder->VolumeTradedBatch;
			}
			else if (pOrder->Direction == '1') { // B卖平今成交回报
				this->stg_position_b_buy_today -= pOrder->VolumeTradedBatch;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '4') { // B平昨成交回报
			if (pOrder->Direction == '0') // B买平昨成交回报
			{
				this->stg_position_b_sell_yesterday -= pOrder->VolumeTradedBatch;
				//std::cout << "Strategy::update_position 2515 stg_position_b_sell_yesterday = " << stg_position_b_sell_yesterday << std::endl;
			}
			else if (pOrder->Direction == '1') { // B卖平昨成交回报
				this->stg_position_b_buy_yesterday -= pOrder->VolumeTradedBatch;
			}
		}

		this->stg_position_b_buy = this->stg_position_b_buy_today + this->stg_position_b_buy_yesterday;
		this->stg_position_b_sell = this->stg_position_b_sell_today + this->stg_position_b_sell_yesterday;
	}

	//this->printStrategyInfo("Strategy::update_position() 更新持仓变量");

	//std::cout << "A合约总买 = " << this->stg_position_a_buy << std::endl;
	//std::cout << "A合约今买 = " << this->stg_position_a_buy_today << std::endl;
	//std::cout << "A合约昨买 = " << this->stg_position_a_buy_yesterday << std::endl;
	//std::cout << "A合约总卖 = " << this->stg_position_a_sell << std::endl;
	//std::cout << "A合约今卖 = " << this->stg_position_a_sell_today << std::endl;
	//std::cout << "A合约昨卖 = " << this->stg_position_a_buy_yesterday << std::endl;

	//std::cout << "B合约总卖 = " << this->stg_position_b_sell << std::endl;
	//std::cout << "B合约今卖 = " << this->stg_position_b_sell_today << std::endl;
	//std::cout << "B合约昨卖 = " << this->stg_position_b_sell_yesterday << std::endl;
	//std::cout << "B合约总买 = " << this->stg_position_b_buy << std::endl;
	//std::cout << "B合约今买 = " << this->stg_position_b_buy_today << std::endl;
	//std::cout << "B合约昨买 = " << this->stg_position_b_buy_yesterday << std::endl;

	std::cout << "Strategy::update_position():" << std::endl;
	std::cout << "\tA买(" << this->stg_position_a_sell << ", " << this->stg_position_a_sell_yesterday << ")" << std::endl;
	std::cout << "\tB买(" << this->stg_position_b_buy << ", " << this->stg_position_b_buy_yesterday << ")" << std::endl;
	std::cout << "\tA买(" << this->stg_position_a_buy << ", " << this->stg_position_a_buy_yesterday << ")" << std::endl;
	std::cout << "\tB卖(" << this->stg_position_b_sell << ", " << this->stg_position_b_sell_yesterday << ")" << std::endl;
	std::cout << "\t挂单列表长度 = " << this->stg_list_order_pending->size() << std::endl;
	std::cout << "\t任务执行状态 = " << this->stg_trade_tasking << std::endl;
	std::cout << "\t本次成交量 = " << pOrder->VolumeTradedBatch << ", 报单引用 = " << pOrder->OrderRef << std::endl;

}

/// 更新持仓量
void Strategy::update_position(CThostFtdcTradeField *pTrade) {
	//std::cout << "update_position(CThostFtdcTradeField *pTrade)" << std::endl;
	/// A成交
	if (!strcmp(pTrade->InstrumentID, this->stg_instrument_id_A.c_str())) {
		if (pTrade->OffsetFlag == '0') { // A开仓成交回报
			if (pTrade->Direction == '0') { // A买开仓
				this->stg_position_a_buy_today += pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖开仓
				this->stg_position_a_sell_today += pTrade->Volume;
			}
		}
		else if (pTrade->OffsetFlag == '3') { // A平今
			if (pTrade->Direction == '0') { // A买平今
				this->stg_position_a_sell_today -= pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖平今
				this->stg_position_a_buy_today -= pTrade->Volume;
			}
		}
		else if (pTrade->OffsetFlag == '4') { // A平昨
			if (pTrade->Direction == '0') { // A买平昨
				this->stg_position_a_sell_yesterday -= pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖平昨
				this->stg_position_a_buy_yesterday -= pTrade->Volume;
			}
		}
		this->stg_position_a_buy = this->stg_position_a_buy_today + this->stg_position_a_buy_yesterday;
		this->stg_position_a_sell = this->stg_position_a_sell_today + this->stg_position_a_sell_yesterday;
	}
	/// B成交
	else if (!strcmp(pTrade->InstrumentID, this->stg_instrument_id_B.c_str())) {
		if (pTrade->OffsetFlag == '0') { // A开仓成交回报
			if (pTrade->Direction == '0') { // A买开仓
				this->stg_position_b_buy_today += pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖开仓
				this->stg_position_b_sell_today += pTrade->Volume;
			}
		}
		else if (pTrade->OffsetFlag == '3') { // A平今
			if (pTrade->Direction == '0') { // A买平今
				this->stg_position_b_sell_today -= pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖平今
				this->stg_position_b_buy_today -= pTrade->Volume;
			}
		}
		else if (pTrade->OffsetFlag == '4') { // A平昨
			if (pTrade->Direction == '0') { // A买平昨
				this->stg_position_b_sell_yesterday -= pTrade->Volume;
				//std::cout << "Strategy::update_position 2622 stg_position_b_sell_yesterday = " << this->stg_position_b_sell_yesterday << std::endl;
			}
			else if (pTrade->Direction == '1') { // A卖平昨
				this->stg_position_b_buy_yesterday -= pTrade->Volume;
			}
		}
		this->stg_position_b_buy = this->stg_position_b_buy_today + this->stg_position_b_buy_yesterday;
		this->stg_position_b_sell = this->stg_position_b_sell_today + this->stg_position_b_sell_yesterday;
	}

	/*std::cout << "A合约今买 = " << this->stg_position_a_buy_today << ", "
		<< "A合约昨买 = " << this->stg_position_a_buy_yesterday << ", "
		<< "A合约总买 = " << this->stg_position_a_buy << ", "
		<< "A合约今卖 = " << this->stg_position_a_sell_today << ", "
		<< "A合约昨卖 = " << this->stg_position_a_buy_yesterday << ", "
		<< "A合约总卖 = " << this->stg_position_a_sell << std::endl;
	
	std::cout << "B合约今买 = " << this->stg_position_b_buy_today << ", "
		<< "B合约昨买 = " << this->stg_position_b_buy_yesterday << ", "
		<< "B合约总买 = " << this->stg_position_b_buy << ", "
		<< "B合约今卖 = " << this->stg_position_b_sell_today << ", "
		<< "B合约昨卖 = " << this->stg_position_b_buy_yesterday << ", "
		<< "B合约总卖 = " << this->stg_position_b_sell << std::endl;*/

	USER_PRINT("A合约今买");
	USER_PRINT(this->stg_position_a_buy_today);
	USER_PRINT("A合约昨买");
	USER_PRINT(this->stg_position_a_buy_yesterday);
	USER_PRINT("A合约总买");
	USER_PRINT(this->stg_position_a_buy);
	USER_PRINT("A合约今卖");
	USER_PRINT(this->stg_position_a_sell_today);
	USER_PRINT("A合约昨卖");
	USER_PRINT(this->stg_position_a_sell_yesterday);
	USER_PRINT("A合约总卖");
	USER_PRINT(this->stg_position_a_sell);

	USER_PRINT("B合约今买");
	USER_PRINT(this->stg_position_b_buy_today);
	USER_PRINT("B合约昨买");
	USER_PRINT(this->stg_position_b_buy_yesterday);
	USER_PRINT("B合约总买");
	USER_PRINT(this->stg_position_b_buy);
	USER_PRINT("B合约今卖");
	USER_PRINT(this->stg_position_b_sell_today);
	USER_PRINT("B合约昨卖");
	USER_PRINT(this->stg_position_b_sell_yesterday);
	USER_PRINT("B合约总卖");
	USER_PRINT(this->stg_position_b_sell);

}

/// 更新持仓明细
void Strategy::update_position_detail(CThostFtdcTradeField *pTrade_cal) {
	CThostFtdcTradeField *pTrade = new CThostFtdcTradeField();
	memset(pTrade, 0x00, sizeof(CThostFtdcTradeField));
	this->CopyTradeData(pTrade, pTrade_cal);


	// 开仓单,添加到list，添加到list尾部
	if (pTrade->OffsetFlag == '0') {
		CThostFtdcTradeField *pTrade_tmp = new CThostFtdcTradeField();
		memset(pTrade_tmp, 0x00, sizeof(CThostFtdcTradeField));
		this->CopyTradeData(pTrade_tmp, pTrade_cal);

		this->stg_list_position_detail->push_back(pTrade_tmp);
	}
	else if (pTrade->OffsetFlag == '1') { // 平仓单,先开先平的原则从list里删除
		list<CThostFtdcTradeField *>::iterator Itor;
		for (Itor = this->stg_list_position_detail->begin(); Itor != this->stg_list_position_detail->end();) {
			if (pTrade->OffsetFlag == '3') { // 平今
				if ((!strcmp((*Itor)->TradingDay, pTrade->TradingDay)) 
					&& (!strcmp((*Itor)->InstrumentID, pTrade->InstrumentID))
					&& ((*Itor)->HedgeFlag == pTrade->HedgeFlag) 
					&& ((*Itor)->Direction == pTrade->Direction)) {
						// pTrade的volume等于持仓明细列表里的volume
						if (((*Itor)->Volume) == (pTrade->Volume)) {
							delete (*Itor);
							Itor = this->stg_list_position_detail->erase(Itor);
						}
						else if (((*Itor)->Volume) > (pTrade->Volume)) {
							(*Itor)->Volume -= pTrade->Volume;
						}
						else if (((*Itor)->Volume) < (pTrade->Volume)) {
							pTrade->Volume -= (*Itor)->Volume;
							Itor = this->stg_list_position_detail->erase(Itor);
						}
				}
			}
			else if (pTrade->OffsetFlag == '4') {
				if ((!strcmp((*Itor)->TradingDay, pTrade->TradingDay))
					&& (!strcmp((*Itor)->InstrumentID, pTrade->InstrumentID))
					&& ((*Itor)->HedgeFlag == pTrade->HedgeFlag)
					&& ((*Itor)->Direction == pTrade->Direction)) {
						// pTrade的volume等于持仓明细列表里的volume
						if (((*Itor)->Volume) == (pTrade->Volume)) {
							delete (*Itor);
							Itor = this->stg_list_position_detail->erase(Itor);
						}
						else if (((*Itor)->Volume) > (pTrade->Volume)) {
							(*Itor)->Volume -= pTrade->Volume;
						}
						else if (((*Itor)->Volume) < (pTrade->Volume)) {
							pTrade->Volume -= (*Itor)->Volume;
							Itor = this->stg_list_position_detail->erase(Itor);
						}
				}
			}
		}
	}
}

/// 更新持仓明细(Order)
void Strategy::update_position_detail(USER_CThostFtdcOrderField *pOrder) {
	USER_PRINT("Strategy::update_position_detail");
	/************************************************************************/
	/* """
		order中的CombOffsetFlag 或 trade中的OffsetFlag值枚举：
		0：开仓
		1：平仓
		3：平今
		4：平昨
		"""
		# 跳过无成交的order记录                                                                     */
	/************************************************************************/

	USER_PRINT(pOrder->VolumeTraded);
	

	if (pOrder->VolumeTraded == 0) {
		return;
	}
	USER_PRINT(pOrder->CombOffsetFlag[0]);
	if (pOrder->CombOffsetFlag[0] == '0') { // 开仓
		USER_PRINT("pOrder->CombOffsetFlag[0] == 0 come in");
		USER_PRINT(pOrder);

		USER_CThostFtdcOrderField *new_order = new USER_CThostFtdcOrderField();
		memset(new_order, 0, sizeof(USER_CThostFtdcOrderField));
		this->CopyNewOrderData(new_order, pOrder);
		USER_PRINT(new_order);

		this->stg_list_position_detail_from_order->push_back(new_order);
		USER_PRINT("pOrder->CombOffsetFlag[0] == 0 away");
	} else if (pOrder->CombOffsetFlag[0] == '3') { // 平今
		USER_PRINT("平今in");
		list<USER_CThostFtdcOrderField *>::iterator itor;
		for (itor = this->stg_list_position_detail_from_order->begin(); 
			itor != this->stg_list_position_detail_from_order->end();)
		{
			USER_PRINT((*itor)->TradingDay);
			USER_PRINT(pOrder->TradingDay);

			USER_PRINT((*itor)->InstrumentID);
			USER_PRINT(pOrder->InstrumentID);

			USER_PRINT((*itor)->CombHedgeFlag[0]);
			USER_PRINT(pOrder->CombHedgeFlag[0]);

			USER_PRINT((*itor)->VolumeTradedBatch);
			USER_PRINT(pOrder->VolumeTradedBatch);
			

			if ((!strcmp((*itor)->TradingDay, pOrder->TradingDay)) 
				&& (!strcmp((*itor)->InstrumentID, pOrder->InstrumentID)) 
				&& ((*itor)->CombHedgeFlag[0] == pOrder->CombHedgeFlag[0])) { // 日期,合约代码,投保标志相同

				if (pOrder->VolumeTradedBatch == (*itor)->VolumeTradedBatch) { // order_new的VolumeTradedBatch等于持仓列表首个满足条件的order的VolumeTradedBatch
					USER_PRINT("order_new的VolumeTradedBatch等于持仓列表首个满足条件的order的VolumeTradedBatch");
					USER_PRINT((*itor));
					delete *itor;
					USER_PRINT("delete itor");
					itor = this->stg_list_position_detail_from_order->erase(itor);
					break;
				}
				else if (pOrder->VolumeTradedBatch < (*itor)->VolumeTradedBatch) // order_new的VolumeTradedBatch小于持仓列表首个满足条件的order的VolumeTradedBatch
				{
					(*itor)->VolumeTradedBatch -= pOrder->VolumeTradedBatch;
					break;
				}
				else if (pOrder->VolumeTradedBatch > (*itor)->VolumeTradedBatch) { // order_new的VolumeTradedBatch大于持仓列表首个满足条件的order的VolumeTradedBatch
					pOrder->VolumeTradedBatch -= (*itor)->VolumeTradedBatch;
					delete (*itor);
					itor = this->stg_list_position_detail_from_order->erase(itor);
				}
			}
			else {
				itor++;
			}
		}
		USER_PRINT("平今out");
	}
	else if (pOrder->CombOffsetFlag[0] == '4') // 平昨
	{
		list<USER_CThostFtdcOrderField *>::iterator itor;
		for (itor = this->stg_list_position_detail_from_order->begin();
			itor != this->stg_list_position_detail_from_order->end();)
		{
			if ((strcmp((*itor)->TradingDay, pOrder->TradingDay))
				&& (!strcmp((*itor)->InstrumentID, pOrder->InstrumentID))
				&& ((*itor)->CombHedgeFlag[0] == pOrder->CombHedgeFlag[0])) { // 日期,合约代码,投保标志相同

				if (pOrder->VolumeTradedBatch == (*itor)->VolumeTradedBatch) { // order_new的VolumeTradedBatch等于持仓列表首个满足条件的order的VolumeTradedBatch
					delete (*itor);
					itor = this->stg_list_position_detail_from_order->erase(itor);
					break;
				}
				else if (pOrder->VolumeTradedBatch < (*itor)->VolumeTradedBatch) // order_new的VolumeTradedBatch小于持仓列表首个满足条件的order的VolumeTradedBatch
				{
					(*itor)->VolumeTradedBatch -= pOrder->VolumeTradedBatch;
					break;
				}
				else if (pOrder->VolumeTradedBatch >(*itor)->VolumeTradedBatch) { // order_new的VolumeTradedBatch大于持仓列表首个满足条件的order的VolumeTradedBatch
					pOrder->VolumeTradedBatch -= (*itor)->VolumeTradedBatch;
					delete (*itor);
					itor = this->stg_list_position_detail_from_order->erase(itor);
				}
			}
			else {
				itor++;
			}
		}
	}

	USER_PRINT("Strategy::update_position_detail out");
}

/// 添加字段本次成交量至order构体中
void Strategy::add_VolumeTradedBatch(CThostFtdcOrderField *pOrder, USER_CThostFtdcOrderField *new_Order) {
	USER_PRINT("Strategy::add_VolumeTradedBatch");
	this->CopyOrderDataToNew(new_Order, pOrder);
	USER_PRINT(new_Order->OrderStatus);

	//if (new_Order->OrderStatus == '1' || new_Order->OrderStatus == '0') { // 全部成交或者部分成交还在队列中
	//	USER_PRINT(new_Order->VolumeTotalOriginal);
	//	if (new_Order->VolumeTotalOriginal == 1) {
	//		new_Order->VolumeTradedBatch = 1;
	//	}
	//	else {
	//		/// 遍历挂单列表
	//		list<CThostFtdcOrderField *>::iterator Itor;
	//		for (Itor = this->stg_list_order_pending->begin(); Itor != this->stg_list_order_pending->end(); Itor++) {
	//			USER_PRINT((*Itor)->OrderRef);
	//			USER_PRINT(new_Order->OrderRef);
	//			if (!strcmp((*Itor)->OrderRef, new_Order->OrderRef)) {
	//				new_Order->VolumeTradedBatch = new_Order->VolumeTraded - (*Itor)->VolumeTraded;
	//				break;
	//			}
	//		}
	//	}
	//}
	//else
	//{
	//	new_Order->VolumeTradedBatch = 0;
	//}


	if (new_Order->OrderStatus == '0') { // 全部成交
		new_Order->VolumeTradedBatch = new_Order->VolumeTotalOriginal;
		
	}
	else if (new_Order->OrderStatus == '1') // 部分成交还在队列中
	{
		bool is_exists = false;
		/// 遍历挂单列表
		list<CThostFtdcOrderField *>::iterator Itor;
		for (Itor = this->stg_list_order_pending->begin(); Itor != this->stg_list_order_pending->end(); Itor++) {
			USER_PRINT((*Itor)->OrderRef);
			USER_PRINT(new_Order->OrderRef);
			if (!strcmp((*Itor)->OrderRef, new_Order->OrderRef)) {
				is_exists = true;
				new_Order->VolumeTradedBatch = new_Order->VolumeTraded - (*Itor)->VolumeTraded;
				break;
			}
		}

		if (!is_exists) {
			new_Order->VolumeTradedBatch = new_Order->VolumeTraded;
		}

		
	}
	else
	{
		new_Order->VolumeTradedBatch = 0;
	}

	std::cout << "Strategy::add_VolumeTradedBatch() 合约 = " << new_Order->InstrumentID << ", 买卖 = " << new_Order->Direction << ", 开平 = " << new_Order->CombOffsetFlag[0] << ", 本次成交量 = " << new_Order->VolumeTradedBatch << ", 报单引用 = " << new_Order->OrderRef << std::endl;
	USER_PRINT(new_Order->VolumeTradedBatch);
}

/// 得到三个数最小值
int Strategy::getMinNum(int num1, int num2, int num3) {
	int minNum = 0;
	if (num1 < num2) {
		minNum = num1;
	}
	else {
		minNum = num2;
	}
	if (num3 < minNum) {
		minNum = num3;
	}
	return minNum;
}

void Strategy::setTrader(Trader *trader) {
	this->trader = trader;
}

Trader * Strategy::getTrader() {
	return this->trader;
}

void Strategy::setUser(User *user) {
	this->user = user;
}

User * Strategy::getUser() {
	return this->user;
}

void Strategy::setStrategyId(string strategyid) {
	this->strategyid = strategyid;
}

string Strategy::getStrategyId() {
	return this->strategyid;
}

void Strategy::setAlgorithm(Algorithm *alg) {
	this->alg = alg;
}

Algorithm * Strategy::getAlgorithm() {
	return this->alg;
}

void Strategy::addInstrumentToList(string instrument) {
	this->l_instruments->push_back(instrument);
}

list<string> * Strategy::getListInstruments() {
	return this->l_instruments;
}

void Strategy::setTraderID(string traderid) {
	this->traderid = traderid;
}
string Strategy::getTraderID() {
	return this->traderid;
}

void Strategy::setUserID(string userid) {
	this->userid = userid;
}
string Strategy::getUserID() {
	return this->userid;
}



CThostFtdcInputOrderField* Strategy::getStgAOrderInsertArgs() {
	return stg_a_order_insert_args;
}

void Strategy::setStgAOrderInsertArgs(
	CThostFtdcInputOrderField* stgAOrderInsertArgs) {
	stg_a_order_insert_args = stgAOrderInsertArgs;
}

double Strategy::getStgAPriceTick() {
	return stg_a_price_tick;
}

void Strategy::setStgAPriceTick(double stgAPriceTick) {
	stg_a_price_tick = stgAPriceTick;
}

double Strategy::getStgAWaitPriceTick() {
	return stg_a_wait_price_tick;
}

void Strategy::setStgAWaitPriceTick(double stgAWaitPriceTick) {
	stg_a_wait_price_tick = stgAWaitPriceTick;
}

CThostFtdcInputOrderField* Strategy::getStgBOrderInsertArgs() {
	return stg_b_order_insert_args;
}

void Strategy::setStgBOrderInsertArgs(
	CThostFtdcInputOrderField* stgBOrderInsertArgs) {
	stg_b_order_insert_args = stgBOrderInsertArgs;
}

double Strategy::getStgBPriceTick() {
	return stg_b_price_tick;
}

void Strategy::setStgBPriceTick(double stgBPriceTick) {
	stg_b_price_tick = stgBPriceTick;
}



double Strategy::getStgBWaitPriceTick() {
	return stg_b_wait_price_tick;
}

void Strategy::setStgBWaitPriceTick(double stgBWaitPriceTick) {
	stg_b_wait_price_tick = stgBWaitPriceTick;
}

double Strategy::getStgBuyClose() {
	return stg_buy_close;
}

void Strategy::setStgBuyClose(double stgBuyClose) {
	stg_buy_close = stgBuyClose;
}

double Strategy::getStgBuyOpen() {
	return stg_buy_open;
}

void Strategy::setStgBuyOpen(double stgBuyOpen) {
	stg_buy_open = stgBuyOpen;
}

DBManager* Strategy::getStgDbm() {
	return stg_DBM;
}

void Strategy::setStgDbm(DBManager* stgDbm) {
	stg_DBM = stgDbm;
}

CThostFtdcDepthMarketDataField* Strategy::getStgInstrumentATick() {
	return stg_instrument_A_tick;
}

void Strategy::setStgInstrumentATick(
	CThostFtdcDepthMarketDataField* stgInstrumentATick) {
	stg_instrument_A_tick = stgInstrumentATick;
}

CThostFtdcDepthMarketDataField* Strategy::getStgInstrumentBTick() {
	return stg_instrument_B_tick;
}

void Strategy::setStgInstrumentBTick(
	CThostFtdcDepthMarketDataField* stgInstrumentBTick) {
	stg_instrument_B_tick = stgInstrumentBTick;
}

CThostFtdcDepthMarketDataField* Strategy::getStgInstrumentATickLast() {
	return stg_instrument_A_tick_last;
}
void Strategy::setStgInstrumentATickLast(
	CThostFtdcDepthMarketDataField* stgInstrumentATick) {
	stg_instrument_A_tick_last = stgInstrumentATick;
}

CThostFtdcDepthMarketDataField* Strategy::getStgInstrumentBTickLast() {
	return stg_instrument_B_tick_last;
}
void Strategy::setStgInstrumentBTickLast(
	CThostFtdcDepthMarketDataField* stgInstrumentBTick) {
	stg_instrument_B_tick_last = stgInstrumentBTick;
}

string Strategy::getStgInstrumentIdA() {
	return stg_instrument_id_A;
}

void Strategy::setStgInstrumentIdA(string stgInstrumentIdA) {
	stg_instrument_id_A = stgInstrumentIdA;
}

string Strategy::getStgInstrumentIdB() {
	return stg_instrument_id_B;
}

void Strategy::setStgInstrumentIdB(string stgInstrumentIdB) {
	stg_instrument_id_B = stgInstrumentIdB;
}

bool Strategy::isStgIsActive() {
	return stg_is_active;
}

void Strategy::setStgIsActive(bool stgIsActive) {
	stg_is_active = stgIsActive;
}

list< CThostFtdcOrderField *> * Strategy::getStgListOrderPending() {
	return stg_list_order_pending;
}

void Strategy::setStgListOrderPending(list<CThostFtdcOrderField *> * stgListOrderPending) {
	stg_list_order_pending = stgListOrderPending;
}

int Strategy::getStgLots() {
	return stg_lots;
}

void Strategy::setStgLots(int stgLots) {
	stg_lots = stgLots;
}

int Strategy::getStgLotsBatch() {
	return stg_lots_batch;
}

void Strategy::setStgLotsBatch(int stgLotsBatch) {
	stg_lots_batch = stgLotsBatch;
}

int Strategy::isStgOnlyClose() {
	return stg_only_close;
}

void Strategy::setStgOnlyClose(int stgOnlyClose) {
	stg_only_close = stgOnlyClose;
}

int Strategy::getStgAOrderActionTiresLimit() {
	return stg_a_order_action_tires_limit;
}

void Strategy::setStgAOrderActionTiresLimit(int stgOrderActionTiresLimit) {
	stg_a_order_action_tires_limit = stgOrderActionTiresLimit;
}

int Strategy::getStgBOrderActionTiresLimit() {
	return stg_b_order_action_tires_limit;
}

void Strategy::setStgBOrderActionTiresLimit(int stgOrderActionTiresLimit) {
	stg_b_order_action_tires_limit = stgOrderActionTiresLimit;
}

int Strategy::getStgAOrderActionCount() {
	return this->stg_a_order_action_count;
}
void Strategy::setStgAOrderActionCount(int stg_a_order_action_count) {
	this->stg_a_order_action_count = stg_a_order_action_count;
}

int Strategy::getStgBOrderActionCount() {
	return this->stg_b_order_action_count;
}
void Strategy::setStgBOrderActionCount(int stg_b_order_action_count) {
	this->stg_b_order_action_count = stg_b_order_action_count;
}

int Strategy::getStgALimitPriceShift() {
	return this->stg_a_limit_price_shift;
}

void Strategy::setStgALimitPriceShift(int stg_a_limit_price_shift) {
	this->stg_a_limit_price_shift = stg_a_limit_price_shift;
}

int Strategy::getStgBLimitPriceShift() {
	return this->stg_b_limit_price_shift;
}

void Strategy::setStgBLimitPriceShift(int stg_b_limit_price_shift) {
	this->stg_b_limit_price_shift = stg_b_limit_price_shift;
}

string Strategy::getStgOrderRefA() {
	return stg_order_ref_a;
}

void Strategy::setStgOrderRefA(string stgOrderRefA) {
	stg_order_ref_a = stgOrderRefA;
}

string Strategy::getStgOrderRefB() {
	return stg_order_ref_b;
}

void Strategy::setStgOrderRefB(string stgOrderRefB) {
	stg_order_ref_b = stgOrderRefB;
}

string Strategy::getStgOrderRefLast() {
	return stg_order_ref_last;
}

void Strategy::setStgOrderRefLast(string stgOrderRefLast) {
	stg_order_ref_last = stgOrderRefLast;
}

int Strategy::getStgPositionABuy() {
	return stg_position_a_buy;
}

void Strategy::setStgPositionABuy(int stgPositionABuy) {
	stg_position_a_buy = stgPositionABuy;
}

int Strategy::getStgPositionABuyToday() {
	return stg_position_a_buy_today;
}

void Strategy::setStgPositionABuyToday(int stgPositionABuyToday) {
	stg_position_a_buy_today = stgPositionABuyToday;
}

int Strategy::getStgPositionABuyYesterday() {
	return stg_position_a_buy_yesterday;
}

void Strategy::setStgPositionABuyYesterday(int stgPositionABuyYesterday) {
	stg_position_a_buy_yesterday = stgPositionABuyYesterday;
}

int Strategy::getStgPositionASell() {
	return stg_position_a_sell;
}

void Strategy::setStgPositionASell(int stgPositionASell) {
	stg_position_a_sell = stgPositionASell;
}

int Strategy::getStgPositionASellToday() {
	return stg_position_a_sell_today;
}

void Strategy::setStgPositionASellToday(int stgPositionASellToday) {
	stg_position_a_sell_today = stgPositionASellToday;
}

int Strategy::getStgPositionASellYesterday() {
	return stg_position_a_sell_yesterday;
}

void Strategy::setStgPositionASellYesterday(int stgPositionASellYesterday) {
	stg_position_a_sell_yesterday = stgPositionASellYesterday;
}

int Strategy::getStgPositionBBuy() {
	return stg_position_b_buy;
}

void Strategy::setStgPositionBBuy(int stgPositionBBuy) {
	stg_position_b_buy = stgPositionBBuy;
}

int Strategy::getStgPositionBBuyToday() {
	return stg_position_b_buy_today;
}

void Strategy::setStgPositionBBuyToday(int stgPositionBBuyToday) {
	stg_position_b_buy_today = stgPositionBBuyToday;
}

int Strategy::getStgPendingAOpen() {
	return this->stg_pending_a_open;
}
void Strategy::setStgPendingAOpen(int stg_pending_a_open) {
	this->stg_pending_a_open = stg_pending_a_open;
}

int Strategy::getStgPositionBBuyYesterday() {
	return stg_position_b_buy_yesterday;
}

void Strategy::setStgPositionBBuyYesterday(int stgPositionBBuyYesterday) {
	stg_position_b_buy_yesterday = stgPositionBBuyYesterday;
}

int Strategy::getStgPositionBSell() {
	return stg_position_b_sell;
}

void Strategy::setStgPositionBSell(int stgPositionBSell) {
	stg_position_b_sell = stgPositionBSell;
}

int Strategy::getStgPositionBSellToday() {
	return stg_position_b_sell_today;
}

void Strategy::setStgPositionBSellToday(int stgPositionBSellToday) {
	stg_position_b_sell_today = stgPositionBSellToday;
}

int Strategy::getStgPositionBSellYesterday() {
	return stg_position_b_sell_yesterday;
}

void Strategy::setStgPositionBSellYesterday(int stgPositionBSellYesterday) {
	stg_position_b_sell_yesterday = stgPositionBSellYesterday;
}

double Strategy::getStgSellClose() {
	return stg_sell_close;
}

void Strategy::setStgSellClose(double stgSellClose) {
	stg_sell_close = stgSellClose;
}

double Strategy::getStgSellOpen() {
	return stg_sell_open;
}

void Strategy::setStgSellOpen(double stgSellOpen) {
	stg_sell_open = stgSellOpen;
}

double Strategy::getStgSpread() {
	return stg_spread;
}

void Strategy::setStgSpread(double stgSpread) {
	stg_spread = stgSpread;
}

double Strategy::getStgSpreadLong() {
	return stg_spread_long;
}

void Strategy::setStgSpreadLong(double stgSpreadLong) {
	stg_spread_long = stgSpreadLong;
}

int Strategy::getStgSpreadLongVolume() {
	return stg_spread_long_volume;
}

void Strategy::setStgSpreadLongVolume(int stgSpreadLongVolume) {
	stg_spread_long_volume = stgSpreadLongVolume;
}

double Strategy::getStgSpreadShift() {
	return stg_spread_shift;
}

void Strategy::setStgSpreadShift(double stgSpreadShift) {
	stg_spread_shift = stgSpreadShift;
}

double Strategy::getStgSpreadShort() {
	return stg_spread_short;
}

void Strategy::setStgSpreadShort(double stgSpreadShort) {
	stg_spread_short = stgSpreadShort;
}

int Strategy::getStgSpreadShortVolume() {
	return stg_spread_short_volume;
}

void Strategy::setStgSpreadShortVolume(int stgSpreadShortVolume) {
	stg_spread_short_volume = stgSpreadShortVolume;
}

double Strategy::getStgStopLoss() {
	return stg_stop_loss;
}

void Strategy::setStgStopLoss(double stgStopLoss) {
	stg_stop_loss = stgStopLoss;
}

string Strategy::getStgStrategyId() {
	return stg_strategy_id;
}

void Strategy::setStgStrategyId(string stgStrategyId) {
	stg_strategy_id = stgStrategyId;
}

bool Strategy::isStgTradeTasking() {
	return stg_trade_tasking;
}

void Strategy::setStgTradeTasking(bool stgTradeTasking) {
	stg_trade_tasking = stgTradeTasking;
}

string Strategy::getStgTraderId() {
	return stg_trader_id;
}

void Strategy::setStgTraderId(string stgTraderId) {
	stg_trader_id = stgTraderId;
}

User* Strategy::getStgUser() {
	return stg_user;
}

void Strategy::setStgUser(User* stgUser) {
	stg_user = stgUser;
}

string Strategy::getStgUserId() {
	return stg_user_id;
}

void Strategy::setStgUserId(string stgUserId) {
	stg_user_id = stgUserId;
}

void Strategy::setStgOrderRefBase(long long stg_order_ref_base) {
	this->stg_order_ref_base = stg_order_ref_base;
}

long long Strategy::getStgOrderRefBase() {
	return this->stg_order_ref_base;
}

void Strategy::setStgTradingDay(string stg_trading_day) {
	this->stg_trading_day = stg_trading_day;
}

string Strategy::getStgTradingDay() {
	return this->stg_trading_day;
}

void Strategy::setStgSelectOrderAlgorithmFlag(string msg, bool stg_select_order_algorithm_flag) {
	std::cout << "Strategy::setStgSelectOrderAlgorithmFlag():" << std::endl;
	std::cout << "\tmsg = " << msg << std::endl;
	std::cout << "\tstg_select_order_algorithm_flag = " << stg_select_order_algorithm_flag << std::endl;
	this->stg_select_order_algorithm_flag = stg_select_order_algorithm_flag;
}

bool Strategy::getStgSelectOrderAlgorithmFlag() {
	return this->stg_select_order_algorithm_flag;
}

void Strategy::setStgLockOrderRef(string stg_lock_order_ref) {
	this->stg_lock_order_ref = stg_lock_order_ref;
}
string Strategy::getStgLockOrderRef() {
	return this->stg_lock_order_ref;
}

/************************************************************************/
/* 交易相关的回报函数                                                      */
/************************************************************************/
//下单
void Strategy::OrderInsert(User *user, char *InstrumentID, char CombOffsetFlag, char Direction, int Volume, double Price, string OrderRef) {
	USER_PRINT("Strategy::OrderInsert");
}

//下单响应
void Strategy::OnRtnOrder(CThostFtdcOrderField *pOrder) {
	USER_PRINT("Strategy::OnRtnOrder");
	/// 如果合约在撤单维护列表里，那么撤单次数增加1
	this->stg_user->add_instrument_id_action_counter(pOrder);
	this->Exec_OnRtnOrder(pOrder);
}

//成交通知
void Strategy::OnRtnTrade(CThostFtdcTradeField *pTrade) {
	USER_PRINT("Strategy::OnRtnTrade");
	this->ExEc_OnRtnTrade(pTrade);
}

//下单错误响应
void Strategy::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder) {
	USER_PRINT("Strategy::OnErrRtnOrderInsert");
	this->Exec_OnErrRtnOrderInsert();
}

///报单录入请求响应
void Strategy::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder) {
	USER_PRINT("Strategy::OnRspOrderInsert");
	this->Exec_OnRspOrderInsert();
}

//撤单
void Strategy::OrderAction(string ExchangeID, string OrderRef, string OrderSysID) {
	USER_PRINT("Strategy::OrderAction");
}

//撤单错误响应
void Strategy::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction) {
	USER_PRINT("Strategy::OnRspOrderAction");
}

//撤单错误
void Strategy::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction) {
	USER_PRINT("Strategy::OnErrRtnOrderAction");
}


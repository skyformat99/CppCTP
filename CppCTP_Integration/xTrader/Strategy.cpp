#include <algorithm>
#include <sstream>
#include <mongo/bson/bson.h>
#include <mutex>
#include "Strategy.h"
#include "Utils.h"
using mongo::BSONArray;
using mongo::BSONArrayBuilder;
using mongo::BSONObj;
using mongo::BSONObjBuilder;
using mongo::BSONElement;
using mongo::ConnectException;

//std::mutex tick_mtx; // locks access to counter
//std::mutex update_status_mtx; // locks access to counter
//std::mutex select_order_algorithm_mtx; // locks access to counter
std::mutex create_position_detail_mtx;

#define ISACTIVE "1"
#define ISNOTACTIVE "0"
#define DB_STRATEGY_COLLECTION					"CTP.strategy"
#define DB_POSITIONDETAIL_COLLECTION			"CTP.positiondetail"
#define DB_POSITIONDETAIL_YESTERDAY_COLLECTION	"CTP.positiondetail_yesterday"
#define DB_POSITIONDETAIL_TRADE_COLLECTION	"CTP.positiondetail_trade"
#define DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION	"CTP.positiondetail_trade_yesterday"
#define DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION	"CTP.positiondetail_changed"
#define DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION	"CTP.positiondetail_trade_changed"

Strategy::Strategy(User *stg_user) {

	this->on_off = 0;						//开关	
	this->on_off_end_task = false;			//收盘任务默认允许执行
	this->stg_only_close = 0;				//只能平仓
	this->sell_open_on_off = 1;				//卖开-开关
	this->buy_close_on_off = 1;				//买平-开关
	this->buy_open_on_off = 1;				//买开-开关
	this->sell_close_on_off = 1;			//卖平-开关
	this->is_position_right = true;			//仓位是否正确,默认正确
	this->stg_position_a_sell = 0;			//持仓A卖
	this->stg_position_b_buy = 0;			//持仓B买
	this->stg_position_a_buy = 0;			//持仓A买
	this->stg_position_b_sell = 0;			//持仓B卖
	this->stg_lots = 0;						//总手
	this->stg_position_a_sell_yesterday = 0;//
	this->stg_position_a_buy_yesterday = 0; //
	this->stg_position_a_sell_today = 0;	//
	this->stg_position_a_buy_today = 0;		//

	this->stg_position_b_sell_yesterday = 0;//
	this->stg_position_b_buy_yesterday = 0; //
	this->stg_position_b_sell_today = 0;	//
	this->stg_position_b_buy_today = 0;		//

	this->stg_buy_close = 0;				//
	this->stg_sell_open = 0;				//
	this->stg_buy_open = 0;					//
	this->stg_sell_close = 0;				//

	this->stg_position_buy = 0;				//
	this->stg_position_sell = 0;			//
	this->stg_trade_volume = 0;				//成交量
	this->stg_amount = 0;					//成交金额
	this->stg_average_shift = 0;			//
	this->stg_position = 0;					//总持仓
	this->stg_spread_shift = 0;
	this->stg_stop_loss = 0;
	this->stg_lots_batch = 0;
	this->stg_order_algorithm = "";
	this->stg_a_order_action_tires_limit = 0;
	this->stg_b_order_action_tires_limit = 0;
	this->stg_a_order_action_count = 0;
	this->stg_b_order_action_count = 0;
	this->stg_trade_model = "";
	this->stg_hold_profit = 0;				// 持仓盈亏
	this->stg_close_profit = 0;				// 平仓盈亏
	this->stg_commission = 0;				// 手续费

	this->stg_a_wait_price_tick = 0;		// A合约挂单等待最小跳数
	this->stg_b_wait_price_tick = 0;		// B合约挂单等待最小跳数
	this->stg_a_limit_price_shift = 0;		// A合约报单偏移
	this->stg_b_limit_price_shift = 0;		// B合约报单偏移
	this->stg_is_active = true;				//默认策略均为激活状态
	this->stg_trading_day = "";
	this->stg_pending_a_open = 0;			//A开仓挂单
	this->stg_select_order_algorithm_flag = false; //默认允许选择下单算法锁为false，可以选择
	this->stg_lock_order_ref = "";
	this->stg_tick_systime_record = "";		// 系统接收tick的时间
	this->stg_update_position_detail_record_time = ""; // 最后一次修改持仓明细记录时间
	this->stg_save_strategy_conn = DBManager::getDBConnection();
	this->stg_last_saved_time = "";	// 最后一次保存策略时间

	this->stg_user = stg_user;					// 默认用户为空

	this->l_instruments = new list<string>();

	this->stg_list_order_pending = new list<CThostFtdcOrderField *>();
	this->stg_list_position_detail = new list<CThostFtdcTradeField *>();
	this->stg_list_position_detail_from_order = new list<USER_CThostFtdcOrderField *>();
	this->stg_list_position_detail_from_trade = new list<USER_CThostFtdcTradeField *>();

	this->stg_a_order_insert_args = new CThostFtdcInputOrderField();
	this->stg_b_order_insert_args = new CThostFtdcInputOrderField();

	this->stg_instrument_A_tick = new CThostFtdcDepthMarketDataField();	// A合约tick（第一腿）
	this->stg_instrument_B_tick = new CThostFtdcDepthMarketDataField();	// B合约tick（第二腿）
	this->stg_instrument_A_tick_last = new CThostFtdcDepthMarketDataField();	// A合约tick（第一腿）交易前最后一次
	this->stg_instrument_B_tick_last = new CThostFtdcDepthMarketDataField();	// B合约tick（第二腿）交易前最后一次
	this->stg_instrument_Last_tick = new CThostFtdcDepthMarketDataField();		// 行情推送最后一次tick

	this->stg_trade_tasking = false;
	init_finished = false;

	this->l_query_order = new list<USER_CThostFtdcOrderField *>();
	this->l_position_detail_from_ctp = new list<CThostFtdcInvestorPositionDetailField *>();
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

	string temp(src->OrderRef);
	int len_order_ref = temp.length();
	string strategyid = temp.substr(len_order_ref - 2, 2);
	///策略id
	strcpy(dst->StrategyID, strategyid.c_str());

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

/// 拷贝结构体CThostFtdcTradeField
void Strategy::CopyTradeDataToNew(USER_CThostFtdcTradeField *dst, CThostFtdcTradeField *src) {

	string temp(src->OrderRef);
	int len_order_ref = temp.length();
	string strategyid = temp.substr(len_order_ref - 2, 2);
	///策略id
	strcpy(dst->StrategyID, strategyid.c_str());

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

	///记录交易日期时间
	strcpy(dst->TradingDayRecord, src->TradingDay);

	///结算编号
	dst->SettlementID = src->SettlementID;

	///经纪公司报单编号
	dst->BrokerOrderSeq = src->BrokerOrderSeq;

	///成交来源
	dst->TradeSource = src->TradeSource;
}

void Strategy::CopyNewTradeData(USER_CThostFtdcTradeField *dst, USER_CThostFtdcTradeField *src) {
	
	///策略id
	strcpy(dst->StrategyID, src->StrategyID);

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

	///记录交易日期时间
	strcpy(dst->TradingDayRecord, src->TradingDayRecord);

	///结算编号
	dst->SettlementID = src->SettlementID;

	///经纪公司报单编号
	dst->BrokerOrderSeq = src->BrokerOrderSeq;

	///成交来源
	dst->TradeSource = src->TradeSource;
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

	///持仓明细
	strcpy(dst->TradingDayRecord, src->TradingDay);

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

/// 收盘保存数据
void Strategy::DropPositionDetail() {
	this->stg_save_strategy_conn->dropCollection("CTP.positiondetail");
}

/// 根据持仓明细进行统计持仓量
void Strategy::calPosition() {
	std::cout << "Strategy::calPosition()" << std::endl;
	this->stg_position_a_sell = 0;			//持仓A卖
	this->stg_position_b_buy = 0;			//持仓B买
	this->stg_position_a_buy = 0;			//持仓A买
	this->stg_position_b_sell = 0;			//持仓B卖
	this->stg_position_a_sell_yesterday = 0;//持仓A昨卖
	this->stg_position_a_buy_yesterday = 0; //持仓A今买
	this->stg_position_a_sell_today = 0;	//持仓A今卖
	this->stg_position_a_buy_today = 0;		//持仓A今买
	this->stg_position_b_sell_yesterday = 0;//持仓B昨卖
	this->stg_position_b_buy_yesterday = 0; //持仓B昨买
	this->stg_position_b_sell_today = 0;	//持仓B今卖
	this->stg_position_b_buy_today = 0;		//持仓B今买

	list<USER_CThostFtdcOrderField *>::iterator itor;
	for (itor = this->stg_list_position_detail_from_order->begin();
		itor != this->stg_list_position_detail_from_order->end(); itor++) {
		if (!strcmp((*itor)->TradingDay, this->getStgTradingDay().c_str())) // 交易日相同,今仓
		{
			// 等于A合约
			if (!strcmp((*itor)->InstrumentID, this->getStgInstrumentIdA().c_str()))
			{
				if ((*itor)->Direction == '0') //买
				{
					this->stg_position_a_buy_today += (*itor)->VolumeTradedBatch;
				}
				else { //卖
					this->stg_position_a_sell_today += (*itor)->VolumeTradedBatch;
				}
			}
			// 等于B合约
			else if (!strcmp((*itor)->InstrumentID, this->getStgInstrumentIdB().c_str())) {
				if ((*itor)->Direction == '0') //买
				{
					this->stg_position_b_buy_today += (*itor)->VolumeTradedBatch;
				}
				else { //卖
					this->stg_position_b_sell_today += (*itor)->VolumeTradedBatch;
				}
			}
		}
		else { // 交易日不同，属于昨仓
			// 等于A合约
			if (!strcmp((*itor)->InstrumentID, this->getStgInstrumentIdA().c_str()))
			{
				if ((*itor)->Direction == '0') //买
				{
					this->stg_position_a_buy_yesterday += (*itor)->VolumeTradedBatch;
				}
				else { //卖
					this->stg_position_a_sell_yesterday += (*itor)->VolumeTradedBatch;
				}
			}
			// 等于B合约
			else if (!strcmp((*itor)->InstrumentID, this->getStgInstrumentIdB().c_str())) {
				if ((*itor)->Direction == '0') //买
				{
					this->stg_position_b_buy_yesterday += (*itor)->VolumeTradedBatch;
				}
				else { //卖
					this->stg_position_b_sell_yesterday += (*itor)->VolumeTradedBatch;
				}
			}
		}
	}
	this->stg_position_a_buy = this->stg_position_a_buy_today + this->stg_position_a_buy_yesterday;
	this->stg_position_a_sell = this->stg_position_a_sell_today + this->stg_position_a_sell_yesterday;
	this->stg_position_b_buy = this->stg_position_b_buy_today + this->stg_position_b_buy_yesterday;
	this->stg_position_b_sell = this->stg_position_b_sell_today + this->stg_position_b_sell_yesterday;
	this->stg_position = this->stg_position_b_buy + this->stg_position_b_sell;
}

/// 更新策略
void Strategy::UpdateStrategy(Strategy *stg) {
	int count_number = 0;

	std::cout << "Strategy::UpdateStrategy()" << std::endl;


	count_number = this->stg_save_strategy_conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	std::cout << "\t查找到 " << count_number << " 条策略记录" << std::endl;

	if (count_number > 0) {
		this->stg_save_strategy_conn->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str() << "is_active" << true), BSON("$set" << BSON("position_a_sell_today" << stg->getStgPositionASellToday()
			<< "position_b_sell" << stg->getStgPositionBSell()
			<< "spread_shift" << stg->getStgSpreadShift()
			<< "position_b_sell_today" << stg->getStgPositionBSellToday()
			<< "position_b_buy_today" << stg->getStgPositionBBuyToday()
			<< "position_a_sell" << stg->getStgPositionASell()
			<< "buy_close" << stg->getStgBuyClose()
			<< "stop_loss" << stg->getStgStopLoss()
			<< "position_b_buy_yesterday" << stg->getStgPositionBBuyYesterday()
			<< "is_active" << stg->isStgIsActive()
			<< "position_b_sell_yesterday" << stg->getStgPositionBSellYesterday()
			<< "strategy_id" << stg->getStgStrategyId()
			<< "position_b_buy" << stg->getStgPositionBBuy()
			<< "lots_batch" << stg->getStgLotsBatch()
			<< "position_a_buy" << stg->getStgPositionABuy()
			<< "sell_open" << stg->getStgSellOpen()
			<< "order_algorithm" << stg->getStgOrderAlgorithm()
			<< "trader_id" << stg->getStgTraderId()
			<< "a_order_action_limit" << stg->getStgAOrderActionTiresLimit()
			<< "b_order_action_limit" << stg->getStgBOrderActionTiresLimit()
			<< "sell_close" << stg->getStgSellClose()
			<< "buy_open" << stg->getStgBuyOpen()

			<< "only_close" << stg->isStgOnlyClose()
			<< "strategy_on_off" << stg->getOn_Off()
			<< "sell_open_on_off" << stg->getStgSellOpenOnOff()
			<< "buy_close_on_off" << stg->getStgBuyCloseOnOff()
			<< "sell_close_on_off" << stg->getStgSellCloseOnOff()
			<< "buy_open_on_off" << stg->getStgBuyOpenOnOff()

			<< "trade_model" << stg->getStgTradeModel()
			<< "hold_profit" << stg->getStgHoldProfit()
			<< "close_profit" << stg->getStgCloseProfit()
			<< "commission" << stg->getStgCommission()
			<< "position" << stg->getStgPosition()
			<< "position_buy" << stg->getStgPositionBuy()
			<< "position_sell" << stg->getStgPositionSell()
			<< "trade_volume" << stg->getStgTradeVolume()
			<< "amount" << stg->getStgAmount()
			<< "average_shift" << stg->getStgAverageShift()

			<< "a_limit_price_shift" << stg->getStgALimitPriceShift()
			<< "b_limit_price_shift" << stg->getStgBLimitPriceShift()

			<< "position_a_buy_yesterday" << stg->getStgPositionABuyYesterday()
			<< "user_id" << stg->getStgUserId()
			<< "position_a_buy_today" << stg->getStgPositionABuyToday()
			<< "position_a_sell_yesterday" << stg->getStgPositionASellYesterday()
			<< "lots" << stg->getStgLots()
			<< "a_wait_price_tick" << stg->getStgAWaitPriceTick()
			<< "b_wait_price_tick" << stg->getStgBWaitPriceTick()
			<< "trading_day" << stg->getStgTradingDay()
			<< "update_position_detail_record_time" << stg->getStgUpdatePositionDetailRecordTime()
			<< "last_save_time" << stg->getStgLastSavedTime()
			<< "list_instrument_id" << BSON_ARRAY(stg->getStgInstrumentIdA() << stg->getStgInstrumentIdB()))));

		USER_PRINT("Strategy::UpdateStrategy ok");
	}
	else
	{
		USER_PRINT("Strategy ID Not Exists!");
	}
}

/// 创建持仓明细
void Strategy::CreatePositionDetail(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("Strategy::CreatePositionDetail");

	int posd_count_num = 0;

	if (posd != NULL) {
		posd_count_num = this->stg_save_strategy_conn->count(DB_POSITIONDETAIL_COLLECTION,
			BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "is_active" << ISACTIVE));

		std::cout << "\t查找到 " << posd_count_num << " 条持仓明细记录(order)" << std::endl;

		if (posd_count_num != 0) { //session_id存在
			std::cout << "持仓明细已经存在了!" << std::endl;
			return;
		}
		else { //posd 不存在
			BSONObjBuilder b;

			b.append("instrumentid", posd->InstrumentID);
			b.append("orderref", posd->OrderRef);
			b.append("userid", posd->UserID);
			b.append("direction", posd->Direction);
			/*b.append("comboffsetflag", string(1, posd->CombOffsetFlag[0]));
			b.append("combhedgeflag", string(1, posd->CombHedgeFlag[0]));*/

			b.append("comboffsetflag", posd->CombOffsetFlag);
			b.append("combhedgeflag", posd->CombHedgeFlag);

			b.append("limitprice", posd->LimitPrice);
			b.append("volumetotaloriginal", posd->VolumeTotalOriginal);
			b.append("tradingday", posd->TradingDay);
			b.append("tradingdayrecord", posd->TradingDayRecord);
			b.append("orderstatus", posd->OrderStatus);
			b.append("volumetraded", posd->VolumeTraded);
			b.append("volumetotal", posd->VolumeTotal);
			b.append("insertdate", posd->InsertDate);
			b.append("inserttime", posd->InsertTime);
			b.append("strategyid", posd->StrategyID);
			b.append("volumetradedbatch", posd->VolumeTradedBatch);
			b.append("is_active", ISACTIVE);

			BSONObj p = b.obj();
			create_position_detail_mtx.lock();
			this->stg_save_strategy_conn->insert(DB_POSITIONDETAIL_COLLECTION, p);
			create_position_detail_mtx.unlock();
			USER_PRINT("DBManager::CreatePositionDetail ok");
		}
	}
	USER_PRINT("Strategy::CreatePositionDetail OK");
}

/// 数据库更新策略持仓明细
void Strategy::Update_Position_Detail_To_DB(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("Strategy::Update_Position_Detail_To_DB");
	//std::cout << "Strategy::Update_Position_Detail_To_DB()" << std::endl;
	int count_number = 0;

	count_number = this->stg_save_strategy_conn->count(DB_POSITIONDETAIL_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//std::cout << "Strategy::Update_Position_Detail_To_DB()" << std::endl;
		//std::cout << "\t收盘保存持仓明细 找到(Order)!" << std::endl;
		this->stg_save_strategy_conn->update(DB_POSITIONDETAIL_COLLECTION, BSON(
			"userid" << posd->UserID 
			<< "strategyid" << posd->StrategyID 
			<< "tradingday" << posd->TradingDay 
			<< "orderref" << posd->OrderRef 
			<< "is_active" << ISACTIVE), BSON("$set" << BSON(
			"instrumentid" << posd->InstrumentID 
			<< "orderref" << posd->OrderRef
			<< "userid" << posd->UserID
			<< "direction" << posd->Direction
			/*<< "comboffsetflag" << string(1, posd->CombOffsetFlag[0])
			<< "combhedgeflag" << string(1, posd->CombHedgeFlag[0])*/
			<< "comboffsetflag" << posd->CombOffsetFlag
			<< "combhedgeflag" << posd->CombHedgeFlag
			<< "limitprice" << posd->LimitPrice
			<< "volumetotaloriginal" << posd->VolumeTotalOriginal
			<< "tradingday" << posd->TradingDay
			<< "tradingdayrecord" << posd->TradingDayRecord
			<< "orderstatus" << posd->OrderStatus
			<< "volumetraded" << posd->VolumeTraded
			<< "volumetotal" << posd->VolumeTotal
			<< "insertdate" << posd->InsertDate
			<< "inserttime" << posd->InsertTime
			<< "strategyid" << posd->StrategyID
			<< "volumetradedbatch" << posd->VolumeTradedBatch
			)));
		USER_PRINT("Strategy::Update_Position_Detail_To_DB ok");
	}
	else {
		//std::cout << "Strategy::Update_Position_Detail_To_DB()" << std::endl;
		//std::cout << "\t收盘保存持仓明细 未 找到(Order)!" << std::endl;
		//std::cout << "\t开始新建持仓明细(Order)!" << std::endl;
		
		BSONObjBuilder b;

		b.append("userid", posd->UserID);
		b.append("tradingday", posd->TradingDay);
		b.append("orderref", posd->OrderRef);
		b.append("is_active", ISACTIVE);
		b.append("instrumentid", posd->InstrumentID);
		b.append("direction", posd->Direction);
		b.append("comboffsetflag", posd->CombOffsetFlag);
		b.append("combhedgeflag", posd->CombHedgeFlag);
		b.append("limitprice", posd->LimitPrice);
		b.append("volumetotaloriginal", posd->VolumeTotalOriginal);
		b.append("tradingdayrecord", posd->TradingDayRecord);
		b.append("orderstatus", posd->OrderStatus);
		b.append("volumetraded", posd->VolumeTraded);
		b.append("volumetotal", posd->VolumeTotal);
		b.append("insertdate", posd->InsertDate);
		b.append("inserttime", posd->InsertTime);
		b.append("strategyid", posd->StrategyID);
		b.append("volumetradedbatch", posd->VolumeTradedBatch);

		BSONObj p = b.obj();
		create_position_detail_mtx.lock();
		this->stg_save_strategy_conn->insert(DB_POSITIONDETAIL_COLLECTION, p);
		create_position_detail_mtx.unlock();
		USER_PRINT("Strategy::Update_Position_Detail_To_DB ok");
	}

	USER_PRINT("Strategy::Update_Position_Detail_To_DB OK");
}

/// 数据库更新策略持仓明细(order changed)
void Strategy::Update_Position_Changed_Detail_To_DB(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("Strategy::Update_Position_Changed_Detail_To_DB");
	//std::cout << "Strategy::Update_Position_Changed_Detail_To_DB()" << std::endl;
	int count_number = 0;

	count_number = this->stg_save_strategy_conn->count(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//std::cout << "Strategy::Update_Position_Changed_Detail_To_DB()" << std::endl;
		//std::cout << "\t收盘保存持仓明细 找到(Order)!" << std::endl;
		this->stg_save_strategy_conn->update(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, BSON(
			"userid" << posd->UserID
			<< "strategyid" << posd->StrategyID
			<< "tradingday" << posd->TradingDay
			<< "orderref" << posd->OrderRef
			<< "is_active" << ISACTIVE), BSON("$set" << BSON(
			"instrumentid" << posd->InstrumentID
			<< "orderref" << posd->OrderRef
			<< "userid" << posd->UserID
			<< "direction" << posd->Direction
			/*<< "comboffsetflag" << string(1, posd->CombOffsetFlag[0])
			<< "combhedgeflag" << string(1, posd->CombHedgeFlag[0])*/
			<< "comboffsetflag" << posd->CombOffsetFlag
			<< "combhedgeflag" << posd->CombHedgeFlag
			<< "limitprice" << posd->LimitPrice
			<< "volumetotaloriginal" << posd->VolumeTotalOriginal
			<< "tradingday" << posd->TradingDay
			<< "tradingdayrecord" << posd->TradingDayRecord
			<< "orderstatus" << posd->OrderStatus
			<< "volumetraded" << posd->VolumeTraded
			<< "volumetotal" << posd->VolumeTotal
			<< "insertdate" << posd->InsertDate
			<< "inserttime" << posd->InsertTime
			<< "strategyid" << posd->StrategyID
			<< "volumetradedbatch" << posd->VolumeTradedBatch
			)));
		USER_PRINT("Strategy::Update_Position_Changed_Detail_To_DB ok");
	}
	else {
		//std::cout << "Strategy::Update_Position_Detail_To_DB()" << std::endl;
		//std::cout << "\t收盘保存持仓明细 未 找到(Order)!" << std::endl;
		//std::cout << "\t开始新建持仓明细(Order)!" << std::endl;

		BSONObjBuilder b;

		b.append("userid", posd->UserID);
		b.append("tradingday", posd->TradingDay);
		b.append("orderref", posd->OrderRef);
		b.append("is_active", ISACTIVE);
		b.append("instrumentid", posd->InstrumentID);
		b.append("direction", posd->Direction);
		b.append("comboffsetflag", posd->CombOffsetFlag);
		b.append("combhedgeflag", posd->CombHedgeFlag);
		b.append("limitprice", posd->LimitPrice);
		b.append("volumetotaloriginal", posd->VolumeTotalOriginal);
		b.append("tradingdayrecord", posd->TradingDayRecord);
		b.append("orderstatus", posd->OrderStatus);
		b.append("volumetraded", posd->VolumeTraded);
		b.append("volumetotal", posd->VolumeTotal);
		b.append("insertdate", posd->InsertDate);
		b.append("inserttime", posd->InsertTime);
		b.append("strategyid", posd->StrategyID);
		b.append("volumetradedbatch", posd->VolumeTradedBatch);

		BSONObj p = b.obj();
		create_position_detail_mtx.lock();
		this->stg_save_strategy_conn->insert(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, p);
		create_position_detail_mtx.unlock();
		USER_PRINT("DBManager::Update_Position_Changed_Detail_To_DB ok");
	}

	USER_PRINT("Strategy::Update_Position_Changed_Detail_To_DB OK");
}

/// 创建持仓明细
void Strategy::CreatePositionTradeDetail(USER_CThostFtdcTradeField *posd) {
	USER_PRINT("Strategy::CreatePositionTradeDetail");

	int posd_count_num = 0;

	if (posd != NULL) {
		posd_count_num = this->stg_save_strategy_conn->count(DB_POSITIONDETAIL_TRADE_COLLECTION,
			BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "is_active" << ISACTIVE));

		std::cout << "\t查找到 " << posd_count_num << " 条持仓明细记录(trade)" << std::endl;

		if (posd_count_num != 0) { //session_id存在
			std::cout << "持仓明细已经存在了!" << std::endl;
			return;
		}
		else { //posd 不存在
			BSONObjBuilder b;

			b.append("instrumentid", posd->InstrumentID);
			b.append("orderref", posd->OrderRef);
			b.append("userid", posd->UserID);
			b.append("direction", posd->Direction);
			/*b.append("comboffsetflag", string(1, posd->CombOffsetFlag[0]));
			b.append("combhedgeflag", string(1, posd->CombHedgeFlag[0]));*/

			b.append("offsetflag", posd->OffsetFlag);
			b.append("hedgeflag", posd->HedgeFlag);
			b.append("price", posd->Price);
			b.append("tradingday", posd->TradingDay);
			b.append("tradingdayrecord", posd->TradingDayRecord);
			b.append("tradedate", posd->TradeDate);
			b.append("strategyid", posd->StrategyID);
			b.append("volume", posd->Volume);
			b.append("is_active", ISACTIVE);

			BSONObj p = b.obj();
			create_position_detail_mtx.lock();
			this->stg_save_strategy_conn->insert(DB_POSITIONDETAIL_TRADE_COLLECTION, p);
			create_position_detail_mtx.unlock();
			USER_PRINT("DBManager::CreatePositionTradeDetail ok");
		}
	}
	USER_PRINT("Strategy::CreatePositionTradeDetail OK");
}

/// 数据库更新策略持仓明细(Trade)
void Strategy::Update_Position_Trade_Detail_To_DB(USER_CThostFtdcTradeField *posd) {
	USER_PRINT("Strategy::Update_Position_Trade_Detail_To_DB");
	//std::cout << "Strategy::Update_Position_Trade_Detail_To_DB()" << std::endl;
	int count_number = 0;

	count_number = this->stg_save_strategy_conn->count(DB_POSITIONDETAIL_TRADE_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//std::cout << "Strategy::Update_Position_Trade_Detail_To_DB()" << std::endl;
		//std::cout << "\t收盘保存持仓明细 已 找到(Trade)!" << std::endl;
		this->stg_save_strategy_conn->update(DB_POSITIONDETAIL_TRADE_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON(
			"instrumentid" << posd->InstrumentID
			<< "orderref" << posd->OrderRef
			<< "userid" << posd->UserID
			<< "direction" << posd->Direction
			/*<< "comboffsetflag" << string(1, posd->CombOffsetFlag[0])
			<< "combhedgeflag" << string(1, posd->CombHedgeFlag[0])*/
			<< "offsetflag" << posd->OffsetFlag
			<< "hedgeflag" << posd->HedgeFlag
			<< "price" << posd->Price
			<< "tradingday" << posd->TradingDay
			<< "tradingdayrecord" << posd->TradingDayRecord
			<< "tradedate" << posd->TradeDate
			<< "strategyid" << posd->StrategyID
			<< "volume" << posd->Volume
			<< "exchangeid" << posd->ExchangeID
			)));
		USER_PRINT("Strategy::Update_Position_Trade_Detail_To_DB ok");
	}
	else {
		//std::cout << "Strategy::Update_Position_Trade_Detail_To_DB()" << std::endl;
		//std::cout << "\t收盘保存持仓明细 未 找到(Trade)!" << std::endl;
		//std::cout << "\t开始新建持仓明细(Trade)!" << std::endl;

		BSONObjBuilder b;

		b.append("userid", posd->UserID);
		b.append("strategyid", posd->StrategyID);
		b.append("tradingday", posd->TradingDay);
		b.append("orderref", posd->OrderRef);
		b.append("is_active", ISACTIVE);
		b.append("instrumentid", posd->InstrumentID);
		b.append("direction", posd->Direction);
		b.append("offsetflag", posd->OffsetFlag);
		b.append("hedgeflag", posd->HedgeFlag);
		b.append("price", posd->Price);
		b.append("tradingdayrecord", posd->TradingDayRecord);
		b.append("exchangeid", posd->ExchangeID);
		b.append("tradedate", posd->TradeDate);
		b.append("volume", posd->Volume);

		BSONObj p = b.obj();
		create_position_detail_mtx.lock();
		this->stg_save_strategy_conn->insert(DB_POSITIONDETAIL_TRADE_COLLECTION, p);
		create_position_detail_mtx.unlock();
	}

	USER_PRINT("Strategy::Update_Position_Trade_Detail_To_DB OK");
}

/// 数据库更新策略持仓明细(trade changed)
void Strategy::Update_Position_Trade_Changed_Detail_To_DB(USER_CThostFtdcTradeField *posd) {
	USER_PRINT("Strategy::Update_Position_Trade_Changed_Detail_To_DB");
	//std::cout << "Strategy::Update_Position_Trade_Changed_Detail_To_DB()" << std::endl;
	this->getStgUser()->getXtsLogger()->info("Strategy::Update_Position_Trade_Changed_Detail_To_DB()");
	int count_number = 0;

	count_number = this->stg_save_strategy_conn->count(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//std::cout << "\t收盘保存持仓明细 已 找到(Trade)!" << std::endl;
		this->getStgUser()->getXtsLogger()->info("\t收盘保存持仓明细 已 找到(Trade)!");
		this->stg_save_strategy_conn->update(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON(
			"instrumentid" << posd->InstrumentID
			<< "orderref" << posd->OrderRef
			<< "userid" << posd->UserID
			<< "direction" << posd->Direction
			/*<< "comboffsetflag" << string(1, posd->CombOffsetFlag[0])
			<< "combhedgeflag" << string(1, posd->CombHedgeFlag[0])*/
			<< "offsetflag" << posd->OffsetFlag
			<< "hedgeflag" << posd->HedgeFlag
			<< "price" << posd->Price
			<< "tradingday" << posd->TradingDay
			<< "tradingdayrecord" << posd->TradingDayRecord
			<< "tradedate" << posd->TradeDate
			<< "strategyid" << posd->StrategyID
			<< "volume" << posd->Volume
			<< "exchangeid" << posd->ExchangeID
			)));
		USER_PRINT("Strategy::Update_Position_Trade_Changed_Detail_To_DB ok");
	}
	else {
		/*std::cout << "Strategy::Update_Position_Trade_Changed_Detail_To_DB()" << std::endl;
		std::cout << "\t收盘保存持仓明细 未 找到(Trade)!" << std::endl;
		std::cout << "\t开始新建持仓明细(Trade)!" << std::endl;*/
		this->getStgUser()->getXtsLogger()->info("Strategy::Update_Position_Trade_Changed_Detail_To_DB()");
		this->getStgUser()->getXtsLogger()->info("\t收盘保存持仓明细 未 找到(Trade)!");
		this->getStgUser()->getXtsLogger()->info("\t开始新建持仓明细(Trade)!");

		BSONObjBuilder b;

		b.append("userid", posd->UserID);
		b.append("strategyid", posd->StrategyID);
		b.append("tradingday", posd->TradingDay);
		b.append("orderref", posd->OrderRef);
		b.append("is_active", ISACTIVE);
		b.append("instrumentid", posd->InstrumentID);
		b.append("direction", posd->Direction);
		b.append("offsetflag", posd->OffsetFlag);
		b.append("hedgeflag", posd->HedgeFlag);
		b.append("price", posd->Price);
		b.append("tradingdayrecord", posd->TradingDayRecord);
		b.append("exchangeid", posd->ExchangeID);
		b.append("tradedate", posd->TradeDate);
		b.append("volume", posd->Volume);

		BSONObj p = b.obj();
		create_position_detail_mtx.lock();
		this->stg_save_strategy_conn->insert(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, p);
		create_position_detail_mtx.unlock();
	}

	USER_PRINT("Strategy::Update_Position_Trade_Changed_Detail_To_DB OK");
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
	//std::cout << "Strategy::update_task_status()" << std::endl;
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

	/*std::cout << "\t期货账户:" << this->stg_user_id << std::endl;
	std::cout << "\t策略编号:" << this->stg_strategy_id << std::endl;*/
	//this->getStgUser()->getXtsLogger()->info("\t期货账户: {}", this->stg_user_id);
	//this->getStgUser()->getXtsLogger()->info("\t策略编号: {}", this->stg_strategy_id);
	/*std::cout << "\t(this->stg_position_a_buy_today == this->stg_position_b_sell_today)(" << this->stg_position_a_buy_today << ", " << this->stg_position_b_sell_today << ")" << std::endl;
	std::cout << "\t(this->stg_position_a_buy_yesterday == this->stg_position_b_sell_yesterday)(" << this->stg_position_a_buy_yesterday << ", " << this->stg_position_b_sell_yesterday << ")" << std::endl;
	std::cout << "\t(this->stg_position_a_sell_today == this->stg_position_b_buy_today)(" << this->stg_position_a_sell_today << ", " << this->stg_position_b_buy_today << ")" << std::endl;
	std::cout << "\t(this->stg_position_a_sell_yesterday == this->stg_position_b_buy_yesterday)(" << this->stg_position_a_sell_yesterday << ", " << this->stg_position_b_buy_yesterday << ")" << std::endl;
	std::cout << "\t(this->stg_list_order_pending->size() == 0)(" << this->stg_list_order_pending->size() << ", " << 0 << ")" << std::endl;*/
	
	

	if ((this->stg_position_a_buy_today == this->stg_position_b_sell_today)
		&& (this->stg_position_a_buy_yesterday == this->stg_position_b_sell_yesterday)
		&& (this->stg_position_a_sell_today == this->stg_position_b_buy_today)
		&& (this->stg_position_a_sell_yesterday == this->stg_position_b_buy_yesterday)
		&& (this->stg_list_order_pending->size() == 0)) {
		/*this->printStrategyInfo("更新交易状态");
		this->printStrategyInfoPosition();*/
		//this->stg_trade_tasking = false;
		this->setStgTradeTasking(false);
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

		this->setStgTradeTasking(true);
	}
	//std::cout << "After update this.trade_tasking = " << this->stg_trade_tasking << endl;
	//std::cout << "\t挂单列表长度 = " << this->stg_list_order_pending->size() << std::endl;
	//std::cout << "\t任务执行状态 = " << this->stg_trade_tasking << std::endl;
	//update_status_mtx.unlock();
	USER_PRINT(this->stg_trade_tasking);

	/************************************************************************/
	/* 任务执行中的时候，tick可以解锁                                                                     */
	/************************************************************************/
	if (this->stg_trade_tasking) {
		this->setStgSelectOrderAlgorithmFlag("Strategy::update_task_status()", false);
	}
	else {
		this->getStgSelectOrderAlgorithmFlag();
	}

}

/// 更新tick锁
void Strategy::update_tick_lock_status(USER_CThostFtdcOrderField *pOrder) {
	std::cout << "Strategy::update_tick_lock_status():" << std::endl;
	bool flag; // tick锁标志位
	if (strlen(pOrder->OrderSysID) != 0) { // 收到交易所回报
		

		if (pOrder->OrderStatus == '0') { // 全部成交
			flag = false; // 释放锁
		}
		else if (pOrder->OrderStatus == '1') { // 部分成交还在队列中
			flag = false; // 释放锁
		}
		else if (pOrder->OrderStatus == '2') { // 部分成交不在队列中
			flag = false; // 释放锁
		}
		else if (pOrder->OrderStatus == '3') { // 未成交还在队列中
			flag = false; // 释放锁
		}
		else if (pOrder->OrderStatus == '4') { // 未成交不在队列中
			flag = false; // 释放锁
		}
		else if (pOrder->OrderStatus == '5') { // 撤单
			flag = false; // 释放锁
		}
		else if (pOrder->OrderStatus == 'a') { // 未知
			//flag = true;
			flag = false; // 释放锁
		}
		else if (pOrder->OrderStatus == 'b') { // 尚未触发
			flag = false; // 释放锁
		}
		else if (pOrder->OrderStatus == 'c') { // 已触发
			flag = false; // 释放锁
		}
		else {
			// 当报单状态不在以上之中,依然打开锁
			flag = true;
		}

		//std::cout << "\t tick flag = " << flag << std::endl;
		this->setStgSelectOrderAlgorithmFlag("Strategy::update_tick_lock_status()", flag); // tick锁
	}
	else {

	}
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

void Strategy::add_position_detail(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("Strategy::add_position_detail");
	/*USER_CThostFtdcOrderField *new_order = new USER_CThostFtdcOrderField();
	memset(new_order, 0, sizeof(USER_CThostFtdcOrderField));
	this->CopyPositionData(posd, new_order);*/
	this->stg_list_position_detail_from_order->push_back(posd);
}

//void Strategy::CopyPositionData(PositionDetail *posd, USER_CThostFtdcOrderField *order) {
//	USER_PRINT("Strategy::CopyPositionData");
//	strcpy(order->InstrumentID, posd->getInstrumentID().c_str());
//	strcpy(order->OrderRef, posd->getOrderRef().c_str());
//	strcpy(order->UserID, posd->getUserID().c_str());
//	order->Direction = posd->getDirection();
//	order->CombOffsetFlag[0] = posd->getCombOffsetFlag().c_str()[0];
//	order->CombHedgeFlag[0] = posd->getCombHedgeFlag().c_str()[0];
//	order->LimitPrice = posd->getLimitPrice();
//	order->VolumeTotalOriginal = posd->getVolumeTotalOriginal();
//	strcpy(order->TradingDay, posd->getTradingDay().c_str());
//	order->OrderStatus = posd->getOrderStatus();
//	order->VolumeTraded = posd->getVolumeTraded();
//	order->VolumeTotal = posd->getVolumeTotal();
//	strcpy(order->InsertDate, posd->getInsertDate().c_str());
//	strcpy(order->InsertTime, posd->getInsertTime().c_str());
//	//order->StrategyID = posd->getStrategyID();
//	strcpy(order->StrategyID, posd->getStrategyID().c_str());
//	order->VolumeTradedBatch = posd->getVolumeTradedBatch();
//}

void Strategy::printStrategyInfo(string message) {
	
	/*std::cout << "Strategy::printStrategyInfo()" << std::endl;
	std::cout << "\t期货账户:" << this->stg_user_id << std::endl;
	std::cout << "\t策略编号:" << this->stg_strategy_id << std::endl;
	std::cout << "\t调试信息:" << message << std::endl;
	std::cout << "\t系统总开关:" << this->stg_user->getCTP_Manager()->getOn_Off() << std::endl;
	std::cout << "\t期货账户开关:" << this->stg_user->getOn_Off() << std::endl;
	std::cout << "\t交易员开关:" << this->stg_user->GetTrader()->getOn_Off() << std::endl;
	std::cout << "\t策略开关:" << this->getOn_Off() << std::endl;
	std::cout << "\tA合约撤单次数:" << this->getStgAOrderActionCount() << ", A合约撤单限制:" << this->getStgAOrderActionTiresLimit() << std::endl;
	std::cout << "\tB合约撤单次数:" << this->getStgBOrderActionCount() << ", B合约撤单限制:" << this->getStgBOrderActionTiresLimit() << std::endl;
	std::cout << "\tA总卖:" << this->stg_position_a_sell << ", A昨卖:" << this->stg_position_a_sell_yesterday << std::endl;
	std::cout << "\tB总买:" << this->stg_position_b_buy << ", B昨买:" << this->stg_position_b_buy_yesterday << std::endl;
	std::cout << "\tA总买:" << this->stg_position_a_buy << ", A昨买:" << this->stg_position_a_buy_yesterday << std::endl;
	std::cout << "\tB总卖:" << this->stg_position_b_sell << ", B昨卖:" << this->stg_position_b_sell_yesterday << std::endl;*/

	this->getStgUser()->getXtsLogger()->info("Strategy::printStrategyInfo()");
	this->getStgUser()->getXtsLogger()->info("\t期货账户:{}", this->stg_user_id);
	this->getStgUser()->getXtsLogger()->info("\t策略编号:{}", this->stg_strategy_id);
	this->getStgUser()->getXtsLogger()->info("\t调试信息:{}", message);
	this->getStgUser()->getXtsLogger()->info("\t系统总开关:{}", this->stg_user->getCTP_Manager()->getOn_Off());
	this->getStgUser()->getXtsLogger()->info("\t期货账户开关:{}", this->stg_user->getOn_Off());
	this->getStgUser()->getXtsLogger()->info("\t交易员开关:{}", this->stg_user->GetTrader()->getOn_Off());
	this->getStgUser()->getXtsLogger()->info("\t策略开关:{}", this->getOn_Off());
	this->getStgUser()->getXtsLogger()->info("\tA合约撤单次数:{}, A合约撤单限制:{}", this->getStgAOrderActionCount(), this->getStgAOrderActionTiresLimit());
	this->getStgUser()->getXtsLogger()->info("\tB合约撤单次数:{}, B合约撤单限制:{}", this->getStgBOrderActionCount(), this->getStgBOrderActionTiresLimit());
	this->getStgUser()->getXtsLogger()->info("\tA总卖:{}, A昨卖:{}", this->stg_position_a_sell, this->stg_position_a_sell_yesterday);
	this->getStgUser()->getXtsLogger()->info("\tB总买:{}, B昨买:{}", this->stg_position_b_buy, this->stg_position_b_buy_yesterday);
	this->getStgUser()->getXtsLogger()->info("\tA总买:{}, A昨买:{}", this->stg_position_a_buy, this->stg_position_a_buy_yesterday);
	this->getStgUser()->getXtsLogger()->info("\tB总卖:{}, B昨卖:{}", this->stg_position_b_sell, this->stg_position_b_sell_yesterday);
	this->getStgUser()->getXtsLogger()->flush();

}

void Strategy::printStrategyInfoPosition() {
	//std::cout << "Strategy::printStrategyInfoPosition()" << std::endl;
	/*std::cout << "\tA合约今买:" << this->stg_position_a_buy_today << ", "
		<< "A合约昨买:" << this->stg_position_a_buy_yesterday << ", "
		<< "A合约总买:" << this->stg_position_a_buy << ", "
		<< "A合约今卖:" << this->stg_position_a_sell_today << ", "
		<< "A合约昨卖:" << this->stg_position_a_sell_yesterday << ", "
		<< "A合约总卖:" << this->stg_position_a_sell << std::endl;

		std::cout << "\tB合约今买:" << this->stg_position_b_buy_today << ", "
		<< "B合约昨买:" << this->stg_position_b_buy_yesterday << ", "
		<< "B合约总买:" << this->stg_position_b_buy << ", "
		<< "B合约今卖:" << this->stg_position_b_sell_today << ", "
		<< "B合约昨卖:" << this->stg_position_b_sell_yesterday << ", "
		<< "B合约总卖:" << this->stg_position_b_sell << std::endl;*/

	/*std::cout << "\tA合约今买:" << this->stg_position_a_buy_today << ", "
		<< "A合约昨买:" << this->stg_position_a_buy_yesterday << ", "
		<< "A合约总买:" << this->stg_position_a_buy << ", "
		<< "A合约今卖:" << this->stg_position_a_sell_today << ", "
		<< "A合约昨卖:" << this->stg_position_a_sell_yesterday << ", "
		<< "A合约总卖:" << this->stg_position_a_sell << std::endl;

		std::cout << "\tB合约今买:" << this->stg_position_b_buy_today << ", "
		<< "B合约昨买:" << this->stg_position_b_buy_yesterday << ", "
		<< "B合约总买:" << this->stg_position_b_buy << ", "
		<< "B合约今卖:" << this->stg_position_b_sell_today << ", "
		<< "B合约昨卖:" << this->stg_position_b_sell_yesterday << ", "
		<< "B合约总卖:" << this->stg_position_b_sell << std::endl;*/

	/*std::cout << "\tA总卖:" << this->stg_position_a_sell << ", A昨卖:" << this->stg_position_a_sell_yesterday << std::endl;
	std::cout << "\tB总买:" << this->stg_position_b_buy << ", B昨买:" << this->stg_position_b_buy_yesterday << std::endl;
	std::cout << "\tA总买:" << this->stg_position_a_buy << ", A昨买:" << this->stg_position_a_buy_yesterday << std::endl;
	std::cout << "\tB总卖:" << this->stg_position_b_sell << ", B昨卖:" << this->stg_position_b_sell_yesterday << std::endl;*/

	//Utils::printGreenColorWithKV("Trade持仓明细大小", this->getStg_List_Position_Detail_From_Trade()->size());
	//Utils::printGreenColor("Trade持仓明细");

	//list<USER_CThostFtdcTradeField *>::iterator posd_itor_trade;
	//// 遍历strategy持仓明细(trade)并保存
	//for (posd_itor_trade = this->getStg_List_Position_Detail_From_Trade()->begin();
	//	posd_itor_trade != this->getStg_List_Position_Detail_From_Trade()->end();
	//	posd_itor_trade++) {

	//	//this->dbm->CreatePositionDetail((*posd_itor));
	//	
	//	std::cout << "\tinstrumentid = " << (*posd_itor_trade)->InstrumentID
	//		<< ", orderref = " << (*posd_itor_trade)->OrderRef
	//		<< ", userid = " << (*posd_itor_trade)->UserID;

	//	if ((*posd_itor_trade)->Direction == '0')
	//	{
	//		std::cout << ", direction = \033[32m" << (*posd_itor_trade)->Direction << "\033[0m";
	//		std::cout << ", volume = \033[32m" << (*posd_itor_trade)->Volume << "\033[0m";
	//	}
	//	else {
	//		std::cout << ", direction = \033[31m" << (*posd_itor_trade)->Direction << "\033[0m";
	//		std::cout << ", volume = \033[31m" << (*posd_itor_trade)->Volume << "\033[0m";
	//	}
	//		 
	//		std::cout << ", offsetflag = " << (*posd_itor_trade)->OffsetFlag
	//		<< ", hedgeflag = " << (*posd_itor_trade)->HedgeFlag 
	//		<< ", price = " << (*posd_itor_trade)->Price 
	//		<< ", tradingday = " << (*posd_itor_trade)->TradingDay 
	//		<< ", tradingdayrecord = " << (*posd_itor_trade)->TradingDayRecord
	//		<< ", tradingdate = " << (*posd_itor_trade)->TradeDate 
	//		<< ", strategyid = " << (*posd_itor_trade)->StrategyID 
	//		<< std::endl;
	//}

	////Utils::printGreenColorWithKV("Order持仓明细大小", this->getStg_List_Position_Detail_From_Order()->size());
	//Utils::printGreenColor("Order持仓明细");

	//list<USER_CThostFtdcOrderField *>::iterator posd_itor;
	//// 遍历strategy持仓明细(order)并保存
	//for (posd_itor = this->getStg_List_Position_Detail_From_Order()->begin();
	//	posd_itor != this->getStg_List_Position_Detail_From_Order()->end();
	//	posd_itor++) {
	//	
	//	std::cout << "\tinstrumentid = " << (*posd_itor)->InstrumentID
	//		<< ", orderref = " << (*posd_itor)->OrderRef
	//		<< ", userid = " << (*posd_itor)->UserID;

	//	if ((*posd_itor)->Direction == '0')
	//	{
	//		std::cout << ", direction = \033[32m" << (*posd_itor)->Direction << "\033[0m";
	//		std::cout << ", volumetradedbatch = \033[32m" << (*posd_itor)->VolumeTradedBatch << "\033[0m";
	//	}
	//	else {
	//		std::cout << ", direction = \033[31m" << (*posd_itor)->Direction << "\033[0m";
	//		std::cout << ", volumetradedbatch = \033[31m" << (*posd_itor)->VolumeTradedBatch << "\033[0m";
	//	}


	//	std::cout << ", comboffsetflag = " << (*posd_itor)->CombOffsetFlag[0]
	//		<< ", combhedgeflag = " << (*posd_itor)->CombHedgeFlag[0]
	//		<< ", limitprice = " << (*posd_itor)->LimitPrice 
	//		<< ", volumetotaloriginal = " << (*posd_itor)->VolumeTotalOriginal 
	//		<< ", tradingday = " << (*posd_itor)->TradingDay 
	//		<< ", tradingdayrecord = " << (*posd_itor)->TradingDayRecord 
	//		<< ", orderstatus = " << (*posd_itor)->OrderStatus 
	//		<< ", volumetraded = " << (*posd_itor)->VolumeTraded 
	//		<< ", volumetotal = " << (*posd_itor)->VolumeTotal 
	//		<< ", insertdate = " << (*posd_itor)->InsertDate 
	//		<< ", inserttime = " << (*posd_itor)->InsertTime 
	//		<< ", strategyid = " << (*posd_itor)->StrategyID 
	//		<< std::endl;
	//}
}

void Strategy::setStgUpdatePositionDetailRecordTime(string stg_update_position_detail_record_time) {
	this->stg_update_position_detail_record_time = stg_update_position_detail_record_time;
}
string Strategy::getStgUpdatePositionDetailRecordTime() {
	return this->stg_update_position_detail_record_time;
}


//仓位是否正确,是否需要调整仓位
void Strategy::setStgIsPositionRight(bool is_position_right) {
	this->is_position_right = is_position_right;
}

bool Strategy::getStgIsPositionRight() {
	return this->is_position_right;
}

//清空持仓明细
void Strategy::clearStgPositionDetail() {
	std::cout << "Strategy::clearStgPositionDetail()" << std::endl;
	list<USER_CThostFtdcOrderField *>::iterator order_itor;
	for (order_itor = this->stg_list_position_detail_from_order->begin();
		order_itor != this->stg_list_position_detail_from_order->end();)
	{
		delete *order_itor;
		order_itor = this->stg_list_position_detail_from_order->erase(order_itor);
	}

	list<USER_CThostFtdcTradeField *>::iterator trade_itor;
	for (trade_itor = this->stg_list_position_detail_from_trade->begin(); trade_itor != this->stg_list_position_detail_from_trade->end();) {
		delete *trade_itor;
		trade_itor = this->stg_list_position_detail_from_trade->erase(trade_itor);
	}

}

//构造模拟平仓order
void Strategy::createFakeOrderPositionDetail(USER_CThostFtdcOrderField *order, string date, string instrumentID, char CombHedgeFlag, char Direction, char CombOffsetFlag, int VolumeTradedBatch) {
	strcpy(order->TradingDay, date.c_str()); // 日期
	strcpy(order->InstrumentID, instrumentID.c_str()); // A合约ID
	order->CombHedgeFlag[0] = CombHedgeFlag; // 1投机 2套利 3保值
	order->Direction = Direction; // '0'买 '1'卖
	order->VolumeTradedBatch = VolumeTradedBatch; // 成交量
	order->CombOffsetFlag[0] = CombOffsetFlag; // '3'平今 '4'平昨
}

//构造模拟平仓trade
void Strategy::createFakeTradePositionDetail(USER_CThostFtdcTradeField *trade, string date, string instrumentID, char HedgeFlag, char Direction, char OffsetFlag, int Volume) {
	strcpy(trade->TradingDay, date.c_str()); // 日期
	strcpy(trade->InstrumentID, instrumentID.c_str()); // A合约ID
	trade->HedgeFlag = HedgeFlag; // 1投机 2套利 3保值
	trade->Direction = Direction; // '0'买 '1'卖
	trade->Volume = Volume; // 成交量
	trade->OffsetFlag = OffsetFlag; // '3'平今 '4'平昨
}

//收盘前5秒处理挂单列表
void Strategy::finish_pending_order_list() {
	USER_PRINT("Strategy::finish_pending_order_list()");
	std::cout << "Strategy::finish_pending_order_list()" << std::endl;
	std::cout << "\t挂单列表长度:" << this->stg_list_order_pending->size() << std::endl;
	std::cout << "\t期货账户:" << this->stg_user_id << std::endl;
	std::cout << "\t策略编号:" << this->stg_strategy_id << std::endl;
	
	// 遍历挂单列表,如果是A合约,立马撤单,如果是B合约,超价发单
	list<CThostFtdcOrderField *>::iterator itor;
	for (itor = this->stg_list_order_pending->begin(); itor != this->stg_list_order_pending->end(); itor++) {
		// A合约
		if (!strcmp((*itor)->InstrumentID, this->stg_instrument_id_A.c_str())) {
			/// A合约撤单
			this->stg_user->getUserTradeSPI()->OrderAction((*itor)->ExchangeID, (*itor)->OrderRef, (*itor)->OrderSysID);
		}
		// B合约
		else if (!strcmp((*itor)->InstrumentID, this->stg_instrument_id_B.c_str()))
		{
			// 1:先撤单
			this->stg_user->getUserTradeSPI()->OrderAction((*itor)->ExchangeID, (*itor)->OrderRef, (*itor)->OrderSysID);
			
			// 2:再超价发单
			// B合约报单参数
			// 合约代码
			std::strcpy(this->stg_b_order_insert_args->InstrumentID, this->stg_instrument_id_B.c_str());
			// 限价判断
			// 如果是买,限价基础上加10个最小跳价格发单; 如果是卖,限价减10个最小跳发单
			if ((*itor)->Direction = '0') { // 买
				this->stg_b_order_insert_args->LimitPrice = (*itor)->LimitPrice + 10 * this->stg_b_price_tick;
			}
			else if ((*itor)->Direction = '1') // 卖
			{
				this->stg_b_order_insert_args->LimitPrice = (*itor)->LimitPrice - 10 * this->stg_b_price_tick;
			}

			// 数量
			//this->stg_b_order_insert_args->VolumeTotalOriginal = order_volume;
			// 买卖方向
			this->stg_b_order_insert_args->Direction = (*itor)->Direction; // 0买 1卖
			// 组合开平标志
			this->stg_b_order_insert_args->CombOffsetFlag[0] = (*itor)->CombOffsetFlag[0]; //组合开平标志0开仓 上期所3平今、4平昨，其他交易所1平仓
			// 组合投机套保标志
			this->stg_b_order_insert_args->CombHedgeFlag[0] = '1'; // 1投机 2套利 3保值
			
			// 发单手数:(剩余的数量)
			this->stg_b_order_insert_args->VolumeTotalOriginal = (*itor)->VolumeTotal;

			// 报单引用
			this->stg_order_ref_b = this->Generate_Order_Ref();
			this->stg_order_ref_last = this->stg_order_ref_b;
			strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_b.c_str());

			// 调用发单方法
			this->Exec_OrderInsert(this->stg_b_order_insert_args);
		}
	}
}

//策略最后一次保存的时间
void Strategy::setStgLastSavedTime(string stg_last_saved_time) {
	this->stg_last_saved_time = stg_last_saved_time;
}
string Strategy::getStgLastSavedTime() {
	return this->stg_last_saved_time;
}

//是否允许收盘任务执行
bool Strategy::getStgOnOffEndTask() {
	return this->on_off_end_task;
}

void Strategy::setStgOnOffEndTask(bool on_off_end_task) {
	this->on_off_end_task = on_off_end_task;
}

// 设置系统xts_logger
void Strategy::setXtsLogger(std::shared_ptr<spdlog::logger> ptr) {
	this->xts_logger = ptr;
}

std::shared_ptr<spdlog::logger> Strategy::getXtsLogger() {
	return this->xts_logger;
}

// 获取持仓明细
list<USER_CThostFtdcOrderField *> *Strategy::getStg_List_Position_Detail_From_Order() {
	return this->stg_list_position_detail_from_order;
}


// 持仓明细(trade)
list<USER_CThostFtdcTradeField *> *Strategy::getStg_List_Position_Detail_From_Trade() {
	return this->stg_list_position_detail_from_trade;
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

	CThostFtdcDepthMarketDataField pDepthMarketData_tmp;
	memset(&pDepthMarketData_tmp, 0x00, sizeof(CThostFtdcDepthMarketDataField));
	this->CopyTickData(&pDepthMarketData_tmp, pDepthMarketData);

	//time_t tt = system_clock::to_time_t(system_clock::now());
	//std::string nowt(std::ctime(&tt));
	//string new_time = Utils::getNowTimeMs();

	if (!strcmp(pDepthMarketData_tmp.InstrumentID, this->getStgInstrumentIdA().c_str())) {
		USER_PRINT("stg_instrument_A_tick ask_volume bid_volume");
		this->CopyTickData(stg_instrument_A_tick, &pDepthMarketData_tmp);
	}
	else if (!strcmp(pDepthMarketData_tmp.InstrumentID, this->getStgInstrumentIdB().c_str()))
	{
		USER_PRINT("stg_instrument_B_tick ask_volume bid_volume");
		this->CopyTickData(stg_instrument_B_tick, &pDepthMarketData_tmp);
	}

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
		/*std::cout << "Strategy::OnRtnDepthMarketData():" << std::endl;
		std::cout << "\t(有交易任务,进入交易任务执行)" << std::endl;
		std::cout << "\t(stg_trade_tasking):(" << this->stg_trade_tasking << ")" << std::endl;*/
		// 有隐患，改为临时变量
		//this->Exec_OnTickComing(pDepthMarketData);
		this->Exec_OnTickComing(&pDepthMarketData_tmp);
	} else { /// 如果没有交易任务，那么选择开始新的交易任务

		// 如果休盘期间
		if (this->stg_user->getCTP_Manager()->getIsMarketClose())
		{
			/*std::cout << "Strategy::OnRtnDepthMarketData()" << std::endl;
			Utils::printRedColor("非盘中时间!");
			Utils::printRedColor("已停止新的任务!");*/

			//收到最后5秒开始强制处理挂单列表命令
			if (this->stg_user->getCTP_Manager()->getIsStartEndTask()) {
				if (this->getStgOnOffEndTask())
				{
					this->setStgOnOffEndTask(false);
					this->finish_pending_order_list();
				}
			}
			return;
		}

		// 如果撤单次数用完
		if (this->getOn_Off()) {
			if ((this->getStgAOrderActionCount() > this->getStgAOrderActionTiresLimit()) ||
				(this->getStgBOrderActionCount() > this->getStgBOrderActionTiresLimit())) {
				std::cout << "Strategy::OnRtnDepthMarketData()" << std::endl;
				Utils::printRedColor("撤单次数已超设置值!");
				std::cout << "\t期货账户:" << this->stg_user_id << std::endl;
				std::cout << "\t策略编号:" << this->stg_strategy_id << std::endl;
				std::cout << "\tA合约撤单次数:" << this->getStgAOrderActionCount() << ", A合约撤单限制:" << this->getStgAOrderActionTiresLimit() << std::endl;
				std::cout << "\tB合约撤单次数:" << this->getStgBOrderActionCount() << ", B合约撤单限制:" << this->getStgBOrderActionTiresLimit() << std::endl;
				Utils::printRedColor("已停止新的任务!");
				return;
			}
		}

		// 如果选择下单算法锁释放,那么开始新的下单
		if (!stg_select_order_algorithm_flag) {
			//this->setStgSelectOrderAlgorithmFlag("Strategy::OnRtnDepthMarketData()", true); // 开启下单锁
			this->Select_Order_Algorithm(this->getStgOrderAlgorithm());
			/*std::cout << "Strategy::OnRtnDepthMarketData():" << std::endl;
			std::cout << "\t(开启tick锁 stg_select_order_algorithm_flag):(" << this->stg_select_order_algorithm_flag << ")" << std::endl;*/
		} else {
			if (this->getOn_Off()) {
				
				/*std::cout << "\t期货账户:" << this->stg_user_id << std::endl;
				std::cout << "\t策略编号:" << this->stg_strategy_id << std::endl;
				std::cout << "\ttick锁已上锁 stg_select_order_algorithm_flag = " << this->stg_select_order_algorithm_flag << ")" << std::endl;*/

				this->getStgUser()->getXtsLogger()->info("Strategy::OnRtnDepthMarketData():");
				this->getStgUser()->getXtsLogger()->info("\t期货账户:{}", this->stg_user_id);
				this->getStgUser()->getXtsLogger()->info("\t策略编号:{}", this->stg_strategy_id);
				this->getStgUser()->getXtsLogger()->info("\ttick锁已上锁:{}", this->stg_select_order_algorithm_flag);
				//std::cout << "\t上一个tick系统时间 = " << this->stg_tick_systime_record << std::endl;
				//std::cout << "\t新tick系统时间 = " << new_time << std::endl;
			}
		}
	}

	this->CopyTickData(this->stg_instrument_Last_tick, &pDepthMarketData_tmp);
	//this->stg_tick_systime_record = new_time;

	USER_PRINT("Strategy::OnRtnDepthMarketData OUT");
	//tick_mtx.unlock();
}

//选择下单算法
void Strategy::Select_Order_Algorithm(string stg_order_algorithm) {
	USER_PRINT("Strategy::Select_Order_Algorithm");

	////如果正在交易,直接返回0
	//if (this->stg_trade_tasking) {
	//	USER_PRINT("正在交易,返回");
	//	//this->printStrategyInfo("正在交易,返回");
	//	return;
	//}
	//如果有挂单,返回0
	if (this->stg_list_order_pending->size() > 0) {
		list<CThostFtdcOrderField *>::iterator itor;
		for (itor = this->stg_list_order_pending->begin(); itor != this->stg_list_order_pending->end(); itor++) {
			USER_PRINT((*itor)->InstrumentID);
		}
		//this->printStrategyInfo("有挂单,返回");
		//std::cout << "Strategy::Select_Order_Algorithm():" << std::endl;
		//std::cout << "\t(有挂单,返回)" << std::endl;
		this->setStgSelectOrderAlgorithmFlag("Strategy::OnRtnDepthMarketData() 有挂单", false); // 关闭下单锁
		return;
	}

	if (!((this->stg_position_a_sell == this->stg_position_b_buy) && 
		(this->stg_position_a_buy == this->stg_position_b_sell))) {
		//this->printStrategyInfo("有撇腿,返回");
		// 有撇腿
		//std::cout << "Strategy::Select_Order_Algorithm():" << std::endl;
		//std::cout << "\t(有撇腿)" << std::endl;
		/*std::cout << "\t当前持仓情况(A卖,B买)(A买,B卖) = (" << this->stg_position_a_sell << ", " <<
			this->stg_position_b_buy << ")(" <<
			this->stg_position_a_buy << ", " <<
			this->stg_position_b_sell << ")" << std::endl;*/
		this->setStgSelectOrderAlgorithmFlag("Strategy::OnRtnDepthMarketData() 有撇腿", false); // 关闭下单锁
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
		//this->setStgSelectOrderAlgorithmFlag("Strategy::Order_Algorithm_One()_1", false);
		return;
	}


	/// 策略开关，期货账户开关，交易员开关，系统总开关
	if (!((this->getOn_Off()) && 
		(this->stg_user->getOn_Off()) && 
		(this->stg_user->GetTrader()->getOn_Off()) && 
		(this->stg_user->getCTP_Manager()->getOn_Off()))) {
		USER_PRINT("请检查开关状态!");
		/*this->printStrategyInfo("请检查开关状态!");
		std::cout << "策略开关 = " << this->getOn_Off() << std::endl;
		std::cout << "期货账户开关 = " << this->stg_user->getOn_Off() << std::endl;
		std::cout << "总开关 = " << this->stg_user->GetTrader()->getOn_Off() << std::endl;*/
		//this->setStgSelectOrderAlgorithmFlag("Strategy::Order_Algorithm_One()_2", false);
		return;
	}

	/// 价差卖平(b)
	if ((this->sell_close_on_off) &&
		(this->stg_position_a_sell == this->stg_position_b_buy) &&
		(this->stg_position_a_buy > 0) &&
		(this->stg_spread_long >= (this->stg_sell_close + this->stg_spread_shift * this->stg_a_price_tick)) &&
		(!this->stg_select_order_algorithm_flag)) {
		
		this->setStgSelectOrderAlgorithmFlag("Strategy::Order_Algorithm_One() 价差卖平", true); // 开启下单锁

		//this->stg_trade_tasking = true;
		//this->printStrategyInfo("价差卖平");
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

		/*if (this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) || this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick)) {
			std::cout << "Strategy::Order_Algorithm_One()" << std::endl;
			std::cout << "\tCompareTickData相同!" << std::endl;
			std::cout << "\tthis->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) = " << this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) << std::endl;
			std::cout << "\tthis->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick) = " << this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick) << std::endl;
			return;
		}*/

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
			this->setStgTradeTasking(false);
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
		(this->stg_spread_short <= (this->stg_buy_close - this->stg_spread_shift * this->stg_a_price_tick)) &&
		(!this->stg_select_order_algorithm_flag)) {

		this->setStgSelectOrderAlgorithmFlag("Strategy::Order_Algorithm_One() 价差买平", true); // 开启下单锁

		//this->setStgTradeTasking(true);
		//this->printStrategyInfo("价差买平");
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

		/*if (this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) || this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick)) {
			std::cout << "Strategy::Order_Algorithm_One()" << std::endl;
			std::cout << "\t CompareTickData相同!" << std::endl;
			std::cout << "\tthis->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) = " << this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) << std::endl;
			std::cout << "\tthis->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick) = " << this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick) << std::endl;
			return;
			}*/

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
			this->setStgTradeTasking(false);
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
		(this->stg_spread_long >= (this->stg_sell_open + this->stg_spread_shift * this->stg_a_price_tick)) &&
		(!this->stg_select_order_algorithm_flag)) {
		std::cout << "Strategy::Order_Algorithm_One()" << std::endl;
		std::cout << "\tthis->stg_select_order_algorithm_flag before set = " << this->stg_select_order_algorithm_flag << std::endl;

		this->setStgSelectOrderAlgorithmFlag("Strategy::Order_Algorithm_One() 价差卖开", true); // 开启下单锁
		
		//this->setStgTradeTasking(true);
		//this->printStrategyInfo("价差卖开");
		//this->update_task_status();

		/** 市场多头价差大于触发参数
		A合约买持仓加B合约买小于总仓位**/

		//std::cout << "策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差卖开" << endl;
		
		//std::cout << "\t策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差卖开" << endl;
		
		std::cout << "user_id = " << this->stg_user_id << ", "
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
			<< "stg_lots = " << this->stg_lots << endl;

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

		/*if (this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) || this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick)) {
			std::cout << "Strategy::Order_Algorithm_One()" << std::endl;
			std::cout << "\t CompareTickData相同!" << std::endl;
			std::cout << "\tthis->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) = " << this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) << std::endl;
			std::cout << "\tthis->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick) = " << this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick) << std::endl;
			return;
			}*/

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
			this->setStgTradeTasking(false);
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
		((this->stg_spread_short <= (this->stg_buy_open - this->stg_spread_shift * this->stg_a_price_tick))) &&
		(!this->stg_select_order_algorithm_flag)) {

		this->setStgSelectOrderAlgorithmFlag("Strategy::Order_Algorithm_One() 价差买开", true); // 开启下单锁

		//this->stg_trade_tasking = true;
		//this->printStrategyInfo("价差买开");
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

		/*if (this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) || this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick)) {
			std::cout << "Strategy::Order_Algorithm_One()" << std::endl;
			std::cout << "\t CompareTickData相同!" << std::endl;
			std::cout << "\tthis->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) = " << this->CompareTickData(stg_instrument_A_tick_last, stg_instrument_A_tick) << std::endl;
			std::cout << "\tthis->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick) = " << this->CompareTickData(stg_instrument_B_tick_last, stg_instrument_B_tick) << std::endl;
			return;
		}*/

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
			this->setStgTradeTasking(false);
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
	/*std::cout << "Strategy::Exec_OrderInsert()" << std::endl;
	std::cout << "\tOrderRef = " << insert_order->OrderRef <<
	", InstrumentID = " << insert_order->InstrumentID <<
	", LimitPrice = " << insert_order->LimitPrice <<
	", VolumeTotalOriginal = " << insert_order->VolumeTotalOriginal <<
	", Direction = " << insert_order->Direction <<
	", CombOffsetFlag = " << insert_order->CombOffsetFlag[0] <<
	", CombHedgeFlag = " << insert_order->CombHedgeFlag[0] << std::endl;*/

	this->getStgUser()->getXtsLogger()->info("Strategy::Exec_OrderInsert()");
	this->getStgUser()->getXtsLogger()->info("\tOrderRef = {}, InstrumentID = {}, LimitPrice = {}, VolumeTotalOriginal = {}, Direction = {}, CombOffsetFlag = {}, CombHedgeFlag = {}",
		insert_order->OrderRef, insert_order->InstrumentID, insert_order->LimitPrice, insert_order->VolumeTotalOriginal, insert_order->Direction,
		insert_order->CombOffsetFlag[0], insert_order->CombHedgeFlag[0]);

	//下单操作
	this->stg_user->getUserTradeSPI()->OrderInsert(this->stg_user, insert_order);
}

// 报单录入请求
void Strategy::Exec_OnRspOrderInsert() {
	USER_PRINT("Exec_OnRspOrderInsert()");
	//std::cout << "Strategy::Exec_OnRspOrderInsert()" << std::endl;
	this->getStgUser()->getXtsLogger()->info("Strategy::Exec_OnRspOrderInsert()");
	this->setStgSelectOrderAlgorithmFlag("Strategy::Exec_OnRspOrderInsert()", false);
}

// 报单操作请求响应
void Strategy::Exec_OnRspOrderAction() {
	USER_PRINT("Exec_OnRspOrderAction()");
}

// 报单回报
void Strategy::Exec_OnRtnOrder(CThostFtdcOrderField *pOrder) {
	//std::cout << "Strategy::Exec_OnRtnOrder()" << std::endl;
	USER_PRINT("Exec_OnRtnOrder");

	string compare_date = pOrder->InsertDate; //报单日期
	string compare_time = pOrder->InsertTime; //报单时间
	// 如果pOrder的时间在修改持仓之前 return
	if (!Utils::compareTradingDaySeconds((compare_date + compare_time).c_str(), this->getStgLastSavedTime().c_str())) {
		return;
	}

	// 添加字段,本次成交量
	USER_CThostFtdcOrderField order_new;
	memset(&order_new, 0x00, sizeof(USER_CThostFtdcOrderField));
	// 添加字段,本次成交量
	USER_CThostFtdcOrderField order_new_tmp;
	memset(&order_new_tmp, 0x00, sizeof(USER_CThostFtdcOrderField));
	CThostFtdcOrderField pOrder_tmp;
	memset(&pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
	

	// 添加本次成交字段VolumeTradedBatch
	this->add_VolumeTradedBatch(pOrder, &order_new);
	this->CopyNewOrderData(&order_new_tmp, &order_new);
	this->CopyOrderData(&pOrder_tmp, pOrder);
	
	// 更新挂单列表，持仓信息
	this->update_pending_order_list(&pOrder_tmp);

	// 更新持仓明细列表
	this->update_position_detail(&order_new_tmp);

	// 更新持仓变量
	this->update_position(&order_new);

	// 更新标志位
	this->update_task_status();

	// 更新tick锁
	//this->update_tick_lock_status(order_new);

	//delete order_new;
	//delete order_new_tmp;

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
	string compare_date = pTrade->TradeDate; //报单日期
	string compare_time = pTrade->TradeTime; //报单时间

	// 如果pTrade的时间在修改持仓之前 return
	if (!Utils::compareTradingDaySeconds((compare_date + compare_time).c_str(), this->getStgLastSavedTime().c_str())) {
		return;
	}

	USER_CThostFtdcTradeField new_trade;
	this->CopyTradeDataToNew(&new_trade, pTrade);
	this->update_position_detail(&new_trade);

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
	//std::cout << "Strategy::update_pending_order_list()" << std::endl;
	USER_PRINT("Strategy::update_pending_order_list");
	/************************************************************************/
	/* 此处pOrder应该谨慎操作                                                  */
	/************************************************************************/
	//std::cout << "pending_order_list size = "<< this->stg_list_order_pending->size() << std::endl;
	USER_PRINT(this->stg_list_order_pending->size());
	USER_PRINT(pOrder->OrderSysID);

	USER_PRINT(pOrder->OrderStatus);

	/*std::cout << "Strategy::update_pending_order_list()" << std::endl;
	std::cout << "\t期货账户:" << this->stg_user_id << std::endl;
	std::cout << "\t策略编号:" << this->stg_strategy_id << std::endl;
	std::cout << "\tBegin:this->stg_pending_a_open = " << this->stg_pending_a_open << std::endl;*/

	if (strlen(pOrder->OrderSysID) != 0) { // 如果报单编号不为空，为交易所返回
		if (pOrder->OrderStatus == '0') { // 全部成交
			//std::cout << "更新挂单,全部成交" << std::endl;
			this->getStgUser()->getXtsLogger()->info("\tpOrder->OrderStatus = 0 全部成交");
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
			this->getStgUser()->getXtsLogger()->info("\tpOrder->OrderStatus = 1 部分成交还在队列中");

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
		else if (pOrder->OrderStatus == '2') { ///部分成交不在队列中
			//std::cout << "\tpOrder->OrderStatus = 2 部分成交不在队列中" << std::endl;
			this->getStgUser()->getXtsLogger()->info("\tpOrder->OrderStatus = 2 部分成交不在队列中");
		}
		else if (pOrder->OrderStatus == '3') { // 未成交还在队列中
			//std::cout << "更新挂单,未成交还在队列中" << std::endl;
			this->getStgUser()->getXtsLogger()->info("\tpOrder->OrderStatus = 3 未成交还在队列中");
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
		else if (pOrder->OrderStatus == '4') //未成交不在队列中
		{
			//std::cout << "\tpOrder->OrderStatus = 4 未成交不在队列中" << std::endl;
			this->getStgUser()->getXtsLogger()->info("\tpOrder->OrderStatus = 4 未成交不在队列中");
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
			//std::cout << "\tpOrder->OrderStatus = a 未知" << std::endl;
			this->getStgUser()->getXtsLogger()->info("\tpOrder->OrderStatus = a 未知");
		}
		else if (pOrder->OrderStatus == 'b') { // 尚未触发
			//std::cout << "\tpOrder->OrderStatus = b 尚未触发" << std::endl;
			this->getStgUser()->getXtsLogger()->info("\tpOrder->OrderStatus = b 尚未触发");
		}
		else if (pOrder->OrderStatus == 'c') { // 已触发
			//std::cout << "\tpOrder->OrderStatus = c 已触发" << std::endl;
			this->getStgUser()->getXtsLogger()->info("\tpOrder->OrderStatus = c 已触发");
		}

		// 遍历挂单列表，找出A合约开仓未成交的量
		list<CThostFtdcOrderField *>::iterator cal_itor;
		this->stg_pending_a_open = 0;
		for (cal_itor = this->stg_list_order_pending->begin(); cal_itor != this->stg_list_order_pending->end(); cal_itor++) {
			// 对比InstrumentID
			if (!strcmp((*cal_itor)->InstrumentID, this->stg_instrument_id_A.c_str()) && ((*cal_itor)->CombOffsetFlag[0] == '0')) { // 查找A合约开仓
				this->stg_pending_a_open += (*cal_itor)->VolumeTotalOriginal - (*cal_itor)->VolumeTraded;
			}
		}

		//std::cout << "Strategy::update_pending_order_list()" << std::endl;
		//std::cout << "\tEnd:this->stg_pending_a_open = " << this->stg_pending_a_open << std::endl;
	}
	else { // 报单编号长度为0, CTP 直接返回的错误或者还未发到交易所
		/************************************************************************/
		/* 1：错误发单情况已处理tick解锁
			详细参考：Strategy::Exec_OnRspOrderInsert()， Strategy::Exec_OnErrRtnOrderInsert()
		*/
		/************************************************************************/
		/************************************************************************/
		/* 2：报单发往交易所未收到回报                                                                     */
		/************************************************************************/
	}
}

#if 0
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
#endif

/// 更新持仓量(UserOrder)
void Strategy::update_position(USER_CThostFtdcOrderField *pOrder) {
	USER_PRINT("Strategy::update_position");
	USER_PRINT(pOrder->InstrumentID);
	USER_PRINT(this->stg_instrument_id_A);
	USER_PRINT(this->stg_instrument_id_B);
	//this->printStrategyInfo("Strategy::update_position() 输出VolumeTradedBatch");
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

	/*std::cout << "Strategy::update_position():" << std::endl;
	std::cout << "\t期货账户:" << this->stg_user_id << std::endl;
	std::cout << "\t策略编号:" << this->stg_strategy_id << std::endl;
	std::cout << "\tA卖(" << this->stg_position_a_sell << ", " << this->stg_position_a_sell_yesterday << ")" << std::endl;
	std::cout << "\tB买(" << this->stg_position_b_buy << ", " << this->stg_position_b_buy_yesterday << ")" << std::endl;
	std::cout << "\tA买(" << this->stg_position_a_buy << ", " << this->stg_position_a_buy_yesterday << ")" << std::endl;
	std::cout << "\tB卖(" << this->stg_position_b_sell << ", " << this->stg_position_b_sell_yesterday << ")" << std::endl;
	std::cout << "\t挂单列表长度 = " << this->stg_list_order_pending->size() << std::endl;
	std::cout << "\t任务执行状态 = " << this->stg_trade_tasking << std::endl;
	std::cout << "\t本次成交量 = " << pOrder->VolumeTradedBatch << ", 报单引用 = " << pOrder->OrderRef << std::endl;*/

}

#if 0
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

	/*USER_PRINT("A合约今买");
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
	USER_PRINT(this->stg_position_b_sell);*/

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
					&& ((*Itor)->Direction != pTrade->Direction)) {
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
					&& ((*Itor)->Direction != pTrade->Direction)) {
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
#endif

/// 更新持仓明细
void Strategy::update_position_detail(USER_CThostFtdcTradeField *pTrade) {
	USER_PRINT("Strategy::update_position_detail");
	//std::cout << "Strategy::update_position_detail() trade" << std::endl;
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

	/*USER_PRINT(pOrder->VolumeTraded);*/
	/*std::cout << "\tTrade 更新持仓明细" << std::endl;
	std::cout << "\tpTrade->OffsetFlag = " << pTrade->OffsetFlag
	<< "， pTrade->InstrumentID = " << pTrade->InstrumentID
	<< "， pTrade->HedgeFlag = " << pTrade->HedgeFlag
	<< "， pTrade->Direction = " << pTrade->Direction
	<< "， pTrade->Volume = " << pTrade->Volume
	<< "， pTrade->TradingDay = " << pTrade->TradingDay << std::endl;*/


	if (pTrade->OffsetFlag == '0') // pTrade中"OffsetFlag"值 = "0"为开仓，不用考虑全部成交还是部分成交，开仓trade直接添加到持仓明细列表里
	{
		if (!strcmp(pTrade->InstrumentID, this->stg_instrument_id_A.c_str())) { //A合约

		} else if (!strcmp(pTrade->InstrumentID, this->stg_instrument_id_B.c_str())) { //B合约

		}

		USER_CThostFtdcTradeField *new_trade = new USER_CThostFtdcTradeField();
		memset(new_trade, 0, sizeof(USER_CThostFtdcTradeField));
		this->CopyNewTradeData(new_trade, pTrade);

		this->stg_list_position_detail_from_trade->push_back(new_trade);

	}
	else if (pTrade->OffsetFlag == '3') { //pTrade中"OffsetFlag"值 = "3"为平今
		list<USER_CThostFtdcTradeField *>::iterator itor;
		for (itor = this->stg_list_position_detail_from_trade->begin(); itor != this->stg_list_position_detail_from_trade->end();) {

			USER_PRINT((*itor)->TradeDate);
			USER_PRINT(pTrade->TradeDate);

			USER_PRINT((*itor)->InstrumentID);
			USER_PRINT(pTrade->InstrumentID);

			USER_PRINT((*itor)->HedgeFlag);
			USER_PRINT(pTrade->HedgeFlag);

			USER_PRINT((*itor)->Volume);
			USER_PRINT(pTrade->Volume);

			if ((!strcmp((*itor)->TradingDay, pTrade->TradingDay)) &&
				(!strcmp((*itor)->InstrumentID, pTrade->InstrumentID)) &&
				((*itor)->HedgeFlag == pTrade->HedgeFlag) &&
				((*itor)->Direction != pTrade->Direction)) { //持仓明细中trade与pTrade比较：交易日相同、合约代码相同、投保标志相同
				if (pTrade->Volume == (*itor)->Volume)
				{
					delete (*itor);
					itor = this->stg_list_position_detail_from_trade->erase(itor);
					break;
				}
				else if (pTrade->Volume < (*itor)->Volume)
				{
					(*itor)->Volume -= pTrade->Volume;
					break;
				} else if (pTrade->Volume > (*itor)->Volume)
				{
					pTrade->Volume -= (*itor)->Volume;
					delete (*itor);
					itor = this->stg_list_position_detail_from_trade->erase(itor);
				}
			}
			else
			{
				itor++;
			}
		}
	}
	else if (pTrade->OffsetFlag == '4') { //pTrade中"OffsetFlag"值 = "4"为平昨
		list<USER_CThostFtdcTradeField *>::iterator itor;
		for (itor = this->stg_list_position_detail_from_trade->begin(); itor != this->stg_list_position_detail_from_trade->end();) {
			if ((strcmp((*itor)->TradingDay, pTrade->TradingDay)) &&
				(!strcmp((*itor)->InstrumentID, pTrade->InstrumentID)) &&
				((*itor)->HedgeFlag == pTrade->HedgeFlag) &&
				((*itor)->Direction != pTrade->Direction)) { //持仓明细中trade与pTrade比较：交易日不相同、合约代码相同、投保标志相同
				if (pTrade->Volume == (*itor)->Volume)
				{
					delete (*itor);
					itor = this->stg_list_position_detail_from_trade->erase(itor);
					break;
				}
				else if (pTrade->Volume < (*itor)->Volume)
				{
					(*itor)->Volume -= pTrade->Volume;
					break;
				}
				else if (pTrade->Volume > (*itor)->Volume)
				{
					pTrade->Volume -= (*itor)->Volume;
					delete (*itor);
					itor = this->stg_list_position_detail_from_trade->erase(itor);
				}
			}
			else {
				itor++;
			}
		}
	}

	USER_PRINT("Strategy::update_position_detail out");
}

/// 更新持仓明细(Order)
void Strategy::update_position_detail(USER_CThostFtdcOrderField *pOrder) {
	//std::cout << "Strategy::update_position_detail() order" << std::endl;
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
	/*std::cout << "\tOrder 更新持仓明细" << std::endl;
	std::cout << "\tpOrder->CombOffsetFlag[0] = " << pOrder->CombOffsetFlag[0]
	<< ", pOrder->InstrumentID = " << pOrder->InstrumentID
	<< ", pOrder->CombHedgeFlag[0] = " << pOrder->CombHedgeFlag[0]
	<< ", pOrder->Direction = " << pOrder->Direction
	<< ", pOrder->VolumeTradedBatch = " << pOrder->VolumeTradedBatch
	<< ", pOrder->TradingDay = " << pOrder->TradingDay
	<< ", pOrder->VolumeTradedBatch = " << pOrder->VolumeTradedBatch << std::endl;*/

	if (pOrder->VolumeTradedBatch == 0) {
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
	} 
	else if (pOrder->CombOffsetFlag[0] == '3') { // 平今
		USER_PRINT("平今in");
		list<USER_CThostFtdcOrderField *>::iterator itor;
		for (itor = this->stg_list_position_detail_from_order->begin(); 
			itor != this->stg_list_position_detail_from_order->end();)
		{
			USER_PRINT((*itor)->InsertDate);
			USER_PRINT(pOrder->InsertDate);

			USER_PRINT((*itor)->InstrumentID);
			USER_PRINT(pOrder->InstrumentID);

			USER_PRINT((*itor)->CombHedgeFlag[0]);
			USER_PRINT(pOrder->CombHedgeFlag[0]);

			USER_PRINT((*itor)->VolumeTradedBatch);
			USER_PRINT(pOrder->VolumeTradedBatch);
			

			if ((!strcmp((*itor)->TradingDay, pOrder->TradingDay))
				&& (!strcmp((*itor)->InstrumentID, pOrder->InstrumentID)) 
				&& ((*itor)->CombHedgeFlag[0] == pOrder->CombHedgeFlag[0])
				&& ((*itor)->Direction != pOrder->Direction)) { // 日期,合约代码,投保标志相同

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

			USER_PRINT((*itor)->TradingDay);
			USER_PRINT(pOrder->TradingDay);

			USER_PRINT((*itor)->InstrumentID);
			USER_PRINT(pOrder->InstrumentID);

			USER_PRINT((*itor)->CombHedgeFlag[0]);
			USER_PRINT(pOrder->CombHedgeFlag[0]);

			USER_PRINT((*itor)->VolumeTradedBatch);
			USER_PRINT(pOrder->VolumeTradedBatch);

			// 日期不相等
			if ((strcmp((*itor)->TradingDay, pOrder->TradingDay)) 
				&& (!strcmp((*itor)->InstrumentID, pOrder->InstrumentID))
				&& ((*itor)->CombHedgeFlag[0] == pOrder->CombHedgeFlag[0])
				&& ((*itor)->Direction != pOrder->Direction)) { // 日期,合约代码,投保标志相同

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
	else if (new_Order->OrderStatus == '2') ///部分成交不在队列中
	{
		//std::cout << "\tnew_Order->OrderStatus == '2' 部分成交不在队列中" << std::endl;
		this->getStgUser()->getXtsLogger()->info("\tnew_Order->OrderStatus == '2' 部分成交不在队列中");
		new_Order->VolumeTradedBatch = 0;
	}
	else if (new_Order->OrderStatus == '3') ///未成交还在队列中
	{
		//std::cout << "\tnew_Order->OrderStatus == '3' 未成交还在队列中" << std::endl;
		this->getStgUser()->getXtsLogger()->info("\tnew_Order->OrderStatus == '3' 未成交还在队列中");
		new_Order->VolumeTradedBatch = 0;
	}
	else if (new_Order->OrderStatus == '4') ///未成交不在队列中
	{
		//std::cout << "\tnew_Order->OrderStatus == '4' 未成交不在队列中" << std::endl;
		this->getStgUser()->getXtsLogger()->info("\tnew_Order->OrderStatus == '4' 未成交不在队列中");
		new_Order->VolumeTradedBatch = 0;
	}
	else if (new_Order->OrderStatus == '5') ///撤单
	{
		//std::cout << "\tnew_Order->OrderStatus == '5' 撤单" << std::endl;
		this->getStgUser()->getXtsLogger()->info("\tnew_Order->OrderStatus == '5' 撤单");
		new_Order->VolumeTradedBatch = 0;
	}
	else if (new_Order->OrderStatus == 'a') ///未知
	{
		//std::cout << "\tnew_Order->OrderStatus == 'a' 未知" << std::endl;
		this->getStgUser()->getXtsLogger()->info("\tnew_Order->OrderStatus == 'a' 未知");
		new_Order->VolumeTradedBatch = 0;
	}
	else if (new_Order->OrderStatus == 'b') ///尚未触发
	{
		//std::cout << "\tnew_Order->OrderStatus == 'b' 尚未触发" << std::endl;
		this->getStgUser()->getXtsLogger()->info("\tnew_Order->OrderStatus == 'b' 尚未触发");
		new_Order->VolumeTradedBatch = 0;
	}
	else if (new_Order->OrderStatus == 'c') ///已触发
	{
		//std::cout << "\tnew_Order->OrderStatus == 'c' 已触发" << std::endl;
		this->getStgUser()->getXtsLogger()->info("\tnew_Order->OrderStatus == 'c' 已触发");
		new_Order->VolumeTradedBatch = 0;
	}
	else
	{
		//std::cout << "\tnew_Order->OrderStatus不在系统范围内!" << std::endl;
		this->getStgUser()->getXtsLogger()->info("\tnew_Order->OrderStatus不在系统范围内!");
		new_Order->VolumeTradedBatch = 0;
	}
	/*std::cout << "Strategy::add_VolumeTradedBatch()" << std::endl;
	std::cout << "\t期货账户:" << this->stg_user_id << std::endl;
	std::cout << "\t策略编号:" << this->stg_strategy_id << std::endl;
	std::cout << "\t合约 = " << new_Order->InstrumentID << ", 买卖 = " << new_Order->Direction << ", 开平 = " << new_Order->CombOffsetFlag[0] << ", 本次成交量 = " << new_Order->VolumeTradedBatch << ", 报单引用 = " << new_Order->OrderRef << std::endl;
	*/
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
	return this->stg_trade_tasking;
}

void Strategy::setStgTradeTasking(bool stgTradeTasking) {
	//std::cout << "Strategy::setStgTradeTasking()" << std::endl;
	this->getStgUser()->getXtsLogger()->info("Strategy::setStgTradeTasking()");
	/*if (((this->getOn_Off()) &&
		(this->stg_user->getOn_Off()) &&
		(this->stg_user->GetTrader()->getOn_Off()) &&
		(this->stg_user->getCTP_Manager()->getOn_Off())))
		{
		this->stg_trade_tasking = stgTradeTasking;
		}
		else {
		std::cout << "\t请检查开关再进行trade_tasking设置!" << std::endl;
		}*/

	this->stg_trade_tasking = stgTradeTasking;
	/*std::cout << "\t期货账号 = " << this->getStgUserId() << std::endl;
	std::cout << "\t策略编号 = " << this->getStgStrategyId() << std::endl;
	std::cout << "\t选择下单算法锁 = " << this->stg_select_order_algorithm_flag << std::endl;
	std::cout << "\t策略执行标志位 = " << this->stg_trade_tasking << std::endl;*/
	this->getStgUser()->getXtsLogger()->info("\t期货账号 = {}", this->getStgUserId());
	this->getStgUser()->getXtsLogger()->info("\t策略编号 = {}", this->getStgStrategyId());
	this->getStgUser()->getXtsLogger()->info("\t选择下单算法锁 = {}", this->stg_select_order_algorithm_flag);
	this->getStgUser()->getXtsLogger()->info("\t交易执行锁 = {}", this->stg_trade_tasking);
}

void Strategy::setStgTradeTaskingRecovery() {
	//std::cout << "Strategy::setStgTradeTaskingRecovery()" << std::endl;
	this->getStgUser()->getXtsLogger()->info("Strategy::setStgTradeTaskingRecovery()");
	this->stg_trade_tasking = false;
	/*std::cout << "\t期货账号 = " << this->getStgUserId() << std::endl;
	std::cout << "\t策略编号 = " << this->getStgStrategyId() << std::endl;
	std::cout << "\tstg_select_order_algorithm_flag = " << this->stg_select_order_algorithm_flag << std::endl;
	std::cout << "\tstg_trade_tasking = " << this->stg_trade_tasking << std::endl;*/
	this->getStgUser()->getXtsLogger()->info("\t期货账号 = {}", this->getStgUserId());
	this->getStgUser()->getXtsLogger()->info("\t策略编号 = {}", this->getStgStrategyId());
	this->getStgUser()->getXtsLogger()->info("\t选择下单算法锁 = {}", this->stg_select_order_algorithm_flag);
	this->getStgUser()->getXtsLogger()->info("\t交易执行锁 = {}", this->stg_trade_tasking);
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
	this->stg_select_order_algorithm_flag = stg_select_order_algorithm_flag;
	this->getStgUser()->getXtsLogger()->info("Strategy::setStgSelectOrderAlgorithmFlag():");
	this->getStgUser()->getXtsLogger()->info("\t期货账号 = {}", this->getStgUserId());
	this->getStgUser()->getXtsLogger()->info("\t策略编号 = {}", this->getStgStrategyId());
	this->getStgUser()->getXtsLogger()->info("\t调用者 = {}", msg);
	this->getStgUser()->getXtsLogger()->info("\t选择下单算法锁 = {}", this->stg_select_order_algorithm_flag);
	this->getStgUser()->getXtsLogger()->info("\t交易执行锁 = {}", this->stg_trade_tasking);

}

bool Strategy::getStgSelectOrderAlgorithmFlag() {
	this->getStgUser()->getXtsLogger()->info("Strategy::getStgSelectOrderAlgorithmFlag():");
	this->getStgUser()->getXtsLogger()->info("\t期货账号 = {}", this->getStgUserId());
	this->getStgUser()->getXtsLogger()->info("\t策略编号 = {}", this->getStgStrategyId());
	this->getStgUser()->getXtsLogger()->info("\t选择下单算法锁 = {}", this->stg_select_order_algorithm_flag);
	this->getStgUser()->getXtsLogger()->info("\t交易执行锁 = {}", this->stg_trade_tasking);

	return this->stg_select_order_algorithm_flag;
}

void Strategy::setStgLockOrderRef(string stg_lock_order_ref) {
	this->stg_lock_order_ref = stg_lock_order_ref;
}
string Strategy::getStgLockOrderRef() {
	return this->stg_lock_order_ref;
}

void Strategy::setStgSaveStrategyConn(mongo::DBClientConnection *stg_save_strategy_conn) {
	this->stg_save_strategy_conn = stg_save_strategy_conn;
}
mongo::DBClientConnection * Strategy::getStgSaveStrategyConn() {
	return this->stg_save_strategy_conn;
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
	//如果已经收盘,不再接收
	/*if (this->getStgUser()->getCTP_Manager()->getIsMarketCloseDone()) {
		return;
	}*/
	/// 如果合约在撤单维护列表里，那么撤单次数增加1
	this->stg_user->add_instrument_id_action_counter(pOrder);
	this->Exec_OnRtnOrder(pOrder);
}

//成交通知
void Strategy::OnRtnTrade(CThostFtdcTradeField *pTrade) {
	USER_PRINT("Strategy::OnRtnTrade");
	////如果已经收盘,不再接收
	//if (this->getStgUser()->getCTP_Manager()->getIsMarketCloseDone()) {
	//	return;
	//}
	this->ExEc_OnRtnTrade(pTrade);
}

//下单错误响应
void Strategy::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder) {
	USER_PRINT("Strategy::OnErrRtnOrderInsert");
	////如果已经收盘,不再接收
	//if (this->getStgUser()->getCTP_Manager()->getIsMarketCloseDone()) {
	//	return;
	//}
	this->Exec_OnErrRtnOrderInsert();
}

///报单录入请求响应
void Strategy::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder) {
	USER_PRINT("Strategy::OnRspOrderInsert()");
	////如果已经收盘,不再接收
	//if (this->getStgUser()->getCTP_Manager()->getIsMarketCloseDone()) {
	//	return;
	//}
	//std::cout << "Strategy::OnRspOrderInsert()" << std::endl;
	this->getStgUser()->getXtsLogger()->info("Strategy::OnRspOrderInsert()");
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
	this->getStgUser()->getXtsLogger()->info("Strategy::OnErrRtnOrderAction()");
	this->Exec_OnErrRtnOrderAction();
}


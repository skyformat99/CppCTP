#include <algorithm>
#include "Strategy.h"

Strategy::Strategy() {
	this->l_instruments = new list<string>();
	this->stg_list_order_pending = new list<CThostFtdcOrderField *>();
}

void Strategy::get_tick(CThostFtdcDepthMarketDataField *pDepthMarketData) {
	//USER_PRINT(this->getStgInstrumentIdA());
	//USER_PRINT(this->getStgInstrumentIdB());
	// Get Own Instrument
	if (!strcmp(pDepthMarketData->InstrumentID, this->getStgInstrumentIdA().c_str())) {
		this->stg_instrument_A_tick = pDepthMarketData;
	}
	else if (!strcmp(pDepthMarketData->InstrumentID, this->getStgInstrumentIdB().c_str()))
	{
		this->stg_instrument_B_tick = pDepthMarketData;
	}
	
	/// 如果正在交易,继续更新tick进行交易
	if (this->stg_trade_tasking) {
		
	}
	else { /// 如果未在交易，那么选择下单算法
		this->Select_Order_Algorithm(this->getStgOrderAlgorithm());
	}

}

//选择下单算法
int Strategy::Select_Order_Algorithm(string stg_order_algorithm) {
	int select_num;
	//如果正在交易,直接返回0
	if (this->stg_trade_tasking) {
		return 0;
	}
	//如果有挂单,返回0
	if (this->stg_list_order_pending->size() > 0) {
		return 0;
	}

	if ((this->stg_position_a_sell == this->stg_position_b_buy) && (this->stg_position_a_buy == this->stg_position_b_sell)) {

	}
	else {
		return 0;
	}

	if (stg_order_algorithm == ALGORITHM_ONE) { //下单算法1
		select_num = 1;
		this->Order_Algorithm_One();
	}
	else if (stg_order_algorithm == ALGORITHM_TWO) { // 下单算法2
		select_num = 2;
		this->Order_Algorithm_Two();
	}
	else if (stg_order_algorithm == ALGORITHM_THREE) { // 下单算法3
		select_num = 3;
		this->Order_Algorithm_Three();
	}
	else {
		std::cout << "Select_Order_Algorithm has no algorithm for you!" << endl;
	}
	return select_num;
}

//下单算法1
void Strategy::Order_Algorithm_One() {
	// 计算盘口价差，量
	if (this->stg_instrument_A_tick && this->stg_instrument_B_tick)
	{
		this->stg_spread_long = this->stg_instrument_A_tick->BidPrice1 - 
								this->stg_instrument_B_tick->AskPrice1;
		this->stg_spread_long_volume = std::min(this->stg_instrument_A_tick->BidVolume1,
			this->stg_instrument_B_tick->AskVolume1);

		this->stg_spread_short = this->stg_instrument_A_tick->AskPrice1 -
			this->stg_instrument_B_tick->BidPrice1;
		this->stg_spread_short_volume = std::min(this->stg_instrument_A_tick->AskVolume1,
			this->stg_instrument_B_tick->BidVolume1);
	} 
	else
	{
		return;
	}

	/// 价差卖平
	if ((this->stg_spread_long >= this->stg_sell_close) && (this->stg_position_a_buy > 0) &&
		(this->stg_position_a_buy == this->stg_position_b_sell) &&
		((this->stg_position_a_buy + this->stg_position_b_buy) < this->stg_lots)) {

	}



}
//下单算法2
void Strategy::Order_Algorithm_Two() {

}
//下单算法3
void Strategy::Order_Algorithm_Three() {

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

bool Strategy::isStgOnlyClose() {
	return stg_only_close;
}

void Strategy::setStgOnlyClose(bool stgOnlyClose) {
	stg_only_close = stgOnlyClose;
}

int Strategy::getStgOrderActionTiresLimit() {
	return stg_order_action_tires_limit;
}

void Strategy::setStgOrderActionTiresLimit(int stgOrderActionTiresLimit) {
	stg_order_action_tires_limit = stgOrderActionTiresLimit;
}

string Strategy::getStgOrderAlgorithm() {
	return stg_order_algorithm;
}

void Strategy::setStgOrderAlgorithm(string stgOrderAlgorithm) {
	stg_order_algorithm = stgOrderAlgorithm;
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
}

//成交通知
void Strategy::OnRtnTrade(CThostFtdcTradeField *pTrade) {
	USER_PRINT("Strategy::OnRtnTrade");
}

//下单错误响应
void Strategy::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder) {
	USER_PRINT("Strategy::OnErrRtnOrderInsert");
}

///报单录入请求响应
void Strategy::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder) {
	USER_PRINT("Strategy::OnRspOrderInsert");
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


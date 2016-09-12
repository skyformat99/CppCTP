#ifndef QUANT_STRATEGY_H
#define QUANT_STRATEGY_H
#include <list>
#include "Trader.h"
#include "Algorithm.h"
#include "Debug.h"
#include "DBManager.h"

using std::list;

class DBManager;

class Strategy {

public:

	Strategy();

	void get_tick(CThostFtdcDepthMarketDataField *pDepthMarketData);

	void setTrader(Trader *trader);
	Trader *getTrader();

	void setUser(User *user);
	User *getUser();

	void setStrategyId(string strategyid);
	string getStrategyId();

	void setAlgorithm(Algorithm *alg);
	Algorithm *getAlgorithm();

	void addInstrumentToList(string instrument);
	list<string> *getListInstruments();

	void setTraderID(string traderid);
	string getTraderID();

	void setUserID(string userid);
	string getUserID();

	void setIsActive(string isActive);
	string getIsActive();

	


	CThostFtdcInputOrderField* getStgAOrderInsertArgs();
	void setStgAOrderInsertArgs(
		CThostFtdcInputOrderField* stgAOrderInsertArgs);

	double getStgAPriceTick();
	void setStgAPriceTick(double stgAPriceTick);

	double getStgAWaitPriceTick();
	void setStgAWaitPriceTick(double stgAWaitPriceTick);

	CThostFtdcInputOrderField* getStgBOrderInsertArgs();
	void setStgBOrderInsertArgs(
		CThostFtdcInputOrderField* stgBOrderInsertArgs);

	double getStgBPriceTick();
	void setStgBPriceTick(double stgBPriceTick);

	double getStgBWaitPriceTick();
	void setStgBWaitPriceTick(double stgBWaitPriceTick);

	double getStgBuyClose();
	void setStgBuyClose(double stgBuyClose);

	double getStgBuyOpen();
	void setStgBuyOpen(double stgBuyOpen);

	DBManager* getStgDbm();
	void setStgDbm(DBManager* stgDbm);

	CThostFtdcDepthMarketDataField* getStgInstrumentATick();
	void setStgInstrumentATick(
		CThostFtdcDepthMarketDataField* stgInstrumentATick);

	CThostFtdcDepthMarketDataField* getStgInstrumentBTick();
	void setStgInstrumentBTick(
		CThostFtdcDepthMarketDataField* stgInstrumentBTick);

	string getStgInstrumentIdA();
	void setStgInstrumentIdA(string stgInstrumentIdA);

	string getStgInstrumentIdB();
	void setStgInstrumentIdB(string stgInstrumentIdB);

	bool isStgIsActive();
	void setStgIsActive(bool stgIsActive);

	list< CThostFtdcOrderField *> *getStgListOrderPending();
	void setStgListOrderPending(list< CThostFtdcOrderField *> *stgListOrderPending);

	int getStgLots();
	void setStgLots(int stgLots);

	int getStgLotsBatch();
	void setStgLotsBatch(int stgLotsBatch);

	bool isStgOnlyClose();
	void setStgOnlyClose(bool stgOnlyClose);

	int getStgOrderActionTiresLimit();
	void setStgOrderActionTiresLimit(int stgOrderActionTiresLimit);

	int getStgOrderAlgorithm();
	void setStgOrderAlgorithm(int stgOrderAlgorithm);

	string getStgOrderRefA();
	void setStgOrderRefA(string stgOrderRefA);

	string getStgOrderRefB();
	void setStgOrderRefB(string stgOrderRefB);

	string getStgOrderRefLast();
	void setStgOrderRefLast(string stgOrderRefLast);

	int getStgPositionABuy();
	void setStgPositionABuy(int stgPositionABuy);

	int getStgPositionABuyToday();
	void setStgPositionABuyToday(int stgPositionABuyToday);

	int getStgPositionABuyYesterday();
	void setStgPositionABuyYesterday(int stgPositionABuyYesterday);

	int getStgPositionASell();
	void setStgPositionASell(int stgPositionASell);

	int getStgPositionASellToday();
	void setStgPositionASellToday(int stgPositionASellToday);

	int getStgPositionASellYesterday();
	void setStgPositionASellYesterday(int stgPositionASellYesterday);

	int getStgPositionBBuy();
	void setStgPositionBBuy(int stgPositionBBuy);

	int getStgPositionBBuyToday();
	void setStgPositionBBuyToday(int stgPositionBBuyToday);

	int getStgPositionBBuyYesterday();
	void setStgPositionBBuyYesterday(int stgPositionBBuyYesterday);

	int getStgPositionBSell();
	void setStgPositionBSell(int stgPositionBSell);

	int getStgPositionBSellToday();
	void setStgPositionBSellToday(int stgPositionBSellToday);

	int getStgPositionBSellYesterday();
	void setStgPositionBSellYesterday(int stgPositionBSellYesterday);

	double getStgSellClose();
	void setStgSellClose(double stgSellClose);

	double getStgSellOpen();
	void setStgSellOpen(double stgSellOpen);

	double getStgSpread();
	void setStgSpread(double stgSpread);

	double getStgSpreadLong();
	void setStgSpreadLong(double stgSpreadLong);

	int getStgSpreadLongVolume();
	void setStgSpreadLongVolume(int stgSpreadLongVolume);

	double getStgSpreadShift();
	void setStgSpreadShift(double stgSpreadShift);

	double getStgSpreadShort();
	void setStgSpreadShort(double stgSpreadShort);

	int getStgSpreadShortVolume();
	void setStgSpreadShortVolume(int stgSpreadShortVolume);

	double getStgStopLoss();
	void setStgStopLoss(double stgStopLoss);

	string getStgStrategyId();
	void setStgStrategyId(string stgStrategyId);

	bool isStgTradeTasking();
	void setStgTradeTasking(bool stgTradeTasking);

	string getStgTraderId();
	void setStgTraderId(string stgTraderId);

	User* getStgUser();
	void setStgUser(User* stgUser);

	string getStgUserId();
	void setStgUserId(string stgUserId);


private:
	Trader *trader;
	User *user;
	string strategyid;
	string userid;
	string traderid;
	string isActive;
	Algorithm *alg;
	list<string> *l_instruments;

	DBManager *stg_DBM;// 数据库连接实例
	User *stg_user;// user实例
	string stg_trader_id; // 交易员账户id
	string stg_user_id; // user_id
	string stg_strategy_id; // 策略id
	int stg_order_algorithm;// 下单算法选择标志位
	string stg_instrument_id_A;// 合约A
	string stg_instrument_id_B;// 合约B

	double stg_buy_open;// 触发买开（开多单）
	double stg_sell_close;// 触发卖平（平多单）
	double stg_sell_open;// 触发卖开（开空单）
	double stg_buy_close;// 触发买平（平空单）
	double stg_spread_shift;// 价差让价
	double stg_a_wait_price_tick;// A合约挂单等待最小跳数
	double stg_b_wait_price_tick;// B合约挂单等待最小跳数
	double stg_stop_loss;// 止损，单位为最小跳数
	int stg_lots;// 总手
	int stg_lots_batch;// 每批下单手数
	bool stg_is_active;// 策略开关状态
	int stg_order_action_tires_limit;// 撤单次数限制
	bool stg_only_close;// 只能平仓

	int stg_position_a_buy_today;// A合约买持仓今仓
	int stg_position_a_buy_yesterday;// A合约买持仓昨仓
	int stg_position_a_buy;// A合约买持仓总仓位
	int stg_position_a_sell_today;// A合约卖持仓今仓
	int stg_position_a_sell_yesterday;// A合约卖持仓昨仓
	int stg_position_a_sell;// A合约卖持仓总仓位
	int stg_position_b_buy_today;// B合约买持仓今仓
	int stg_position_b_buy_yesterday;// B合约买持仓昨仓
	int stg_position_b_buy;// B合约买持仓总仓位
	int stg_position_b_sell_today;// B合约卖持仓今仓
	int stg_position_b_sell_yesterday;// B合约卖持仓昨仓
	int stg_position_b_sell;// B合约卖持仓总仓位

	CThostFtdcDepthMarketDataField *stg_instrument_A_tick;// A合约tick（第一腿）
	CThostFtdcDepthMarketDataField *stg_instrument_B_tick;// B合约tick（第二腿）
	double stg_spread_long;// 市场多头价差：A合约买一价 - B合约买一价
	int stg_spread_long_volume;// 市场多头价差盘口挂单量min(A合约买一量 - B合约买一量)
	double stg_spread_short;// 市场空头价差：A合约卖一价 - B合约卖一价
	int stg_spread_short_volume;// 市场空头价差盘口挂单量：min(A合约买一量 - B合约买一量)
	double stg_spread;// 市场最新价价差

	string stg_order_ref_last;// 最后一次实际使用的报单引用
	string stg_order_ref_a;// A合约报单引用
	string stg_order_ref_b;// B合约报单引用
	double stg_a_price_tick;// A合约最小跳价
	double stg_b_price_tick;// B合约最小跳价

	bool stg_trade_tasking;// 交易任务进行中
	CThostFtdcInputOrderField *stg_a_order_insert_args;// a合约报单参数
	CThostFtdcInputOrderField *stg_b_order_insert_args;// b合约报单参数
	list<CThostFtdcOrderField *> *stg_list_order_pending;// 挂单列表，报单、成交、撤单回报

};

#endif
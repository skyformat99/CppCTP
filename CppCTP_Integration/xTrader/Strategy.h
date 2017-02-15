#ifndef QUANT_STRATEGY_H
#define QUANT_STRATEGY_H
#include <list>
#include "Trader.h"
#include "Algorithm.h"
#include "Debug.h"
#include "DBManager.h"
#include "xTradeStruct.h"
#include "PositionDetail.h"

using std::list;
class DBManager;

#define ALGORITHM_ONE	"01"
#define ALGORITHM_TWO	"02"
#define ALGORITHM_THREE "03"
#define ALGORITHM_FOUR	"04"

class Strategy {

public:

	Strategy(User *stg_user=NULL);

	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

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

	CThostFtdcDepthMarketDataField* getStgInstrumentATickLast();
	void setStgInstrumentATickLast(
		CThostFtdcDepthMarketDataField* stgInstrumentATick);

	CThostFtdcDepthMarketDataField* getStgInstrumentBTickLast();
	void setStgInstrumentBTickLast(
		CThostFtdcDepthMarketDataField* stgInstrumentBTick);

	string getStgInstrumentIdA();
	void setStgInstrumentIdA(string stgInstrumentIdA);

	string getStgInstrumentIdB();
	void setStgInstrumentIdB(string stgInstrumentIdB);

	bool isStgIsActive();
	void setStgIsActive(bool stgIsActive);

	list<CThostFtdcOrderField *> *getStgListOrderPending();
	void setStgListOrderPending(list<CThostFtdcOrderField *> *stgListOrderPending);

	int getStgLots();
	void setStgLots(int stgLots);

	int getStgLotsBatch();
	void setStgLotsBatch(int stgLotsBatch);

	int isStgOnlyClose();
	void setStgOnlyClose(int stgOnlyClose);

	int getStgAOrderActionTiresLimit();
	void setStgAOrderActionTiresLimit(int stgOrderActionTiresLimit);

	int getStgBOrderActionTiresLimit();
	void setStgBOrderActionTiresLimit(int stgOrderActionTiresLimit);

	int getStgAOrderActionCount();
	void setStgAOrderActionCount(int stg_a_order_action_count);

	int getStgBOrderActionCount();
	void setStgBOrderActionCount(int stg_b_order_action_count);

	int getStgALimitPriceShift(); // A合约报单偏移
	void setStgALimitPriceShift(int stg_a_limit_price_shift);
	
	int getStgBLimitPriceShift(); // B合约报单偏移
	void setStgBLimitPriceShift(int stg_b_limit_price_shift);


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

	
	int getStgPendingAOpen();
	void setStgPendingAOpen(int stg_pending_a_open);

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

	void setStgOrderRefBase(long long stg_order_ref_base);
	long long getStgOrderRefBase();

	void setStgTradingDay(string stg_trading_day);
	string getStgTradingDay();

	/************************************************************************/
	/* 交易相关的回报函数                                                      */
	/************************************************************************/
	//下单
	void OrderInsert(User *user, char *InstrumentID, char CombOffsetFlag, char Direction, int Volume, double Price, string OrderRef);

	//下单响应
	void OnRtnOrder(CThostFtdcOrderField *pOrder);

	//成交通知
	void OnRtnTrade(CThostFtdcTradeField *pTrade);

	//下单错误响应
	void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder);

	///报单录入请求响应
	void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder);

	//撤单
	void OrderAction(string ExchangeID, string OrderRef, string OrderSysID);

	//撤单错误响应
	void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction);

	//撤单错误
	void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction);

	//选择下单算法
	void Select_Order_Algorithm(string stg_order_algorithm);

	//下单算法1
	void Order_Algorithm_One();
	//下单算法2
	void Order_Algorithm_Two();
	//下单算法3
	void Order_Algorithm_Three();

	/// 生成报单引用
	string Generate_Order_Ref();

	/// 执行任务函数
	void Exec_OrderInsert(CThostFtdcInputOrderField *insert_order);			// 报单
	void Exec_OnRspOrderInsert();		// 报单录入请求
	void Exec_OnRspOrderAction();		// 报单操作请求响应
	void Exec_OnRtnOrder(CThostFtdcOrderField *pOrder);				// 报单回报
	void ExEc_OnRtnTrade(CThostFtdcTradeField *pTrade);				// 成交回报
	void Exec_OnErrRtnOrderInsert();	// 报单录入错误回报
	void Exec_OnErrRtnOrderAction();	// 报单操作错误回报
	void Exec_OnTickComing(CThostFtdcDepthMarketDataField *pDepthMarketData);			// 行情回调,执行交易任务

	/// 更新挂单list
	void update_pending_order_list(CThostFtdcOrderField *pOrder);

	/// 更新持仓量(Order)
	void update_position(CThostFtdcOrderField *pOrder);

	/// 更新持仓量(UserOrder)
	void update_position(USER_CThostFtdcOrderField *pOrder);
	
	/// 更新持仓量(Trade)
	void update_position(CThostFtdcTradeField *pTrade);

	/// 更新持仓明细(Trade)
	void update_position_detail(CThostFtdcTradeField *pTrade);

	/// 更新持仓明细(Order)
	void update_position_detail(USER_CThostFtdcOrderField *pOrder);

	/// 添加字段本次成交量至order构体中
	void add_VolumeTradedBatch(CThostFtdcOrderField *pOrder, USER_CThostFtdcOrderField *new_Order);


	/// 得到三个数最小值
	int getMinNum(int num1, int num2, int num3);

	/// 初始化
	void InitStrategy();

	/// 拷贝结构体CThostFtdcDepthMarketDataField
	void CopyTickData(CThostFtdcDepthMarketDataField *dst, CThostFtdcDepthMarketDataField *src);

	/// 拷贝结构体CThostFtdcOrderField
	void CopyOrderData(CThostFtdcOrderField *dst, CThostFtdcOrderField *src);

	void CopyOrderDataToNew(USER_CThostFtdcOrderField *dst, CThostFtdcOrderField *src);

	/// 拷贝结构体USER_CThostFtdcOrderField
	void CopyNewOrderData(USER_CThostFtdcOrderField *dst, USER_CThostFtdcOrderField *src);

	/// 拷贝结构体CThostFtdcTradeField
	void CopyTradeData(CThostFtdcTradeField *dst, CThostFtdcTradeField *src);

	

	/// 设置开关
	int getOn_Off();
	void setOn_Off(int on_off);

	void setStgSellOpenOnOff(int sell_open_on_off);				//卖开-开关
	int getStgSellOpenOnOff();

	void setStgBuyCloseOnOff(int buy_close_on_off);				//买平-开关
	int getStgBuyCloseOnOff();

	void setStgBuyOpenOnOff(int buy_open_on_off);				//买开-开关
	int getStgBuyOpenOnOff();

	void setStgSellCloseOnOff(int sell_close_on_off);			//卖平-开关
	int getStgSellCloseOnOff();

	/// 更新交易状态
	void update_task_status();

	/// 交易模型
	void setStgTradeModel(string stg_trade_model);

	string getStgTradeModel();

	string getStgOrderAlgorithm();			// 下单算法

	void setStgOrderAlgorithm(string stg_order_algorithm);

	double getStgHoldProfit();				// 持仓盈亏

	void setStgHoldProfit(double stg_hold_profit);

	double getStgCloseProfit();			// 平仓盈亏

	void setStgCloseProfit(double stg_close_profit);

	double getStgCommission();				// 手续费

	void setStgCommisstion(double stg_commission);

	int getStgPosition();					// 总持仓

	void setStgPosition(int stg_position);

	int getStgPositionBuy();				// 买持仓

	void setStgPositionBuy(int stg_position_buy);

	int getStgPositionSell();				// 卖持仓
	
	int setStgPositionSell(int stg_position_sell);

	int getStgTradeVolume();				// 成交量

	int setStgTradeVolume(int stg_trade_volume);

	double getStgAmount();					// 成交金额

	void setStgAmount(double stg_amount);

	double getStgAverageShift();			// 平均滑点

	void setStgAverageShift(double stg_average_shift);

	void setInit_Finished(bool init_finished);

	bool getInit_Finished();

	void init_today_position();

	void setL_query_trade(list<CThostFtdcTradeField *> *l_query_trade);
	void addOrderToListQueryOrder(CThostFtdcOrderField *order);
	void setL_query_order(list<CThostFtdcOrderField *> *l_query_order);

	void add_position_detail(PositionDetail *posd);

	void CopyPositionData(PositionDetail *posd, USER_CThostFtdcOrderField *order);

	list<USER_CThostFtdcOrderField *> * getStg_List_Position_Detail_From_Order(); // 持仓明细

	void printStrategyInfo(string message); //调试输出
	void printStrategyInfoPosition();

private:
	Trader *trader;
	User *user;
	string strategyid;
	string userid;
	string traderid;
	Algorithm *alg;
	list<string> *l_instruments;

	DBManager *stg_DBM;			// 数据库连接实例
	User *stg_user;				// user实例
	string stg_trader_id;		// 交易员账户id
	string stg_user_id;			// user_id
	string stg_strategy_id;		// 策略id
	string stg_instrument_id_A;	// 合约A
	string stg_instrument_id_B;	// 合约B

	double stg_buy_open;				// 触发买开（开多单）
	double stg_sell_close;				// 触发卖平（平多单）
	double stg_sell_open;				// 触发卖开（开空单）
	double stg_buy_close;				// 触发买平（平空单）
	double stg_spread_shift;			// 价差让价
	double stg_a_wait_price_tick;		// A合约挂单等待最小跳数
	double stg_b_wait_price_tick;		// B合约挂单等待最小跳数
	double stg_stop_loss;				// 止损，单位为最小跳数
	int stg_lots;						// 总手
	int stg_lots_batch;					// 每批下单手数
	bool stg_is_active;					// 策略开关状态
	int stg_a_order_action_tires_limit;	// A合约撤单次数限制
	int stg_b_order_action_tires_limit;	// B合约撤单次数限制
	int stg_a_order_action_count;		// A合约撤单次数
	int stg_b_order_action_count;		// B合约撤单次数
	
	/*新增字段*/
	string stg_trade_model;				// 交易模型
	string stg_order_algorithm;			// 下单算法
	double stg_hold_profit;				// 持仓盈亏
	double stg_close_profit;			// 平仓盈亏
	double stg_commission;				// 手续费
	int stg_position;					// 总持仓
	int stg_position_buy;				// 买持仓
	int stg_position_sell;				// 卖持仓
	int stg_trade_volume;				// 成交量
	double stg_amount;					// 成交金额
	double stg_average_shift;			// 平均滑点
	string stg_trading_day;				// 交易日

	int stg_position_a_buy_today;		// A合约买持仓今仓
	int stg_position_a_buy_yesterday;	// A合约买持仓昨仓
	int stg_position_a_buy;				// A合约买持仓总仓位
	int stg_position_a_sell_today;		// A合约卖持仓今仓
	int stg_position_a_sell_yesterday;	// A合约卖持仓昨仓
	int stg_position_a_sell;			// A合约卖持仓总仓位
	int stg_position_b_buy_today;		// B合约买持仓今仓
	int stg_position_b_buy_yesterday;	// B合约买持仓昨仓
	int stg_position_b_buy;				// B合约买持仓总仓位
	int stg_position_b_sell_today;		// B合约卖持仓今仓
	int stg_position_b_sell_yesterday;	// B合约卖持仓昨仓
	int stg_position_b_sell;			// B合约卖持仓总仓位
	int stg_pending_a_open;				// A合约挂单买开仓量



	CThostFtdcDepthMarketDataField *stg_instrument_A_tick;	// A合约tick（第一腿）
	CThostFtdcDepthMarketDataField *stg_instrument_B_tick;	// B合约tick（第二腿）
	CThostFtdcDepthMarketDataField *stg_instrument_A_tick_last;	// A合约tick（第一腿）交易前最后一次
	CThostFtdcDepthMarketDataField *stg_instrument_B_tick_last;	// B合约tick（第二腿）交易前最后一次
	double stg_spread_long;									// 市场多头价差：A合约买一价 - B合约买一价
	int stg_spread_long_volume;								// 市场多头价差盘口挂单量min(A合约买一量 - B合约买一量)
	double stg_spread_short;								// 市场空头价差：A合约卖一价 - B合约卖一价
	int stg_spread_short_volume;							// 市场空头价差盘口挂单量：min(A合约买一量 - B合约买一量)
	double stg_spread;										// 市场最新价价差

	string stg_order_ref_last;	// 最后一次实际使用的报单引用
	string stg_order_ref_a;		// A合约报单引用
	string stg_order_ref_b;		// B合约报单引用
	double stg_a_price_tick;	// A合约最小跳价
	double stg_b_price_tick;	// B合约最小跳价
	int stg_a_limit_price_shift; // A合约报单偏移
	int stg_b_limit_price_shift; // B合约报单偏移

	bool stg_trade_tasking;		// 交易任务进行中
	CThostFtdcInputOrderField *stg_a_order_insert_args;		// a合约报单参数
	CThostFtdcInputOrderField *stg_b_order_insert_args;		// b合约报单参数
	list<CThostFtdcOrderField *> *stg_list_order_pending;	// 挂单列表，报单、成交、撤单回报
	list<CThostFtdcTradeField *> *stg_list_position_detail; // 持仓明细
	list<USER_CThostFtdcOrderField *> *stg_list_position_detail_from_order; // 持仓明细
	list<CThostFtdcTradeField *> *l_query_trade;
	list<USER_CThostFtdcOrderField *> *l_query_order;

	long long stg_order_ref_base; // 报单引用计数

	int on_off;							//开关
	int stg_only_close;					//只能平仓
	int sell_open_on_off;				//卖开-开关
	int buy_close_on_off;				//买平-开关
	int buy_open_on_off;				//买开-开关
	int sell_close_on_off;				//卖平-开关
	
	bool init_finished;

};

#endif
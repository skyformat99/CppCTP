#ifndef QUANT_STRATEGY_H
#define QUANT_STRATEGY_H
#include <list>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include "Trader.h"
#include "Algorithm.h"
#include "Debug.h"
#include "DBManager.h"
#include "xTradeStruct.h"
#include "INIReader.h"
//#include "concurrentqueue/blockingconcurrentqueue.h"
#include "safequeue.h"
#include <spdlog/spdlog.h>
using namespace spdlog;
using spdlog::logger;
//using namespace moodycamel;
using std::list;
class DBManager;

#define ALGORITHM_ONE	"01"
#define ALGORITHM_TWO	"02"
#define ALGORITHM_THREE "03"
#define ALGORITHM_FOUR	"04"

class Strategy {

public:

	Strategy(bool fake = true, User *stg_user=NULL);

	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	bool CompareTickData(CThostFtdcDepthMarketDataField *last_tick_data, CThostFtdcDepthMarketDataField *pDepthMarketData);

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
			
	int getStgInstrumentAScale();		// A合约比例乘数
	void setStgInstrumentAScale(int stg_instrument_A_scale);

	int getStgInstrumentBScale();		// B合约比例乘数
	void setStgInstrumentBScale(int stg_instrument_B_scale);

	bool isStgTradeTasking();
	void setStgTradeTasking(bool stgTradeTasking);
	void setStgTradeTaskingRecovery();

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

	void setStgSelectOrderAlgorithmFlag(string msg, bool stg_select_order_algorithm_flag);
	bool getStgSelectOrderAlgorithmFlag();

	void setStgLockOrderRef(string stg_lock_order_ref);
	string getStgLockOrderRef();


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
	void Generate_Order_Ref(CThostFtdcInputOrderField *insert_order);

	/// 执行任务函数
	void Exec_OrderInsert(CThostFtdcInputOrderField *insert_order);			// 报单
	void Exec_OnRspOrderInsert();		// 报单录入请求
	void Exec_OnRspOrderAction();		// 报单操作请求响应
	void Exec_OnRtnOrder(CThostFtdcOrderField *pOrder);				// 报单回报
	void ExEc_OnRtnTrade(CThostFtdcTradeField *pTrade);				// 成交回报
	void Exec_OnErrRtnOrderInsert();	// 报单录入错误回报
	void Exec_OnErrRtnOrderAction();	// 报单操作错误回报
	void Exec_OnTickComing(THREAD_CThostFtdcDepthMarketDataField *pDepthMarketData);			// 行情回调,执行交易任务

	/// 更新挂单list
	void update_pending_order_list(CThostFtdcOrderField *pOrder);

	/// 更新持仓量(Order)
	//void update_position(CThostFtdcOrderField *pOrder);

	/// 更新持仓量(UserOrder)
	void update_position(USER_CThostFtdcOrderField *pOrder);
	
	/// 更新持仓量(Trade)
	//void update_position(CThostFtdcTradeField *pTrade);

	/// 更新持仓明细(Trade)
	//void update_position_detail(CThostFtdcTradeField *pTrade);

	/// 更新自定义持仓明细结构(Order)
	void update_position_detail(USER_CThostFtdcOrderField *pOrder);

	/// 更新自定义持仓明细结构(Trade)
	void update_position_detail(USER_CThostFtdcTradeField *pTrade);

	/// 更新交易状态
	void update_task_status();

	/// 更新tick锁
	void update_tick_lock_status(USER_CThostFtdcOrderField *order);

	/// 添加字段本次成交量至order构体中
	void add_VolumeTradedBatch(THREAD_CThostFtdcOrderField *pOrder, USER_CThostFtdcOrderField *new_Order);

	/// 得到三个数最小值
	int getMinNum(int num1, int num2, int num3);

	/// 初始化
	void InitStrategy();

	/// 拷贝结构体CThostFtdcDepthMarketDataField
	void CopyTickData(CThostFtdcDepthMarketDataField *dst, CThostFtdcDepthMarketDataField *src);

	

	/// 拷贝结构体THREAD_CThostFtdcDepthMarketDataField
	void CopyThreadTickData(THREAD_CThostFtdcDepthMarketDataField *dst, CThostFtdcDepthMarketDataField *src, bool isLastElement = false);


	/// 拷贝结构体CThostFtdcDepthMarketDataField
	void CopyTickData(CThostFtdcDepthMarketDataField *dst, THREAD_CThostFtdcDepthMarketDataField *src);

	/// 拷贝结构体CThostFtdcOrderField
	void CopyOrderData(CThostFtdcOrderField *dst, CThostFtdcOrderField *src);

	void CopyThreadOrderData(THREAD_CThostFtdcOrderField *dst, CThostFtdcOrderField *src, bool isLastElement = false);

	void CopyOrderData(CThostFtdcOrderField *dst, THREAD_CThostFtdcOrderField *src);

	void CopyOrderDataToNew(USER_CThostFtdcOrderField *dst, CThostFtdcOrderField *src);

	void CopyThreadOrderDataToNew(USER_CThostFtdcOrderField *dst, THREAD_CThostFtdcOrderField *src);

	/// 拷贝结构体USER_CThostFtdcOrderField
	void CopyNewOrderData(USER_CThostFtdcOrderField *dst, USER_CThostFtdcOrderField *src);


	/// 拷贝结构体CThostFtdcTradeField
	void CopyTradeData(CThostFtdcTradeField *dst, CThostFtdcTradeField *src);

	void CopyThreadTradeData(THREAD_CThostFtdcTradeField *dst, CThostFtdcTradeField *src, bool isLastElement = false);

	void CopyTradeData(CThostFtdcTradeField *dst, THREAD_CThostFtdcTradeField *src);

	/// 拷贝结构体CThostFtdcTradeField
	void CopyTradeDataToNew(USER_CThostFtdcTradeField *dst, CThostFtdcTradeField *src);

	void CopyThreadTradeDataToNew(USER_CThostFtdcTradeField *dst, THREAD_CThostFtdcTradeField *src);

	void CopyNewTradeData(USER_CThostFtdcTradeField *dst, USER_CThostFtdcTradeField *src);

	/// 收盘保存数据
	void DropPositionDetail();

	/// 根据持仓明细进行统计持仓量(order)
	void calPosition();

	/// 更新策略
	void UpdateStrategy(Strategy *stg);

	/// 创建持仓明细
	void CreatePositionDetail(USER_CThostFtdcOrderField *posd);
	
	/// 数据库更新策略持仓明细
	void Update_Position_Detail_To_DB(USER_CThostFtdcOrderField *posd);

	/// 数据库更新策略持仓明细(order changed)
	void Update_Position_Changed_Detail_To_DB(USER_CThostFtdcOrderField *posd);

	/// 创建持仓明细
	void CreatePositionTradeDetail(USER_CThostFtdcTradeField *posd);

	/// 数据库更新策略持仓明细
	void Update_Position_Trade_Detail_To_DB(USER_CThostFtdcTradeField *posd);

	/// 数据库更新策略持仓明细(trade changed)
	void Update_Position_Trade_Changed_Detail_To_DB(USER_CThostFtdcTradeField *posd);

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
	void add_position_detail(USER_CThostFtdcOrderField *posd);


	//void CopyPositionData(PositionDetail *posd, USER_CThostFtdcOrderField *order);

	list<USER_CThostFtdcOrderField *> * getStg_List_Position_Detail_From_Order(); // 持仓明细(order)
	list<USER_CThostFtdcTradeField *> * getStg_List_Position_Detail_From_Trade(); // 持仓明细(trade)

	void printStrategyInfo(string message); //调试输出
	void printStrategyInfoPosition(); //输出当前持仓明细

	void setStgUpdatePositionDetailRecordTime(string stg_update_position_detail_record_time);
	string getStgUpdatePositionDetailRecordTime();

	//仓位是否正确,是否需要调整仓位
	void setStgIsPositionRight(bool is_position_right);
	bool getStgIsPositionRight();

	//清空持仓明细
	void clearStgPositionDetail();

	//构造模拟平仓order
	void createFakeOrderPositionDetail(USER_CThostFtdcOrderField *order, string date, string instrumentID, char CombHedgeFlag, char Direction, char CombOffsetFlag, int VolumeTradedBatch);

	//构造模拟平仓trade
	void createFakeTradePositionDetail(USER_CThostFtdcTradeField *trade, string date, string instrumentID, char HedgeFlag, char Direction, char OffsetFlag, int Volume);

	//收盘前5秒处理挂单列表
	void finish_pending_order_list();

	//策略最后一次保存的时间
	void setStgLastSavedTime(string stg_last_saved_time);
	string getStgLastSavedTime();

	//是否允许收盘任务执行
	bool getStgOnOffEndTask();
	void setStgOnOffEndTask(bool on_off_end_task);

	// 设置系统xts_logger
	void setXtsLogger(std::shared_ptr<spdlog::logger> ptr);
	std::shared_ptr<spdlog::logger> getXtsLogger();

	// 行情队列
	void thread_queue_OnRtnDepthMarketData();
	// order回调队列
	void thread_queue_OnRtnOrder();
	// trade回调队列
	void thread_queue_OnRtnTrade();
	// 停止线程
	void end_thread();
	// 获取停止线程状态
	bool getEndThreadStatus();

	// 行情队列开关
	void setQueue_OnRtnDepthMarketData_on_off(bool queue_OnRtnDepthMarketData);
	bool getQueue_OnRtnDepthMarketData_on_off();
	// order回调队列开关
	void setQueue_OnRtnOrder_on_off(bool queue_OnRtnOrder_on_off);
	bool getQueue_OnRtnOrder_on_off();
	// trade回调队列开关
	void setQueue_OnRtnTrade_on_off(bool queue_OnRtnTrade_on_off);
	bool getQueue_OnRtnTrade_on_off();

	string getMorning_opentime();

	void setMorning_opentime(string morning_opentime);

	string getMorning_begin_breaktime();

	void setMorning_begin_breaktime(string morning_begin_breaktime);

	string getMorning_breaktime();

	void setMorning_breaktime(string morning_breaktime);

	string getMorning_recoverytime();

	void setMorning_recoverytime(string morning_recoverytime);

	string getMorning_begin_closetime();

	void setMorning_begin_closetime(string morning_begin_closetime);

	string getMorning_closetime();

	void setMorning_closetime(string morning_closetime);

	string getAfternoon_opentime();

	void setAfternoon_opentime(string afternoon_opentime);

	string getAfternoon_begin_closetime();

	void setAfternoon_begin_closetime(string afternoon_begin_closetime);

	string getAfternoon_closetime();

	void setAfternoon_closetime(string afternoon_closetime);

	string getEvening_opentime();

	void setEvening_opentime(string evening_opentime);

	string getEvening_closetime();

	void setEvening_closetime(string evening_closetime);

	string getMorning_opentime_instrument_A();

	void setMorning_opentime_instrument_A(string morning_opentime_instrument_A);

	string getMorning_begin_breaktime_instrument_A();

	void setMorning_begin_breaktime_instrument_A(string morning_begin_breaktime_instrument_A);

	string getMorning_breaktime_instrument_A();

	void setMorning_breaktime_instrument_A(string morning_breaktime_instrument_A);

	string getMorning_recoverytime_instrument_A();

	void setMorning_recoverytime_instrument_A(string morning_recoverytime_instrument_A);

	string getMorning_begin_closetime_instrument_A();

	void setMorning_begin_closetime_instrument_A(string morning_begin_closetime_instrument_A);

	string getMorning_closetime_instrument_A();

	void setMorning_closetime_instrument_A(string morning_closetime_instrument_A);

	string getAfternoon_opentime_instrument_A();

	void setAfternoon_opentime_instrument_A(string afternoon_opentime_instrument_A);

	string getAfternoon_begin_closetime_instrument_A();

	void setAfternoon_begin_closetime_instrument_A(string afternoon_begin_closetime_instrument_A);

	string getAfternoon_closetime_instrument_A();

	void setAfternoon_closetime_instrument_A(string afternoon_closetime_instrument_A);

	string getEvening_opentime_instrument_A();

	void setEvening_opentime_instrument_A(string evening_opentime_instrument_A);

	string getEvening_closetime_instrument_A();

	void setEvening_closetime_instrument_A(string evening_closetime_instrument_A);

	string getMorning_opentime_instrument_B();

	void setMorning_opentime_instrument_B(string morning_opentime_instrument_B);

	string getMorning_begin_breaktime_instrument_B();

	void setMorning_begin_breaktime_instrument_B(string morning_begin_breaktime_instrument_B);

	string getMorning_breaktime_instrument_B();

	void setMorning_breaktime_instrument_B(string morning_breaktime_instrument_B);

	string getMorning_recoverytime_instrument_B();

	void setMorning_recoverytime_instrument_B(string morning_recoverytime_instrument_B);

	string getMorning_begin_closetime_instrument_B();

	void setMorning_begin_closetime_instrument_B(string morning_begin_closetime_instrument_B);

	string getMorning_closetime_instrument_B();

	void setMorning_closetime_instrument_B(string morning_closetime_instrument_B);

	string getAfternoon_opentime_instrument_B();

	void setAfternoon_opentime_instrument_B(string afternoon_opentime_instrument_B);

	string getAfternoon_begin_closetime_instrument_B();

	void setAfternoon_begin_closetime_instrument_B(string afternoon_begin_closetime_instrument_B);

	string getAfternoon_closetime_instrument_B();

	void setAfternoon_closetime_instrument_B(string afternoon_closetime_instrument_B);

	string getEvening_opentime_instrument_B();

	void setEvening_opentime_instrument_B(string evening_opentime_instrument_B);

	string getEvening_closetime_instrument_B();

	void setEvening_closetime_instrument_B(string evening_closetime_instrument_B);

	string getEvening_stop_opentime();

	void setEvening_stop_opentime(string evening_stop_opentime);

	string getEvening_first_end_tasktime();

	void setEvening_first_end_tasktime(string evening_first_end_tasktime);

	string getEvening_second_end_tasktime();

	void setEvening_second_end_tasktime(string evening_second_end_tasktime);

	string getEvening_stop_opentime_instrument_A();

	void setEvening_stop_opentime_instrument_A(string evening_stop_opentime_instrument_A);

	string getEvening_first_end_tasktime_instrument_A();

	void setEvening_first_end_tasktime_instrument_A(string evening_first_end_tasktime_instrument_A);

	string getEvening_second_end_tasktime_instrument_A();

	void setEvening_second_end_tasktime_instrument_A(string evening_second_end_tasktime_instrument_A);

	string getEvening_stop_opentime_instrument_B();

	void setEvening_stop_opentime_instrument_B(string evening_stop_opentime_instrument_B);

	string getEvening_first_end_tasktime_instrument_B();

	void setEvening_first_end_tasktime_instrument_B(string evening_first_end_tasktime_instrument_B);

	string getEvening_second_end_tasktime_instrument_B();

	void setEvening_second_end_tasktime_instrument_B(string evening_second_end_tasktime_instrument_B);

	void StgTimeCal();

	// 结束任务标志位
	bool getIsStartEndTaskFlag();
	void setIsStartEndTaskFlag(bool is_start_end_task_flag);
	// 开盘交易标志位
	bool getIsMarketCloseFlag();
	void setIsMarketCloseFlag(bool is_market_close_flag);

	// 是否有夜盘
	bool getHasNightMarket();
	void setHasNightMarket(bool has_night_market);

	bool getHasMorningBreakTime();
	void setHasMorningBreakTime(bool has_morning_break_time);

	bool getHasMidnightMarket();
	void setHasMidnightMarket(bool has_midnight_market);

	bool getEnd_task_afternoon_first();

	void setEnd_task_afternoon_first(bool end_task_afternoon_first);

	bool getEnd_task_afternoon_second();

	void setEnd_task_afternoon_second(bool end_task_afternoon_second);

	bool getEnd_task_evening_first();

	void setEnd_task_evening_first(bool end_task_evening_first);

	bool getEnd_task_evening_second();

	void setEnd_task_evening_second(bool end_task_evening_second);

	bool getEnd_task_morning_first();

	void setEnd_task_morning_first(bool end_task_morning_first);

	bool getEnd_task_morning_second();

	void setEnd_task_morning_second(bool end_task_morning_second);

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
	string stg_update_position_detail_record_time; // 策略持仓明细修改记录时间
	string stg_last_saved_time;			// 策略最后一次保存的时间

	int stg_instrument_A_scale;			// A合约比例乘数
	int stg_instrument_B_scale;			// B合约比例乘数

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
	bool stg_select_order_algorithm_flag;	// 下单算法锁标志位
	//string stg_lock_order_ref;			// 选择下单算法过程中产生的order_ref
	string stg_tick_systime_record;		// 收到tick的系统时间

	CThostFtdcDepthMarketDataField *stg_instrument_Last_tick;	// A合约tick（第一腿）
	CThostFtdcDepthMarketDataField *stg_instrument_A_tick;	// A合约tick（第一腿）
	CThostFtdcDepthMarketDataField *stg_instrument_B_tick;	// B合约tick（第二腿）
	CThostFtdcDepthMarketDataField *stg_instrument_A_tick_last;	// A合约tick（第一腿）交易前最后一次
	CThostFtdcDepthMarketDataField *stg_instrument_B_tick_last;	// B合约tick（第二腿）交易前最后一次
	double stg_spread_long;									// 市场多头价差：A合约买一价 - B合约买一价
	int stg_spread_long_volume;								// 市场多头价差盘口挂单量min(A合约买一量 - B合约买一量)
	double stg_spread_short;								// 市场空头价差：A合约卖一价 - B合约卖一价
	int stg_spread_short_volume;							// 市场空头价差盘口挂单量：min(A合约买一量 - B合约买一量)
	double stg_spread;										// 市场最新价价差

	bool end_task_afternoon_first;							// 下午收盘第一次结束任务标志位
	bool end_task_afternoon_second;							// 下午收盘第二次结束任务标志位

	bool end_task_evening_first;							// 夜间收盘第一次结束任务标志位
	bool end_task_evening_second;							// 夜间收盘第二次结束任务标志位

	bool end_task_morning_first;							// 凌晨收盘第一次结束任务标志位
	bool end_task_morning_second;							// 凌晨收盘第二次结束任务标志位

	string morning_opentime;								// 中午开盘时间
	string morning_begin_breaktime;							// 中午休盘时间前10秒
	string morning_breaktime;								// 中午休盘时间
	string morning_recoverytime;							// 中午休盘恢复时间
	string morning_begin_closetime;							// 中午收盘时间前10秒
	string morning_closetime;								// 中午收盘
	string afternoon_opentime;								// 下午开盘时间
	string afternoon_begin_closetime;						// 下午收盘时间前10秒
	string afternoon_closetime;								// 下午收盘时间
	string evening_opentime;								// 夜间开盘时间
	string evening_stop_opentime;							// 夜间收盘前10秒不允许发新单
	string evening_first_end_tasktime;						// 夜间收盘前5秒
	string evening_second_end_tasktime;						// 夜间收盘前3秒
	string evening_closetime;								// 夜间收盘时间

	string morning_opentime_instrument_A;								// 中午开盘时间_instrument_A
	string morning_begin_breaktime_instrument_A;						// 中午休盘时间前10秒_instrument_A
	string morning_breaktime_instrument_A;								// 中午休盘时间_instrument_A
	string morning_recoverytime_instrument_A;							// 中午休盘恢复时间_instrument_A
	string morning_begin_closetime_instrument_A;						// 中午收盘时间前10秒_instrument_A
	string morning_closetime_instrument_A;								// 中午收盘_instrument_A
	string afternoon_opentime_instrument_A;								// 下午开盘时间_instrument_A
	string afternoon_begin_closetime_instrument_A;						// 下午收盘时间前10秒_instrument_A
	string afternoon_closetime_instrument_A;							// 下午收盘时间_instrument_A
	string evening_opentime_instrument_A;								// 夜间开盘时间_instrument_A
	string evening_stop_opentime_instrument_A;							// 夜间收盘前10秒不允许发新单_instrument_A
	string evening_first_end_tasktime_instrument_A;						// 夜间收盘前5秒_instrument_A
	string evening_second_end_tasktime_instrument_A;					// 夜间收盘前3秒_instrument_A
	string evening_closetime_instrument_A;								// 夜间收盘时间_instrument_A

	string morning_opentime_instrument_B;								// 中午开盘时间_instrument_B
	string morning_begin_breaktime_instrument_B;						// 中午休盘时间前10秒_instrument_B
	string morning_breaktime_instrument_B;								// 中午休盘时间_instrument_B
	string morning_recoverytime_instrument_B;							// 中午休盘恢复时间_instrument_B
	string morning_begin_closetime_instrument_B;						// 中午收盘时间前10秒_instrument_B
	string morning_closetime_instrument_B;								// 中午收盘_instrument_B
	string afternoon_opentime_instrument_B;								// 下午开盘时间_instrument_B
	string afternoon_begin_closetime_instrument_B;						// 下午收盘时间前10秒_instrument_B
	string afternoon_closetime_instrument_B;							// 下午收盘时间_instrument_B
	string evening_opentime_instrument_B;								// 夜间开盘时间_instrument_B
	string evening_stop_opentime_instrument_B;							// 夜间收盘前10秒不允许发新单_instrument_B
	string evening_first_end_tasktime_instrument_B;						// 夜间收盘前5秒_instrument_B
	string evening_second_end_tasktime_instrument_B;					// 夜间收盘前3秒_instrument_B
	string evening_closetime_instrument_B;								// 夜间收盘时间_instrument_B

	bool is_start_end_task_flag;		// 结束任务标志位
	bool is_market_close_flag;			// 开盘交易标志位

	string stg_order_ref_last;	// 最后一次实际使用的报单引用
	string stg_order_ref_a;		// A合约报单引用
	string stg_order_ref_b;		// B合约报单引用
	double stg_a_price_tick;	// A合约最小跳价
	double stg_b_price_tick;	// B合约最小跳价
	int stg_a_limit_price_shift; // A合约报单偏移
	int stg_b_limit_price_shift; // B合约报单偏移

	bool stg_trade_tasking;			// 交易任务进行中
	bool on_off_end_task;			// 是否允许收盘任务执行
	bool has_night_market;			// 是否有夜盘
	bool has_morning_break_time;	// 是否有10:15分休息时间
	bool has_midnight_market;		// 是否有过00:00:00的交易

	CThostFtdcInputOrderField *stg_a_order_insert_args;		// a合约报单参数
	CThostFtdcInputOrderField *stg_b_order_insert_args;		// b合约报单参数
	list<CThostFtdcOrderField *> *stg_list_order_pending;	// 挂单列表，报单、成交、撤单回报
	list<CThostFtdcTradeField *> *stg_list_position_detail; // 持仓明细
	list<USER_CThostFtdcOrderField *> *stg_list_position_detail_from_order; // 持仓明细
	list<USER_CThostFtdcTradeField *> *stg_list_position_detail_from_trade; // 持仓明细
	list<CThostFtdcTradeField *> *l_query_trade;
	list<USER_CThostFtdcOrderField *> *l_query_order;
	list<CThostFtdcInvestorPositionDetailField *> *l_position_detail_from_ctp;

	long long stg_order_ref_base; // 报单引用计数

	int on_off;							//开关
	int stg_only_close;					//只能平仓
	int sell_open_on_off;				//卖开-开关
	int buy_close_on_off;				//买平-开关
	int buy_open_on_off;				//买开-开关
	int sell_close_on_off;				//卖平-开关
	
	bool init_finished;
	bool is_position_right;				//仓位是否正确,是否需要调整仓位
	std::shared_ptr<spdlog::logger> xts_logger;

	// 阻塞队列
	SafeQueue<THREAD_CThostFtdcDepthMarketDataField *> queue_OnRtnDepthMarketData;	// 行情队列
	SafeQueue<THREAD_CThostFtdcOrderField *> queue_OnRtnOrder;						// order回调队列
	SafeQueue<THREAD_CThostFtdcTradeField *> queue_OnRtnTrade;						// trade回调队列

	bool queue_OnRtnDepthMarketData_on_off; // 行情队列开关
	bool queue_OnRtnOrder_on_off;			// order回调队列开关
	bool queue_OnRtnTrade_on_off;			// trade回调队列开关

	sem_t sem_list_order_pending;			// 信号量,用来保证同一时间只能一处地方调用挂单列表
	sem_t sem_generate_order_ref;			// 信号量,用来保证同一时间只能一处地方调用生成报单引用
	sem_t sem_list_position_detail_order;	// 信号量,用来保证同一时间只能一处地方调用操作持仓明细(order)
	sem_t sem_list_position_detail_trade;	// 信号量,用来保证同一时间只能一处地方调用操作持仓明细(trade)
	sem_t sem_order_insert;					// 信号量,用来保证同一时间只能一处地方调用下单，避免orderref重复
	
	sem_t sem_thread_queue_OnRtnDepthMarketData;
	sem_t sem_thread_queue_OnRtnOrder;	
	sem_t sem_thread_queue_OnRtnTrade;
	
};

#endif
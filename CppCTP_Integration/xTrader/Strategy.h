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

	void OnRtnDepthMarketData(CSgitFtdcDepthMarketDataField *pDepthMarketData);

	bool CompareTickData(CSgitFtdcDepthMarketDataField *last_tick_data, CSgitFtdcDepthMarketDataField *pDepthMarketData);

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

	CSgitFtdcInputOrderField* getStgAOrderInsertArgs();
	void setStgAOrderInsertArgs(
		CSgitFtdcInputOrderField* stgAOrderInsertArgs);

	double getStgAPriceTick();
	void setStgAPriceTick(double stgAPriceTick);

	double getStgAWaitPriceTick();
	void setStgAWaitPriceTick(double stgAWaitPriceTick);

	CSgitFtdcInputOrderField* getStgBOrderInsertArgs();
	void setStgBOrderInsertArgs(
		CSgitFtdcInputOrderField* stgBOrderInsertArgs);

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

	CSgitFtdcDepthMarketDataField* getStgInstrumentATick();
	void setStgInstrumentATick(
		CSgitFtdcDepthMarketDataField* stgInstrumentATick);

	CSgitFtdcDepthMarketDataField* getStgInstrumentBTick();
	void setStgInstrumentBTick(
		CSgitFtdcDepthMarketDataField* stgInstrumentBTick);

	CSgitFtdcDepthMarketDataField* getStgInstrumentATickLast();
	void setStgInstrumentATickLast(
		CSgitFtdcDepthMarketDataField* stgInstrumentATick);

	CSgitFtdcDepthMarketDataField* getStgInstrumentBTickLast();
	void setStgInstrumentBTickLast(
		CSgitFtdcDepthMarketDataField* stgInstrumentBTick);

	string getStgInstrumentIdA();
	void setStgInstrumentIdA(string stgInstrumentIdA);

	string getStgInstrumentIdB();
	void setStgInstrumentIdB(string stgInstrumentIdB);

	bool isStgIsActive();
	void setStgIsActive(bool stgIsActive);

	list<CSgitFtdcOrderField *> *getStgListOrderPending();
	void setStgListOrderPending(list<CSgitFtdcOrderField *> *stgListOrderPending);

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

	int getStgALimitPriceShift(); // A��Լ����ƫ��
	void setStgALimitPriceShift(int stg_a_limit_price_shift);
	
	int getStgBLimitPriceShift(); // B��Լ����ƫ��
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
			
	int getStgInstrumentAScale();		// A��Լ��������
	void setStgInstrumentAScale(int stg_instrument_A_scale);

	int getStgInstrumentBScale();		// B��Լ��������
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
	/* ������صĻر�����                                                      */
	/************************************************************************/
	//�µ�
	void OrderInsert(User *user, char *InstrumentID, char CombOffsetFlag, char Direction, int Volume, double Price, string OrderRef);

	//�µ���Ӧ
	void OnRtnOrder(CSgitFtdcOrderField *pOrder);

	//�ɽ�֪ͨ
	void OnRtnTrade(CSgitFtdcTradeField *pTrade);

	//�µ�������Ӧ
	void OnErrRtnOrderInsert(CSgitFtdcInputOrderField *pInputOrder);

	///����¼��������Ӧ
	void OnRspOrderInsert(CSgitFtdcInputOrderField *pInputOrder);

	//����
	void OrderAction(string ExchangeID, string OrderRef, string OrderSysID);

	//����������Ӧ
	void OnRspOrderAction(CSgitFtdcInputOrderActionField *pInputOrderAction);

	//��������
	void OnErrRtnOrderAction(CSgitFtdcOrderActionField *pOrderAction);

	//ѡ���µ��㷨
	void Select_Order_Algorithm(string stg_order_algorithm);


	//�µ��㷨1
	void Order_Algorithm_One();
	//�µ��㷨2
	void Order_Algorithm_Two();
	//�µ��㷨3
	void Order_Algorithm_Three();

	/// ���ɱ�������
	void Generate_Order_Ref(CSgitFtdcInputOrderField *insert_order);

	/// ִ��������
	void Exec_OrderCloseConvert(CSgitFtdcInputOrderField *insert_order);   // ����ƽ���Զ�����
	void Exec_OrderInsert(CSgitFtdcInputOrderField *insert_order);			// ����
	void Exec_OrderAction(CSgitFtdcOrderField *action_order);				// ����
	void Exec_OnRspOrderInsert();											// ����¼������
	void Exec_OnRspOrderAction(CSgitFtdcInputOrderActionField *pInputOrderAction);											// ��������������Ӧ
	void Exec_OnRtnOrder(CSgitFtdcOrderField *pOrder);						// �����ر�
	void ExEc_OnRtnTrade(CSgitFtdcTradeField *pTrade);						// �ɽ��ر�
	void Exec_OnErrRtnOrderInsert();										// ����¼�����ر�
	void Exec_OnErrRtnOrderAction();										// ������������ر�
	void Exec_OnTickComing(THREAD_CSgitFtdcDepthMarketDataField *pDepthMarketData);			// ����ص�,ִ�н�������

	/// ���¹ҵ�list(Order)
	void update_pending_order_list(CSgitFtdcOrderField *pOrder);

	/// ���¹ҵ�list(Trade)
	void update_pending_order_list(USER_CSgitFtdcTradeField *pTrade);

	/// ���¹ҵ��б�list(Action)
	void update_pending_order_list(CSgitFtdcInputOrderActionField *pInputOrderAction);

	/// ��pending_order_list������Ԫ��
	void add_update_pending_order_list(CSgitFtdcOrderField *pOrder);

	/// ��������ر�ɾ��pending_order_list�е�Ԫ��
	void remove_pending_order_list(CSgitFtdcOrderField *pOrder); 
	
	/// ��������ر�ɾ��pending_order_list�е�Ԫ��
	void remove_pending_order_list(CSgitFtdcOrderActionField *pOrderAction);

	void remove_pending_order_list(CSgitFtdcInputOrderActionField *pOrderAction);
	
	/// ����ر����ص�Ԫ��Ҫ�ӹҵ��б����Ƴ�
	void remove_pending_order_list(CSgitFtdcInputOrderField *pOrder);

	/// ���³ֲ���(Order)
	//void update_position(CSgitFtdcOrderField *pOrder);

	/// ���³ֲ���(UserOrder)
	void update_position(USER_CSgitFtdcOrderField *pOrder);
	
	/// ���³ֲ���(Trade)
	void update_position(USER_CSgitFtdcTradeField *pTrade);

	/// ���³ֲ���ϸ(Trade)
	//void update_position_detail(CSgitFtdcTradeField *pTrade);

	/// �����Զ���ֲ���ϸ�ṹ(Order)
	void update_position_detail(USER_CSgitFtdcOrderField *pOrder);

	/// �����Զ���ֲ���ϸ�ṹ(Trade)
	void update_position_detail(USER_CSgitFtdcTradeField *pTrade);

	/// �����Զ���ֲ���ϸ�ṹ(Action)
	void update_position_detail(CSgitFtdcInputOrderActionField *pInputOrderAction);

	/// ���½���״̬
	void update_task_status();

	/// ����tick��
	void update_tick_lock_status(USER_CSgitFtdcOrderField *order);

	/// ����ֶα��γɽ�����order������
	void add_VolumeTradedBatch(THREAD_CSgitFtdcOrderField *pOrder, USER_CSgitFtdcOrderField *new_Order);

	/// �õ���������Сֵ
	int getMinNum(int num1, int num2, int num3);

	/// ��ʼ��
	void InitStrategy();

	/// �����ṹ��CSgitFtdcDepthMarketDataField
	void CopyTickData(CSgitFtdcDepthMarketDataField *dst, CSgitFtdcDepthMarketDataField *src);

	

	/// �����ṹ��THREAD_CSgitFtdcDepthMarketDataField
	void CopyThreadTickData(THREAD_CSgitFtdcDepthMarketDataField *dst, CSgitFtdcDepthMarketDataField *src, bool isLastElement = false);


	/// �����ṹ��CSgitFtdcDepthMarketDataField
	void CopyTickData(CSgitFtdcDepthMarketDataField *dst, THREAD_CSgitFtdcDepthMarketDataField *src);

	/// �����ṹ��CSgitFtdcOrderField
	void CopyOrderData(CSgitFtdcOrderField *dst, CSgitFtdcOrderField *src);

	void CopyThreadOrderData(THREAD_CSgitFtdcOrderField *dst, CSgitFtdcOrderField *src, bool isLastElement = false);

	void CopyOrderData(CSgitFtdcOrderField *dst, THREAD_CSgitFtdcOrderField *src);

	void CopyOrderDataToNew(USER_CSgitFtdcOrderField *dst, CSgitFtdcOrderField *src);

	void CopyThreadOrderDataToNew(USER_CSgitFtdcOrderField *dst, THREAD_CSgitFtdcOrderField *src);

	/// �����ṹ��USER_CSgitFtdcOrderField
	void CopyNewOrderData(USER_CSgitFtdcOrderField *dst, USER_CSgitFtdcOrderField *src);


	/// �����ṹ��CSgitFtdcTradeField
	void CopyTradeData(CSgitFtdcTradeField *dst, CSgitFtdcTradeField *src);

	void CopyThreadTradeData(THREAD_CSgitFtdcTradeField *dst, CSgitFtdcTradeField *src, bool isLastElement = false);

	void CopyTradeData(CSgitFtdcTradeField *dst, THREAD_CSgitFtdcTradeField *src);

	/// �����ṹ��CSgitFtdcTradeField
	void CopyTradeDataToNew(USER_CSgitFtdcTradeField *dst, CSgitFtdcTradeField *src);

	void CopyThreadTradeDataToNew(USER_CSgitFtdcTradeField *dst, THREAD_CSgitFtdcTradeField *src);

	void CopyNewTradeData(USER_CSgitFtdcTradeField *dst, USER_CSgitFtdcTradeField *src);

	/// ���̱�������
	void DropPositionDetail();

	/// ���ݳֲ���ϸ����ͳ�Ƴֲ���(order)
	void calPosition();

	/// ���²���
	void UpdateStrategy(Strategy *stg);

	/// �����ֲ���ϸ
	void CreatePositionDetail(USER_CSgitFtdcOrderField *posd);
	
	/// ���ݿ���²��Գֲ���ϸ
	void Update_Position_Detail_To_DB(USER_CSgitFtdcOrderField *posd);

	/// ���ݿ���²��Գֲ���ϸ(order changed)
	void Update_Position_Changed_Detail_To_DB(USER_CSgitFtdcOrderField *posd);

	/// �����ֲ���ϸ
	void CreatePositionTradeDetail(USER_CSgitFtdcTradeField *posd);

	/// ���ݿ���²��Գֲ���ϸ
	void Update_Position_Trade_Detail_To_DB(USER_CSgitFtdcTradeField *posd);

	/// ���ݿ���²��Գֲ���ϸ(trade changed)
	void Update_Position_Trade_Changed_Detail_To_DB(USER_CSgitFtdcTradeField *posd);

	/// ���ÿ���
	int getOn_Off();
	void setOn_Off(int on_off);

	void setStgSellOpenOnOff(int sell_open_on_off);				//����-����
	int getStgSellOpenOnOff();

	void setStgBuyCloseOnOff(int buy_close_on_off);				//��ƽ-����
	int getStgBuyCloseOnOff();

	void setStgBuyOpenOnOff(int buy_open_on_off);				//��-����
	int getStgBuyOpenOnOff();

	void setStgSellCloseOnOff(int sell_close_on_off);			//��ƽ-����
	int getStgSellCloseOnOff();

	/// ����ģ��
	void setStgTradeModel(string stg_trade_model);

	string getStgTradeModel();

	string getStgOrderAlgorithm();			// �µ��㷨

	void setStgOrderAlgorithm(string stg_order_algorithm);

	double getStgHoldProfit();				// �ֲ�ӯ��

	void setStgHoldProfit(double stg_hold_profit);

	double getStgCloseProfit();			// ƽ��ӯ��

	void setStgCloseProfit(double stg_close_profit);

	double getStgCommission();				// ������

	void setStgCommisstion(double stg_commission);

	int getStgPosition();					// �ֲܳ�

	void setStgPosition(int stg_position);

	int getStgPositionBuy();				// ��ֲ�

	void setStgPositionBuy(int stg_position_buy);

	int getStgPositionSell();				// ���ֲ�
	
	int setStgPositionSell(int stg_position_sell);

	int getStgTradeVolume();				// �ɽ���

	int setStgTradeVolume(int stg_trade_volume);

	double getStgAmount();					// �ɽ����

	void setStgAmount(double stg_amount);

	double getStgAverageShift();			// ƽ������

	void setStgAverageShift(double stg_average_shift);

	void setInit_Finished(bool init_finished);

	bool getInit_Finished();

	void init_today_position();

	void setL_query_trade(list<CSgitFtdcTradeField *> *l_query_trade);
	void addOrderToListQueryOrder(CSgitFtdcOrderField *order);
	void setL_query_order(list<CSgitFtdcOrderField *> *l_query_order);
	void add_position_detail(USER_CSgitFtdcOrderField *posd);


	//void CopyPositionData(PositionDetail *posd, USER_CSgitFtdcOrderField *order);

	list<USER_CSgitFtdcOrderField *> * getStg_List_Position_Detail_From_Order(); // �ֲ���ϸ(order)
	list<USER_CSgitFtdcTradeField *> * getStg_List_Position_Detail_From_Trade(); // �ֲ���ϸ(trade)

	void printStrategyInfo(string message); //�������
	void printStrategyInfoPosition(); //�����ǰ�ֲ���ϸ

	void setStgUpdatePositionDetailRecordTime(string stg_update_position_detail_record_time);
	string getStgUpdatePositionDetailRecordTime();

	//��λ�Ƿ���ȷ,�Ƿ���Ҫ������λ
	void setStgIsPositionRight(bool is_position_right);
	bool getStgIsPositionRight();

	//��ճֲ���ϸ
	void clearStgPositionDetail();

	//����ģ��ƽ��order
	void createFakeOrderPositionDetail(USER_CSgitFtdcOrderField *order, string date, string instrumentID, char CombHedgeFlag, char Direction, char CombOffsetFlag, int VolumeTradedBatch);

	//����ģ��ƽ��trade
	void createFakeTradePositionDetail(USER_CSgitFtdcTradeField *trade, string date, string instrumentID, char HedgeFlag, char Direction, char OffsetFlag, int Volume);

	//����ǰ5�봦��ҵ��б�
	void finish_pending_order_list();

	//�������һ�α����ʱ��
	void setStgLastSavedTime(string stg_last_saved_time);
	string getStgLastSavedTime();

	//�Ƿ�������������ִ��
	bool getStgOnOffEndTask();
	void setStgOnOffEndTask(bool on_off_end_task);

	// ����ϵͳxts_logger
	void setXtsLogger(std::shared_ptr<spdlog::logger> ptr);
	std::shared_ptr<spdlog::logger> getXtsLogger();

	// �������
	void thread_queue_OnRtnDepthMarketData();
	// order�ص�����
	void thread_queue_OnRtnOrder();
	// trade�ص�����
	void thread_queue_OnRtnTrade();
	// ֹͣ�߳�
	void end_thread();
	// ��ȡֹͣ�߳�״̬
	bool getEndThreadStatus();

	// ������п���
	void setQueue_OnRtnDepthMarketData_on_off(bool queue_OnRtnDepthMarketData);
	bool getQueue_OnRtnDepthMarketData_on_off();
	// order�ص����п���
	void setQueue_OnRtnOrder_on_off(bool queue_OnRtnOrder_on_off);
	bool getQueue_OnRtnOrder_on_off();
	// trade�ص����п���
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

	// ���������־λ
	bool getIsStartEndTaskFlag();
	void setIsStartEndTaskFlag(bool is_start_end_task_flag);
	// ���̽��ױ�־λ
	bool getIsMarketCloseFlag();
	void setIsMarketCloseFlag(bool is_market_close_flag);

	// �Ƿ���ҹ��
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

	DBManager *stg_DBM;			// ���ݿ�����ʵ��
	User *stg_user;				// userʵ��
	string stg_trader_id;		// ����Ա�˻�id
	string stg_user_id;			// user_id
	string stg_strategy_id;		// ����id
	string stg_instrument_id_A;	// ��ԼA
	string stg_instrument_id_B;	// ��ԼB
	
	int stg_b_order_already_send_batch;	// B��Լ�Ѿ�����ȥ������

	double stg_buy_open;				// �����򿪣����൥��
	double stg_sell_close;				// ������ƽ��ƽ�൥��
	double stg_sell_open;				// �������������յ���
	double stg_buy_close;				// ������ƽ��ƽ�յ���
	double stg_spread_shift;			// �۲��ü�
	double stg_a_wait_price_tick;		// A��Լ�ҵ��ȴ���С����
	double stg_b_wait_price_tick;		// B��Լ�ҵ��ȴ���С����
	double stg_stop_loss;				// ֹ�𣬵�λΪ��С����
	int stg_lots;						// ����
	int stg_lots_batch;					// ÿ���µ�����
	bool stg_is_active;					// ���Կ���״̬
	int stg_a_order_action_tires_limit;	// A��Լ������������
	int stg_b_order_action_tires_limit;	// B��Լ������������
	int stg_a_order_action_count;		// A��Լ��������
	int stg_b_order_action_count;		// B��Լ��������
	
	/*�����ֶ�*/
	string stg_trade_model;				// ����ģ��
	string stg_order_algorithm;			// �µ��㷨
	double stg_hold_profit;				// �ֲ�ӯ��
	double stg_close_profit;			// ƽ��ӯ��
	double stg_commission;				// ������
	int stg_position;					// �ֲܳ�
	int stg_position_buy;				// ��ֲ�
	int stg_position_sell;				// ���ֲ�
	int stg_trade_volume;				// �ɽ���
	double stg_amount;					// �ɽ����
	double stg_average_shift;			// ƽ������
	string stg_trading_day;				// ������
	string stg_update_position_detail_record_time; // ���Գֲ���ϸ�޸ļ�¼ʱ��
	string stg_last_saved_time;			// �������һ�α����ʱ��

	int stg_instrument_A_scale;			// A��Լ��������
	int stg_instrument_B_scale;			// B��Լ��������

	int stg_position_a_buy_today;		// A��Լ��ֲֽ��
	int stg_position_a_buy_yesterday;	// A��Լ��ֲ����
	int stg_position_a_buy;				// A��Լ��ֲ��ܲ�λ
	int stg_position_a_sell_today;		// A��Լ���ֲֽ��
	int stg_position_a_sell_yesterday;	// A��Լ���ֲ����
	int stg_position_a_sell;			// A��Լ���ֲ��ܲ�λ
	int stg_position_b_buy_today;		// B��Լ��ֲֽ��
	int stg_position_b_buy_yesterday;	// B��Լ��ֲ����
	int stg_position_b_buy;				// B��Լ��ֲ��ܲ�λ
	int stg_position_b_sell_today;		// B��Լ���ֲֽ��
	int stg_position_b_sell_yesterday;	// B��Լ���ֲ����
	int stg_position_b_sell;			// B��Լ���ֲ��ܲ�λ
	int stg_pending_a_open;				// A��Լ�ҵ��򿪲���
	bool stg_select_order_algorithm_flag;	// �µ��㷨����־λ
	//string stg_lock_order_ref;			// ѡ���µ��㷨�����в�����order_ref
	string stg_tick_systime_record;		// �յ�tick��ϵͳʱ��

	CSgitFtdcDepthMarketDataField *stg_instrument_Last_tick;	// A��Լtick����һ�ȣ�
	CSgitFtdcDepthMarketDataField *stg_instrument_A_tick;	// A��Լtick����һ�ȣ�
	CSgitFtdcDepthMarketDataField *stg_instrument_B_tick;	// B��Լtick���ڶ��ȣ�
	CSgitFtdcDepthMarketDataField *stg_instrument_A_tick_last;	// A��Լtick����һ�ȣ�����ǰ���һ��
	CSgitFtdcDepthMarketDataField *stg_instrument_B_tick_last;	// B��Լtick���ڶ��ȣ�����ǰ���һ��
	double stg_spread_long;									// �г���ͷ�۲A��Լ��һ�� - B��Լ��һ��
	int stg_spread_long_volume;								// �г���ͷ�۲��̿ڹҵ���min(A��Լ��һ�� - B��Լ��һ��)
	double stg_spread_short;								// �г���ͷ�۲A��Լ��һ�� - B��Լ��һ��
	int stg_spread_short_volume;							// �г���ͷ�۲��̿ڹҵ�����min(A��Լ��һ�� - B��Լ��һ��)
	double stg_spread;										// �г����¼ۼ۲�

	bool end_task_afternoon_first;							// �������̵�һ�ν��������־λ
	bool end_task_afternoon_second;							// �������̵ڶ��ν��������־λ

	bool end_task_evening_first;							// ҹ�����̵�һ�ν��������־λ
	bool end_task_evening_second;							// ҹ�����̵ڶ��ν��������־λ

	bool end_task_morning_first;							// �賿���̵�һ�ν��������־λ
	bool end_task_morning_second;							// �賿���̵ڶ��ν��������־λ

	string morning_opentime;								// ���翪��ʱ��
	string morning_begin_breaktime;							// ��������ʱ��ǰ10��
	string morning_breaktime;								// ��������ʱ��
	string morning_recoverytime;							// �������ָ̻�ʱ��
	string morning_begin_closetime;							// ��������ʱ��ǰ10��
	string morning_closetime;								// ��������
	string afternoon_opentime;								// ���翪��ʱ��
	string afternoon_begin_closetime;						// ��������ʱ��ǰ10��
	string afternoon_closetime;								// ��������ʱ��
	string evening_opentime;								// ҹ�俪��ʱ��
	string evening_stop_opentime;							// ҹ������ǰ10�벻�����µ�
	string evening_first_end_tasktime;						// ҹ������ǰ5��
	string evening_second_end_tasktime;						// ҹ������ǰ3��
	string evening_closetime;								// ҹ������ʱ��

	string morning_opentime_instrument_A;								// ���翪��ʱ��_instrument_A
	string morning_begin_breaktime_instrument_A;						// ��������ʱ��ǰ10��_instrument_A
	string morning_breaktime_instrument_A;								// ��������ʱ��_instrument_A
	string morning_recoverytime_instrument_A;							// �������ָ̻�ʱ��_instrument_A
	string morning_begin_closetime_instrument_A;						// ��������ʱ��ǰ10��_instrument_A
	string morning_closetime_instrument_A;								// ��������_instrument_A
	string afternoon_opentime_instrument_A;								// ���翪��ʱ��_instrument_A
	string afternoon_begin_closetime_instrument_A;						// ��������ʱ��ǰ10��_instrument_A
	string afternoon_closetime_instrument_A;							// ��������ʱ��_instrument_A
	string evening_opentime_instrument_A;								// ҹ�俪��ʱ��_instrument_A
	string evening_stop_opentime_instrument_A;							// ҹ������ǰ10�벻�����µ�_instrument_A
	string evening_first_end_tasktime_instrument_A;						// ҹ������ǰ5��_instrument_A
	string evening_second_end_tasktime_instrument_A;					// ҹ������ǰ3��_instrument_A
	string evening_closetime_instrument_A;								// ҹ������ʱ��_instrument_A

	string morning_opentime_instrument_B;								// ���翪��ʱ��_instrument_B
	string morning_begin_breaktime_instrument_B;						// ��������ʱ��ǰ10��_instrument_B
	string morning_breaktime_instrument_B;								// ��������ʱ��_instrument_B
	string morning_recoverytime_instrument_B;							// �������ָ̻�ʱ��_instrument_B
	string morning_begin_closetime_instrument_B;						// ��������ʱ��ǰ10��_instrument_B
	string morning_closetime_instrument_B;								// ��������_instrument_B
	string afternoon_opentime_instrument_B;								// ���翪��ʱ��_instrument_B
	string afternoon_begin_closetime_instrument_B;						// ��������ʱ��ǰ10��_instrument_B
	string afternoon_closetime_instrument_B;							// ��������ʱ��_instrument_B
	string evening_opentime_instrument_B;								// ҹ�俪��ʱ��_instrument_B
	string evening_stop_opentime_instrument_B;							// ҹ������ǰ10�벻�����µ�_instrument_B
	string evening_first_end_tasktime_instrument_B;						// ҹ������ǰ5��_instrument_B
	string evening_second_end_tasktime_instrument_B;					// ҹ������ǰ3��_instrument_B
	string evening_closetime_instrument_B;								// ҹ������ʱ��_instrument_B

	bool is_start_end_task_flag;		// ���������־λ
	bool is_market_close_flag;			// ���̽��ױ�־λ

	string stg_order_ref_last;	// ���һ��ʵ��ʹ�õı�������
	string stg_order_ref_a;		// A��Լ��������
	string stg_order_ref_b;		// B��Լ��������
	double stg_a_price_tick;	// A��Լ��С����
	double stg_b_price_tick;	// B��Լ��С����
	int stg_a_limit_price_shift; // A��Լ����ƫ��
	int stg_b_limit_price_shift; // B��Լ����ƫ��

	bool stg_trade_tasking;			// �������������
	bool on_off_end_task;			// �Ƿ�������������ִ��
	bool has_night_market;			// �Ƿ���ҹ��
	bool has_morning_break_time;	// �Ƿ���10:15����Ϣʱ��
	bool has_midnight_market;		// �Ƿ��й�00:00:00�Ľ���

	CSgitFtdcInputOrderField *stg_a_order_insert_args;		// a��Լ��������
	CSgitFtdcInputOrderField *stg_b_order_insert_args;		// b��Լ��������
	list<CSgitFtdcOrderField *> *stg_list_order_pending;	// �ҵ��б��������ɽ��������ر�
	list<CSgitFtdcTradeField *> *stg_list_position_detail; // �ֲ���ϸ
	list<USER_CSgitFtdcOrderField *> *stg_list_position_detail_from_order; // �ֲ���ϸ
	list<USER_CSgitFtdcTradeField *> *stg_list_position_detail_from_trade; // �ֲ���ϸ
	list<CSgitFtdcTradeField *> *l_query_trade;
	list<USER_CSgitFtdcOrderField *> *l_query_order;
	list<CSgitFtdcInvestorPositionDetailField *> *l_position_detail_from_ctp;

	long long stg_order_ref_base; // �������ü���

	int on_off;							//����
	int stg_only_close;					//ֻ��ƽ��
	int sell_open_on_off;				//����-����
	int buy_close_on_off;				//��ƽ-����
	int buy_open_on_off;				//��-����
	int sell_close_on_off;				//��ƽ-����
	
	bool init_finished;
	bool is_position_right;				//��λ�Ƿ���ȷ,�Ƿ���Ҫ������λ
	std::shared_ptr<spdlog::logger> xts_logger;

	// ��������
	SafeQueue<THREAD_CSgitFtdcDepthMarketDataField *> queue_OnRtnDepthMarketData;	// �������
	SafeQueue<THREAD_CSgitFtdcOrderField *> queue_OnRtnOrder;						// order�ص�����
	SafeQueue<THREAD_CSgitFtdcTradeField *> queue_OnRtnTrade;						// trade�ص�����

	bool queue_OnRtnDepthMarketData_on_off; // ������п���
	bool queue_OnRtnOrder_on_off;			// order�ص����п���
	bool queue_OnRtnTrade_on_off;			// trade�ص����п���

	sem_t sem_list_order_pending;			// �ź���,������֤ͬһʱ��ֻ��һ���ط����ùҵ��б�
	sem_t sem_generate_order_ref;			// �ź���,������֤ͬһʱ��ֻ��һ���ط��������ɱ�������
	sem_t sem_list_position_detail_order;	// �ź���,������֤ͬһʱ��ֻ��һ���ط����ò����ֲ���ϸ(order)
	sem_t sem_list_position_detail_trade;	// �ź���,������֤ͬһʱ��ֻ��һ���ط����ò����ֲ���ϸ(trade)
	sem_t sem_order_insert;					// �ź���,������֤ͬһʱ��ֻ��һ���ط������µ�������orderref�ظ�
	
	sem_t sem_thread_queue_OnRtnDepthMarketData;
	sem_t sem_thread_queue_OnRtnOrder;	
	sem_t sem_thread_queue_OnRtnTrade;
	
};

#endif
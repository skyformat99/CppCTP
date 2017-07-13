#ifndef QUANT_CTP_MANAGER_H
#define QUANT_CTP_MANAGER_H

#include <list>
#include <map>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include "Login.h"
#include "MdSpi.h"
#include "TdSpi.h"
#include "DBManager.h"
#include "Trader.h"
#include "Session.h"
#include "PositionDetail.h"
#include "xTradeStruct.h"
#include "Timer.h"
#include "safequeue.h"
#include "ApiCommand.h"
#include <spdlog/spdlog.h>
using namespace spdlog;
using spdlog::logger;

using std::map;


class MdSpi;
class MarketConfig;
class ApiCommand;

class CTP_Manager {

public:
	CTP_Manager();
	///��½
	//bool CheckIn(Login *login);

	/// trader login
	bool TraderLogin(string traderid, string password, Trader *op);

	/// admin login
	bool AdminLogin(string adminid, string password);

	///��������
	int submarketData(char *instrument[]);

	/// �������׶����ҵ�½
	User * CreateAccount(User *user, list<Strategy *> *l_strategys);

	/// ��������
	MdSpi * CreateMd(string td_frontAddress, string td_broker, string td_user, string td_pass, list<Strategy *> *l_strategys);

	///�ͷ�
	void ReleaseAccount(User *user);

	///��������
	void SubmarketData(MdSpi *mdspi, list<string > *l_instrument);

	///ȡ����������
	void UnSubmarketData(MdSpi *mdspi, string instrumentID, list<string > *l_instrument);

	/// ��Ӻ�Լ
	list<string> * addSubInstrument(string instrumentID, list<string> *l_instrument);

	/// ɾ�����ĺ�Լ
	list<string> * delSubInstrument(string instrumentID, list<string> *l_instrument);

	/// ͳ�ƺ�Լ����
	int calInstrument(string instrumentID, list<string> *l_instrument);

	/// �˶���Լ����
	list<string> addUnSubInstrument(string instrumentID, list<string> l_instrument);

	/// �õ�l_instrument
	list<string> *getL_Instrument();

	/// �õ�l_unsubinstrument
	list<string> *getL_UnsubInstrument();

	/// �õ����ݿ��������
	DBManager *getDBManager();

	/// ����l_trader
	void addTraderToLTrader(string trader);

	/// ����ϵͳ����״̬
	void updateSystemFlag();

	/// ��ȡtrader�Ƿ���l_trader��
	bool checkInLTrader(string trader);

	bool getTenMinFlag();
	void setTenMinFlag(bool ten_min_flag);

	bool getOneMinFlag();
	void setOneMinFlag(bool one_min_flag);

	bool getOneSecondFlag();
	void setOneSecondFlag(bool one_second_flag);

	/// �õ�l_trader
	list<string> *getL_Trader();

	/// �õ�l_obj_trader
	list<Trader *> * getL_Obj_Trader();

	/// �Ƴ�Ԫ��
	void removeFromLTrader(string trader);

	/// �����û��ڻ��˻�
	void addFuturesToTrader(string traderid, User *user);

	/// ��ȡ�ڻ��˻�map
	map<string, list<User *> *> getTraderMap();

	/// �����û��б�
	list<User *> *getL_User();

	list<User *> *getL_User_Bee();

	/// �õ�strategy_list
	list<Strategy *> *getListStrategy();

	/// �õ�strategy_list
	list<Strategy *> *getListStrategyYesterday();

	/// �õ�����ǰ�õ�ַ
	list<MarketConfig *> *getL_MarketConfig();

	/// �õ��µ��㷨
	list<Algorithm *> * getL_Alg();

	/// ����strategy_list
	void setListStrategy(list<Strategy *> *l_strategys);

	/// ���ö�ʱ��
	void setCalTimer(Timer *cal_timer);

	/// ��ȡ��ʱ��
	Timer *getCalTimer();

	/// ����strategy_list
	void saveStrategy();

	/// �������в��Գֲ���ϸ
	void saveAllStrategyPositionDetail();

	/// ����һ�����Գֲ���ϸ
	void saveStrategyPositionDetail(Strategy *stg);

	/// ����һ�����Գֲ���ϸ�޸Ĺ���
	void saveStrategyChangedPositionDetail(Strategy *stg);

	/// ����mdspi
	void setMdSpi(MdSpi *mdspi);

	/// ���mdspi
	MdSpi *getMdSpi();

	/// ��ʼ��
	bool init(bool is_online = true);

	/// ���ÿ���
	int getOn_Off();
	void setOn_Off(int on_off);

	/// ���������Ƿ��½���
	bool getMdLogin();
	void setMdLogin(bool isMdLogin);

	/// ����ctp��λ�Ƿ��ʼ�����
	bool getCTPFinishedPositionInit();
	void setCTPFinishedPositionInit(bool isCTPFinishedPositionInit);

	/// ����ͻ��˷�������Ϣ
	static void HandleMessage(int fd, char *msg, CTP_Manager *ctp_m);

	/// ��ʼ�����ݷ���
	static void InitClientData(int fd, CTP_Manager *ctp_m, string s_TraderID, int i_MsgRef, int i_MsgSrc, string s_UserID = "", string s_StrategyID = "");

	/// �������֪ͨ


	/// ���׶���֪ͨ


	/// ���ý�����
	void setTradingDay(string trading_day);

	/// ��ý�����
	string getTradingDay();

	/// ��ʼ������(���Ը���trading_day, �ڻ��˻�����order_ref)
	bool initStrategyAndFutureAccount();

	/// ͳ�Ʊ���order,trade�ֲ���ϸ
	void initPositionDetailDataFromLocalOrderAndTrade();

	/// ����SOCKET FD
	void addSocketFD(string user_id, int fd);

	/// ɾ��SOCKET FD
	void delSocketFD(string user_id, int fd);

	/// �����������֪ͨ
	void sendMarketOffLineMessage(int on_off_status);

	/// ���ͽ��׶���֪ͨ
	void sendTradeOffLineMessage(string user_id, int on_off_status);

	/// ��ʼ�������ϸ
	//bool initYesterdayPositionDetail();

	/// �������Ӻ�ͬ����Users
	bool syncStrategyAddToUsers(Strategy *stg);

	/// ����ɾ����ͬ����Users
	bool syncStrategyDeleteToUsers(string traderid, string userid, string strategyid);

	//����
	void setIsMarketClose(bool is_market_close);
	bool getIsMarketClose();

	//ϵͳ�������5����Ҫ��ɵ���β������־λ
	void setIsStartEndTask(bool is_start_end_task);
	bool getIsStartEndTask();
	void StrategyIsStartEndTask();

	//�Ѿ����̱�־λ
	void setIsMarketCloseDone(bool is_market_close_done);
	bool getIsMarketCloseDone();

	//��ȡ����ĳ����Ա�����ڻ��˻�
	void getUserListByTraderID(string traderid, CTP_Manager *ctp_m, list<User *> *l_user_trader);

	//// ����ϵͳxts_logger
	//void setXtsLogger(std::shared_ptr<spdlog::logger> ptr);
	std::shared_ptr<spdlog::logger> getXtsLogger();

	sem_t *getSem_strategy_handler();

	bool getIsClosingSaved();
	void setIsClosingSaved(bool isClosingSaved);

	void thread_queue_Command();

	void addCommand(ApiCommand *command);

private:
	//Login *login;
	list<string> *l_instrument;
	list<string > *l_unsubinstrument;
	list<string> *l_trader;
	list<User *> *l_user;
	list<User *> *l_user_bee;
	list<Trader *> *l_obj_trader;
	list<Algorithm *> *l_alg;
	list<MarketConfig *> *l_marketconfig;
	map<string, list<User *> *> m_trader;
	map<string, list<int> *> m_socket_fds;
	DBManager *dbm;
	list<Strategy *> *l_strategys;
	list<Strategy *> *l_strategys_yesterday;
	MdSpi *mdspi;

	//���ݷ���CTP
	MarketConfig *mc_bee;

	int on_off; //����
	bool is_market_close; //���̿�ʼ��־
	bool is_start_end_task; //ϵͳ�������5����Ҫ��ɵ���β������־λ
	
	bool is_market_close_done; //�Ѿ�����

	string trading_day;
	bool isClosingSaved;
	bool isMdLogin;
	bool isCTPFinishedPositionInit;
	list<Session *> *l_sessions;
	list<USER_CSgitFtdcOrderField *> *l_posdetail;
	list<USER_CSgitFtdcOrderField *> *l_posdetail_yesterday;

	list<USER_CSgitFtdcTradeField *> *l_posdetail_trade;
	list<USER_CSgitFtdcTradeField *> *l_posdetail_trade_yesterday;

	Timer *cal_timer;
	std::vector<std::thread> user_threads;

	static bool ten_min_flag;
	static bool one_min_flag;
	static bool one_second_flag;
	bool system_init_flag;
	std::shared_ptr<spdlog::logger> xts_logger;

	sem_t sem_strategy_handler;			// �ź���,������֤ͬһʱ��ֻ��һ���̲߳�������

	// �������
	SafeQueue<ApiCommand *> queue_Command;

	// ��¼���һ�η�����������
	int last_command_type;
};
#endif
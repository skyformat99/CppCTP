#ifndef QUANT_DBMANAGER_H
#define QUANT_DBMANAGER_H

#include <iostream>
#include <cstdlib>
#include <list>
#include <vector>
#include <mongo/client/dbclient.h>

#include "Trader.h"
#include "FutureAccount.h"
#include "Strategy.h"
#include "MarketConfig.h"
#include "Algorithm.h"
#include "Session.h"
#include "xTradeStruct.h"

class Strategy;
class MarketConfig;
class Session;
class PositionDetail;


using namespace mongo;
using namespace std;
using std::string;
using mongo::DBClientConnection;

class DBManager {

public:
	DBManager();
	~DBManager();
	static mongo::DBClientConnection * getDBConnection();

	/************************************************************************/
	/* 设置交易模式(simnow 盘中/离线                                           */
	/************************************************************************/
	void setIs_Online(bool is_online);
	bool getIs_Online();

	/************************************************************************/
	/* 查询mongodb是否存在昨持仓明细(trade,order)集合。
	如果存在说明上次程序正常关闭；
	如果不存在说明程序关闭有误*/
	/************************************************************************/
	bool CheckSystemStartFlag();

	/************************************************************************/
	/* 创建交易员
	   删除交易员
	   更新交易员
	   查找交易员(username)
	   查找交易员(nickname)                                                  */
	/************************************************************************/
	void CreateTrader(Trader *op);
	void DeleteTrader(Trader *op);
	void UpdateTrader(string traderid, Trader *op);
	void SearchTraderByTraderID(string traderid);
	void SearchTraderByTraderName(string tradername);
	void SearchTraderByTraderIdAndPassword(string traderid, string password);
	bool FindTraderByTraderIdAndPassword(string traderid, string password, Trader *op);
	void getAllTrader(list<string> *l_trader);
	void getAllObjTrader(list<Trader *> *l_obj_trader);


	/************************************************************************/
	/* 查找管理员                                                             */
	/************************************************************************/
	bool FindAdminByAdminIdAndPassword(string adminid, string password);


	/************************************************************************/
	/* 创建账户
	   删除账户
	   更新账户
	   查找账户(userid)
	   查找账户(tradername)                                                  */
	/************************************************************************/
	void CreateFutureAccount(Trader *op, FutureAccount *fa);
	void DeleteFutureAccount(FutureAccount *fa);
	void UpdateFutureAccount(User *u);
	void UpdateFutureAccountOrderRef(mongo::DBClientConnection * conn, User *u, string order_ref_base);
	void SearchFutrueByUserID(string userid);
	void SearchFutrueByTraderID(string traderid);
	void SearchFutrueListByTraderID(string traderid, list<FutureAccount *> *l_futureaccount);
	void getAllFutureAccount(list<User *> *l_user);

	/************************************************************************/
	/* 创建策略
	   删除策略
	   更新策略
	   查找策略				                                                */
	/************************************************************************/
	int CreateStrategy(Strategy *stg);
	int DeleteStrategy(Strategy *stg);
	void UpdateStrategy(Strategy *stg);
	int UpdateStrategyOnOff(Strategy *stg);
	int UpdateStrategyOnlyCloseOnOff(Strategy *stg);
	void getAllStrategy(list<Strategy *> *l_strategys, string traderid = "", string userid = "");
	void getAllStrategyByActiveUser(list<Strategy *> *l_strategys, list<User *> *l_users, string traderid = "");

	/************************************************************************/
	/* 创建策略(昨仓)
	   删除策略(昨仓)
	   更新策略(昨仓)
	   查找策略(昨仓)			                                                */
	/************************************************************************/
	int CreateStrategyYesterday(Strategy *stg);
	int DeleteStrategyYesterday(Strategy *stg);
	void UpdateStrategyYesterday(Strategy *stg);
	void getAllStrategyYesterday(list<Strategy *> *l_strategys, string traderid = "", string userid = "");
	void getAllStrategyYesterdayByTraderIdAndUserIdAndStrategyId(list<Strategy *> *l_strategys, string traderid = "", string userid = "", string strategyid = "");
	void getAllStrategyYesterdayByActiveUser(list<Strategy *> *l_strategys, list<User *> *l_users, string traderid = "");


	/************************************************************************/
	/* 创建MD配置
	   删除MD配置
	   更新MD配置
	   获取所有MD配置
	   获取一条MD记录															*/
	/************************************************************************/
	void CreateMarketConfig(MarketConfig *mc);
	void DeleteMarketConfig(MarketConfig *mc);
	void UpdateMarketConfig(MarketConfig *mc);
	void getAllMarketConfig(list<MarketConfig *> *l_marketconfig);
	MarketConfig *getOneMarketConfig();

	/************************************************************************/
	/*  创建算法
		删除算法
		更新算法
		获取算法															*/
	/************************************************************************/
	void CreateAlgorithm(Algorithm *alg);
	void DeleteAlgorithm(Algorithm *alg);
	void UpdateAlgorithm(Algorithm *alg);
	void getAllAlgorithm(list<Algorithm *> *l_alg);

	/************************************************************************/
	/* 创建sessionid
	   删除sessionid
	   获取sessionid*/
	/************************************************************************/
	//void CreateSession(Session *sid);
	//void DeleteSession(Session *sid);
	//void getAllSession(list<Session *> *l_sessions);

	/************************************************************************/
	/*    
	创建持仓明细(order)
	删除持仓明细(order)
	更新持仓明细(order)*/
	/************************************************************************/
	//void CreatePositionDetail(PositionDetail *posd);
	//void CreatePositionDetail(USER_CThostFtdcOrderField *posd);
	//void DeletePositionDetail(PositionDetail *posd);
	void UpdatePositionDetail(USER_CThostFtdcOrderField *posd);
	void getAllPositionDetail(list<USER_CThostFtdcOrderField *> *l_posd, string trader_id = "", string userid = "");
	void DropPositionDetail();

	void CreatePositionDetailYesterday(USER_CThostFtdcOrderField *posd);
	void DeletePositionDetailYesterday(USER_CThostFtdcOrderField *posd);
	void UpdatePositionDetailYesterday(USER_CThostFtdcOrderField *posd);
	void getAllPositionDetailYesterday(list<USER_CThostFtdcOrderField *> *l_posd, string trader_id = "", string userid = "");
	void DropPositionDetailYesterday();


	/************************************************************************/
	/*
	创建持仓明细(trade)
	删除持仓明细(trade)
	更新持仓明细(trade)*/
	/************************************************************************/
	void UpdatePositionDetailTrade(USER_CThostFtdcTradeField *posd);
	void getAllPositionDetailTrade(list<USER_CThostFtdcTradeField *> *l_posd, string trader_id = "", string userid = "");
	void DropPositionDetailTrade();

	void CreatePositionDetailTradeYesterday(USER_CThostFtdcTradeField *posd);
	void DeletePositionDetailTradeYesterday(USER_CThostFtdcTradeField *posd);
	void UpdatePositionDetailTradeYesterday(USER_CThostFtdcTradeField *posd);
	void getAllPositionDetailTradeYesterday(list<USER_CThostFtdcTradeField *> *l_posd, string trader_id = "", string userid = "");
	void DropPositionDetailTradeYesterday();

	/************************************************************************/
	/* 系统状态维护
	系统每次启动之后，初始化系统状态running为on,当系统正常关闭,running状态更新为off
	如果启动发现running状态为on，说明上次未正常关闭,那么今天的持仓明细维护可能出错,私有流采取THOST_TERT_RESTART模式,对今持仓明细全部重新统计
	如果启动发现running状态为off，说明上次正常关闭，私有流采取THOST_TERT_RESUME模式，从上次断开的地方重新传送数据即可

	任何模式初始化完成之后,本地仓位统计结果需要和CTP接口查询持仓数据结果保持一致,一旦存在不相同情况,系统要重新初始化.
	*/
	/************************************************************************/
	void UpdateSystemRunningStatus(string key, string value);
	bool GetSystemRunningStatus();

	void setConn(mongo::DBClientConnection *conn);
	mongo::DBClientConnection *getConn();

private:
	mongo::DBClientConnection *conn;
	bool is_online;
};

#endif
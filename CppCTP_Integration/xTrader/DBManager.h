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

class Strategy;
class MarketConfig;


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

	/************************************************************************/
	/* 创建策略(昨仓)
	   删除策略(昨仓)
	   更新策略(昨仓)
	   查找策略(昨仓)			                                                */
	/************************************************************************/
	void CreateStrategyYesterday(Strategy *stg);
	void DeleteStrategyYesterday(Strategy *stg);
	void UpdateStrategyYesterday(Strategy *stg);
	void getAllStrategyYesterday(list<Strategy *> *l_strategys, string traderid = "", string userid = "");
	void getAllStrategyYesterdayByTraderIdAndUserIdAndStrategyId(list<Strategy *> *l_strategys, string traderid = "", string userid = "", string strategyid = "");


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

	

	void setConn(mongo::DBClientConnection *conn);
	mongo::DBClientConnection *getConn();

private:
	mongo::DBClientConnection *conn;
};

#endif
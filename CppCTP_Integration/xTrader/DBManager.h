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
	bool FindTraderByTraderIdAndPassword(string traderid, string password);
	void getAllTrader(list<string> *l_trader);


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
	void UpdateFutureAccount(string userid, Trader *op, FutureAccount *fa);
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
	void CreateStrategy(Strategy *stg);
	void DeleteStrategy(Strategy *stg);
	void UpdateStrategy(Strategy *stg);
	void getAllStragegy(list<Strategy *> *l_strategys);

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
	

	void setConn(mongo::DBClientConnection *conn);
	mongo::DBClientConnection *getConn();

private:
	mongo::DBClientConnection *conn;
};

#endif
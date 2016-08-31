#include <mongo/bson/bson.h>
#include <iostream>
#include "DBManager.h"
#include "Debug.h"

using std::auto_ptr;
using std::unique_ptr;
using namespace std;


#define DB_OPERATOR_COLLECTION            "CTP.trader"
#define DB_FUTUREACCOUNT_COLLECTION       "CTP.futureaccount"
#define DB_ADMIN_COLLECTION               "CTP.admin"
#define DB_STRATEGY_COLLECTION            "CTP.strategy"
#define DB_MARKETCONFIG_COLLECTION        "CTP.marketconfig"
#define ISACTIVE "1"
#define ISNOTACTIVE "0"

DBManager::DBManager() {
	this->conn = new mongo::DBClientConnection(false, 0, 5);
	this->conn->connect("localhost");
	USER_PRINT("Original DB Connection[DBManager::DBManager()]!");
	USER_PRINT(conn);
	//USER_PRINT(this->conn);
}

DBManager::~DBManager() {
	delete this;
}


/************************************************************************/
/* static method return mongo connection                                */
/************************************************************************/
mongo::DBClientConnection * DBManager::getDBConnection() {
	mongo::DBClientConnection *conn = new mongo::DBClientConnection(false, 0, 5);
	conn->connect("localhost");
	USER_PRINT("Original DB Connection[DBManager::getDBConnection()]!");
	USER_PRINT(conn);
	return conn;
}

void DBManager::CreateTrader(Trader *op) {
	
	int count_number = 0;

	count_number = this->conn->count(DB_OPERATOR_COLLECTION,
		BSON("traderid" << op->getTraderID()));

	if (count_number > 0) {
		cout << "Trader Already Exists!" << endl;
	} else {
		BSONObjBuilder b;
		b.append("tradername", op->getTraderName());
		b.append("traderid", op->getTraderID());
		b.append("password", op->getPassword());
		b.append("isactive", op->getIsActive());
		BSONObj p = b.obj();

		conn->insert(DB_OPERATOR_COLLECTION, p);
		
		USER_PRINT("DBManager::CreateOperator ok");
	}
}

void DBManager::DeleteTrader(Trader *op) {

	int count_number = 0;

	count_number = this->conn->count(DB_OPERATOR_COLLECTION,
		BSON("traderid" << op->getTraderID()));

	if (count_number > 0) {
		this->conn->update(DB_OPERATOR_COLLECTION, BSON("traderid" << (op->getTraderID().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeleteOperator ok");
	}
	else {
		cout << "Trader ID Not Exists!" << endl;
	}
	
}

void DBManager::UpdateTrader(string traderid, Trader *op) {

	int count_number = 0;

	count_number = this->conn->count(DB_OPERATOR_COLLECTION,
		BSON("traderid" << traderid));

	if (count_number > 0) {
		this->conn->update(DB_OPERATOR_COLLECTION, BSON("traderid" << (traderid.c_str())), BSON("$set" << BSON("tradername" << op->getTraderName() << "traderid" << op->getTraderID() << "password" << op->getPassword() << "isactive" << op->getIsActive())));
		USER_PRINT("DBManager::UpdateOperator ok");
	}
	else
	{
		cout << "Trader ID Not Exists!" << endl;
	}
}

void DBManager::SearchTraderByTraderID(string traderid) {
	unique_ptr<DBClientCursor> cursor =
		this->conn->query(DB_OPERATOR_COLLECTION, MONGO_QUERY("traderid" << traderid));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		cout << "userid = " << p.getStringField("traderid") << endl;
		cout << "username = " << p.getStringField("tradername") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;
	}
	USER_PRINT("DBManager::SearchTraderByTraderID ok");
}
void DBManager::SearchTraderByTraderName(string tradername) {
	unique_ptr<DBClientCursor> cursor =
		this->conn->query(DB_OPERATOR_COLLECTION, MONGO_QUERY("tradername" << tradername));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		cout << "traderid = " << p.getStringField("traderid") << endl;
		cout << "tradername = " << p.getStringField("tradername") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;
	}
	USER_PRINT("DBManager::SearchTraderByTraderName ok");
}

void DBManager::SearchTraderByTraderIdAndPassword(string traderid, string password) {
	unique_ptr<DBClientCursor> cursor =
		this->conn->query(DB_OPERATOR_COLLECTION, MONGO_QUERY("traderid" << traderid << "password" << password));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		cout << "traderid = " << p.getStringField("traderid") << endl;
		cout << "tradername = " << p.getStringField("tradername") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;
	}
	USER_PRINT("DBManager::SearchTraderByTraderIdAndPassword ok");
}

bool DBManager::FindTraderByTraderIdAndPassword(string traderid, string password) {
	int count_number = 0;
	bool flag = false;

	count_number = this->conn->count(DB_OPERATOR_COLLECTION,
		BSON("traderid" << traderid << "password" << password));

	if (count_number == 0) {
		flag = false;
	} else {
		flag = true;
	}

	return flag;
}

void DBManager::getAllTrader(list<string> *l_trader) {

	/// 初始化的时候，必须保证list为空
	if (l_trader->size() > 0) {
		list<string>::iterator Itor;
		for (Itor = l_trader->begin(); Itor != l_trader->end();) {
			Itor = l_trader->erase(Itor);
		}
	}

	int countnum = this->conn->count(DB_OPERATOR_COLLECTION);
	USER_PRINT(countnum);
	if (countnum == 0) {
		cout << "DBManager::getAllTrader is NONE!" << endl;
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			this->conn->query(DB_OPERATOR_COLLECTION);
		while (cursor->more()) {

			BSONObj p = cursor->next();
			cout << "*" << "traderid:" << p.getStringField("traderid") << "  "
				<< "tradername:" << p.getStringField("tradername") << "  "
				<< "password:" << p.getStringField("password") << "  "
				<< "isactive:" << p.getStringField("isactive") << "*" << endl;

			l_trader->push_back(p.getStringField("traderid"));
		}
		USER_PRINT("DBManager::getAllTrader1 ok");
	}
}

bool DBManager::FindAdminByAdminIdAndPassword(string adminid, string password) {
	int count_number = 0;
	bool flag = false;

	count_number = this->conn->count(DB_ADMIN_COLLECTION,
		BSON("adminid" << adminid << "password" << password));

	if (count_number == 0) {
		flag = false;
	}
	else {
		flag = true;
	}

	return flag;
}

void DBManager::CreateFutureAccount(Trader *op, FutureAccount *fa) {
	
	int count_number = 0;
	int trader_count_number = 0;

	if (op != NULL) {
		trader_count_number = this->conn->count(DB_OPERATOR_COLLECTION,
			BSON("traderid" << op->getTraderID()));
		if (trader_count_number == 0) { //交易员id不存在
			cout << "Trader ID Not Exists!" << endl;
		}
		else { //交易员存在
			count_number = this->conn->count(DB_FUTUREACCOUNT_COLLECTION,
				BSON("userid" << fa->getUserID()));

			if (count_number > 0) { //期货账户已经存在
				cout << "UserID Already Exists!" << endl;
			}
			else {
				BSONObjBuilder b;

				b.append("brokerid", fa->getBrokerID());
				b.append("traderid", op->getTraderID());
				b.append("password", fa->getPassword());
				b.append("userid", fa->getUserID());
				b.append("frontaddress", fa->getFrontAddress());
				b.append("isactive", fa->getIsActive());

				BSONObj p = b.obj();
				conn->insert(DB_FUTUREACCOUNT_COLLECTION, p);
				USER_PRINT("DBManager::CreateFutureAccount ok");
			}
		}
	}	
}
void DBManager::DeleteFutureAccount(FutureAccount *fa) {
	this->conn->update(DB_FUTUREACCOUNT_COLLECTION, BSON("userid" << (fa->getUserID().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
	USER_PRINT("DBManager::DeleteFutureAccount ok");
}
void DBManager::UpdateFutureAccount(string userid, Trader *op, FutureAccount *fa) {
	this->conn->update(DB_FUTUREACCOUNT_COLLECTION, BSON("userid" << (userid.c_str())), BSON("$set" << BSON("userid" << fa->getUserID() << "brokerid" << fa->getBrokerID() << "traderid" << op->getTraderID() << "password" << fa->getPassword() << "frontaddress" << fa->getFrontAddress() << "isactive" << fa->getIsActive())));
	USER_PRINT("DBManager::UpdateOperator ok");
}
void DBManager::SearchFutrueByUserID(string userid) {
	unique_ptr<DBClientCursor> cursor =
		this->conn->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("userid" << userid));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		cout << "brokerid = " << p.getStringField("brokerid") << endl;
		cout << "traderid = " << p.getStringField("traderid") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "userid = " << p.getStringField("userid") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;
	}
	USER_PRINT("DBManager::SearchFutrueByUserID ok");
}

void DBManager::SearchFutrueByTraderID(string traderid) {
	unique_ptr<DBClientCursor> cursor =
		this->conn->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("traderid" << traderid));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		cout << "brokerid = " << p.getStringField("brokerid") << endl;
		cout << "traderid = " << p.getStringField("traderid") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "userid = " << p.getStringField("userid") << endl;
		cout << "frontAddress = " << p.getStringField("frontaddress") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;
	}
	USER_PRINT("DBManager::SearchFutrueByTraderName ok");
}

void DBManager::SearchFutrueListByTraderID(string traderid, list<FutureAccount *> *l_futureaccount) {

	/*list<FutureAccount *>::iterator Itor;
	for (Itor = l_futureaccount->begin(); Itor != l_futureaccount->end();) {
	Itor = l_futureaccount->erase(Itor);
	}*/
	if (l_futureaccount->size() > 0) {
		l_futureaccount->clear();
	}

	unique_ptr<DBClientCursor> cursor =
		this->conn->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("traderid" << traderid));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		FutureAccount *fa = new FutureAccount();
		/*cout << "brokerid = " << p.getStringField("brokerid") << endl;
		cout << "traderid = " << p.getStringField("traderid") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "userid = " << p.getStringField("userid") << endl;
		cout << "frontAddress = " << p.getStringField("frontaddress") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;*/
		fa->setBrokerID(p.getStringField("brokerid"));
		fa->setTraderID(p.getStringField("traderid"));
		fa->setPassword(p.getStringField("password"));
		fa->setUserID(p.getStringField("userid"));
		fa->setFrontAddress(p.getStringField("frontaddress"));
		fa->setIsActive(p.getStringField("isactive"));
		l_futureaccount->push_back(fa);
	}
	USER_PRINT("DBManager::SearchFutrueByTraderName ok");
}

void DBManager::getAllFutureAccount(list<User *> *l_user) {

	/// 初始化的时候，必须保证list为空
	if (l_user->size() > 0) {
		list<User *>::iterator user_itor;
		for (user_itor = l_user->begin(); user_itor != l_user->end();) {
			user_itor = l_user->erase(user_itor);
		}
	}

	int countnum = this->conn->count(DB_FUTUREACCOUNT_COLLECTION);
	if (countnum == 0) {
		cout << "DBManager::getAllFutureAccount None!" << endl;
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			this->conn->query(DB_FUTUREACCOUNT_COLLECTION);
		while (cursor->more()) {
			BSONObj p = cursor->next();
			cout << "*" << "brokerid:" << p.getStringField("brokerid") << "  "
				<< "traderid:" << p.getStringField("traderid") << "  "
				<< "password:" << p.getStringField("password") << "  "
				<< "userid:" << p.getStringField("userid") << "  "
				<< "frontAddress:" << p.getStringField("frontaddress") << "  "
				<< "isactive:" << p.getStringField("isactive") << "*" << endl;
			User *user = new User(p.getStringField("frontaddress"), p.getStringField("brokerid"), p.getStringField("userid"), p.getStringField("password"), p.getStringField("userid"), p.getStringField("traderid"));
			l_user->push_back(user);
		}
		
		USER_PRINT("DBManager::getAllFutureAccount ok");
	}
}

void DBManager::CreateStrategy(Strategy *stg) {
	int count_number = 0;

	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategyid" << stg->getStrategyId()));

	if (count_number > 0) {
		cout << "Strategy Already Exists!" << endl;
	}
	else {
		BSONObjBuilder b;
		b.append("strategyid", stg->getStrategyId());
		b.append("userid", stg->getUserID());
		b.append("traderid", stg->getTraderID());
		b.append("isactive", stg->getIsActive());
		BSONObj p = b.obj();

		conn->insert(DB_STRATEGY_COLLECTION, p);
		USER_PRINT("DBManager::CreateStrategy ok");
	}
}
void DBManager::DeleteStrategy(Strategy *stg) {
	int count_number = 0;

	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategyid" << stg->getStrategyId()));

	if (count_number > 0) {
		this->conn->update(DB_STRATEGY_COLLECTION, BSON("strategyid" << (stg->getStrategyId().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeleteStrategy ok");
	}
	else {
		cout << "Strategy ID Not Exists!" << endl;
	}
}
void DBManager::UpdateStrategy(Strategy *stg) {
	int count_number = 0;

	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategyid" << stg->getStrategyId()));

	if (count_number > 0) {
		this->conn->update(DB_STRATEGY_COLLECTION, BSON("strategyid" << (stg->getStrategyId().c_str())), BSON("$set" << BSON("strategyid" << stg->getStrategyId() << "userid" << stg->getUserID() << "traderid" << stg->getTraderID() << "isactive" << stg->getIsActive())));
		USER_PRINT("DBManager::UpdateStrategy ok");
	}
	else
	{
		cout << "Strategy ID Not Exists!" << endl;
	}
}
void DBManager::getAllStragegy(list<Strategy *> *l_strategys) {
	/// 初始化的时候，必须保证list为空
	if (l_strategys->size() > 0) {
		list<Strategy *>::iterator Itor;
		for (Itor = l_strategys->begin(); Itor != l_strategys->end();) {
			Itor = l_strategys->erase(Itor);
		}
	}

	unique_ptr<DBClientCursor> cursor =
		this->conn->query(DB_STRATEGY_COLLECTION);
	while (cursor->more()) {
		BSONObj p = cursor->next();
		Strategy *stg = new Strategy();
		cout << "strategyid = " << p.getStringField("strategyid") << endl;
		cout << "traderid = " << p.getStringField("traderid") << endl;
		cout << "userid = " << p.getStringField("userid") << endl;

		stg->setStrategyId(p.getStringField("strategyid"));
		stg->setTraderID(p.getStringField("traderid"));
		stg->setUserID(p.getStringField("userid"));
		
		l_strategys->push_back(stg);
	}

	USER_PRINT("DBManager::getAllStragegy ok");
}

/************************************************************************/
/* 创建MD配置
删除MD配置
更新MD配置
获取所有MD配置
获取一条MD记录															*/
/************************************************************************/
void DBManager::CreateMarketConfig(MarketConfig *mc) {
	int count_number = 0;

	count_number = this->conn->count(DB_MARKETCONFIG_COLLECTION,
		BSON("market_id" << mc->getMarketID()));

	if (count_number > 0) {
		cout << "MarketConfig Already Exists!" << endl;
	}
	else {
		BSONObjBuilder b;
		b.append("market_id", mc->getMarketID());
		b.append("market_frontAddr", mc->getMarketFrontAddr());
		b.append("broker_id", mc->getBrokerID());
		b.append("user_id", mc->getUserID());
		b.append("password", mc->getPassword());
		b.append("isactive", ISACTIVE);
		BSONObj p = b.obj();

		conn->insert(DB_MARKETCONFIG_COLLECTION, p);
		USER_PRINT("DBManager::CreateMarketConfig ok");
	}
}

void DBManager::DeleteMarketConfig(MarketConfig *mc) {
	int count_number = 0;

	count_number = this->conn->count(DB_MARKETCONFIG_COLLECTION,
		BSON("market_id" << mc->getMarketID()));

	if (count_number > 0) {
		this->conn->update(DB_MARKETCONFIG_COLLECTION, BSON("market_id" << (mc->getMarketID().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeleteMarketConfig ok");
	}
	else {
		cout << "MarketConfig ID Not Exists!" << endl;
	}
}

void DBManager::UpdateMarketConfig(MarketConfig *mc) {
	int count_number = 0;

	count_number = this->conn->count(DB_MARKETCONFIG_COLLECTION,
		BSON("market_id" << mc->getMarketID()));

	if (count_number > 0) {
		this->conn->update(DB_MARKETCONFIG_COLLECTION, BSON("market_id" << (mc->getMarketID().c_str())), BSON("$set" << BSON("market_id" << mc->getMarketID() << "market_frontAddr" << mc->getMarketFrontAddr() << "broker_id" << mc->getBrokerID() << "user_id" << mc->getUserID() << "password" << mc->getPassword() << "isactive" << mc->getIsActive())));
		USER_PRINT("DBManager::UpdateMarketConfig ok");
	}
	else
	{
		cout << "MarketConfig ID Not Exists!" << endl;
	}
}

void DBManager::getAllMarketConfig(list<MarketConfig *> *l_marketconfig) {
	/// 初始化的时候，必须保证list为空
	if (l_marketconfig->size() > 0) {
		list<MarketConfig *>::iterator market_itor;
		for (market_itor = l_marketconfig->begin(); market_itor != l_marketconfig->end();) {
			market_itor = l_marketconfig->erase(market_itor);
		}
	}

	int countnum = this->conn->count(DB_MARKETCONFIG_COLLECTION);
	if (countnum == 0) {
		cout << "DBManager::getAllMarketConfig None!" << endl;
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			this->conn->query(DB_MARKETCONFIG_COLLECTION);
		while (cursor->more()) {
			BSONObj p = cursor->next();
			cout << "*" << "market_id:" << p.getStringField("market_id") << "  "
				<< "market_frontAddr:" << p.getStringField("market_frontAddr") << "  "
				<< "broker_id:" << p.getStringField("broker_id") << "  "
				<< "userid:" << p.getStringField("user_id") << "  "
				<< "password:" << p.getStringField("password") << "  "
				<< "isactive:" << p.getStringField("isactive") << "*" << endl;
			MarketConfig *mc = new MarketConfig();
			mc->setMarketID(p.getStringField("market_id"));
			mc->setMarketFrontAddr(p.getStringField("market_frontAddr"));
			mc->setBrokerID(p.getStringField("broker_id"));
			mc->setUserID(p.getStringField("user_id"));
			mc->setPassword(p.getStringField("password"));
			mc->setIsActive(p.getStringField("isactive"));

			l_marketconfig->push_back(mc);
		}
		USER_PRINT("DBManager::getAllMarketConfig ok");
	}
}

MarketConfig * DBManager::getOneMarketConfig() {
	int countnum = this->conn->count(DB_MARKETCONFIG_COLLECTION);
	if (countnum == 0) {
		cout << "DBManager::getOneMarketConfig None!" << endl;
	}
	else {
		cout << "isactive" << ISACTIVE << endl;
		BSONObj p = this->conn->findOne(DB_MARKETCONFIG_COLLECTION, BSON("isactive" << ISACTIVE));
		cout << "market_id = " << p.getStringField("market_id") << endl;
		if ((strcmp(p.getStringField("market_id"), ""))) {
			cout << "*" << "market_id:" << p.getStringField("market_id") << "  "
				<< "market_frontAddr:" << p.getStringField("market_frontAddr") << "  "
				<< "broker_id:" << p.getStringField("broker_id") << "  "
				<< "userid:" << p.getStringField("user_id") << "  "
				<< "password:" << p.getStringField("password") << "  "
				<< "isactive:" << p.getStringField("isactive") << "*" << endl;
			USER_PRINT("DBManager::getOneMarketConfig ok");
			return new MarketConfig(p.getStringField("market_id"), p.getStringField("market_frontAddr"), p.getStringField("broker_id"),
				p.getStringField("user_id"), p.getStringField("password"), p.getStringField("isactive"));
		}
		else {
			USER_PRINT("DBManager::getOneMarketConfig None");
		}
	}
	return NULL;
}


void DBManager::setConn(mongo::DBClientConnection *conn) {
	this->conn = conn;
}
mongo::DBClientConnection * DBManager::getConn() {
	return this->conn;
}
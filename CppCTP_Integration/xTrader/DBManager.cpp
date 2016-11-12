#include <mongo/bson/bson.h>
#include <iostream>
#include "DBManager.h"
#include "Debug.h"

using namespace std;
using mongo::BSONArray;
using mongo::BSONArrayBuilder;
using mongo::BSONObj;
using mongo::BSONObjBuilder;
using mongo::BSONElement;
using mongo::ConnectException;
using std::cout;
using std::endl;
using std::list;
using std::vector;
using std::auto_ptr;
using std::unique_ptr;


#define DB_OPERATOR_COLLECTION            "CTP.trader"
#define DB_FUTUREACCOUNT_COLLECTION       "CTP.futureaccount"
#define DB_ADMIN_COLLECTION               "CTP.admin"
#define DB_STRATEGY_COLLECTION            "CTP.strategy"
#define DB_MARKETCONFIG_COLLECTION        "CTP.marketconfig"
#define DB_STRATEGY_YESTERDAY_COLLECTION  "CTP.strategy_yesterday"
#define ISACTIVE "1"
#define ISNOTACTIVE "0"

DBManager::DBManager() {
	try
	{
		this->conn = new mongo::DBClientConnection(false, 0, 5);
		this->conn->connect("localhost");
		USER_PRINT("Original DB Connection[DBManager::DBManager()]!");
		USER_PRINT(conn);
	}
	catch (const mongo::SocketException& e)
	{
		std::cout << "MongoDB无法访问!" << std::endl;
	}
	catch (const mongo::ConnectException& e) {
		std::cout << "MongoDB无法访问!" << std::endl;
	}
	
	//USER_PRINT(this->conn);
}

DBManager::~DBManager() {
	delete this;
}


/************************************************************************/
/* static method return mongo connection                                */
/************************************************************************/
mongo::DBClientConnection * DBManager::getDBConnection() {
	try
	{
		mongo::DBClientConnection *conn = new mongo::DBClientConnection(false, 0, 5);
		conn->connect("localhost");
		USER_PRINT("Original DB Connection[DBManager::getDBConnection()]!");
		USER_PRINT(conn);
		return conn;
	}
	catch (const mongo::SocketException& e)
	{
		std::cout << "MongoDB无法访问!" << std::endl;
		return NULL;
	}
	catch (const mongo::ConnectException& e) {
		std::cout << "MongoDB无法访问!" << std::endl;
		return NULL;
	}
}

// 创建Trader
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
		b.append("on_off", op->getOn_Off());
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
		this->conn->update(DB_OPERATOR_COLLECTION, BSON("traderid" << (traderid.c_str())), BSON("$set" << BSON("tradername" << op->getTraderName() << "traderid" << op->getTraderID() << "password" << op->getPassword() << "on_off" << op->getOn_Off() << "isactive" << op->getIsActive())));
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
		cout << "on_off = " << p.getIntField("on_off") << endl;
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
		cout << "on_off = " << p.getIntField("on_off") << endl;
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
		cout << "on_off = " << p.getIntField("on_off") << endl;
	}
	USER_PRINT("DBManager::SearchTraderByTraderIdAndPassword ok");
}

bool DBManager::FindTraderByTraderIdAndPassword(string traderid, string password, Trader *op) {
	int count_number = 0;
	bool flag = false;

	count_number = this->conn->count(DB_OPERATOR_COLLECTION,
		BSON("traderid" << traderid.c_str() << "password" << password.c_str() << "isactive" << ISACTIVE));

	if (count_number == 0) {
		flag = false;
	} else {
		flag = true;
		unique_ptr<DBClientCursor> cursor =
			this->conn->query(DB_OPERATOR_COLLECTION, MONGO_QUERY("traderid" << traderid << "password" << password << "isactive" << ISACTIVE));
		while (cursor->more()) {
			BSONObj p = cursor->next();
			cout << "traderid = " << p.getStringField("traderid") << endl;
			cout << "tradername = " << p.getStringField("tradername") << endl;
			cout << "password = " << p.getStringField("password") << endl;
			cout << "isactive = " << p.getStringField("isactive") << endl;
			cout << "on_off = " << p.getIntField("on_off") << endl;
			op->setTraderID(p.getStringField("traderid"));
			op->setTraderName(p.getStringField("tradername"));
			op->setPassword(p.getStringField("password"));
			op->setOn_Off(p.getIntField("on_off"));
		}


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
				<< "isactive:" << p.getStringField("isactive") << " "
				<< "on_off" << p.getIntField("on_off") << "*" << endl;

			l_trader->push_back(p.getStringField("traderid"));
		}
		USER_PRINT("DBManager::getAllTrader1 ok");
	}
}

void DBManager::getAllObjTrader(list<Trader *> *l_trader) {
	/// 初始化的时候，必须保证list为空
	if (l_trader->size() > 0) {
		list<Trader *>::iterator Itor;
		for (Itor = l_trader->begin(); Itor != l_trader->end();) {
			delete (*Itor);
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
			Trader *op = new Trader();
			BSONObj p = cursor->next();
			cout << "*" << "traderid:" << p.getStringField("traderid") << "  "
				<< "tradername:" << p.getStringField("tradername") << "  "
				<< "password:" << p.getStringField("password") << "  "
				<< "isactive:" << p.getStringField("isactive") << " "
				<< "on_off" << p.getIntField("on_off") << "*" << endl;

			op->setTraderID(p.getStringField("traderid"));
			op->setTraderName(p.getStringField("tradername"));
			op->setPassword(p.getStringField("password"));
			op->setIsActive(p.getStringField("isactive"));
			op->setOn_Off(p.getIntField("on_off"));

			l_trader->push_back(op);
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
				b.append("on_off", op->getOn_Off());

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
void DBManager::UpdateFutureAccount(User *u) {
	this->conn->update(DB_FUTUREACCOUNT_COLLECTION, BSON("userid" << (u->getUserID().c_str())), BSON("$set" << BSON("userid" << u->getUserID() << "brokerid" << u->getBrokerID() << "traderid" << u->getTraderID() << "password" << u->getPassword() << "frontaddress" << u->getFrontAddress() << "isactive" << u->getIsActive() << "on_off" << u->getOn_Off())));
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
		cout << "on_off = " << p.getIntField("on_off") << endl;
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
		cout << "on_off = " << p.getIntField("on_off") << endl;
	}
	USER_PRINT("DBManager::SearchFutrueByTraderName ok");
}

void DBManager::SearchFutrueListByTraderID(string traderid, list<FutureAccount *> *l_futureaccount) {

	/*list<FutureAccount *>::iterator Itor;
	for (Itor = l_futureaccount->begin(); Itor != l_futureaccount->end();) {
	Itor = l_futureaccount->erase(Itor);
	}*/
	if (l_futureaccount->size() > 0) {
		list<FutureAccount *>::iterator future_itor;
		for (future_itor = l_futureaccount->begin(); future_itor != l_futureaccount->end();) {
			delete (*future_itor);
			future_itor = l_futureaccount->erase(future_itor);
		}
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
		fa->setOn_Off(p.getIntField("on_off"));
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
			User *user = new User(p.getStringField("frontaddress"), p.getStringField("brokerid"), p.getStringField("userid"), p.getStringField("password"), p.getStringField("userid"), p.getIntField("on_off"), p.getStringField("traderid"));
			l_user->push_back(user);
		}
		
		USER_PRINT("DBManager::getAllFutureAccount ok");
	}
}

void DBManager::CreateStrategy(Strategy *stg) {
	int count_number = 0;

	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str())));

	if (count_number > 0) {
		cout << "Strategy Already Exists!" << endl;
	}
	else {
		BSONObjBuilder b;
		// 增加属性
		b.append("position_a_sell_today", stg->getStgPositionASellToday());
		b.append("position_b_sell", stg->getStgPositionBSell());
		b.append("spread_shift", stg->getStgSpreadShift());
		b.append("position_b_sell_today", stg->getStgPositionBSellToday());
		b.append("position_b_buy_today", stg->getStgPositionBBuyToday());
		b.append("position_a_sell", stg->getStgPositionASell());
		b.append("buy_close", stg->getStgBuyClose());
		b.append("stop_loss", stg->getStgStopLoss());
		b.append("position_b_buy_yesterday", stg->getStgPositionBBuyYesterday());
		b.append("is_active", stg->isStgIsActive());
		b.append("position_b_sell_yesterday", stg->getStgPositionBSell());
		b.append("strategy_id", stg->getStgStrategyId());
		b.append("position_b_buy", stg->getStgPositionBBuy());
		b.append("lots_batch", stg->getStgLotsBatch());
		b.append("position_a_buy", stg->getStgPositionABuy());
		b.append("sell_open", stg->getStgSellOpen());
		b.append("order_algorithm", stg->getStgOrderAlgorithm());
		b.append("trader_id", stg->getStgTraderId());
		b.append("order_action_tires_limit", stg->getStgOrderActionTiresLimit());
		b.append("sell_close", stg->getStgSellClose());
		b.append("buy_open", stg->getStgBuyOpen());
		//b.append("only_close", stg->isStgOnlyClose());

		// 创建一个数组对象
		BSONArrayBuilder bab;
		bab.append(stg->getStgInstrumentIdA());
		bab.append(stg->getStgInstrumentIdB());

		b.appendArray("list_instrument_id", bab.arr());

		b.append("position_a_buy_yesterday", stg->getStgPositionABuyYesterday());
		b.append("user_id", stg->getStgUserId());
		b.append("position_a_buy_today", stg->getStgPositionABuyToday());
		b.append("position_a_sell_yesterday", stg->getStgPositionASellYesterday());
		b.append("lots", stg->getStgLotsBatch());
		b.append("a_wait_price_tick", stg->getStgAWaitPriceTick());
		b.append("b_wait_price_tick", stg->getStgBWaitPriceTick());
		b.append("StrategyOnoff", stg->getOn_Off());

		BSONObj p = b.obj();

		conn->insert(DB_STRATEGY_COLLECTION, p);
		USER_PRINT("DBManager::CreateStrategy ok");
	}
}

void DBManager::DeleteStrategy(Strategy *stg) {
	int count_number = 0;

	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str()));

	if (count_number > 0) {
		this->conn->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId())), BSON("$set" << BSON("is_active" << false)));
		USER_PRINT("DBManager::DeleteStrategy ok");
	}
	else {
		cout << "Strategy ID Not Exists!" << endl;
	}
}
void DBManager::UpdateStrategy(Strategy *stg) {
	int count_number = 0;

	std::cout << "DBManager::UpdateStrategy" << std::endl;


	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str()));

	std::cout << "count_number = " << count_number << std::endl;

	if (count_number > 0) {
		this->conn->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str())), BSON("$set" << BSON("position_a_sell_today" << stg->getStgPositionASellToday() 
			<< "position_b_sell" << stg->getStgPositionBSell() 
			<< "spread_shift" << stg->getStgSpreadShift()
			<< "position_b_sell_today" << stg->getStgPositionBSellToday()
			<< "position_b_buy_today" << stg->getStgPositionBBuyToday()
			<< "position_a_sell" << stg->getStgPositionASell()
			<< "buy_close" << stg->getStgBuyClose()
			<< "stop_loss" << stg->getStgStopLoss()
			<< "position_b_buy_yesterday" << stg->getStgPositionBBuyYesterday()
			<< "is_active" << stg->isStgIsActive()
			<< "position_b_sell_yesterday" << stg->getStgPositionBSell()
			<< "strategy_id" << stg->getStgStrategyId()
			<< "position_b_buy" << stg->getStgPositionBBuy() 
			<< "lots_batch" << stg->getStgLotsBatch() 
			<< "position_a_buy" << stg->getStgPositionABuy() 
			<< "sell_open" << stg->getStgSellOpen() 
			<< "order_algorithm" << stg->getStgOrderAlgorithm() 
			<< "trader_id" << stg->getStgTraderId() 
			<< "order_action_tires_limit" << stg->getStgOrderActionTiresLimit() 
			<< "sell_close" << stg->getStgSellClose() 
			<< "buy_open" << stg->getStgBuyOpen() 
			//<< "only_close" << stg->isStgOnlyClose() 
			<< "position_a_buy_yesterday" << stg->getStgPositionABuyYesterday() 
			<< "user_id" << stg->getStgUserId() 
			<< "position_a_buy_today" << stg->getStgPositionABuyToday() 
			<< "position_a_sell_yesterday" << stg->getStgPositionASellYesterday() 
			<< "lots" << stg->getStgLotsBatch() 
			<< "a_wait_price_tick" << stg->getStgAWaitPriceTick() 
			<< "b_wait_price_tick" << stg->getStgBWaitPriceTick()
			<< "StrategyOnoff" << stg->getOn_Off()
			<< "list_instrument_id" << BSON_ARRAY(stg->getStgInstrumentIdA() << stg->getStgInstrumentIdB()))));

		USER_PRINT("DBManager::UpdateStrategy ok");
	}
	else
	{
		cout << "Strategy ID Not Exists!" << endl;
	}
}
void DBManager::getAllStrategy(list<Strategy *> *l_strategys, string traderid, string userid) {
	/// 初始化的时候，必须保证list为空
	if (l_strategys->size() > 0) {
		list<Strategy *>::iterator Itor;
		for (Itor = l_strategys->begin(); Itor != l_strategys->end();) {
			delete (*Itor);
			Itor = l_strategys->erase(Itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	if (traderid.compare("")) { //如果traderid不为空
		
		if (userid.compare("")) { //如果userid不为空
			cursor = this->conn->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid));
		}
		else {
			cursor = this->conn->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("trader_id" << traderid));
		}
	}
	else {
		cursor = this->conn->query(DB_STRATEGY_COLLECTION);
	}
		
	while (cursor->more()) {
		BSONObj p = cursor->next();
		Strategy *stg = new Strategy();
		cout << "position_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
		cout << "position_b_sell = " << p.getIntField("position_b_sell") << ", ";
		cout << "spread_shift = " << p.getField("spread_shift").Double() << ", ";
		cout << "position_b_sell_today = " << p.getIntField("position_b_sell_today") << ", ";
		cout << "position_b_buy_today = " << p.getIntField("position_b_buy_today") << ", ";
		cout << "position_a_sell = " << p.getIntField("position_a_sell") << ", ";
		cout << "buy_close = " << p.getField("buy_close").Double() << ", ";
		cout << "stop_loss = " << p.getField("stop_loss").Double() << ", ";
		cout << "position_b_buy_yesterday = " << p.getIntField("position_b_buy_yesterday") << ", ";
		cout << "is_active = " << p.getField("is_active").Bool() << ", ";
		cout << "position_b_sell_yesterday = " << p.getIntField("position_b_sell_yesterday") << ", ";
		cout << "strategy_id = " << p.getStringField("strategy_id") << ", "; //string type
		cout << "position_b_buy = " << p.getIntField("position_b_buy") << ", ";
		cout << "lots_batch = " << p.getIntField("lots_batch") << ", ";
		cout << "position_a_buy = " << p.getIntField("position_a_buy") << ", ";
		cout << "sell_open = " << p.getField("sell_open").Double() << ", ";
		cout << "order_algorithm = " << p.getStringField("order_algorithm") << ", "; //string type
		cout << "trader_id = " << p.getStringField("trader_id") << ", "; // string type
		cout << "order_action_tires_limit = " << p.getIntField("order_action_tires_limit") << ", ";
		cout << "sell_close = " << p.getField("sell_close").Double() << ", ";
		cout << "buy_open = " << p.getField("buy_open").Double() << ", ";
		//cout << "only_close = " << p.getField("only_close").Bool() << ", ";
		cout << "position_a_buy_yesterday = " << p.getIntField("position_a_buy_yesterday") << ", ";
		cout << "user_id = " << p.getStringField("user_id") << ", "; // string type
		cout << "position_a_buy_today = " << p.getIntField("position_a_buy_today") << ", ";
		cout << "position_a_sell_yesterday = " << p.getIntField("position_a_sell_yesterday") << ", ";
		cout << "lots = " << p.getIntField("lots") << ", ";
		cout << "a_wait_price_tick = " << p.getField("a_wait_price_tick").Double() << ", ";
		cout << "b_wait_price_tick = " << p.getField("b_wait_price_tick").Double() << ", ";
		cout << "StrategyOnoff = " << p.getField("StrategyOnoff").Int() << endl;

		stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
		stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
		stg->setStgBuyClose(p.getField("buy_close").Double());
		stg->setStgBuyOpen(p.getField("buy_open").Double());
		stg->setStgIsActive(p.getField("is_active").Bool());
		stg->setStgLots(p.getIntField("lots"));
		stg->setStgLotsBatch(p.getIntField("lots_batch"));
		//stg->setStgOnlyClose(p.getField("only_close").Bool());
		stg->setStgOrderActionTiresLimit(p.getIntField("order_action_tires_limit"));
		stg->setStgOrderAlgorithm(p.getStringField("order_algorithm"));
		stg->setStgPositionABuy(p.getIntField("position_a_buy"));
		stg->setStgPositionABuyToday(p.getIntField("position_a_buy_today"));
		stg->setStgPositionABuyYesterday(p.getIntField("position_a_buy_yesterday"));
		stg->setStgPositionASell(p.getIntField("position_a_sell"));
		stg->setStgPositionASellToday(p.getIntField("position_a_sell_today"));
		stg->setStgPositionASellYesterday(p.getIntField("position_a_sell_yesterday"));
		stg->setStgPositionBBuy(p.getIntField("position_b_buy"));
		stg->setStgPositionBBuyToday(p.getIntField("position_b_buy_today"));
		stg->setStgPositionBBuyYesterday(p.getIntField("position_b_buy_yesterday"));
		stg->setStgPositionBSell(p.getIntField("position_b_sell"));
		stg->setStgPositionBSellToday(p.getIntField("position_b_sell_today"));
		stg->setStgPositionBSellYesterday(p.getIntField("position_b_sell_yesterday"));
		stg->setStgSellClose(p.getField("sell_close").Double());
		stg->setStgSellOpen(p.getField("sell_open").Double());
		stg->setStgSpreadShift(p.getField("spread_shift").Double());
		stg->setStgStopLoss(p.getField("stop_loss").Double());
		stg->setStgStrategyId(p.getStringField("strategy_id"));
		stg->setStgTraderId(p.getStringField("trader_id"));
		stg->setStgUserId(p.getStringField("user_id"));
		stg->setOn_Off(p.getField("StrategyOnoff").Int());


		vector<BSONElement> elements = p["list_instrument_id"].Array();
		if (elements.size() > 1) {
			stg->setStgInstrumentIdA(elements[0].String());
			stg->setStgInstrumentIdB(elements[1].String());
			stg->addInstrumentToList(stg->getStgInstrumentIdA());
			stg->addInstrumentToList(stg->getStgInstrumentIdB());
			cout << "stg->setStgInstrumentIdA(elements[0])" << stg->getStgInstrumentIdA() << endl;
			cout << "stg->setStgInstrumentIdA(elements[1])" << stg->getStgInstrumentIdB() << endl;
		}

		//for (vector<BSONElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
		//	cout << *it << endl;
		//}
		
		l_strategys->push_back(stg);
	}

	USER_PRINT("DBManager::getAllStragegy ok");
}


/************************************************************************/
/* 创建策略(昨仓)
删除策略(昨仓)
更新策略(昨仓)
查找策略(昨仓)			                                                */
/************************************************************************/
void DBManager::CreateStrategyYesterday(Strategy *stg) {
	int count_number = 0;

	count_number = this->conn->count(DB_STRATEGY_YESTERDAY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str())));

	if (count_number > 0) {
		cout << "Strategy Already Exists!" << endl;
	}
	else {
		BSONObjBuilder b;
		// 增加属性
		b.append("position_a_sell_today", stg->getStgPositionASellToday());
		b.append("position_b_sell", stg->getStgPositionBSell());
		b.append("spread_shift", stg->getStgSpreadShift());
		b.append("position_b_sell_today", stg->getStgPositionBSellToday());
		b.append("position_b_buy_today", stg->getStgPositionBBuyToday());
		b.append("position_a_sell", stg->getStgPositionASell());
		b.append("buy_close", stg->getStgBuyClose());
		b.append("stop_loss", stg->getStgStopLoss());
		b.append("position_b_buy_yesterday", stg->getStgPositionBBuyYesterday());
		b.append("is_active", stg->isStgIsActive());
		b.append("position_b_sell_yesterday", stg->getStgPositionBSell());
		b.append("strategy_id", stg->getStgStrategyId());
		b.append("position_b_buy", stg->getStgPositionBBuy());
		b.append("lots_batch", stg->getStgLotsBatch());
		b.append("position_a_buy", stg->getStgPositionABuy());
		b.append("sell_open", stg->getStgSellOpen());
		b.append("order_algorithm", stg->getStgOrderAlgorithm());
		b.append("trader_id", stg->getStgTraderId());
		b.append("order_action_tires_limit", stg->getStgOrderActionTiresLimit());
		b.append("sell_close", stg->getStgSellClose());
		b.append("buy_open", stg->getStgBuyOpen());
		//b.append("only_close", stg->isStgOnlyClose());

		// 创建一个数组对象
		BSONArrayBuilder bab;
		bab.append(stg->getStgInstrumentIdA());
		bab.append(stg->getStgInstrumentIdB());

		b.appendArray("list_instrument_id", bab.arr());

		b.append("position_a_buy_yesterday", stg->getStgPositionABuyYesterday());
		b.append("user_id", stg->getStgUserId());
		b.append("position_a_buy_today", stg->getStgPositionABuyToday());
		b.append("position_a_sell_yesterday", stg->getStgPositionASellYesterday());
		b.append("lots", stg->getStgLotsBatch());
		b.append("a_wait_price_tick", stg->getStgAWaitPriceTick());
		b.append("b_wait_price_tick", stg->getStgBWaitPriceTick());
		b.append("StrategyOnoff", stg->getOn_Off());

		BSONObj p = b.obj();

		conn->insert(DB_STRATEGY_YESTERDAY_COLLECTION, p);
		USER_PRINT("DBManager::CreateStrategy ok");
	}
}

void DBManager::DeleteStrategyYesterday(Strategy *stg) {
	int count_number = 0;

	count_number = this->conn->count(DB_STRATEGY_YESTERDAY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str()));

	if (count_number > 0) {
		this->conn->update(DB_STRATEGY_YESTERDAY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId())), BSON("$set" << BSON("is_active" << false)));
		USER_PRINT("DBManager::DeleteStrategy ok");
	}
	else {
		cout << "Strategy ID Not Exists!" << endl;
	}
}

void DBManager::UpdateStrategyYesterday(Strategy *stg) {
	int count_number = 0;

	std::cout << "DBManager::UpdateStrategy" << std::endl;


	count_number = this->conn->count(DB_STRATEGY_YESTERDAY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str()));

	std::cout << "count_number = " << count_number << std::endl;

	if (count_number > 0) {
		this->conn->update(DB_STRATEGY_YESTERDAY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str())), BSON("$set" << BSON("position_a_sell_today" << stg->getStgPositionASellToday()
			<< "position_b_sell" << stg->getStgPositionBSell()
			<< "spread_shift" << stg->getStgSpreadShift()
			<< "position_b_sell_today" << stg->getStgPositionBSellToday()
			<< "position_b_buy_today" << stg->getStgPositionBBuyToday()
			<< "position_a_sell" << stg->getStgPositionASell()
			<< "buy_close" << stg->getStgBuyClose()
			<< "stop_loss" << stg->getStgStopLoss()
			<< "position_b_buy_yesterday" << stg->getStgPositionBBuyYesterday()
			<< "is_active" << stg->isStgIsActive()
			<< "position_b_sell_yesterday" << stg->getStgPositionBSell()
			<< "strategy_id" << stg->getStgStrategyId()
			<< "position_b_buy" << stg->getStgPositionBBuy()
			<< "lots_batch" << stg->getStgLotsBatch()
			<< "position_a_buy" << stg->getStgPositionABuy()
			<< "sell_open" << stg->getStgSellOpen()
			<< "order_algorithm" << stg->getStgOrderAlgorithm()
			<< "trader_id" << stg->getStgTraderId()
			<< "order_action_tires_limit" << stg->getStgOrderActionTiresLimit()
			<< "sell_close" << stg->getStgSellClose()
			<< "buy_open" << stg->getStgBuyOpen()
			//<< "only_close" << stg->isStgOnlyClose() 
			<< "position_a_buy_yesterday" << stg->getStgPositionABuyYesterday()
			<< "user_id" << stg->getStgUserId()
			<< "position_a_buy_today" << stg->getStgPositionABuyToday()
			<< "position_a_sell_yesterday" << stg->getStgPositionASellYesterday()
			<< "lots" << stg->getStgLotsBatch()
			<< "a_wait_price_tick" << stg->getStgAWaitPriceTick()
			<< "b_wait_price_tick" << stg->getStgBWaitPriceTick()
			<< "StrategyOnoff" << stg->getOn_Off()
			<< "list_instrument_id" << BSON_ARRAY(stg->getStgInstrumentIdA() << stg->getStgInstrumentIdB()))));

		USER_PRINT("DBManager::UpdateStrategy ok");
	}
	else
	{
		cout << "Strategy ID Not Exists!" << endl;
	}
}

void DBManager::getAllStrategyYesterday(list<Strategy *> *l_strategys, string traderid, string userid) {
	/// 初始化的时候，必须保证list为空
	if (l_strategys->size() > 0) {
		list<Strategy *>::iterator Itor;
		for (Itor = l_strategys->begin(); Itor != l_strategys->end();) {
			delete (*Itor);
			Itor = l_strategys->erase(Itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	if (traderid.compare("")) { //如果traderid不为空

		if (userid.compare("")) { //如果userid不为空
			cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid));
		}
		else {
			cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid));
		}
	}
	else {
		cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION);
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();
		Strategy *stg = new Strategy();
		cout << "position_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
		cout << "position_b_sell = " << p.getIntField("position_b_sell") << ", ";
		cout << "spread_shift = " << p.getField("spread_shift").Double() << ", ";
		cout << "position_b_sell_today = " << p.getIntField("position_b_sell_today") << ", ";
		cout << "position_b_buy_today = " << p.getIntField("position_b_buy_today") << ", ";
		cout << "position_a_sell = " << p.getIntField("position_a_sell") << ", ";
		cout << "buy_close = " << p.getField("buy_close").Double() << ", ";
		cout << "stop_loss = " << p.getField("stop_loss").Double() << ", ";
		cout << "position_b_buy_yesterday = " << p.getIntField("position_b_buy_yesterday") << ", ";
		cout << "is_active = " << p.getField("is_active").Bool() << ", ";
		cout << "position_b_sell_yesterday = " << p.getIntField("position_b_sell_yesterday") << ", ";
		cout << "strategy_id = " << p.getStringField("strategy_id") << ", "; //string type
		cout << "position_b_buy = " << p.getIntField("position_b_buy") << ", ";
		cout << "lots_batch = " << p.getIntField("lots_batch") << ", ";
		cout << "position_a_buy = " << p.getIntField("position_a_buy") << ", ";
		cout << "sell_open = " << p.getField("sell_open").Double() << ", ";
		cout << "order_algorithm = " << p.getStringField("order_algorithm") << ", "; //string type
		cout << "trader_id = " << p.getStringField("trader_id") << ", "; // string type
		cout << "order_action_tires_limit = " << p.getIntField("order_action_tires_limit") << ", ";
		cout << "sell_close = " << p.getField("sell_close").Double() << ", ";
		cout << "buy_open = " << p.getField("buy_open").Double() << ", ";
		//cout << "only_close = " << p.getField("only_close").Bool() << ", ";
		cout << "position_a_buy_yesterday = " << p.getIntField("position_a_buy_yesterday") << ", ";
		cout << "user_id = " << p.getStringField("user_id") << ", "; // string type
		cout << "position_a_buy_today = " << p.getIntField("position_a_buy_today") << ", ";
		cout << "position_a_sell_yesterday = " << p.getIntField("position_a_sell_yesterday") << ", ";
		cout << "lots = " << p.getIntField("lots") << ", ";
		cout << "a_wait_price_tick = " << p.getField("a_wait_price_tick").Double() << ", ";
		cout << "b_wait_price_tick = " << p.getField("b_wait_price_tick").Double() << ", ";
		cout << "StrategyOnoff = " << p.getField("StrategyOnoff").Int() << endl;

		stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
		stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
		stg->setStgBuyClose(p.getField("buy_close").Double());
		stg->setStgBuyOpen(p.getField("buy_open").Double());
		stg->setStgIsActive(p.getField("is_active").Bool());
		stg->setStgLots(p.getIntField("lots"));
		stg->setStgLotsBatch(p.getIntField("lots_batch"));
		//stg->setStgOnlyClose(p.getField("only_close").Bool());
		stg->setStgOrderActionTiresLimit(p.getIntField("order_action_tires_limit"));
		stg->setStgOrderAlgorithm(p.getStringField("order_algorithm"));
		stg->setStgPositionABuy(p.getIntField("position_a_buy"));
		stg->setStgPositionABuyToday(p.getIntField("position_a_buy_today"));
		stg->setStgPositionABuyYesterday(p.getIntField("position_a_buy_yesterday"));
		stg->setStgPositionASell(p.getIntField("position_a_sell"));
		stg->setStgPositionASellToday(p.getIntField("position_a_sell_today"));
		stg->setStgPositionASellYesterday(p.getIntField("position_a_sell_yesterday"));
		stg->setStgPositionBBuy(p.getIntField("position_b_buy"));
		stg->setStgPositionBBuyToday(p.getIntField("position_b_buy_today"));
		stg->setStgPositionBBuyYesterday(p.getIntField("position_b_buy_yesterday"));
		stg->setStgPositionBSell(p.getIntField("position_b_sell"));
		stg->setStgPositionBSellToday(p.getIntField("position_b_sell_today"));
		stg->setStgPositionBSellYesterday(p.getIntField("position_b_sell_yesterday"));
		stg->setStgSellClose(p.getField("sell_close").Double());
		stg->setStgSellOpen(p.getField("sell_open").Double());
		stg->setStgSpreadShift(p.getField("spread_shift").Double());
		stg->setStgStopLoss(p.getField("stop_loss").Double());
		stg->setStgStrategyId(p.getStringField("strategy_id"));
		stg->setStgTraderId(p.getStringField("trader_id"));
		stg->setStgUserId(p.getStringField("user_id"));
		stg->setOn_Off(p.getField("StrategyOnoff").Int());


		vector<BSONElement> elements = p["list_instrument_id"].Array();
		if (elements.size() > 1) {
			stg->setStgInstrumentIdA(elements[0].String());
			stg->setStgInstrumentIdB(elements[1].String());
			stg->addInstrumentToList(stg->getStgInstrumentIdA());
			stg->addInstrumentToList(stg->getStgInstrumentIdB());
			cout << "stg->setStgInstrumentIdA(elements[0])" << stg->getStgInstrumentIdA() << endl;
			cout << "stg->setStgInstrumentIdA(elements[1])" << stg->getStgInstrumentIdB() << endl;
		}

		//for (vector<BSONElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
		//	cout << *it << endl;
		//}

		l_strategys->push_back(stg);
	}

	USER_PRINT("DBManager::getAllStragegy ok");
}

void DBManager::getAllStrategyYesterdayByTraderIdAndUserIdAndStrategyId(list<Strategy *> *l_strategys, string traderid, string userid, string strategyid) {
	/// 初始化的时候，必须保证list为空
	if (l_strategys->size() > 0) {
		list<Strategy *>::iterator Itor;
		for (Itor = l_strategys->begin(); Itor != l_strategys->end();) {
			delete (*Itor);
			Itor = l_strategys->erase(Itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	if (traderid.compare("")) { //如果traderid不为空

		if (userid.compare("")) { //如果userid不为空

			if (strategyid.compare("")) { //如果strategyid不为空
				cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid << "strategy_id" << strategyid));
			}
			else {
				cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid));
			}
			
		}
		else {
			cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid));
		}
	}
	else {
		cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION);
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();
		Strategy *stg = new Strategy();
		cout << "position_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
		cout << "position_b_sell = " << p.getIntField("position_b_sell") << ", ";
		cout << "spread_shift = " << p.getField("spread_shift").Double() << ", ";
		cout << "position_b_sell_today = " << p.getIntField("position_b_sell_today") << ", ";
		cout << "position_b_buy_today = " << p.getIntField("position_b_buy_today") << ", ";
		cout << "position_a_sell = " << p.getIntField("position_a_sell") << ", ";
		cout << "buy_close = " << p.getField("buy_close").Double() << ", ";
		cout << "stop_loss = " << p.getField("stop_loss").Double() << ", ";
		cout << "position_b_buy_yesterday = " << p.getIntField("position_b_buy_yesterday") << ", ";
		cout << "is_active = " << p.getField("is_active").Bool() << ", ";
		cout << "position_b_sell_yesterday = " << p.getIntField("position_b_sell_yesterday") << ", ";
		cout << "strategy_id = " << p.getStringField("strategy_id") << ", "; //string type
		cout << "position_b_buy = " << p.getIntField("position_b_buy") << ", ";
		cout << "lots_batch = " << p.getIntField("lots_batch") << ", ";
		cout << "position_a_buy = " << p.getIntField("position_a_buy") << ", ";
		cout << "sell_open = " << p.getField("sell_open").Double() << ", ";
		cout << "order_algorithm = " << p.getStringField("order_algorithm") << ", "; //string type
		cout << "trader_id = " << p.getStringField("trader_id") << ", "; // string type
		cout << "order_action_tires_limit = " << p.getIntField("order_action_tires_limit") << ", ";
		cout << "sell_close = " << p.getField("sell_close").Double() << ", ";
		cout << "buy_open = " << p.getField("buy_open").Double() << ", ";
		//cout << "only_close = " << p.getField("only_close").Bool() << ", ";
		cout << "position_a_buy_yesterday = " << p.getIntField("position_a_buy_yesterday") << ", ";
		cout << "user_id = " << p.getStringField("user_id") << ", "; // string type
		cout << "position_a_buy_today = " << p.getIntField("position_a_buy_today") << ", ";
		cout << "position_a_sell_yesterday = " << p.getIntField("position_a_sell_yesterday") << ", ";
		cout << "lots = " << p.getIntField("lots") << ", ";
		cout << "a_wait_price_tick = " << p.getField("a_wait_price_tick").Double() << ", ";
		cout << "b_wait_price_tick = " << p.getField("b_wait_price_tick").Double() << ", ";
		cout << "StrategyOnoff = " << p.getField("StrategyOnoff").Int() << endl;

		stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
		stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
		stg->setStgBuyClose(p.getField("buy_close").Double());
		stg->setStgBuyOpen(p.getField("buy_open").Double());
		stg->setStgIsActive(p.getField("is_active").Bool());
		stg->setStgLots(p.getIntField("lots"));
		stg->setStgLotsBatch(p.getIntField("lots_batch"));
		//stg->setStgOnlyClose(p.getField("only_close").Bool());
		stg->setStgOrderActionTiresLimit(p.getIntField("order_action_tires_limit"));
		stg->setStgOrderAlgorithm(p.getStringField("order_algorithm"));
		stg->setStgPositionABuy(p.getIntField("position_a_buy"));
		stg->setStgPositionABuyToday(p.getIntField("position_a_buy_today"));
		stg->setStgPositionABuyYesterday(p.getIntField("position_a_buy_yesterday"));
		stg->setStgPositionASell(p.getIntField("position_a_sell"));
		stg->setStgPositionASellToday(p.getIntField("position_a_sell_today"));
		stg->setStgPositionASellYesterday(p.getIntField("position_a_sell_yesterday"));
		stg->setStgPositionBBuy(p.getIntField("position_b_buy"));
		stg->setStgPositionBBuyToday(p.getIntField("position_b_buy_today"));
		stg->setStgPositionBBuyYesterday(p.getIntField("position_b_buy_yesterday"));
		stg->setStgPositionBSell(p.getIntField("position_b_sell"));
		stg->setStgPositionBSellToday(p.getIntField("position_b_sell_today"));
		stg->setStgPositionBSellYesterday(p.getIntField("position_b_sell_yesterday"));
		stg->setStgSellClose(p.getField("sell_close").Double());
		stg->setStgSellOpen(p.getField("sell_open").Double());
		stg->setStgSpreadShift(p.getField("spread_shift").Double());
		stg->setStgStopLoss(p.getField("stop_loss").Double());
		stg->setStgStrategyId(p.getStringField("strategy_id"));
		stg->setStgTraderId(p.getStringField("trader_id"));
		stg->setStgUserId(p.getStringField("user_id"));
		stg->setOn_Off(p.getField("StrategyOnoff").Int());


		vector<BSONElement> elements = p["list_instrument_id"].Array();
		if (elements.size() > 1) {
			stg->setStgInstrumentIdA(elements[0].String());
			stg->setStgInstrumentIdB(elements[1].String());
			stg->addInstrumentToList(stg->getStgInstrumentIdA());
			stg->addInstrumentToList(stg->getStgInstrumentIdB());
			cout << "stg->setStgInstrumentIdA(elements[0])" << stg->getStgInstrumentIdA() << endl;
			cout << "stg->setStgInstrumentIdA(elements[1])" << stg->getStgInstrumentIdB() << endl;
		}

		//for (vector<BSONElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
		//	cout << *it << endl;
		//}

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
		this->conn->update(DB_MARKETCONFIG_COLLECTION, BSON("market_id" << (mc->getMarketID().c_str())), BSON("$set" << BSON("market_id" << mc->getMarketID() 
			<< "market_frontAddr" << mc->getMarketFrontAddr() 
			<< "broker_id" << mc->getBrokerID() 
			<< "user_id" << mc->getUserID() 
			<< "password" << mc->getPassword() 
			<< "isactive" << mc->getIsActive())));
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
			this->conn->query(DB_MARKETCONFIG_COLLECTION, MONGO_QUERY("isactive" << ISACTIVE));
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
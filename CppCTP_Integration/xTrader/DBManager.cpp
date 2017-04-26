#include <mongo/bson/bson.h>
#include <iostream>
#include <mutex>
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

#define DB_NAME									"CTP"
#define DB_OPERATOR_COLLECTION					"CTP.trader"
#define DB_SESSIONS_COLLECTION					"CTP.sessions"
#define DB_ADMIN_COLLECTION						"CTP.admin"
#define DB_STRATEGY_COLLECTION					"CTP.strategy"
#define DB_STRATEGY_CHANGED_COLLECTION			"CTP.strategy_changed"
#define DB_POSITIONDETAIL_COLLECTION			"CTP.positiondetail"
#define DB_POSITIONDETAIL_CHANGED_COLLECTION	"CTP.positiondetail_changed"
#define DB_POSITIONMODIFYRECORD_COLLECTION		"CTP.positionmodifyrecord"
#define DB_POSITIONDETAIL_YESTERDAY_COLLECTION	"CTP.positiondetail_yesterday"
#define DB_POSITIONDETAIL_TRADE_COLLECTION		"CTP.positiondetail_trade"
#define DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION	"CTP.positiondetail_trade_yesterday"
#define DB_SYSTEM_RUNNING_STATUS_COLLECTION		"CTP.system_running_status"
#define DB_RUNNING_KEY							"BEES_HOME"
#define DB_STRATEGY_YESTERDAY_COLLECTION		"CTP.strategy_yesterday"
#define DB_ALGORITHM_COLLECTION					"CTP.algorithm"
#define ISACTIVE "1"
#define ISNOTACTIVE "0"

std::mutex update_futureaccount_mtx; // locks access to counter

DBManager::DBManager() {
	this->db_connect_status = true;
	this->conn = NULL;
	try {
		this->conn = new mongo::DBClientConnection(false, 0, 5);
	}
	catch (const mongo::ConnectException &e) {
		std::cout << "MongoDB无法访问!" << std::endl;
		this->db_connect_status = false;
	}
	catch (const mongo::SocketException &e) {
		std::cout << "MongoDB无法访问!" << std::endl;
		this->db_connect_status = false;
	}
	catch (const mongo::DBException &e) {
		std::cout << "MongoDB无法访问! 问题:" << e.what() << std::endl;
		this->db_connect_status = false;
	}
	try {
		this->conn->connect("localhost");
	}
	catch (const mongo::ConnectException &e) {
		std::cout << "MongoDB无法访问!" << std::endl;
		this->db_connect_status = false;
	}
	catch (const mongo::SocketException &e) {
		std::cout << "MongoDB无法访问!" << std::endl;
		this->db_connect_status = false;
	}
	catch (const mongo::DBException &e) {
		std::cout << "MongoDB无法访问! 问题:" << e.what() << std::endl;
		this->db_connect_status = false;
	}
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

/************************************************************************/
/* 设置交易模式(simnow 盘中/离线                                           */
/************************************************************************/
void DBManager::setIs_Online(bool is_online) {
	this->is_online = is_online;
}

bool DBManager::getIs_Online() {
	return this->is_online;
}

/************************************************************************/
/* 查询mongodb是否存在昨持仓明细(trade,order)集合。
如果存在说明上次程序正常关闭；
如果不存在说明程序关闭有误*/
/************************************************************************/
bool DBManager::CheckSystemStartFlag() {
	std::cout << "DBManager::CheckSystemStartFlag()" << std::endl;
	list<string> collection_names;
	bool flag_one = false;
	bool flag_two = false;
	collection_names = this->conn->getCollectionNames(DB_NAME);
	list<string>::iterator str_itor;
	for (str_itor = collection_names.begin(); str_itor != collection_names.end(); str_itor++) {
		std::cout << "\tcollection_name = " << (*str_itor) << std::endl;
		if ((*str_itor) == "positiondetail_yesterday") {
			flag_one = true;
		}
		else if ((*str_itor) == "positiondetail_trade_yesterday") {
			flag_two = true;
		}
	}
	
	if (flag_one && flag_two) {
	}
	else {
		if (flag_one) { std::cout << "\tpositiondetail_yesterday exists." << std::endl; }
		else { std::cout << "\tpositiondetail_yesterday NOT exists." << std::endl; }
		if (flag_two) { std::cout << "\tpositiondetail_trade_yesterday exists." << std::endl; }
		else { std::cout << "\tpositiondetail_trade_yesterday NOT exists." << std::endl; }
		return false;
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
	std::cout << "DBManager::getAllTrader()" << std::endl;
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
			cout << "\t*" << "traderid:" << p.getStringField("traderid") << "  "
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
	std::cout << "DBManager::getAllObjTrader()" << std::endl;
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
		cout << "\tDBManager::getAllTrader is NONE!" << endl;
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			this->conn->query(DB_OPERATOR_COLLECTION);
		while (cursor->more()) {
			Trader *op = new Trader();
			BSONObj p = cursor->next();
			cout << "\t*" << "traderid:" << p.getStringField("traderid") << "  "
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
	
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}

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
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
	this->conn->update(DB_FUTUREACCOUNT_COLLECTION, BSON("userid" << (fa->getUserID().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
	USER_PRINT("DBManager::DeleteFutureAccount ok");
}
void DBManager::UpdateFutureAccount(User *u) {
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
	this->conn->update(DB_FUTUREACCOUNT_COLLECTION, BSON("userid" << (u->getUserID().c_str())), BSON("$set" << BSON("userid" << u->getUserID() << "brokerid" << u->getBrokerID() << "traderid" << u->getTraderID() << "password" << u->getPassword() << "frontaddress" << u->getFrontAddress() << "on_off" << u->getOn_Off())));
	USER_PRINT("DBManager::UpdateOperator ok");
}

void DBManager::UpdateFutureAccountOrderRef(mongo::DBClientConnection * conn_tmp, User *u, string order_ref_base) {
	USER_PRINT("order_ref_base");
	USER_PRINT(order_ref_base);
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
	
	USER_PRINT(DB_FUTUREACCOUNT_COLLECTION);
	
	update_futureaccount_mtx.lock();
	
	conn_tmp->update(DB_FUTUREACCOUNT_COLLECTION, BSON("userid" << (u->getUserID().c_str())), BSON("$set" << BSON("order_ref_base" << order_ref_base)));

	update_futureaccount_mtx.unlock();
	USER_PRINT("DBManager::UpdateFutureAccountOrderRef ok");
}

void DBManager::SearchFutrueByUserID(string userid) {
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
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
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
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
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
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
		this->conn->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("traderid" << traderid << "isactive" << ISACTIVE));
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
	USER_PRINT(this->is_online);
	std::cout << "DBManager::getAllFutureAccount()" << std::endl;
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}

	/// 初始化的时候，必须保证list为空
	if (l_user->size() > 0) {
		list<User *>::iterator user_itor;
		for (user_itor = l_user->begin(); user_itor != l_user->end();) {
			user_itor = l_user->erase(user_itor);
		}
	}

	int countnum = this->conn->count(DB_FUTUREACCOUNT_COLLECTION, BSON("isactive" << ISACTIVE));
	if (countnum == 0) {
		cout << "DBManager::getAllFutureAccount None!" << endl;
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			this->conn->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("isactive" << ISACTIVE));
		while (cursor->more()) {
			BSONObj p = cursor->next();
			cout << "\t*" << "brokerid:" << p.getStringField("brokerid") << "  "
				<< "traderid:" << p.getStringField("traderid") << "  "
				<< "password:" << p.getStringField("password") << "  "
				<< "userid:" << p.getStringField("userid") << "  "
				<< "frontAddress:" << p.getStringField("frontaddress") << "  "
				<< "on_off:" << p.getIntField("on_off") << "  "
				<< "sessionid:" << p.getIntField("sessionid") << "  "
				<< "isactive:" << p.getStringField("isactive") << "*" << endl;
			User *user = new User(p.getStringField("frontaddress"), p.getStringField("brokerid"), p.getStringField("userid"), p.getStringField("password"), p.getStringField("userid"), p.getIntField("on_off"), p.getStringField("traderid"), p.getStringField("order_ref_base"));
			l_user->push_back(user);
		}
		
		USER_PRINT("DBManager::getAllFutureAccount ok");
	}
}

int DBManager::CreateStrategy(Strategy *stg) {
	std::cout << "DBManager::CreateStrategy()" << std::endl;
	int count_number = 0;
	int flag = 0;

	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str()) << "trading_day" << (stg->getStgTradingDay().c_str())));

	if (count_number > 0) {
		cout << "\t策略已经存在!!" << endl;
		flag = 1;
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
		b.append("position_b_sell_yesterday", stg->getStgPositionBSellYesterday());
		b.append("strategy_id", stg->getStgStrategyId());
		b.append("position_b_buy", stg->getStgPositionBBuy());
		b.append("lots_batch", stg->getStgLotsBatch());
		b.append("position_a_buy", stg->getStgPositionABuy());
		b.append("sell_open", stg->getStgSellOpen());
		b.append("order_algorithm", stg->getStgOrderAlgorithm());
		b.append("trader_id", stg->getStgTraderId());
		b.append("a_order_action_limit", stg->getStgAOrderActionTiresLimit());
		b.append("b_order_action_limit", stg->getStgBOrderActionTiresLimit());
		b.append("sell_close", stg->getStgSellClose());
		b.append("buy_open", stg->getStgBuyOpen());

		b.append("only_close", stg->isStgOnlyClose());
		b.append("strategy_on_off", stg->getOn_Off());
		b.append("sell_open_on_off", stg->getStgSellOpenOnOff());
		b.append("buy_close_on_off", stg->getStgBuyCloseOnOff());
		b.append("sell_close_on_off", stg->getStgSellCloseOnOff());
		b.append("buy_open_on_off", stg->getStgBuyOpenOnOff());

		/*新增字段*/
		b.append("trade_model", stg->getStgTradeModel());
		b.append("hold_profit", stg->getStgHoldProfit());
		b.append("close_profit", stg->getStgCloseProfit());
		b.append("commission", stg->getStgCommission());
		b.append("position", stg->getStgPosition());
		b.append("position_buy", stg->getStgPositionBuy());
		b.append("position_sell", stg->getStgPositionSell());
		b.append("trade_volume", stg->getStgTradeVolume());
		b.append("amount", stg->getStgAmount());
		b.append("average_shift", stg->getStgAverageShift());
		b.append("a_limit_price_shift", stg->getStgALimitPriceShift());
		b.append("b_limit_price_shift", stg->getStgBLimitPriceShift());
		b.append("update_position_detail_record_time", stg->getStgUpdatePositionDetailRecordTime());


		// 创建一个数组对象
		BSONArrayBuilder bab;
		bab.append(stg->getStgInstrumentIdA());
		bab.append(stg->getStgInstrumentIdB());

		b.appendArray("list_instrument_id", bab.arr());

		b.append("position_a_buy_yesterday", stg->getStgPositionABuyYesterday());
		b.append("user_id", stg->getStgUserId());
		b.append("position_a_buy_today", stg->getStgPositionABuyToday());
		b.append("position_a_sell_yesterday", stg->getStgPositionASellYesterday());
		b.append("lots", stg->getStgLots());
		b.append("a_wait_price_tick", stg->getStgAWaitPriceTick());
		b.append("b_wait_price_tick", stg->getStgBWaitPriceTick());
		b.append("trading_day", stg->getStgTradingDay());

		BSONObj p = b.obj();

		conn->insert(DB_STRATEGY_COLLECTION, p);
		USER_PRINT("DBManager::CreateStrategy ok");
		flag = 0;
	}
	return flag;
}

int DBManager::DeleteStrategy(Strategy *stg) {
	int count_number = 0;
	int flag = 0;
	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	if (count_number > 0) {
		this->conn->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str()) << "is_active" << true), BSON("$set" << BSON("is_active" << false)));
		USER_PRINT("DBManager::DeleteStrategy ok");
		flag = 0;
	}
	else {
		cout << "Strategy Not Exists!" << endl;
		flag = 1;
	}
	return flag;
}

int DBManager::UpdateStrategyOnOff(Strategy *stg) {
	int count_number = 0;
	int flag = 0;
	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	if (count_number > 0) {

		std::cout << "UpdateStrategyOnOff strategy_id = " << stg->getStgStrategyId() << std::endl;
		std::cout << "UpdateStrategyOnOff user_id = " << stg->getStgUserId() << std::endl;
		std::cout << "UpdateStrategyOnOff strategy_on_off = " << stg->getOn_Off() << std::endl;

		this->conn->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId()) << "is_active" << true), BSON("$set" << BSON("strategy_on_off" << stg->getOn_Off())));
		USER_PRINT("DBManager::UpdateStrategyOnOff ok");
		flag = 0;
	}
	else {
		cout << "Strategy ID Not Exists!" << endl;
		flag = 1;
	}
	return flag;
}

int DBManager::UpdateStrategyOnlyCloseOnOff(Strategy *stg) {
	int count_number = 0;
	int flag = 0;
	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	if (count_number > 0) {

		std::cout << "UpdateStrategyOnlyCloseOnOff strategy_id = " << stg->getStgStrategyId() << std::endl;
		std::cout << "UpdateStrategyOnlyCloseOnOff user_id = " << stg->getStgUserId() << std::endl;
		std::cout << "UpdateStrategyOnlyCloseOnOff only_close = " << stg->isStgOnlyClose() << std::endl;

		this->conn->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId()) << "is_active" << true), BSON("$set" << BSON("only_close" << stg->isStgOnlyClose())));
		USER_PRINT("DBManager::UpdateStrategyOnlyCloseOnOff ok");
		flag = 0;
	}
	else {
		cout << "Strategy ID Not Exists!" << endl;
		flag = 1;
	}
	return flag;
}

void DBManager::UpdateStrategy(Strategy *stg) {
	USER_PRINT("DBManager::UpdateStrategy()");
	int count_number = 0;

	std::cout << "DBManager::UpdateStrategy()" << std::endl;


	count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	std::cout << "\tcount_number = " << count_number << std::endl;

	if (count_number > 0) {
		this->conn->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str() << "is_active" << true), BSON("$set" << BSON("position_a_sell_today" << stg->getStgPositionASellToday()
			<< "position_b_sell" << stg->getStgPositionBSell()
			<< "spread_shift" << stg->getStgSpreadShift()
			<< "position_b_sell_today" << stg->getStgPositionBSellToday()
			<< "position_b_buy_today" << stg->getStgPositionBBuyToday()
			<< "position_a_sell" << stg->getStgPositionASell()
			<< "buy_close" << stg->getStgBuyClose()
			<< "stop_loss" << stg->getStgStopLoss()
			<< "position_b_buy_yesterday" << stg->getStgPositionBBuyYesterday()
			<< "is_active" << stg->isStgIsActive()
			<< "position_b_sell_yesterday" << stg->getStgPositionBSellYesterday()
			<< "strategy_id" << stg->getStgStrategyId()
			<< "position_b_buy" << stg->getStgPositionBBuy()
			<< "lots_batch" << stg->getStgLotsBatch()
			<< "position_a_buy" << stg->getStgPositionABuy()
			<< "sell_open" << stg->getStgSellOpen()
			<< "order_algorithm" << stg->getStgOrderAlgorithm()
			<< "trader_id" << stg->getStgTraderId()
			<< "a_order_action_limit" << stg->getStgAOrderActionTiresLimit()
			<< "b_order_action_limit" << stg->getStgBOrderActionTiresLimit()
			<< "sell_close" << stg->getStgSellClose()
			<< "buy_open" << stg->getStgBuyOpen()

			<< "only_close" << stg->isStgOnlyClose()
			<< "strategy_on_off" << stg->getOn_Off()
			<< "sell_open_on_off" << stg->getStgSellOpenOnOff()
			<< "buy_close_on_off" << stg->getStgBuyCloseOnOff()
			<< "sell_close_on_off" << stg->getStgSellCloseOnOff()
			<< "buy_open_on_off" << stg->getStgBuyOpenOnOff()

			<< "trade_model" << stg->getStgTradeModel()
			<< "hold_profit" << stg->getStgHoldProfit()
			<< "close_profit" << stg->getStgCloseProfit()
			<< "commission" << stg->getStgCommission()
			<< "position" << stg->getStgPosition()
			<< "position_buy" << stg->getStgPositionBuy()
			<< "position_sell" << stg->getStgPositionSell()
			<< "trade_volume" << stg->getStgTradeVolume()
			<< "amount" << stg->getStgAmount()
			<< "average_shift" << stg->getStgAverageShift()

			<< "a_limit_price_shift" << stg->getStgALimitPriceShift()
			<< "b_limit_price_shift" << stg->getStgBLimitPriceShift()

			<< "position_a_buy_yesterday" << stg->getStgPositionABuyYesterday()
			<< "user_id" << stg->getStgUserId()
			<< "position_a_buy_today" << stg->getStgPositionABuyToday()
			<< "position_a_sell_yesterday" << stg->getStgPositionASellYesterday()
			<< "lots" << stg->getStgLots()
			<< "a_wait_price_tick" << stg->getStgAWaitPriceTick()
			<< "b_wait_price_tick" << stg->getStgBWaitPriceTick()
			<< "trading_day" << stg->getStgTradingDay()
			<< "update_position_detail_record_time" << stg->getStgUpdatePositionDetailRecordTime()
			<< "list_instrument_id" << BSON_ARRAY(stg->getStgInstrumentIdA() << stg->getStgInstrumentIdB()))));

		USER_PRINT("DBManager::UpdateStrategy ok");
	}
	else
	{
		USER_PRINT("Strategy ID Not Exists!");
	}
}
void DBManager::getAllStrategy(list<Strategy *> *l_strategys, string traderid, string userid) {
	std::cout << "DBManager::getAllStrategy()" << std::endl;
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
			cursor = this->conn->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid << "is_active" << true));
		}
		else {
			cursor = this->conn->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "is_active" << true));
		}
	}
	else {
		cursor = this->conn->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("is_active" << true));
	}
		
	while (cursor->more()) {
		BSONObj p = cursor->next();
		Strategy *stg = new Strategy();
		//cout << "\tposition_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
		//cout << "\tposition_b_sell = " << p.getIntField("position_b_sell") << ", ";
		//cout << "\tspread_shift = " << p.getField("spread_shift").Double() << ", ";
		//cout << "\tposition_b_sell_today = " << p.getIntField("position_b_sell_today") << ", ";
		//cout << "\tposition_b_buy_today = " << p.getIntField("position_b_buy_today") << ", ";
		//cout << "\tposition_a_sell = " << p.getIntField("position_a_sell") << ", ";
		//cout << "\tbuy_close = " << p.getField("buy_close").Double() << ", ";
		//cout << "\tstop_loss = " << p.getField("stop_loss").Double() << ", ";
		//cout << "\tposition_b_buy_yesterday = " << p.getIntField("position_b_buy_yesterday") << ", ";
		//cout << "\tis_active = " << p.getField("is_active").Bool() << ", ";
		//cout << "\tposition_b_sell_yesterday = " << p.getIntField("position_b_sell_yesterday") << ", ";
		//cout << "\tstrategy_id = " << p.getStringField("strategy_id") << ", "; //string type
		//cout << "\tposition_b_buy = " << p.getIntField("position_b_buy") << ", ";
		//cout << "\tlots_batch = " << p.getIntField("lots_batch") << ", ";
		//cout << "\tposition_a_buy = " << p.getIntField("position_a_buy") << ", ";
		//cout << "\tsell_open = " << p.getField("sell_open").Double() << ", ";
		//cout << "\torder_algorithm = " << p.getStringField("order_algorithm") << ", "; //string type
		//cout << "\ttrader_id = " << p.getStringField("trader_id") << ", "; // string type
		//cout << "\ta_order_action_limit = " << p.getIntField("a_order_action_limit") << ", ";
		//cout << "\tb_order_action_limit = " << p.getIntField("b_order_action_limit") << ", ";
		//cout << "\tsell_close = " << p.getField("sell_close").Double() << ", ";
		//cout << "\tbuy_open = " << p.getField("buy_open").Double() << ", ";
		//cout << "\tonly_close = " << p.getIntField("only_close") << ", ";

		///*新增字段*/

		//cout << "\ttrade_model" << p.getStringField("trade_model") << ", ";
		//cout << "\thold_profit" << p.getField("hold_profit").Double() << ", ";
		//cout << "\tclose_profit" << p.getField("close_profit").Double() << ", ";
		//cout << "\tcommission" << p.getField("commission").Double() << ", ";
		//cout << "\tposition" << p.getIntField("position") << ", ";
		//cout << "\tposition_buy" << p.getIntField("position_buy") << ", ";
		//cout << "\tposition_sell" << p.getIntField("position_sell") << ", ";
		//cout << "\ttrade_volume" << p.getIntField("trade_volume") << ", ";
		//cout << "\tamount" << p.getField("amount").Double() << ", ";
		//cout << "\taverage_shift" << p.getField("average_shift").Double() << ", ";

		//cout << "\ta_limit_price_shift" << p.getIntField("a_limit_price_shift") << ", ";
		//cout << "\tb_limit_price_shift" << p.getIntField("b_limit_price_shift") << ", ";

		//cout << "\tposition_a_buy_yesterday = " << p.getIntField("position_a_buy_yesterday") << ", ";
		//cout << "\tuser_id = " << p.getStringField("user_id") << ", "; // string type
		//cout << "\tposition_a_buy_today = " << p.getIntField("position_a_buy_today") << ", ";
		//cout << "\tposition_a_sell_yesterday = " << p.getIntField("position_a_sell_yesterday") << ", ";
		//cout << "\tlots = " << p.getIntField("lots") << ", ";
		//cout << "\ta_wait_price_tick = " << p.getField("a_wait_price_tick").Double() << ", ";
		//cout << "\tb_wait_price_tick = " << p.getField("b_wait_price_tick").Double() << ", ";
		////cout << "strategy_on_off = " << p.getField("strategy_on_off").Int() << endl;
		//cout << "\ttrading_day = " << p.getStringField("trading_day") << endl;

		stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
		stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
		stg->setStgBuyClose(p.getField("buy_close").Double());
		stg->setStgBuyOpen(p.getField("buy_open").Double());
		stg->setStgIsActive(p.getField("is_active").Bool());
		stg->setStgLots(p.getIntField("lots"));
		stg->setStgLotsBatch(p.getIntField("lots_batch"));

		stg->setStgOnlyClose(p.getIntField("only_close"));
		stg->setOn_Off(p.getIntField("strategy_on_off"));
		stg->setStgSellOpenOnOff(p.getIntField("sell_open_on_off"));
		stg->setStgBuyCloseOnOff(p.getIntField("buy_close_on_off"));
		stg->setStgSellCloseOnOff(p.getIntField("sell_close_on_off"));
		stg->setStgBuyOpenOnOff(p.getIntField("buy_open_on_off"));

		/*新增*/
		stg->setStgTradeModel(p.getStringField("trade_model"));
		stg->setStgOrderAlgorithm(p.getStringField("order_algorithm"));
		stg->setStgHoldProfit(p.getField("hold_profit").Double());
		stg->setStgCloseProfit(p.getField("close_profit").Double());
		stg->setStgCommisstion(p.getField("commission").Double());
		stg->setStgPosition(p.getIntField("position"));
		stg->setStgPositionBuy(p.getIntField("position_buy"));
		stg->setStgPositionSell(p.getIntField("position_sell"));
		stg->setStgTradeVolume(p.getIntField("trade_volume"));
		stg->setStgAmount(p.getField("amount").Double());
		stg->setStgAverageShift(p.getField("average_shift").Double());
		stg->setStgTradingDay(p.getStringField("trading_day"));

		stg->setStgALimitPriceShift(p.getIntField("a_limit_price_shift"));
		stg->setStgBLimitPriceShift(p.getIntField("b_limit_price_shift"));

		/*std::cout << "\tafter stg->setStgTradingDay = " << stg->getStgTradingDay() << std::endl;
		std::cout << "\taddress stg = " << stg << std::endl;*/

		stg->setStgAOrderActionTiresLimit(p.getIntField("a_order_action_limit"));
		stg->setStgBOrderActionTiresLimit(p.getIntField("b_order_action_limit"));
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

void DBManager::getAllStrategyByActiveUser(list<Strategy *> *l_strategys, list<User *> *l_users, string traderid) {
	std::cout << "DBManager::getAllStrategyByActiveUser()" << std::endl;
	USER_PRINT("getAllStrategyByActiveUser");
	list<User *>::iterator user_itor;
	/// 初始化的时候，必须保证list为空
	if (l_strategys->size() > 0) {
		list<Strategy *>::iterator Itor;
		for (Itor = l_strategys->begin(); Itor != l_strategys->end();) {
			delete (*Itor);
			Itor = l_strategys->erase(Itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	for (user_itor = l_users->begin(); user_itor != l_users->end(); user_itor++) {
		if (traderid.compare("")) { //如果traderid不为空

			cursor = this->conn->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << (*user_itor)->getUserID() << "is_active" << true));
		}
		else {
			cursor = this->conn->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("user_id" << (*user_itor)->getUserID() << "is_active" << true));
		}

		while (cursor->more()) {
			BSONObj p = cursor->next();
			Strategy *stg = new Strategy();
			cout << "\t*position_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
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
			cout << "a_order_action_limit = " << p.getIntField("a_order_action_limit") << ", ";
			cout << "b_order_action_limit = " << p.getIntField("b_order_action_limit") << ", ";
			cout << "sell_close = " << p.getField("sell_close").Double() << ", ";
			cout << "buy_open = " << p.getField("buy_open").Double() << ", ";
			cout << "only_close = " << p.getIntField("only_close") << ", ";

			/*新增字段*/

			cout << "trade_model" << p.getStringField("trade_model") << ", ";
			cout << "hold_profit" << p.getField("hold_profit").Double() << ", ";
			cout << "close_profit" << p.getField("close_profit").Double() << ", ";
			cout << "commission" << p.getField("commission").Double() << ", ";
			cout << "position" << p.getIntField("position") << ", ";
			cout << "position_buy" << p.getIntField("position_buy") << ", ";
			cout << "position_sell" << p.getIntField("position_sell") << ", ";
			cout << "trade_volume" << p.getIntField("trade_volume") << ", ";
			cout << "amount" << p.getField("amount").Double() << ", ";
			cout << "average_shift" << p.getField("average_shift").Double() << ", ";

			cout << "a_limit_price_shift" << p.getIntField("a_limit_price_shift") << ", ";
			cout << "b_limit_price_shift" << p.getIntField("b_limit_price_shift") << ", ";

			cout << "position_a_buy_yesterday = " << p.getIntField("position_a_buy_yesterday") << ", ";
			cout << "user_id = " << p.getStringField("user_id") << ", "; // string type
			cout << "position_a_buy_today = " << p.getIntField("position_a_buy_today") << ", ";
			cout << "position_a_sell_yesterday = " << p.getIntField("position_a_sell_yesterday") << ", ";
			cout << "lots = " << p.getIntField("lots") << ", ";
			cout << "a_wait_price_tick = " << p.getField("a_wait_price_tick").Double() << ", ";
			cout << "b_wait_price_tick = " << p.getField("b_wait_price_tick").Double() << ", ";
			cout << "strategy_on_off = " << p.getIntField("strategy_on_off") << ", ";
			cout << "sell_open_on_off = " << p.getIntField("sell_open_on_off") << ", ";
			cout << "buy_close_on_off = " << p.getIntField("buy_close_on_off") << ", ";
			cout << "sell_close_on_off = " << p.getIntField("sell_close_on_off") << ", ";
			cout << "buy_open_on_off = " << p.getIntField("buy_open_on_off") << ", ";
			cout << "trading_day = " << p.getStringField("trading_day") << ", ";
			cout << "update_position_detail_record_time = " << p.getStringField("update_position_detail_record_time") << ", ";

			stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
			stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
			stg->setStgBuyClose(p.getField("buy_close").Double());
			stg->setStgBuyOpen(p.getField("buy_open").Double());
			stg->setStgIsActive(p.getField("is_active").Bool());
			stg->setStgLots(p.getIntField("lots"));
			stg->setStgLotsBatch(p.getIntField("lots_batch"));

			stg->setStgOnlyClose(p.getIntField("only_close"));
			stg->setOn_Off(p.getIntField("strategy_on_off"));
			stg->setStgSellOpenOnOff(p.getIntField("sell_open_on_off"));
			stg->setStgBuyCloseOnOff(p.getIntField("buy_close_on_off"));
			stg->setStgSellCloseOnOff(p.getIntField("sell_close_on_off"));
			stg->setStgBuyOpenOnOff(p.getIntField("buy_open_on_off"));

			/*新增*/
			stg->setStgTradeModel(p.getStringField("trade_model"));
			stg->setStgOrderAlgorithm(p.getStringField("order_algorithm"));
			stg->setStgHoldProfit(p.getField("hold_profit").Double());
			stg->setStgCloseProfit(p.getField("close_profit").Double());
			stg->setStgCommisstion(p.getField("commission").Double());
			stg->setStgPosition(p.getIntField("position"));
			stg->setStgPositionBuy(p.getIntField("position_buy"));
			stg->setStgPositionSell(p.getIntField("position_sell"));
			stg->setStgTradeVolume(p.getIntField("trade_volume"));
			stg->setStgAmount(p.getField("amount").Double());
			stg->setStgAverageShift(p.getField("average_shift").Double());
			stg->setStgALimitPriceShift(p.getIntField("a_limit_price_shift"));
			stg->setStgBLimitPriceShift(p.getIntField("b_limit_price_shift"));
			stg->setStgTradingDay(p.getStringField("trading_day"));
			// 设置是否有修改过持仓
			stg->setStgUpdatePositionDetailRecordTime(p.getStringField("update_position_detail_record_time"));
			// 当持仓修改时间不为空，说明曾经有修改
			if (stg->getStgUpdatePositionDetailRecordTime() != "")
			{
				stg->setStgIsPositionRight(false);
			}
			else {
				stg->setStgIsPositionRight(true);
			}
			

			/*std::cout << "\tafter stg->setStgTradingDay = " << stg->getStgTradingDay() << std::endl;
			std::cout << "\taddress stg = " << stg << std::endl;*/

			stg->setStgAOrderActionTiresLimit(p.getIntField("a_order_action_limit"));
			stg->setStgBOrderActionTiresLimit(p.getIntField("b_order_action_limit"));

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


			vector<BSONElement> elements = p["list_instrument_id"].Array();
			if (elements.size() > 1) {
				stg->setStgInstrumentIdA(elements[0].String());
				stg->setStgInstrumentIdB(elements[1].String());
				stg->addInstrumentToList(stg->getStgInstrumentIdA());
				stg->addInstrumentToList(stg->getStgInstrumentIdB());
				cout << "stg->setStgInstrumentIdA(elements[0]) = " << stg->getStgInstrumentIdA() << ", ";
				cout << "stg->setStgInstrumentIdA(elements[1]) = " << stg->getStgInstrumentIdB() << endl;
			}

			//for (vector<BSONElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
			//	cout << *it << endl;
			//}

			l_strategys->push_back(stg);
		}
	}

	USER_PRINT("DBManager::getAllStrategyByActiveUser ok");
}


/************************************************************************/
/* 创建策略(昨仓)
删除策略(昨仓)
更新策略(昨仓)
查找策略(昨仓)			                                                */
/************************************************************************/
int DBManager::CreateStrategyYesterday(Strategy *stg) {
	int count_number = 0;
	int flag = 0;

	count_number = this->conn->count(DB_STRATEGY_YESTERDAY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str()) << "trading_day" << (stg->getStgTradingDay().c_str())));

	if (count_number > 0) {
		cout << "Strategy Already Exists!" << endl;
		flag = 1;
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
		b.append("position_b_sell_yesterday", stg->getStgPositionBSellYesterday());
		b.append("strategy_id", stg->getStgStrategyId());
		b.append("position_b_buy", stg->getStgPositionBBuy());
		b.append("lots_batch", stg->getStgLotsBatch());
		b.append("position_a_buy", stg->getStgPositionABuy());
		b.append("sell_open", stg->getStgSellOpen());
		b.append("order_algorithm", stg->getStgOrderAlgorithm());
		b.append("trader_id", stg->getStgTraderId());
		b.append("a_order_action_limit", stg->getStgAOrderActionTiresLimit());
		b.append("b_order_action_limit", stg->getStgBOrderActionTiresLimit());
		b.append("sell_close", stg->getStgSellClose());
		b.append("buy_open", stg->getStgBuyOpen());
		b.append("only_close", stg->isStgOnlyClose());

		/*新增字段*/
		b.append("trade_model", stg->getStgTradeModel());
		b.append("hold_profit", stg->getStgHoldProfit());
		b.append("close_profit", stg->getStgCloseProfit());
		b.append("commission", stg->getStgCommission());
		b.append("position", stg->getStgPosition());
		b.append("position_buy", stg->getStgPositionBuy());
		b.append("position_sell", stg->getStgPositionSell());
		b.append("trade_volume", stg->getStgTradeVolume());
		b.append("amount", stg->getStgAmount());
		b.append("average_shift", stg->getStgAverageShift());
		b.append("a_limit_price_shift", stg->getStgALimitPriceShift());
		b.append("b_limit_price_shift", stg->getStgBLimitPriceShift());
		b.append("update_position_detail_record_time", stg->getStgUpdatePositionDetailRecordTime());


		// 创建一个数组对象
		BSONArrayBuilder bab;
		bab.append(stg->getStgInstrumentIdA());
		bab.append(stg->getStgInstrumentIdB());

		b.appendArray("list_instrument_id", bab.arr());

		b.append("position_a_buy_yesterday", stg->getStgPositionABuyYesterday());
		b.append("user_id", stg->getStgUserId());
		b.append("position_a_buy_today", stg->getStgPositionABuyToday());
		b.append("position_a_sell_yesterday", stg->getStgPositionASellYesterday());
		b.append("lots", stg->getStgLots());
		b.append("a_wait_price_tick", stg->getStgAWaitPriceTick());
		b.append("b_wait_price_tick", stg->getStgBWaitPriceTick());
		b.append("StrategyOnoff", stg->getOn_Off());
		b.append("trading_day", stg->getStgTradingDay());

		BSONObj p = b.obj();

		conn->insert(DB_STRATEGY_YESTERDAY_COLLECTION, p);
		flag = 0;
		USER_PRINT("DBManager::CreateStrategyYesterday ok");
	}
	return flag;
}

int DBManager::DeleteStrategyYesterday(Strategy *stg) {
	int count_number = 0;
	int flag = 0;

	count_number = this->conn->count(DB_STRATEGY_YESTERDAY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	if (count_number > 0) {
		this->conn->update(DB_STRATEGY_YESTERDAY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str() << "is_active" << true), BSON("$set" << BSON("is_active" << false)));
		USER_PRINT("DBManager::DeleteStrategy ok");
	}
	else {
		cout << "Strategy ID Not Exists!" << endl;
		flag = 1;
	}
	return flag;
}

void DBManager::UpdateStrategyYesterday(Strategy *stg) {
	int count_number = 0;

	std::cout << "DBManager::UpdateStrategy" << std::endl;


	count_number = this->conn->count(DB_STRATEGY_YESTERDAY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));
	std::cout << "DBManager::UpdateStrategyYesterday()" << std::endl;
	std::cout << "\tcount_number = " << count_number << std::endl;

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
			<< "position_b_sell_yesterday" << stg->getStgPositionBSellYesterday()
			<< "strategy_id" << stg->getStgStrategyId()
			<< "position_b_buy" << stg->getStgPositionBBuy()
			<< "lots_batch" << stg->getStgLotsBatch()
			<< "position_a_buy" << stg->getStgPositionABuy()
			<< "sell_open" << stg->getStgSellOpen()
			<< "order_algorithm" << stg->getStgOrderAlgorithm()
			<< "trader_id" << stg->getStgTraderId()
			<< "a_order_action_limit" << stg->getStgAOrderActionTiresLimit()
			<< "b_order_action_limit" << stg->getStgBOrderActionTiresLimit()
			<< "sell_close" << stg->getStgSellClose()
			<< "buy_open" << stg->getStgBuyOpen()
			<< "only_close" << stg->isStgOnlyClose()

			<< "trade_model" << stg->getStgTradeModel()
			<< "hold_profit" << stg->getStgHoldProfit()
			<< "close_profit" << stg->getStgCloseProfit()
			<< "commission" << stg->getStgCommission()
			<< "position" << stg->getStgPosition()
			<< "position_buy" << stg->getStgPositionBuy()
			<< "position_sell" << stg->getStgPositionSell()
			<< "trade_volume" << stg->getStgTradeVolume()
			<< "amount" << stg->getStgAmount()
			<< "average_shift" << stg->getStgAverageShift()
			<< "a_limit_price_shift" << stg->getStgALimitPriceShift()
			<< "b_limit_price_shift" << stg->getStgBLimitPriceShift()

			<< "position_a_buy_yesterday" << stg->getStgPositionABuyYesterday()
			<< "user_id" << stg->getStgUserId()
			<< "position_a_buy_today" << stg->getStgPositionABuyToday()
			<< "position_a_sell_yesterday" << stg->getStgPositionASellYesterday()
			<< "lots" << stg->getStgLots()
			<< "a_wait_price_tick" << stg->getStgAWaitPriceTick()
			<< "b_wait_price_tick" << stg->getStgBWaitPriceTick()
			<< "StrategyOnoff" << stg->getOn_Off()
			<< "trading_day" << stg->getStgTradingDay()
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
			cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid << "is_active" << true));
		}
		else {
			cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "is_active" << true));
		}
	}
	else {
		cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << true));
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();
		Strategy *stg = new Strategy();
		//cout << "position_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
		//cout << "position_b_sell = " << p.getIntField("position_b_sell") << ", ";
		//cout << "spread_shift = " << p.getField("spread_shift").Double() << ", ";
		//cout << "position_b_sell_today = " << p.getIntField("position_b_sell_today") << ", ";
		//cout << "position_b_buy_today = " << p.getIntField("position_b_buy_today") << ", ";
		//cout << "position_a_sell = " << p.getIntField("position_a_sell") << ", ";
		//cout << "buy_close = " << p.getField("buy_close").Double() << ", ";
		//cout << "stop_loss = " << p.getField("stop_loss").Double() << ", ";
		//cout << "position_b_buy_yesterday = " << p.getIntField("position_b_buy_yesterday") << ", ";
		//cout << "is_active = " << p.getField("is_active").Bool() << ", ";
		//cout << "position_b_sell_yesterday = " << p.getIntField("position_b_sell_yesterday") << ", ";
		//cout << "strategy_id = " << p.getStringField("strategy_id") << ", "; //string type
		//cout << "position_b_buy = " << p.getIntField("position_b_buy") << ", ";
		//cout << "lots_batch = " << p.getIntField("lots_batch") << ", ";
		//cout << "position_a_buy = " << p.getIntField("position_a_buy") << ", ";
		//cout << "sell_open = " << p.getField("sell_open").Double() << ", ";
		//cout << "order_algorithm = " << p.getStringField("order_algorithm") << ", "; //string type
		//cout << "trader_id = " << p.getStringField("trader_id") << ", "; // string type
		//cout << "a_order_action_limit = " << p.getIntField("a_order_action_limit") << ", ";
		//cout << "b_order_action_limit = " << p.getIntField("b_order_action_limit") << ", ";
		//cout << "sell_close = " << p.getField("sell_close").Double() << ", ";
		//cout << "buy_open = " << p.getField("buy_open").Double() << ", ";
		//cout << "only_close = " << p.getIntField("only_close") << ", ";

		///*新增字段*/

		//cout << "trade_model" << p.getStringField("trade_model") << ", ";
		//cout << "hold_profit" << p.getField("hold_profit").Double() << ", ";
		//cout << "close_profit" << p.getField("close_profit").Double() << ", ";
		//cout << "commission" << p.getField("commission").Double() << ", ";
		//cout << "position" << p.getIntField("position") << ", ";
		//cout << "position_buy" << p.getIntField("position_buy") << ", ";
		//cout << "position_sell" << p.getIntField("position_sell") << ", ";
		//cout << "trade_volume" << p.getIntField("trade_volume") << ", ";
		//cout << "amount" << p.getField("amount").Double() << ", ";
		//cout << "average_shift" << p.getField("average_shift").Double() << ", ";
		//cout << "a_limit_price_shift" << p.getIntField("a_limit_price_shift") << ", ";
		//cout << "b_limit_price_shift" << p.getIntField("b_limit_price_shift") << ", ";

		//cout << "position_a_buy_yesterday = " << p.getIntField("position_a_buy_yesterday") << ", ";
		//cout << "user_id = " << p.getStringField("user_id") << ", "; // string type
		//cout << "position_a_buy_today = " << p.getIntField("position_a_buy_today") << ", ";
		//cout << "position_a_sell_yesterday = " << p.getIntField("position_a_sell_yesterday") << ", ";
		//cout << "lots = " << p.getIntField("lots") << ", ";
		//cout << "a_wait_price_tick = " << p.getField("a_wait_price_tick").Double() << ", ";
		//cout << "b_wait_price_tick = " << p.getField("b_wait_price_tick").Double() << ", ";
		////cout << "strategy_on_off = " << p.getField("strategy_on_off").Int() << endl;
		//cout << "trading_day = " << p.getStringField("trading_day") << endl;

		stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
		stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
		stg->setStgBuyClose(p.getField("buy_close").Double());
		stg->setStgBuyOpen(p.getField("buy_open").Double());
		stg->setStgIsActive(p.getField("is_active").Bool());
		stg->setStgLots(p.getIntField("lots"));
		stg->setStgLotsBatch(p.getIntField("lots_batch"));

		stg->setStgOnlyClose(p.getIntField("only_close"));
		stg->setOn_Off(p.getIntField("strategy_on_off"));
		stg->setStgSellOpenOnOff(p.getIntField("sell_open_on_off"));
		stg->setStgBuyCloseOnOff(p.getIntField("buy_close_on_off"));
		stg->setStgSellCloseOnOff(p.getIntField("sell_close_on_off"));
		stg->setStgBuyOpenOnOff(p.getIntField("buy_open_on_off"));

		/*新增*/
		stg->setStgTradeModel(p.getStringField("trade_model"));
		stg->setStgOrderAlgorithm(p.getStringField("order_algorithm"));
		stg->setStgHoldProfit(p.getField("hold_profit").Double());
		stg->setStgCloseProfit(p.getField("close_profit").Double());
		stg->setStgCommisstion(p.getField("commission").Double());
		stg->setStgPosition(p.getIntField("position"));
		stg->setStgPositionBuy(p.getIntField("position_buy"));
		stg->setStgPositionSell(p.getIntField("position_sell"));
		stg->setStgTradeVolume(p.getIntField("trade_volume"));
		stg->setStgAmount(p.getField("amount").Double());
		stg->setStgAverageShift(p.getField("average_shift").Double());
		stg->setStgTradingDay(p.getStringField("trading_day"));

		//std::cout << "after stg->setStgTradingDay = " << stg->getStgTradingDay() << std::endl;

		stg->setStgAOrderActionTiresLimit(p.getIntField("a_order_action_limit"));
		stg->setStgBOrderActionTiresLimit(p.getIntField("b_order_action_limit"));
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

		stg->setStgALimitPriceShift(p.getIntField("a_limit_price_shift"));
		stg->setStgBLimitPriceShift(p.getIntField("b_limit_price_shift"));

		stg->setStgStopLoss(p.getField("stop_loss").Double());
		stg->setStgStrategyId(p.getStringField("strategy_id"));
		stg->setStgTraderId(p.getStringField("trader_id"));
		stg->setStgUserId(p.getStringField("user_id"));


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

	USER_PRINT("DBManager::getAllYesterdayStragegy ok");
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
				cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid << "strategy_id" << strategyid << "is_active" << true));
			}
			else {
				cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid << "is_active" << true));
			}
			
		}
		else {
			cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "is_active" << true));
		}
	}
	else {
		cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << true));
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();
		Strategy *stg = new Strategy();
		//cout << "position_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
		//cout << "position_b_sell = " << p.getIntField("position_b_sell") << ", ";
		//cout << "spread_shift = " << p.getField("spread_shift").Double() << ", ";
		//cout << "position_b_sell_today = " << p.getIntField("position_b_sell_today") << ", ";
		//cout << "position_b_buy_today = " << p.getIntField("position_b_buy_today") << ", ";
		//cout << "position_a_sell = " << p.getIntField("position_a_sell") << ", ";
		//cout << "buy_close = " << p.getField("buy_close").Double() << ", ";
		//cout << "stop_loss = " << p.getField("stop_loss").Double() << ", ";
		//cout << "position_b_buy_yesterday = " << p.getIntField("position_b_buy_yesterday") << ", ";
		//cout << "is_active = " << p.getField("is_active").Bool() << ", ";
		//cout << "position_b_sell_yesterday = " << p.getIntField("position_b_sell_yesterday") << ", ";
		//cout << "strategy_id = " << p.getStringField("strategy_id") << ", "; //string type
		//cout << "position_b_buy = " << p.getIntField("position_b_buy") << ", ";
		//cout << "lots_batch = " << p.getIntField("lots_batch") << ", ";
		//cout << "position_a_buy = " << p.getIntField("position_a_buy") << ", ";
		//cout << "sell_open = " << p.getField("sell_open").Double() << ", ";
		//cout << "order_algorithm = " << p.getStringField("order_algorithm") << ", "; //string type
		//cout << "trader_id = " << p.getStringField("trader_id") << ", "; // string type
		//cout << "a_order_action_limit = " << p.getIntField("a_order_action_limit") << ", ";
		//cout << "b_order_action_limit = " << p.getIntField("b_order_action_limit") << ", ";
		//cout << "sell_close = " << p.getField("sell_close").Double() << ", ";
		//cout << "buy_open = " << p.getField("buy_open").Double() << ", ";
		//cout << "only_close = " << p.getIntField("only_close") << ", ";


		///*新增字段*/

		//cout << "trade_model" << p.getStringField("trade_model") << ", ";
		//cout << "hold_profit" << p.getField("hold_profit").Double() << ", ";
		//cout << "close_profit" << p.getField("close_profit").Double() << ", ";
		//cout << "commission" << p.getField("commission").Double() << ", ";
		//cout << "position" << p.getIntField("position") << ", ";
		//cout << "position_buy" << p.getIntField("position_buy") << ", ";
		//cout << "position_sell" << p.getIntField("position_sell") << ", ";
		//cout << "trade_volume" << p.getIntField("trade_volume") << ", ";
		//cout << "amount" << p.getField("amount").Double() << ", ";
		//cout << "average_shift" << p.getField("average_shift").Double() << ", ";

		//cout << "a_limit_price_shift" << p.getIntField("a_limit_price_shift") << ", ";
		//cout << "b_limit_price_shift" << p.getIntField("b_limit_price_shift") << ", ";
		//cout << "trading_day" << p.getStringField("trading_day") << ", ";


		//cout << "position_a_buy_yesterday = " << p.getIntField("position_a_buy_yesterday") << ", ";
		//cout << "user_id = " << p.getStringField("user_id") << ", "; // string type
		//cout << "position_a_buy_today = " << p.getIntField("position_a_buy_today") << ", ";
		//cout << "position_a_sell_yesterday = " << p.getIntField("position_a_sell_yesterday") << ", ";
		//cout << "lots = " << p.getIntField("lots") << ", ";
		//cout << "a_wait_price_tick = " << p.getField("a_wait_price_tick").Double() << ", ";
		//cout << "b_wait_price_tick = " << p.getField("b_wait_price_tick").Double() << ", ";
		//cout << "StrategyOnoff = " << p.getField("StrategyOnoff").Int() << endl;

		stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
		stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
		stg->setStgBuyClose(p.getField("buy_close").Double());
		stg->setStgBuyOpen(p.getField("buy_open").Double());
		stg->setStgIsActive(p.getField("is_active").Bool());
		stg->setStgLots(p.getIntField("lots"));
		stg->setStgLotsBatch(p.getIntField("lots_batch"));
		stg->setStgOnlyClose(p.getIntField("only_close"));



		/*新增*/
		stg->setStgTradeModel(p.getStringField("trade_model"));
		stg->setStgOrderAlgorithm(p.getStringField("order_algorithm"));
		stg->setStgHoldProfit(p.getField("hold_profit").Double());
		stg->setStgCloseProfit(p.getField("close_profit").Double());
		stg->setStgCommisstion(p.getField("commission").Double());
		stg->setStgPosition(p.getIntField("position"));
		stg->setStgPositionBuy(p.getIntField("position_buy"));
		stg->setStgPositionSell(p.getIntField("position_sell"));
		stg->setStgTradeVolume(p.getIntField("trade_volume"));
		stg->setStgAmount(p.getField("amount").Double());
		stg->setStgAverageShift(p.getField("average_shift").Double());
		stg->setStgTradingDay(p.getStringField("trading_day"));


		stg->setStgAOrderActionTiresLimit(p.getIntField("a_order_action_limit"));
		stg->setStgBOrderActionTiresLimit(p.getIntField("b_order_action_limit"));
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
		stg->setStgALimitPriceShift(p.getIntField("a_limit_price_shift"));
		stg->setStgBLimitPriceShift(p.getIntField("b_limit_price_shift"));
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

void DBManager::getAllStrategyYesterdayByActiveUser(list<Strategy *> *l_strategys, list<User *> *l_users, string traderid) {
	std::cout << "DBManager::getAllStrategyYesterdayByActiveUser()" << std::endl;
	USER_PRINT("getAllStrategyYesterdayByActiveUser");
	list<User *>::iterator user_itor;
	/// 初始化的时候，必须保证list为空
	if (l_strategys->size() > 0) {
		list<Strategy *>::iterator Itor;
		for (Itor = l_strategys->begin(); Itor != l_strategys->end();) {
			delete (*Itor);
			Itor = l_strategys->erase(Itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	for (user_itor = l_users->begin(); user_itor != l_users->end(); user_itor++) {
		if (traderid.compare("")) { //如果traderid不为空

			cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << (*user_itor)->getUserID() << "is_active" << true));
		}
		else {
			cursor = this->conn->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("user_id" << (*user_itor)->getUserID() << "is_active" << true));
		}

		while (cursor->more()) {
			BSONObj p = cursor->next();
			Strategy *stg = new Strategy();
			cout << "\t*position_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
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
			cout << "a_order_action_limit = " << p.getIntField("a_order_action_limit") << ", ";
			cout << "b_order_action_limit = " << p.getIntField("b_order_action_limit") << ", ";
			cout << "sell_close = " << p.getField("sell_close").Double() << ", ";
			cout << "buy_open = " << p.getField("buy_open").Double() << ", ";
			cout << "only_close = " << p.getIntField("only_close") << ", ";

			///*新增字段*/

			cout << "trade_model" << p.getStringField("trade_model") << ", ";
			cout << "hold_profit" << p.getField("hold_profit").Double() << ", ";
			cout << "close_profit" << p.getField("close_profit").Double() << ", ";
			cout << "commission" << p.getField("commission").Double() << ", ";
			cout << "position" << p.getIntField("position") << ", ";
			cout << "position_buy" << p.getIntField("position_buy") << ", ";
			cout << "position_sell" << p.getIntField("position_sell") << ", ";
			cout << "trade_volume" << p.getIntField("trade_volume") << ", ";
			cout << "amount" << p.getField("amount").Double() << ", ";
			cout << "average_shift" << p.getField("average_shift").Double() << ", ";
			cout << "a_limit_price_shift" << p.getIntField("a_limit_price_shift") << ", ";
			cout << "b_limit_price_shift" << p.getIntField("b_limit_price_shift") << ", ";

			cout << "position_a_buy_yesterday = " << p.getIntField("position_a_buy_yesterday") << ", ";
			cout << "user_id = " << p.getStringField("user_id") << ", "; // string type
			cout << "position_a_buy_today = " << p.getIntField("position_a_buy_today") << ", ";
			cout << "position_a_sell_yesterday = " << p.getIntField("position_a_sell_yesterday") << ", ";
			cout << "lots = " << p.getIntField("lots") << ", ";
			cout << "a_wait_price_tick = " << p.getField("a_wait_price_tick").Double() << ", ";
			cout << "b_wait_price_tick = " << p.getField("b_wait_price_tick").Double() << ", ";
			//cout << "strategy_on_off = " << p.getField("strategy_on_off").Int() << endl;
			cout << "trading_day = " << p.getStringField("trading_day") << ", ";

			stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
			stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
			stg->setStgBuyClose(p.getField("buy_close").Double());
			stg->setStgBuyOpen(p.getField("buy_open").Double());
			stg->setStgIsActive(p.getField("is_active").Bool());
			stg->setStgLots(p.getIntField("lots"));
			stg->setStgLotsBatch(p.getIntField("lots_batch"));

			stg->setStgOnlyClose(p.getIntField("only_close"));
			stg->setOn_Off(p.getIntField("strategy_on_off"));
			stg->setStgSellOpenOnOff(p.getIntField("sell_open_on_off"));
			stg->setStgBuyCloseOnOff(p.getIntField("buy_close_on_off"));
			stg->setStgSellCloseOnOff(p.getIntField("sell_close_on_off"));
			stg->setStgBuyOpenOnOff(p.getIntField("buy_open_on_off"));

			/*新增*/
			stg->setStgTradeModel(p.getStringField("trade_model"));
			stg->setStgOrderAlgorithm(p.getStringField("order_algorithm"));
			stg->setStgHoldProfit(p.getField("hold_profit").Double());
			stg->setStgCloseProfit(p.getField("close_profit").Double());
			stg->setStgCommisstion(p.getField("commission").Double());
			stg->setStgPosition(p.getIntField("position"));
			stg->setStgPositionBuy(p.getIntField("position_buy"));
			stg->setStgPositionSell(p.getIntField("position_sell"));
			stg->setStgTradeVolume(p.getIntField("trade_volume"));
			stg->setStgAmount(p.getField("amount").Double());
			stg->setStgAverageShift(p.getField("average_shift").Double());
			stg->setStgALimitPriceShift(p.getIntField("a_limit_price_shift"));
			stg->setStgBLimitPriceShift(p.getIntField("b_limit_price_shift"));
			stg->setStgTradingDay(p.getStringField("trading_day"));

			// 设置是否有修改过持仓
			stg->setStgUpdatePositionDetailRecordTime(p.getStringField("update_position_detail_record_time"));
			// 当持仓修改时间不为空，说明曾经有修改
			if (stg->getStgUpdatePositionDetailRecordTime() != "")
			{
				stg->setStgIsPositionRight(false);
			}
			else {
				stg->setStgIsPositionRight(true);
			}

			/*std::cout << "after stg->setStgTradingDay = " << stg->getStgTradingDay() << std::endl;
			std::cout << "address stg = " << stg << std::endl;*/

			stg->setStgAOrderActionTiresLimit(p.getIntField("a_order_action_limit"));
			stg->setStgBOrderActionTiresLimit(p.getIntField("b_order_action_limit"));
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

			vector<BSONElement> elements = p["list_instrument_id"].Array();
			if (elements.size() > 1) {
				stg->setStgInstrumentIdA(elements[0].String());
				stg->setStgInstrumentIdB(elements[1].String());
				stg->addInstrumentToList(stg->getStgInstrumentIdA());
				stg->addInstrumentToList(stg->getStgInstrumentIdB());
				cout << "stg->setStgInstrumentIdA(elements[0]) = " << stg->getStgInstrumentIdA() << ", ";
				cout << "stg->setStgInstrumentIdA(elements[1]) = " << stg->getStgInstrumentIdB() << endl;
			}

			//for (vector<BSONElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
			//	cout << *it << endl;
			//}

			l_strategys->push_back(stg);
		}
	}

	USER_PRINT("DBManager::getAllStrategyYesterdayByActiveUser ok");
}

/************************************************************************/
/* 创建MD配置
删除MD配置
更新MD配置
获取所有MD配置
获取一条MD记录															*/
/************************************************************************/
void DBManager::CreateMarketConfig(MarketConfig *mc) {
	string DB_MARKETCONFIG_COLLECTION;
	if (this->is_online) {

		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig";
	}
	else
	{

		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig_panhou";
	}
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
	string DB_MARKETCONFIG_COLLECTION;
	if (this->is_online) {
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig";
	}
	else
	{
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig_panhou";
	}
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
	string DB_MARKETCONFIG_COLLECTION;
	if (this->is_online) {
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig";
	}
	else
	{
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig_panhou";
	}
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
	std::cout << "DBManager::getAllMarketConfig()" << std::endl;
	string DB_MARKETCONFIG_COLLECTION;
	if (this->is_online) {
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig";
	}
	else
	{
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig_panhou";
	}
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
			cout << "\t*" << "market_id:" << p.getStringField("market_id") << "  "
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
	std::cout << "DBManager::getOneMarketConfig()" << std::endl;
	USER_PRINT(this->is_online);
	string DB_MARKETCONFIG_COLLECTION;
	if (this->is_online) {
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig";
	}
	else
	{
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig_panhou";
	}
	int countnum = this->conn->count(DB_MARKETCONFIG_COLLECTION);
	if (countnum == 0) {
		cout << "\tDBManager::getOneMarketConfig None!" << endl;
	}
	else {
		cout << "\tMarketConfig * DBManager::getOneMarketConfig() isactive" << ISACTIVE << endl;
		BSONObj p = this->conn->findOne(DB_MARKETCONFIG_COLLECTION, BSON("isactive" << ISACTIVE));
		cout << "\tmarket_id = " << p.getStringField("market_id") << endl;
		if ((strcmp(p.getStringField("market_id"), ""))) {
			cout << "\t*" << "market_id:" << p.getStringField("market_id") << "  "
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

/************************************************************************/
/*  创建算法
删除算法
更新算法
获取算法															*/
/************************************************************************/
void DBManager::CreateAlgorithm(Algorithm *alg) {
	int count_number = 0;

	count_number = this->conn->count(DB_ALGORITHM_COLLECTION,
		BSON("name" << alg->getAlgName()));

	if (count_number > 0) {
		cout << "Algorithm Already Exists!" << endl;
	}
	else {
		BSONObjBuilder b;
		b.append("name", alg->getAlgName());
		b.append("isactive", alg->getIsActive());
		BSONObj p = b.obj();

		conn->insert(DB_ALGORITHM_COLLECTION, p);
		USER_PRINT("DBManager::CreateAlgorithm ok");
	}

}
void DBManager::DeleteAlgorithm(Algorithm *alg) {
	int count_number = 0;

	count_number = this->conn->count(DB_ALGORITHM_COLLECTION,
		BSON("name" << alg->getAlgName()));

	if (count_number > 0) {
		this->conn->update(DB_ALGORITHM_COLLECTION, BSON("name" << (alg->getAlgName().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeleteAlgorithm ok");
	}
	else {
		cout << "MarketConfig ID Not Exists!" << endl;
	}
}
void DBManager::UpdateAlgorithm(Algorithm *alg) {
	int count_number = 0;

	count_number = this->conn->count(DB_ALGORITHM_COLLECTION,
		BSON("name" << alg->getAlgName()));

	if (count_number > 0) {
		this->conn->update(DB_ALGORITHM_COLLECTION, BSON("name" << (alg->getAlgName().c_str())), BSON("$set" << BSON("name" << alg->getAlgName() << "isactive" << alg->getIsActive())));
		USER_PRINT("DBManager::UpdateAlgorithm ok");
	}
	else
	{
		cout << "Algorithm name Not Exists!" << endl;
	}

}
void DBManager::getAllAlgorithm(list<Algorithm *> *l_alg) {
	std::cout << "DBManager::getAllAlgorithm()" << std::endl;
	/// 初始化的时候，必须保证list为空
	if (l_alg->size() > 0) {
		list<Algorithm *>::iterator alg_itor;
		for (alg_itor = l_alg->begin(); alg_itor != l_alg->end();) {
			alg_itor = l_alg->erase(alg_itor);
		}
	}

	int countnum = this->conn->count(DB_ALGORITHM_COLLECTION);
	if (countnum == 0) {
		cout << "DBManager::getAllAlgorithm None!" << endl;
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			this->conn->query(DB_ALGORITHM_COLLECTION, MONGO_QUERY("isactive" << ISACTIVE));
		while (cursor->more()) {
			BSONObj p = cursor->next();
			cout << "\t*" << "name:" << p.getStringField("name") << "  "
				<< "isactive:" << p.getStringField("isactive") << "*" << endl;
			Algorithm *alg = new Algorithm();
			alg->setAlgName(p.getStringField("name"));
			alg->setIsActive(p.getStringField("isactive"));

			l_alg->push_back(alg);
		}
		USER_PRINT("DBManager::getAllAlgorithm ok");
	}
}

#if 0
/************************************************************************/
/* 创建sessionid
删除sessionid
获取sessionid*/
/************************************************************************/
void DBManager::CreateSession(Session *sid) {

	int session_id_count_number = 0;

	if (sid != NULL) {
		session_id_count_number = this->conn->count(DB_SESSIONS_COLLECTION,
			BSON("sessionid" << sid->getSessionID()));
		if (session_id_count_number != 0) { //session_id存在
			std::cout << "Session ID 已经存在了!" << std::endl;
			return;
		}
		else { //SessionID不存在
			BSONObjBuilder b;

			b.append("userid", sid->getUserID());
			b.append("sessionid", sid->getSessionID());
			b.append("frontid", sid->getFrontID());
			b.append("trading_day", sid->getTradingDay());

			BSONObj p = b.obj();
			conn->insert(DB_SESSIONS_COLLECTION, p);
			USER_PRINT("DBManager::CreateSessionID ok");
			
		}
	}
}

void DBManager::DeleteSession(Session *sid) {
	USER_PRINT("DBManager::DeleteSessionID");
	USER_PRINT(sid->getUserID());
	USER_PRINT(sid->getTradingDay());
	//this->conn->dropCollection(DB_SESSIONS_COLLECTION);
	this->conn->remove(DB_SESSIONS_COLLECTION, BSON("sessionid" << sid->getSessionID() << "frontid" << sid->getFrontID()));
	USER_PRINT("DBManager::DeleteSessionID ok");
}


void DBManager::getAllSession(list<Session *> *l_sessions) {
	/// 初始化的时候，必须保证list为空
	if (l_sessions->size() > 0) {
		list<Session *>::iterator Itor;
		for (Itor = l_sessions->begin(); Itor != l_sessions->end();) {
			Itor = l_sessions->erase(Itor);
		}
	}

	int countnum = this->conn->count(DB_SESSIONS_COLLECTION);
	USER_PRINT(countnum);
	if (countnum == 0) {
		cout << "DBManager::getAllSessionID is NONE!" << endl;
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			this->conn->query(DB_SESSIONS_COLLECTION);
		while (cursor->more()) {

			BSONObj p = cursor->next();
			cout << "*" << "userid:" << p.getStringField("userid") << "  "
				<< "sessionid:" << p.getIntField("sessionid") << "  "
				<< "trading_day:" << p.getStringField("trading_day") << "  "
				<< "frontid:" << p.getIntField("frontid") << endl;

			Session *sid = new Session(p.getStringField("userid"), p.getIntField("sessionid"), p.getIntField("frontid"), p.getStringField("trading_day"));

			l_sessions->push_back(sid);
		}
		USER_PRINT("DBManager::getAllSessionID ok");
	}
}

/************************************************************************/
/*
创建持仓明细
删除持仓明细
更新持仓明细*/
/************************************************************************/
void DBManager::CreatePositionDetail(PositionDetail *posd) {
	USER_PRINT("DBManager::CreatePositionDetail");

	int posd_count_num = 0;

	if (posd != NULL) {
		posd_count_num = this->conn->count(DB_POSITIONDETAIL_COLLECTION,
			BSON("userid" << posd->getUserID() << "strategyid" << posd->getStrategyID() << "tradingday" << posd->getTradingDay() << "is_active" << ISACTIVE));
		if (posd_count_num != 0) { //session_id存在
			std::cout << "持仓明细已经存在了!" << std::endl;
			return;
		}
		else { //posd 不存在
			BSONObjBuilder b;

			b.append("instrumentid", posd->getInstrumentID());
			b.append("orderref", posd->getOrderRef());
			b.append("userid", posd->getUserID());
			b.append("direction", posd->getDirection());
			b.append("comboffsetflag", posd->getCombOffsetFlag());
			b.append("combhedgeflag", posd->getCombHedgeFlag());
			b.append("limitprice", posd->getLimitPrice());
			b.append("volumetotaloriginal", posd->getVolumeTotalOriginal());
			b.append("tradingday", posd->getTradingDay());
			b.append("orderstatus", posd->getOrderStatus());
			b.append("volumetraded", posd->getVolumeTraded());
			b.append("volumetotal", posd->getVolumeTotal());
			b.append("insertdate", posd->getInsertDate());
			b.append("inserttime", posd->getInsertTime());
			b.append("strategyid", posd->getStrategyID());
			b.append("volumetradedbatch", posd->getVolumeTradedBatch());

			BSONObj p = b.obj();
			conn->insert(DB_POSITIONDETAIL_COLLECTION, p);
			USER_PRINT("DBManager::CreatePositionDetail ok");
		}
	}
	USER_PRINT("DBManager::CreatePositionDetail OK");
}

void DBManager::CreatePositionDetail(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("DBManager::CreatePositionDetail");

	int posd_count_num = 0;

	if (posd != NULL) {
		posd_count_num = this->conn->count(DB_POSITIONDETAIL_COLLECTION,
			BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "is_active" << ISACTIVE));
		if (posd_count_num != 0) { //session_id存在
			std::cout << "持仓明细已经存在了!" << std::endl;
			return;
		}
		else { //posd 不存在
			BSONObjBuilder b;

			b.append("instrumentid", posd->InstrumentID);
			b.append("orderref", posd->OrderRef);
			b.append("userid", posd->UserID);
			b.append("direction", posd->Direction);
			b.append("comboffsetflag", string(1, posd->CombOffsetFlag[0]));
			b.append("combhedgeflag", string(1, posd->CombHedgeFlag[0]));
			b.append("limitprice", posd->LimitPrice);
			b.append("volumetotaloriginal", posd->VolumeTotalOriginal);
			b.append("tradingday", posd->TradingDay);
			b.append("orderstatus", posd->OrderStatus);
			b.append("volumetraded", posd->VolumeTraded);
			b.append("volumetotal", posd->VolumeTotal);
			b.append("insertdate", posd->InsertDate);
			b.append("inserttime", posd->InsertTime);
			b.append("strategyid", posd->StrategyID);
			b.append("volumetradedbatch", posd->VolumeTradedBatch);

			BSONObj p = b.obj();
			conn->insert(DB_POSITIONDETAIL_COLLECTION, p);
			USER_PRINT("DBManager::CreatePositionDetail ok");
		}
	}
	USER_PRINT("DBManager::CreatePositionDetail OK");
}
#endif

void DBManager::DeletePositionDetail(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("DBManager::DeletePositionDetail");
	int count_number = 0;

	count_number = this->conn->count(DB_POSITIONDETAIL_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		this->conn->update(DB_POSITIONDETAIL_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeletePositionDetail ok");
	}
	else {
		cout << "删除order持仓明细,持仓明细不存在!" << endl;
	}
	USER_PRINT("DBManager::DeletePositionDetail OK");
}


void DBManager::UpdatePositionDetail(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("DBManager::UpdatePositionDetail");
	std::cout << "DBManager::UpdatePositionDetail()" << std::endl;
	
	int count_number = 0;

	count_number = this->conn->count(DB_POSITIONDETAIL_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		
		std::cout << "\tposd->TradingDayRecord = " << posd->TradingDayRecord << std::endl;
		this->conn->update(DB_POSITIONDETAIL_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
			<< "orderref" << posd->OrderRef
			<< "userid" << posd->UserID
			<< "direction" << posd->Direction
			<< "comboffsetflag" << posd->CombOffsetFlag
			<< "combhedgeflag" << posd->CombHedgeFlag
			<< "limitprice" << posd->LimitPrice
			<< "volumetotaloriginal" << posd->VolumeTotalOriginal
			<< "tradingday" << posd->TradingDay
			<< "tradingdayrecord" << posd->TradingDayRecord
			<< "orderstatus" << posd->OrderStatus
			<< "volumetraded" << posd->VolumeTraded
			<< "volumetotal" << posd->VolumeTotal
			<< "insertdate" << posd->InsertDate
			<< "inserttime" << posd->InsertTime
			<< "strategyid" << posd->StrategyID
			<< "volumetradedbatch" << posd->VolumeTradedBatch
			)));
		USER_PRINT("DBManager::UpdatePositionDetail ok");
	}
	else {
		std::cout << "更新今持仓明细,不存在!" << std::endl;
	}

	USER_PRINT("DBManager::UpdatePositionDetail OK");
}



void DBManager::getAllPositionDetail(list<USER_CThostFtdcOrderField *> *l_posd, string traderid, string userid) {
	USER_PRINT("DBManager::getAllPositionDetail");
	std::cout << "DBManager::getAllPositionDetail()" << std::endl;
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CThostFtdcOrderField *>::iterator itor;
		for (itor = l_posd->begin(); itor != l_posd->end();) {
			delete (*itor);
			itor = l_posd->erase(itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	if (traderid.compare("")) { //如果traderid不为空

		if (userid.compare("")) { //如果userid不为空
			cursor = this->conn->query(DB_POSITIONDETAIL_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			cursor = this->conn->query(DB_POSITIONDETAIL_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		cursor = this->conn->query(DB_POSITIONDETAIL_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {
		
		BSONObj p = cursor->next();

		USER_CThostFtdcOrderField *new_pos = new USER_CThostFtdcOrderField();

		strcpy(new_pos->InstrumentID, p.getStringField("instrumentid"));
		strcpy(new_pos->OrderRef, p.getStringField("orderref"));
		strcpy(new_pos->UserID, p.getStringField("userid"));
		new_pos->Direction = p.getIntField("direction");
		strcpy(new_pos->CombOffsetFlag, p.getStringField("comboffsetflag"));
		strcpy(new_pos->CombHedgeFlag, p.getStringField("combhedgeflag"));
		new_pos->LimitPrice = p.getField("limitprice").Double();
		new_pos->VolumeTotalOriginal = p.getIntField("volumetotaloriginal");
		strcpy(new_pos->TradingDay, p.getStringField("tradingday"));
		strcpy(new_pos->TradingDayRecord, p.getStringField("tradingdayrecord"));
		new_pos->OrderStatus = p.getIntField("orderstatus");
		new_pos->VolumeTraded = p.getIntField("volumetraded");
		new_pos->VolumeTotal = p.getIntField("volumetotal");
		strcpy(new_pos->InsertDate, p.getStringField("insertdate"));
		strcpy(new_pos->InsertTime, p.getStringField("inserttime"));
		strcpy(new_pos->StrategyID, p.getStringField("strategyid"));
		new_pos->VolumeTradedBatch = p.getIntField("volumetradedbatch");

		std::cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
		std::cout << "orderref = " << p.getStringField("orderref") << ", ";
		std::cout << "userid = " << p.getStringField("userid") << ", ";
		std::cout << "direction = " << p.getIntField("direction") << ", ";
		std::cout << "comboffsetflag = " << p.getStringField("comboffsetflag") << ", ";
		std::cout << "combhedgeflag = " << p.getStringField("combhedgeflag") << ", ";
		std::cout << "limitprice = " << p.getField("limitprice").Double() << ", ";
		std::cout << "volumetotaloriginal = " << p.getIntField("volumetotaloriginal") << ", ";
		std::cout << "tradingday = " << p.getStringField("tradingday") << ", ";
		std::cout << "tradingdayrecord = " << p.getStringField("tradingdayrecord") << ", ";
		std::cout << "orderstatus = " << p.getIntField("orderstatus") << ", ";
		std::cout << "volumetraded = " << p.getIntField("volumetraded") << ", ";
		std::cout << "volumetotal = " << p.getIntField("volumetotal") << ", ";
		std::cout << "insertdate = " << p.getStringField("insertdate") << ", ";
		std::cout << "inserttime = " << p.getStringField("inserttime") << ", ";
		std::cout << "strategyid = " << p.getStringField("strategyid") << ", ";
		std::cout << "volumetradedbatch = " << p.getIntField("volumetradedbatch") << ", ";
		std::cout << "new_pos->StrategyID = " << new_pos->StrategyID << std::endl;

		l_posd->push_back(new_pos);
	}

	USER_PRINT("DBManager::getAllPositionDetail OK");
}

void DBManager::DropPositionDetail() {
	USER_PRINT("DBManager::DropPositionDetail");
	this->conn->dropCollection(DB_POSITIONDETAIL_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetail ok");
}

void DBManager::CreatePositionDetailYesterday(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("DBManager::CreatePositionDetailYesterday");

	int posd_count_num = 0;

	if (posd != NULL) {
		posd_count_num = this->conn->count(DB_POSITIONDETAIL_YESTERDAY_COLLECTION,
			BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));
		if (posd_count_num != 0) { //session_id存在
			std::cout << "持仓明细已经存在了!" << std::endl;
			return;
		}
		else { //posd 不存在
			BSONObjBuilder b;

			b.append("instrumentid", posd->InstrumentID);
			b.append("orderref", posd->OrderRef);
			b.append("userid", posd->UserID);
			b.append("direction", posd->Direction);
			b.append("comboffsetflag", posd->CombOffsetFlag);
			b.append("combhedgeflag", posd->CombHedgeFlag);
			b.append("limitprice", posd->LimitPrice);
			b.append("volumetotaloriginal", posd->VolumeTotalOriginal);
			b.append("tradingday", posd->TradingDay);
			b.append("tradingdayrecord", posd->TradingDayRecord);
			b.append("orderstatus", posd->OrderStatus);
			b.append("volumetraded", posd->VolumeTraded);
			b.append("volumetotal", posd->VolumeTotal);
			b.append("insertdate", posd->InsertDate);
			b.append("inserttime", posd->InsertTime);
			b.append("strategyid", posd->StrategyID);
			b.append("volumetradedbatch", posd->VolumeTradedBatch);
			b.append("is_active", ISACTIVE);

			BSONObj p = b.obj();
			conn->insert(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, p);
			USER_PRINT("DBManager::CreatePositionDetailYesterday ok");
		}
	}
	USER_PRINT("DBManager::CreatePositionDetailYesterday OK");
}
void DBManager::DeletePositionDetailYesterday(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("DBManager::DeletePositionDetailYesterday");
	int count_number = 0;

	count_number = this->conn->count(DB_POSITIONDETAIL_YESTERDAY_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		this->conn->update(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeletePositionDetail ok");
	}
	else {
		cout << "删除昨持仓明细,持仓明细不存在!" << endl;
	}
	USER_PRINT("DBManager::DeletePositionDetailYesterday OK");
}

void DBManager::UpdatePositionDetailYesterday(USER_CThostFtdcOrderField *posd) {
	USER_PRINT("DBManager::UpdatePositionDetailYesterday");
	int count_number = 0;

	count_number = this->conn->count(DB_POSITIONDETAIL_YESTERDAY_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		this->conn->update(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
			<< "orderref" << posd->OrderRef
			<< "userid" << posd->UserID
			<< "direction" << posd->Direction
			<< "comboffsetflag" << posd->CombOffsetFlag
			<< "combhedgeflag" << posd->CombHedgeFlag
			<< "limitprice" << posd->LimitPrice
			<< "volumetotaloriginal" << posd->VolumeTotalOriginal
			<< "tradingday" << posd->TradingDay
			<< "tradingdayrecord" << posd->TradingDayRecord
			<< "orderstatus" << posd->OrderStatus
			<< "volumetraded" << posd->VolumeTraded
			<< "volumetotal" << posd->VolumeTotal
			<< "insertdate" << posd->InsertDate
			<< "inserttime" << posd->InsertTime
			<< "strategyid" << posd->StrategyID
			<< "volumetradedbatch" << posd->VolumeTradedBatch
			)));
		USER_PRINT("DBManager::UpdatePositionDetailYesterday ok");
	}
	else {
		cout << "更新昨持仓明细,持仓明细未找到!" << endl;
	}
	USER_PRINT("DBManager::UpdatePositionDetailYesterday OK");
}

void DBManager::getAllPositionDetailYesterday(list<USER_CThostFtdcOrderField *> *l_posd,
	string traderid, string userid) {
	USER_PRINT("DBManager::getAllPositionDetailYesterday");
	std::cout << "DBManager::getAllPositionDetailYesterday()" << std::endl;
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CThostFtdcOrderField *>::iterator itor;
		for (itor = l_posd->begin(); itor != l_posd->end();) {
			delete (*itor);
			itor = l_posd->erase(itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	if (traderid.compare("")) { //如果traderid不为空

		USER_PRINT("如果traderid不为空");

		if (userid.compare("")) { //如果userid不为空
			USER_PRINT("如果userid不为空");
			cursor = this->conn->query(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			USER_PRINT("如果userid为空");
			cursor = this->conn->query(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		USER_PRINT("如果traderid为空");
		cursor = this->conn->query(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();

		USER_CThostFtdcOrderField *new_pos = new USER_CThostFtdcOrderField();

		strcpy(new_pos->InstrumentID, p.getStringField("instrumentid"));
		strcpy(new_pos->OrderRef, p.getStringField("orderref"));
		strcpy(new_pos->UserID, p.getStringField("userid"));
		new_pos->Direction = p.getIntField("direction");
		strcpy(new_pos->CombOffsetFlag, p.getStringField("comboffsetflag"));
		strcpy(new_pos->CombHedgeFlag, p.getStringField("combhedgeflag"));
		new_pos->LimitPrice = p.getField("limitprice").Double();
		new_pos->VolumeTotalOriginal = p.getIntField("volumetotaloriginal");
		strcpy(new_pos->TradingDay, p.getStringField("tradingday"));
		strcpy(new_pos->TradingDayRecord, p.getStringField("tradingdayrecord"));
		new_pos->OrderStatus = p.getIntField("orderstatus");
		new_pos->VolumeTraded = p.getIntField("volumetraded");
		new_pos->VolumeTotal = p.getIntField("volumetotal");
		strcpy(new_pos->InsertDate, p.getStringField("insertdate"));
		strcpy(new_pos->InsertTime, p.getStringField("inserttime"));
		strcpy(new_pos->StrategyID, p.getStringField("strategyid"));
		new_pos->VolumeTradedBatch = p.getIntField("volumetradedbatch");

		cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
		cout << "orderref = " << p.getStringField("orderref") << ", ";
		cout << "userid = " << p.getStringField("userid") << ", ";
		cout << "direction = " << p.getIntField("direction") << ", ";
		cout << "comboffsetflag = " << p.getStringField("comboffsetflag") << ", ";
		cout << "combhedgeflag = " << p.getStringField("combhedgeflag") << ", ";
		cout << "limitprice = " << p.getField("limitprice").Double() << ", ";
		cout << "volumetotaloriginal = " << p.getIntField("volumetotaloriginal") << ", ";
		cout << "tradingday = " << p.getStringField("tradingday") << ", ";
		cout << "tradingdayrecord = " << p.getStringField("tradingdayrecord") << ", ";
		cout << "orderstatus = " << p.getIntField("orderstatus") << ", ";
		cout << "volumetraded = " << p.getIntField("volumetraded") << ", ";
		cout << "volumetotal = " << p.getIntField("volumetotal") << ", ";
		cout << "insertdate = " << p.getStringField("insertdate") << ", ";
		cout << "inserttime = " << p.getStringField("inserttime") << ", ";
		cout << "strategyid = " << p.getStringField("strategyid") << ", ";
		cout << "volumetradedbatch = " << p.getIntField("volumetradedbatch") << std::endl;

		l_posd->push_back(new_pos);
	}

	USER_PRINT("DBManager::getAllPositionDetailYesterday OK");
}

void DBManager::DropPositionDetailYesterday() {
	USER_PRINT("DBManager::DropPositionDetailYesterday");
	this->conn->dropCollection(DB_POSITIONDETAIL_YESTERDAY_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetailYesterday ok");
}

//根据策略,删除策略对应的order,trade持仓明细
bool DBManager::DeletePositionDetailByStrategy(Strategy *stg) {
	USER_PRINT("DBManager::DeletePositionDetailByStrategy");
	std::cout << "DBManager::DeletePositionDetailByStrategy()" << std::endl;
	bool flag = true;
	int count_number = this->conn->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str()) << "trading_day" << (stg->getStgTradingDay().c_str()) << "is_active" << true));

	if (count_number > 0) {
		std::cout << "\t策略存在,开始删除对应策略持仓明细" << std::endl;
		std::cout << "\t策略存在,开始删除持仓明细order" << std::endl;
		this->conn->remove(DB_POSITIONDETAIL_COLLECTION, MONGO_QUERY("strategyid" << stg->getStgStrategyId().c_str() << "userid" << stg->getStgUserId().c_str() << "is_active" << ISACTIVE));
		std::cout << "\t策略存在,开始删除持仓明细trade" << std::endl;
		this->conn->remove(DB_POSITIONDETAIL_TRADE_COLLECTION, MONGO_QUERY("strategyid" << stg->getStgStrategyId().c_str() << "userid" << stg->getStgUserId().c_str() << "is_active" << ISACTIVE));
		flag = true;
	}
	else {
		cout << "\t策略不存在!!" << endl;
		flag = false;
	}

	USER_PRINT("DBManager::DeletePositionDetailByStrategy ok");
	return flag;
}

void DBManager::DeletePositionDetailTrade(USER_CThostFtdcTradeField *posd) {
	USER_PRINT("DBManager::DeletePositionDetailTrade");
	int count_number = 0;

	count_number = this->conn->count(DB_POSITIONDETAIL_TRADE_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		this->conn->update(DB_POSITIONDETAIL_TRADE_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeletePositionDetailTrade ok");
	}
	else {
		cout << "删除trade持仓明细,持仓明细不存在!" << endl;
	}
	USER_PRINT("DBManager::DeletePositionDetailTrade OK");
}

void DBManager::UpdatePositionDetailTrade(USER_CThostFtdcTradeField *posd) {
	USER_PRINT("DBManager::UpdatePositionDetailTrade");

	int count_number = 0;

	count_number = this->conn->count(DB_POSITIONDETAIL_TRADE_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		this->conn->update(DB_POSITIONDETAIL_TRADE_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
			<< "orderref" << posd->OrderRef
			<< "userid" << posd->UserID
			<< "direction" << posd->Direction
			<< "offsetflag" << posd->OffsetFlag
			<< "hedgeflag" << posd->HedgeFlag
			<< "price" << posd->Price
			<< "tradingday" << posd->TradingDay
			<< "tradingdayrecord" << posd->TradingDayRecord
			<< "tradedate" << posd->TradeDate
			<< "strategyid" << posd->StrategyID
			<< "volume" << posd->Volume
			)));
		USER_PRINT("DBManager::UpdatePositionDetail ok");
	}
	else {
		cout << "更新今持仓明细trade,不存在!" << endl;
	}

	USER_PRINT("DBManager::UpdatePositionDetailTrade OK");
}

void DBManager::getAllPositionDetailTrade(list<USER_CThostFtdcTradeField *> *l_posd, string trader_id, string userid) {
	USER_PRINT("DBManager::getAllPositionDetailTrade");
	std::cout << "DBManager::getAllPositionDetailTrade()" << std::endl;
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CThostFtdcTradeField *>::iterator itor;
		for (itor = l_posd->begin(); itor != l_posd->end();) {
			delete (*itor);
			itor = l_posd->erase(itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	if (trader_id.compare("")) { //如果traderid不为空

		if (userid.compare("")) { //如果userid不为空
			cursor = this->conn->query(DB_POSITIONDETAIL_TRADE_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			cursor = this->conn->query(DB_POSITIONDETAIL_TRADE_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		cursor = this->conn->query(DB_POSITIONDETAIL_TRADE_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {

		BSONObj p = cursor->next();

		USER_CThostFtdcTradeField *new_pos = new USER_CThostFtdcTradeField();

		

		strcpy(new_pos->InstrumentID, p.getStringField("instrumentid"));
		strcpy(new_pos->OrderRef, p.getStringField("orderref"));
		strcpy(new_pos->UserID, p.getStringField("userid"));
		new_pos->Direction = p.getIntField("direction");
		new_pos->OffsetFlag = p.getIntField("offsetflag");
		new_pos->HedgeFlag = p.getIntField("hedgeflag");
		new_pos->Price = p.getField("price").Double();
		strcpy(new_pos->TradingDay, p.getStringField("tradingday"));
		strcpy(new_pos->TradingDayRecord, p.getStringField("tradingdayrecord"));
		strcpy(new_pos->TradeDate, p.getStringField("tradedate"));
		strcpy(new_pos->StrategyID, p.getStringField("strategyid"));
		new_pos->Volume = p.getIntField("volume");


		cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
		cout << "orderref = " << p.getStringField("orderref") << ", ";
		cout << "userid = " << p.getStringField("userid") << ", ";
		cout << "direction = " << p.getIntField("direction") << ", ";
		cout << "offsetflag = " << p.getStringField("offsetflag") << ", ";
		cout << "hedgeflag = " << p.getStringField("hedgeflag") << ", ";
		cout << "price = " << p.getField("price").Double() << ", ";
		cout << "tradingday = " << p.getStringField("tradingday") << ", ";
		cout << "tradingdayrecord = " << p.getStringField("tradingdayrecord") << ", ";
		cout << "tradedate = " << p.getStringField("tradedate") << ", ";
		cout << "strategyid = " << p.getStringField("strategyid") << ", ";
		cout << "volume = " << p.getIntField("volume") << ", ";
		std::cout << "new_pos->StrategyID = " << new_pos->StrategyID << std::endl;

		l_posd->push_back(new_pos);
	}

	USER_PRINT("DBManager::getAllPositionDetailTrade OK");
}

void DBManager::DropPositionDetailTrade() {
	USER_PRINT("DBManager::DropPositionDetailTrade");
	this->conn->dropCollection(DB_POSITIONDETAIL_TRADE_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetailTrade ok");
}

/************************************************************************/
/* 更新修改持仓明细记录                                                    */
/************************************************************************/
bool DBManager::UpdatePositionModifyRecord(Strategy *stg, string recordtime) {
	std::cout << "DBManager::UpdatePositionModifyRecord()" << std::endl;
	int count_number = 0;
	bool flag = true;
	count_number = this->conn->count(DB_POSITIONMODIFYRECORD_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str()) << "is_active" << true));

	if (count_number > 0) {
		cout << "\t策略已存在,进行更新操作!" << endl;

	}
	else {
		cout << "\t策略不存在!" << endl;
		flag = false;
	}
	return flag;
}

void DBManager::CreatePositionDetailTradeYesterday(USER_CThostFtdcTradeField *posd) {
	USER_PRINT("DBManager::CreatePositionDetailTradeYesterday");

	int posd_count_num = 0;

	if (posd != NULL) {
		
		posd_count_num = this->conn->count(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION,
			BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));
		if (posd_count_num != 0) { //session_id存在
			std::cout << "持仓明细trade已经存在了!" << std::endl;
			return;
		}
		else { //posd 不存在
			BSONObjBuilder b;

			b.append("instrumentid", posd->InstrumentID);
			b.append("orderref", posd->OrderRef);
			b.append("userid", posd->UserID);
			b.append("direction", posd->Direction);
			b.append("offsetflag", posd->OffsetFlag);
			b.append("hedgeflag", posd->HedgeFlag);
			b.append("price", posd->Price);
			b.append("tradingday", posd->TradingDay);
			b.append("tradingdayrecord", posd->TradingDayRecord);
			b.append("tradedate", posd->TradeDate);
			b.append("strategyid", posd->StrategyID);
			b.append("volume", posd->Volume);
			b.append("is_active", ISACTIVE);

			BSONObj p = b.obj();
			conn->insert(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, p);
			USER_PRINT("DBManager::CreatePositionDetailTradeYesterday ok");
		}
	}
	USER_PRINT("DBManager::CreatePositionDetailYesterday OK");
}

void DBManager::DeletePositionDetailTradeYesterday(USER_CThostFtdcTradeField *posd) {
	USER_PRINT("DBManager::DeletePositionDetailTradeYesterday");
	int count_number = 0;

	count_number = this->conn->count(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		this->conn->update(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeletePositionDetailTradeYesterday ok");
	}
	else {
		cout << "删除昨持仓明细,持仓明细不存在!" << endl;
	}
	USER_PRINT("DBManager::DeletePositionDetailTradeYesterday OK");
}

void DBManager::UpdatePositionDetailTradeYesterday(USER_CThostFtdcTradeField *posd) {
	USER_PRINT("DBManager::UpdatePositionDetailTradeYesterday");
	int count_number = 0;

	count_number = this->conn->count(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		this->conn->update(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
			<< "orderref" << posd->OrderRef
			<< "userid" << posd->UserID
			<< "direction" << posd->Direction
			<< "offsetflag" << posd->OffsetFlag
			<< "hedgeflag" << posd->HedgeFlag
			<< "price" << posd->Price
			<< "tradingday" << posd->TradingDay
			<< "tradingdayrecord" << posd->TradingDayRecord
			<< "tradedate" << posd->TradeDate
			<< "strategyid" << posd->StrategyID
			<< "volume" << posd->Volume
			)));
		USER_PRINT("DBManager::UpdatePositionDetailTradeYesterday ok");
	}
	else {
		cout << "更新昨持仓明细,持仓明细未找到!" << endl;
	}
	USER_PRINT("DBManager::UpdatePositionDetailTradeYesterday OK");
}

void DBManager::getAllPositionDetailTradeYesterday(list<USER_CThostFtdcTradeField *> *l_posd, string trader_id, string userid) {
	USER_PRINT("DBManager::getAllPositionDetailYesterday");
	std::cout << "DBManager::getAllPositionDetailTradeYesterday()" << std::endl;
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CThostFtdcTradeField *>::iterator itor;
		for (itor = l_posd->begin(); itor != l_posd->end();) {
			delete (*itor);
			itor = l_posd->erase(itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	if (trader_id.compare("")) { //如果traderid不为空

		USER_PRINT("如果traderid不为空");

		if (userid.compare("")) { //如果userid不为空
			USER_PRINT("如果userid不为空");
			cursor = this->conn->query(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			USER_PRINT("如果userid为空");
			cursor = this->conn->query(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		USER_PRINT("如果traderid为空");
		cursor = this->conn->query(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();

		USER_CThostFtdcTradeField *new_pos = new USER_CThostFtdcTradeField();

		strcpy(new_pos->InstrumentID, p.getStringField("instrumentid"));
		strcpy(new_pos->OrderRef, p.getStringField("orderref"));
		strcpy(new_pos->UserID, p.getStringField("userid"));
		new_pos->Direction = p.getIntField("direction");
		new_pos->OffsetFlag = p.getIntField("offsetflag");
		new_pos->HedgeFlag = p.getIntField("hedgeflag");
		new_pos->Price = p.getField("price").Double();
		strcpy(new_pos->TradingDay, p.getStringField("tradingday"));
		strcpy(new_pos->TradingDayRecord, p.getStringField("tradingdayrecord"));
		strcpy(new_pos->TradeDate, p.getStringField("tradedate"));
		strcpy(new_pos->StrategyID, p.getStringField("strategyid"));
		new_pos->Volume = p.getIntField("volume");

		cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
		cout << "orderref = " << p.getStringField("orderref") << ", ";
		cout << "userid = " << p.getStringField("userid") << ", ";
		cout << "direction = " << p.getIntField("direction") << ", ";
		cout << "offsetflag = " << p.getIntField("offsetflag") << ", ";
		cout << "hedgeflag = " << p.getIntField("hedgeflag") << ", ";
		cout << "price = " << p.getField("price").Double() << ", ";
		cout << "tradingday = " << p.getStringField("tradingday") << ", ";
		cout << "tradingdayrecord = " << p.getStringField("tradingdayrecord") << ", ";
		cout << "tradedate = " << p.getIntField("tradedate") << ", ";
		cout << "strategyid = " << p.getStringField("strategyid") << ", ";
		cout << "volume = " << p.getIntField("volume") << std::endl;

		l_posd->push_back(new_pos);
	}

	USER_PRINT("DBManager::getAllPositionDetailYesterday OK");
}

void DBManager::DropPositionDetailTradeYesterday() {
	USER_PRINT("DBManager::DropPositionDetailTradeYesterday");
	this->conn->dropCollection(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetailTradeYesterday ok");
}

void DBManager::UpdateSystemRunningStatus(string value) {
	USER_PRINT("DBManager::UpdateSystemRunningStatus");

	std::cout << "DBManager::UpdateSystemRunningStatus value = " << value << std::endl;
	std::cout << "DBManager::UpdateSystemRunningStatus key = " << DB_RUNNING_KEY << std::endl;
	int count_number = this->conn->count(DB_SYSTEM_RUNNING_STATUS_COLLECTION,
		BSON("key" << DB_RUNNING_KEY));

	if (count_number > 0) {
		this->conn->update(DB_SYSTEM_RUNNING_STATUS_COLLECTION, MONGO_QUERY("key" << DB_RUNNING_KEY), BSON("$set" << BSON("status" << value)));
	}
	else {
		std::cout << "\t查找数量为0！" << std::endl;
	}
	
	USER_PRINT("DBManager::UpdateSystemRunningStatus ok");
}

bool DBManager::GetSystemRunningStatus() {
	USER_PRINT("DBManager::GetSystemRunningStatus");
	bool flag = false;;
	BSONObj obj = this->conn->findOne(DB_SYSTEM_RUNNING_STATUS_COLLECTION, MONGO_QUERY("key" << DB_RUNNING_KEY));
	if (!obj.isEmpty()) {
		if (!strcmp("off", obj.getStringField("status"))) {
			flag = true;
		}
		else {
			flag = false;
		}
	}
	return flag;
	USER_PRINT("DBManager::GetSystemRunningStatus ok");
}


void DBManager::setConn(mongo::DBClientConnection *conn) {
	this->conn = conn;
}

mongo::DBClientConnection * DBManager::getConn() {
	return this->conn;
}

void DBManager::setDB_Connect_Status(bool db_connect_status) {
	this->db_connect_status = db_connect_status;
}

bool DBManager::getDB_Connect_Status() {
	return this->db_connect_status;
}
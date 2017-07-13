#include <mongo/bson/bson.h>
#include <iostream>
#include <mutex>
#include "DBManager.h"
#include "Debug.h"
#include "Utils.h"
#include "INIReader.h"

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
#define DB_AUTH_DB								"admin"
#define DB_OPERATOR_COLLECTION					"CTP.trader"
#define DB_SESSIONS_COLLECTION					"CTP.sessions"
#define DB_ADMIN_COLLECTION						"CTP.admin"
#define DB_STRATEGY_COLLECTION					"CTP.strategy"
#define DB_STRATEGY_CHANGED_COLLECTION			"CTP.strategy_changed"
#define DB_POSITIONDETAIL_COLLECTION			"CTP.positiondetail"
#define DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION	"CTP.positiondetail_changed"
#define DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION	"CTP.positiondetail_trade_changed"
#define DB_POSITIONMODIFYRECORD_COLLECTION		"CTP.positionmodifyrecord"
#define DB_POSITIONDETAIL_YESTERDAY_COLLECTION	"CTP.positiondetail_yesterday"
#define DB_POSITIONDETAIL_TRADE_COLLECTION		"CTP.positiondetail_trade"
#define DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION	"CTP.positiondetail_trade_yesterday"
#define DB_SYSTEM_RUNNING_STATUS_COLLECTION		"CTP.system_running_status"
#define DB_RUNNING_KEY							"BEES_HOME"
#define DB_STRATEGY_YESTERDAY_COLLECTION		"CTP.strategy_yesterday"
#define DB_ALGORITHM_COLLECTION					"CTP.algorithm"
#define DB_FUTURE_BEE_COLLECTION				"CTP.futureaccount_bee"
#define DB_MARKET_BEE_COLLECTION				"CTP.marketconfig_bee"
#define ISACTIVE "1"
#define ISNOTACTIVE "0"

DBManager::DBManager(int max_size) {

	bool auto_conn = true;
	double time_out = 3;
	// 默认连接状态为true
	this->db_connect_status = true;
	// 默认权限认证状态为true
	this->auth_failed = false;

	// 读取mongo配置文件
	INIReader reader("config/mongo.ini");

	if (reader.ParseError() < 0) {
		Utils::printRedColor("无法打开mongo.ini配置文件");
		exit(1);
	}

	string str_mechanism = reader.Get("manager", "mechanism", "UNKNOWN");
	string str_user = reader.Get("manager", "user", "UNKNOWN");
	string str_password = reader.Get("manager", "password", "UNKNOWN");

	if (str_mechanism == "UNKNOWN" || str_user == "UNKNOWN" || str_password == "UNKNOWN")
	{
		Utils::printRedColor("mongo.ini配置有误");
		exit(1);
	}

	//建立连接队列
	for (int i = 0; i < max_size; i++) {
		if (!this->db_connect_status) {
			break;
		}
		try {
			mongo::DBClientConnection *conn = new mongo::DBClientConnection(auto_conn, 0, time_out);
			if (conn != NULL)
			{
				conn->connect("localhost");
				conn->auth(BSON("mechanism" << str_mechanism << "user" << str_user << "pwd" << str_password << "db" << DB_AUTH_DB));
				this->queue_DBClient.enqueue(conn);
			}
			else {
				this->db_connect_status = false;
			}
		}
		catch (const mongo::ConnectException &e) {
			std::cout << "MongoDB无法访问! 问题:" << e.what() << std::endl;
			this->db_connect_status = false;
		}
		catch (const mongo::SocketException &e) {
			std::cout << "MongoDB无法访问! 问题:" << e.what() << std::endl;
			this->db_connect_status = false;
		}
		catch (const mongo::DBException &e) {
			std::cout << "MongoDB无法访问! 问题:" << e.what() << std::endl;
			this->db_connect_status = false;
		}
	}

	string logpath = "logs/";
	int flag = Utils::CreateFolder(logpath.c_str());
	if (flag != 0) {
		Utils::printRedColor("无法创建日志文件夹!");
		exit(1);
	}
	try {
		this->xts_db_logger = spdlog::get("xts_async_db_logger");
		if (!this->xts_db_logger)
		{
			// 设置缓冲区大小
			spdlog::set_async_mode(4096);
			// 创建日志文件
			this->xts_db_logger = spdlog::daily_logger_mt("xts_async_db_logger", "logs/xts_log_db.txt");
		}
		
	}
	catch (const spdlog::spdlog_ex& ex) {
		Utils::printRedColor("DB Log初始化失败,错误!");
		Utils::printRedColor(ex.what());
		exit(1);
	}
}

DBManager::~DBManager() {
	delete this;
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
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	std::cout << "DBManager::CheckSystemStartFlag()" << std::endl;
	list<string> collection_names;
	bool flag_one = false;
	bool flag_two = false;
	bool flag = false;
	collection_names = client->getCollectionNames(DB_NAME);
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
		flag = true;
	}
	else {
		if (flag_one) {
			std::cout << "\tpositiondetail_yesterday exists." << std::endl; 
		}
		else {
			std::cout << "\tpositiondetail_yesterday NOT exists." << std::endl; 
		}
		if (flag_two) {
			std::cout << "\tpositiondetail_trade_yesterday exists." << std::endl; 
		}
		else {
			std::cout << "\tpositiondetail_trade_yesterday NOT exists." << std::endl; 
		}
		flag = false;
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	return flag;
}


// 创建Trader
void DBManager::CreateTrader(Trader *op) {
	
	// 从队列中获取连接
	mongo::DBClientConnection *client = this->getConn();

	int count_number = 0;

	count_number = client->count(DB_OPERATOR_COLLECTION,
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

		client->insert(DB_OPERATOR_COLLECTION, p);
		
		USER_PRINT("DBManager::CreateOperator ok");
	}

	// 重新加到连接队列
	this->recycleConn(client);
}

void DBManager::DeleteTrader(Trader *op) {

	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();

	int count_number = 0;

	count_number = client->count(DB_OPERATOR_COLLECTION,
		BSON("traderid" << op->getTraderID()));

	if (count_number > 0) {
		client->update(DB_OPERATOR_COLLECTION, BSON("traderid" << (op->getTraderID().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeleteOperator ok");
	}
	else {
		cout << "Trader ID Not Exists!" << endl;
	}
	
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::UpdateTrader(string traderid, Trader *op) {

	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();

	int count_number = 0;

	count_number = client->count(DB_OPERATOR_COLLECTION,
		BSON("traderid" << traderid));

	if (count_number > 0) {
		client->update(DB_OPERATOR_COLLECTION, BSON("traderid" << (traderid.c_str())), BSON("$set" << BSON("tradername" << op->getTraderName() << "traderid" << op->getTraderID() << "password" << op->getPassword() << "on_off" << op->getOn_Off() << "isactive" << op->getIsActive())));
		USER_PRINT("DBManager::UpdateOperator ok");
	}
	else
	{
		cout << "Trader ID Not Exists!" << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::SearchTraderByTraderID(string traderid) {

	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();

	unique_ptr<DBClientCursor> cursor = client->query(DB_OPERATOR_COLLECTION, MONGO_QUERY("traderid" << traderid));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		cout << "userid = " << p.getStringField("traderid") << endl;
		cout << "username = " << p.getStringField("tradername") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;
		cout << "on_off = " << p.getIntField("on_off") << endl;
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::SearchTraderByTraderID ok");
}

void DBManager::SearchTraderByTraderName(string tradername) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	unique_ptr<DBClientCursor> cursor = client->query(DB_OPERATOR_COLLECTION, MONGO_QUERY("tradername" << tradername));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		cout << "traderid = " << p.getStringField("traderid") << endl;
		cout << "tradername = " << p.getStringField("tradername") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;
		cout << "on_off = " << p.getIntField("on_off") << endl;
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::SearchTraderByTraderName ok");
}

void DBManager::SearchTraderByTraderIdAndPassword(string traderid, string password) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	unique_ptr<DBClientCursor> cursor = client->query(DB_OPERATOR_COLLECTION, MONGO_QUERY("traderid" << traderid << "password" << password));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		cout << "traderid = " << p.getStringField("traderid") << endl;
		cout << "tradername = " << p.getStringField("tradername") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;
		cout << "on_off = " << p.getIntField("on_off") << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::SearchTraderByTraderIdAndPassword ok");
}

bool DBManager::FindTraderByTraderIdAndPassword(string traderid, string password, Trader *op) {
	this->getXtsDBLogger()->info("DBManager::FindTraderByTraderIdAndPassword()");
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;
	bool flag = false;

	count_number = client->count(DB_OPERATOR_COLLECTION,
		BSON("traderid" << traderid.c_str() << "password" << password.c_str() << "isactive" << ISACTIVE));

	if (count_number == 0) {
		flag = false;
	} else {
		flag = true;
		unique_ptr<DBClientCursor> cursor = client->query(DB_OPERATOR_COLLECTION, MONGO_QUERY("traderid" << traderid << "password" << password << "isactive" << ISACTIVE));
		while (cursor->more()) {
			BSONObj p = cursor->next();
			this->getXtsDBLogger()->info("\ttraderid = {}", p.getStringField("traderid"));
			this->getXtsDBLogger()->info("\ttradername = {}", p.getStringField("tradername"));
			this->getXtsDBLogger()->info("\tpassword = {}", p.getStringField("password"));
			this->getXtsDBLogger()->info("\tisactive = {}", p.getStringField("isactive"));
			this->getXtsDBLogger()->info("\ton_off = {}", p.getIntField("on_off"));
			op->setTraderID(p.getStringField("traderid"));
			op->setTraderName(p.getStringField("tradername"));
			op->setPassword(p.getStringField("password"));
			op->setOn_Off(p.getIntField("on_off"));
		}
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);

	return flag;
}

void DBManager::getAllTrader(list<string> *l_trader) {
	//std::cout << "DBManager::getAllTrader()" << std::endl;
	this->getXtsDBLogger()->info("DBManager::getAllTrader()");
	/// 初始化的时候，必须保证list为空
	if (l_trader->size() > 0) {
		list<string>::iterator Itor;
		for (Itor = l_trader->begin(); Itor != l_trader->end();) {
			Itor = l_trader->erase(Itor);
		}
	}

	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();

	int countnum = client->count(DB_OPERATOR_COLLECTION);
	USER_PRINT(countnum);
	if (countnum == 0) {
		cout << "DBManager::getAllTrader is NONE!" << endl;
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			client->query(DB_OPERATOR_COLLECTION);
		while (cursor->more()) {

			BSONObj p = cursor->next();
			/*cout << "\t*" << "traderid:" << p.getStringField("traderid") << "  "
				<< "tradername:" << p.getStringField("tradername") << "  "
				<< "password:" << p.getStringField("password") << "  "
				<< "isactive:" << p.getStringField("isactive") << " "
				<< "on_off" << p.getIntField("on_off") << "*" << endl;*/
			this->getXtsDBLogger()->info("\t*traderid:{} tradername:{} password:{} isactive:{} on_off:{}",
				p.getStringField("traderid"), p.getStringField("tradername"), p.getStringField("password"), p.getStringField("isactive"), p.getIntField("on_off"));

			l_trader->push_back(p.getStringField("traderid"));
		}
		USER_PRINT("DBManager::getAllTrader1 ok");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::getAllObjTrader(list<Trader *> *l_trader) {
	//std::cout << "DBManager::getAllObjTrader()" << std::endl;
	this->getXtsDBLogger()->info("DBManager::getAllObjTrader()");
	/// 初始化的时候，必须保证list为空
	if (l_trader->size() > 0) {
		list<Trader *>::iterator Itor;
		for (Itor = l_trader->begin(); Itor != l_trader->end();) {
			delete (*Itor);
			Itor = l_trader->erase(Itor);
		}
	}

	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();

	int countnum = client->count(DB_OPERATOR_COLLECTION);
	USER_PRINT(countnum);
	if (countnum == 0) {
		cout << "\tDBManager::getAllTrader is NONE!" << endl;
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			client->query(DB_OPERATOR_COLLECTION);
		while (cursor->more()) {
			Trader *op = new Trader();
			BSONObj p = cursor->next();
			/*cout << "\t*" << "traderid:" << p.getStringField("traderid") << "  "
				<< "tradername:" << p.getStringField("tradername") << "  "
				<< "password:" << p.getStringField("password") << "  "
				<< "isactive:" << p.getStringField("isactive") << " "
				<< "on_off" << p.getIntField("on_off") << "*" << endl;*/

			this->getXtsDBLogger()->info("\t*traderid:{} tradername:{} password:{} isactive:{} on_off:{}", p.getStringField("traderid"), p.getStringField("tradername"), p.getStringField("password"),
				p.getStringField("isactive"), p.getIntField("on_off"));

			op->setTraderID(p.getStringField("traderid"));
			op->setTraderName(p.getStringField("tradername"));
			op->setPassword(p.getStringField("password"));
			op->setIsActive(p.getStringField("isactive"));
			op->setOn_Off(p.getIntField("on_off"));

			l_trader->push_back(op);
		}
		USER_PRINT("DBManager::getAllTrader1 ok");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

bool DBManager::FindAdminByAdminIdAndPassword(string adminid, string password) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;
	bool flag = false;

	count_number = client->count(DB_ADMIN_COLLECTION,
		BSON("adminid" << adminid << "password" << password));

	if (count_number == 0) {
		flag = false;
	}
	else {
		flag = true;
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);

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

	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();

	int count_number = 0;
	int trader_count_number = 0;

	if (op != NULL) {
		trader_count_number = client->count(DB_OPERATOR_COLLECTION,
			BSON("traderid" << op->getTraderID()));
		if (trader_count_number == 0) { //交易员id不存在
			cout << "Trader ID Not Exists!" << endl;
		}
		else { //交易员存在
			count_number = client->count(DB_FUTUREACCOUNT_COLLECTION,
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
				client->insert(DB_FUTUREACCOUNT_COLLECTION, p);
				USER_PRINT("DBManager::CreateFutureAccount ok");
			}
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}
void DBManager::DeleteFutureAccount(FutureAccount *fa) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
	client->update(DB_FUTUREACCOUNT_COLLECTION, BSON("userid" << (fa->getUserID().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::DeleteFutureAccount ok");
}
void DBManager::UpdateFutureAccount(User *u) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
	client->update(DB_FUTUREACCOUNT_COLLECTION, BSON("userid" << (u->getUserID().c_str())), BSON("$set" << BSON("userid" << u->getUserID() << "brokerid" << u->getBrokerID() << "traderid" << u->getTraderID() << "password" << u->getPassword() << "frontaddress" << u->getFrontAddress() << "on_off" << u->getOn_Off())));
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::UpdateFutureAccountBee(User *u) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	client->update(DB_FUTURE_BEE_COLLECTION, BSON("userid" << (u->getUserID().c_str())), BSON("$set" << BSON("userid" << u->getUserID() << "brokerid" << u->getBrokerID() << "traderid" << u->getTraderID() << "password" << u->getPassword() << "frontaddress" << u->getFrontAddress() << "on_off" << u->getOn_Off())));
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::UpdateFutureAccountOrderRef(User *u, string order_ref_base) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}

	client->update(DB_FUTUREACCOUNT_COLLECTION, BSON("userid" << (u->getUserID().c_str())), BSON("$set" << BSON("order_ref_base" << order_ref_base)));

	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::SearchFutrueByUserID(string userid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
	unique_ptr<DBClientCursor> cursor = client->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("userid" << userid));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		cout << "brokerid = " << p.getStringField("brokerid") << endl;
		cout << "traderid = " << p.getStringField("traderid") << endl;
		cout << "password = " << p.getStringField("password") << endl;
		cout << "userid = " << p.getStringField("userid") << endl;
		cout << "isactive = " << p.getStringField("isactive") << endl;
		cout << "on_off = " << p.getIntField("on_off") << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::SearchFutrueByUserID ok");
}

void DBManager::SearchFutrueByTraderID(string traderid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	string DB_FUTUREACCOUNT_COLLECTION;
	if (this->is_online) {
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount";
	}
	else
	{
		DB_FUTUREACCOUNT_COLLECTION = "CTP.futureaccount_panhou";
	}
	unique_ptr<DBClientCursor> cursor = client->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("traderid" << traderid));
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
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::SearchFutrueByTraderName ok");
}

void DBManager::SearchFutrueListByTraderID(string traderid, list<FutureAccount *> *l_futureaccount) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
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

	unique_ptr<DBClientCursor> cursor = client->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("traderid" << traderid << "isactive" << ISACTIVE));
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
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::SearchFutrueByTraderName ok");
}

void DBManager::SearchFutrueListByTraderID(string traderid, list<User *> *l_user) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
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
	if (l_user->size() > 0) {
		list<User *>::iterator future_itor;
		for (future_itor = l_user->begin(); future_itor != l_user->end();) {
			delete (*future_itor);
			future_itor = l_user->erase(future_itor);
		}
	}

	unique_ptr<DBClientCursor> cursor = client->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("traderid" << traderid << "isactive" << ISACTIVE));
	while (cursor->more()) {
		BSONObj p = cursor->next();
		this->getXtsDBLogger()->info("\t*brokerid:{} traderid:{} password:{} userid:{} frontAdress:{} on_off:{}",
			p.getStringField("brokerid"), p.getStringField("traderid"), p.getStringField("password"), p.getStringField("userid"),
			p.getStringField("frontaddress"), p.getIntField("on_off"));
		User *user = new User(p.getStringField("frontaddress"), p.getStringField("brokerid"), p.getStringField("userid"), p.getStringField("password"), p.getStringField("userid"), p.getIntField("on_off"), p.getStringField("traderid"), p.getStringField("order_ref_base"));
		l_user->push_back(user);
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::SearchFutrueByTraderName ok");
}

void DBManager::getAllFutureAccount(list<User *> *l_user) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT(this->is_online);
	//std::cout << "DBManager::getAllFutureAccount()" << std::endl;
	this->getXtsDBLogger()->info("DBManager::getAllFutureAccount()");
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

	int countnum = client->count(DB_FUTUREACCOUNT_COLLECTION, BSON("isactive" << ISACTIVE));
	if (countnum == 0) {
		//cout << "DBManager::getAllFutureAccount None!" << endl;
		this->getXtsDBLogger()->info("DBManager::getAllFutureAccount None!");
	}
	else {
		unique_ptr<DBClientCursor> cursor = client->query(DB_FUTUREACCOUNT_COLLECTION, MONGO_QUERY("isactive" << ISACTIVE));
		while (cursor->more()) {
			BSONObj p = cursor->next();
			/*cout << "\t*" << "brokerid:" << p.getStringField("brokerid") << "  "
				<< "traderid:" << p.getStringField("traderid") << "  "
				<< "password:" << p.getStringField("password") << "  "
				<< "userid:" << p.getStringField("userid") << "  "
				<< "frontAddress:" << p.getStringField("frontaddress") << "  "
				<< "on_off:" << p.getIntField("on_off") << "  "
				<< "sessionid:" << p.getIntField("sessionid") << "  "
				<< "isactive:" << p.getStringField("isactive") << "*" << endl;*/
			this->getXtsDBLogger()->info("\t*brokerid:{} traderid:{} password:{} userid:{} frontAdress:{} on_off:{}", 
				p.getStringField("brokerid"), p.getStringField("traderid"), p.getStringField("password"), p.getStringField("userid"),
				p.getStringField("frontaddress"), p.getIntField("on_off"));
			User *user = new User(p.getStringField("frontaddress"), p.getStringField("brokerid"), p.getStringField("userid"), p.getStringField("password"), p.getStringField("userid"), p.getIntField("on_off"), p.getStringField("traderid"), p.getStringField("order_ref_base"));
			l_user->push_back(user);
		}
		
		USER_PRINT("DBManager::getAllFutureAccount ok");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::getAllFutureAccountBee(list<User *> *l_user) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getAllFutureAccountBee()");

	/// 初始化的时候，必须保证list为空
	if (l_user->size() > 0) {
		list<User *>::iterator user_itor;
		for (user_itor = l_user->begin(); user_itor != l_user->end();) {
			user_itor = l_user->erase(user_itor);
		}
	}

	int countnum = client->count(DB_FUTURE_BEE_COLLECTION, BSON("isactive" << ISACTIVE));
	if (countnum == 0) {
		//cout << "DBManager::getAllFutureAccount None!" << endl;
		this->getXtsDBLogger()->info("DBManager::getAllFutureAccountBee() None!");
	}
	else {
		unique_ptr<DBClientCursor> cursor = client->query(DB_FUTURE_BEE_COLLECTION, MONGO_QUERY("isactive" << ISACTIVE));
		while (cursor->more()) {
			BSONObj p = cursor->next();
			/*cout << "\t*" << "brokerid:" << p.getStringField("brokerid") << "  "
			<< "traderid:" << p.getStringField("traderid") << "  "
			<< "password:" << p.getStringField("password") << "  "
			<< "userid:" << p.getStringField("userid") << "  "
			<< "frontAddress:" << p.getStringField("frontaddress") << "  "
			<< "on_off:" << p.getIntField("on_off") << "  "
			<< "sessionid:" << p.getIntField("sessionid") << "  "
			<< "isactive:" << p.getStringField("isactive") << "*" << endl;*/
			this->getXtsDBLogger()->info("\t*brokerid:{} traderid:{} password:{} userid:{} frontAdress:{} on_off:{}",
				p.getStringField("brokerid"), p.getStringField("traderid"), p.getStringField("password"), p.getStringField("userid"),
				p.getStringField("frontaddress"), p.getIntField("on_off"));
			User *user = new User(p.getStringField("frontaddress"), p.getStringField("brokerid"), p.getStringField("userid"), p.getStringField("password"), p.getStringField("userid"), p.getIntField("on_off"), p.getStringField("traderid"), p.getStringField("order_ref_base"));
			l_user->push_back(user);
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}


int DBManager::CheckStrategyExist(string strategy_id, string user_id, string trading_day) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	//std::cout << "DBManager::CheckStrategyExist()" << std::endl;
	this->getXtsDBLogger()->info("DBManager::CheckStrategyExist()");
	int count_number = 0;
	int flag = 0;

	count_number = client->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (strategy_id.c_str()) << "user_id" << (user_id.c_str()) << "trading_day" << (trading_day.c_str()) << "is_active" << true));

	if (count_number > 0) {
		std::cout << "\t策略已经存在!!" << std::endl;
		flag = 1;
	}
	return flag;
}

int DBManager::CreateStrategy(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	//std::cout << "DBManager::CreateStrategy()" << std::endl;
	this->getXtsDBLogger()->info("DBManager::CreateStrategy()");
	int count_number = 0;
	int flag = 0;

	count_number = client->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str()) << "trading_day" << (stg->getStgTradingDay().c_str()) << "is_active" << true));

	if (count_number > 0) {
		std::cout << "\t策略已经存在!!" << std::endl;
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

		b.append("instrument_a_scale", stg->getStgInstrumentAScale());
		b.append("instrument_b_scale", stg->getStgInstrumentBScale());

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
		b.append("last_save_time", stg->getStgLastSavedTime());


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

		client->insert(DB_STRATEGY_COLLECTION, p);
		USER_PRINT("DBManager::CreateStrategy ok");
		flag = 0;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	return flag;
}

int DBManager::DeleteStrategy(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;
	int flag = 0;
	count_number = client->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	if (count_number > 0) {
		client->remove(DB_STRATEGY_COLLECTION, MONGO_QUERY("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str())));
		USER_PRINT("DBManager::DeleteStrategy ok");
		flag = 0;
	}
	else {
		//cout << "Strategy Not Exists!" << endl;
		Utils::printRedColor("策略不存在!");
		flag = 1;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	return flag;
}

int DBManager::UpdateStrategyOnOff(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::UpdateStrategyOnOff()");
	int count_number = 0;
	int flag = 0;
	count_number = client->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	if (count_number > 0) {

		this->getXtsDBLogger()->info("UpdateStrategyOnOff strategy_id = {}", stg->getStgStrategyId());
		this->getXtsDBLogger()->info("UpdateStrategyOnOff user_id = {}", stg->getStgUserId());
		this->getXtsDBLogger()->info("UpdateStrategyOnOff strategy_on_off = {}", stg->getOn_Off());

		client->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId()) << "is_active" << true), BSON("$set" << BSON("strategy_on_off" << stg->getOn_Off())));
		USER_PRINT("DBManager::UpdateStrategyOnOff ok");
		flag = 0;
	}
	else {
		cout << "Strategy ID Not Exists!" << endl;
		flag = 1;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	return flag;
}

int DBManager::UpdateStrategyOnlyCloseOnOff(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;
	int flag = 0;
	count_number = client->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	if (count_number > 0) {

		std::cout << "UpdateStrategyOnlyCloseOnOff strategy_id = " << stg->getStgStrategyId() << std::endl;
		std::cout << "UpdateStrategyOnlyCloseOnOff user_id = " << stg->getStgUserId() << std::endl;
		std::cout << "UpdateStrategyOnlyCloseOnOff only_close = " << stg->isStgOnlyClose() << std::endl;

		client->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId()) << "is_active" << true), BSON("$set" << BSON("only_close" << stg->isStgOnlyClose())));
		USER_PRINT("DBManager::UpdateStrategyOnlyCloseOnOff ok");
		flag = 0;
	}
	else {
		cout << "Strategy ID Not Exists!" << endl;
		flag = 1;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	return flag;
}

void DBManager::UpdateStrategy(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::UpdateStrategy()");
	int count_number = 0;

	this->getXtsDBLogger()->info("DBManager::UpdateStrategy()");


	count_number = client->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	this->getXtsDBLogger()->info("\tcount_number = {}", count_number);

	if (count_number > 0) {
		client->update(DB_STRATEGY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str() << "is_active" << true), BSON("$set" << BSON("position_a_sell_today" << stg->getStgPositionASellToday()
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

			<< "instrument_a_scale" << stg->getStgInstrumentAScale()
			<< "instrument_b_scale" << stg->getStgInstrumentBScale()

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
			<< "last_save_time" << stg->getStgLastSavedTime()
			<< "list_instrument_id" << BSON_ARRAY(stg->getStgInstrumentIdA() << stg->getStgInstrumentIdB()))));

		USER_PRINT("DBManager::UpdateStrategy ok");
	}
	else
	{
		USER_PRINT("Strategy ID Not Exists!");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}
void DBManager::getAllStrategy(list<Strategy *> *l_strategys, string traderid, string userid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
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
			cursor = client->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid << "is_active" << true));
		}
		else {
			cursor = client->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "is_active" << true));
		}
	}
	else {
		cursor = client->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("is_active" << true));
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

		stg->setStgInstrumentAScale(p.getIntField("instrument_a_scale"));
		stg->setStgInstrumentBScale(p.getIntField("instrument_b_scale"));

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
		stg->setStgUpdatePositionDetailRecordTime(p.getStringField("update_position_detail_record_time"));
		stg->setStgLastSavedTime(p.getStringField("last_save_time"));


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
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::getAllStragegy ok");
}

void DBManager::getAllStrategyByActiveUser(bool fake, list<Strategy *> *l_strategys, list<User *> *l_users, string traderid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getAllStrategyByActiveUser()");
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

			cursor = client->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << (*user_itor)->getUserID() << "is_active" << true));
		}
		else {
			cursor = client->query(DB_STRATEGY_COLLECTION, MONGO_QUERY("user_id" << (*user_itor)->getUserID() << "is_active" << true));
		}

		while (cursor->more()) {
			BSONObj p = cursor->next();
			Strategy *stg = new Strategy(fake);
			//cout << "\t*position_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
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
			//cout << "strategy_on_off = " << p.getIntField("strategy_on_off") << ", ";
			//cout << "sell_open_on_off = " << p.getIntField("sell_open_on_off") << ", ";
			//cout << "buy_close_on_off = " << p.getIntField("buy_close_on_off") << ", ";
			//cout << "sell_close_on_off = " << p.getIntField("sell_close_on_off") << ", ";
			//cout << "buy_open_on_off = " << p.getIntField("buy_open_on_off") << ", ";
			//cout << "trading_day = " << p.getStringField("trading_day") << ", ";
			//cout << "update_position_detail_record_time = " << p.getStringField("update_position_detail_record_time") << ", ";
			//cout << "last_save_time = " << p.getStringField("last_save_time") << ", ";
			

			stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
			stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
			stg->setStgBuyClose(p.getField("buy_close").Double());
			stg->setStgBuyOpen(p.getField("buy_open").Double());
			stg->setStgIsActive(p.getField("is_active").Bool());
			stg->setStgLots(p.getIntField("lots"));
			stg->setStgLotsBatch(p.getIntField("lots_batch"));

			stg->setStgInstrumentAScale(p.getIntField("instrument_a_scale"));
			stg->setStgInstrumentBScale(p.getIntField("instrument_b_scale"));

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
			stg->setStgLastSavedTime(p.getStringField("last_save_time"));
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

				// 计算策略时间
				stg->StgTimeCal();

				stg->addInstrumentToList(stg->getStgInstrumentIdA());
				stg->addInstrumentToList(stg->getStgInstrumentIdB());
				//cout << "stg->setStgInstrumentIdA(elements[0]) = " << stg->getStgInstrumentIdA() << ", ";
				//cout << "stg->setStgInstrumentIdA(elements[1]) = " << stg->getStgInstrumentIdB() << endl;
			}

			//for (vector<BSONElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
			//	cout << *it << endl;
			//}

			l_strategys->push_back(stg);
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::getAllStrategyByActiveUser ok");
}


/************************************************************************/
/* 创建策略(昨仓)
删除策略(昨仓)
更新策略(昨仓)
查找策略(昨仓)			                                                */
/************************************************************************/
int DBManager::CreateStrategyYesterday(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;
	int flag = 0;

	count_number = client->count(DB_STRATEGY_YESTERDAY_COLLECTION,
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

		b.append("instrument_a_scale", stg->getStgInstrumentAScale());
		b.append("instrument_b_scale", stg->getStgInstrumentBScale());

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
		b.append("last_save_time", stg->getStgLastSavedTime());


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

		client->insert(DB_STRATEGY_YESTERDAY_COLLECTION, p);
		flag = 0;
		USER_PRINT("DBManager::CreateStrategyYesterday ok");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	return flag;
}

int DBManager::DeleteStrategyYesterday(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;
	int flag = 0;

	count_number = client->count(DB_STRATEGY_YESTERDAY_COLLECTION,
		BSON("strategy_id" << stg->getStgStrategyId().c_str() << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));

	if (count_number > 0) {
		client->remove(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str()));
		USER_PRINT("DBManager::DeleteStrategy ok");
	}
	else {
		//cout << "Strategy ID Not Exists!" << endl;
		Utils::printRedColor("策略不存在!");
		flag = 1;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	return flag;
}

void DBManager::UpdateStrategyYesterday(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;

	std::cout << "DBManager::UpdateStrategy" << std::endl;


	count_number = client->count(DB_STRATEGY_YESTERDAY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << stg->getStgUserId().c_str() << "is_active" << true));
	std::cout << "DBManager::UpdateStrategyYesterday()" << std::endl;
	std::cout << "\tcount_number = " << count_number << std::endl;

	if (count_number > 0) {
		client->update(DB_STRATEGY_YESTERDAY_COLLECTION, BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str())), BSON("$set" << BSON("position_a_sell_today" << stg->getStgPositionASellToday()
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

			<< "instrument_a_scale" << stg->getStgInstrumentAScale()
			<< "instrument_b_scale" << stg->getStgInstrumentBScale()

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
			<< "update_position_detail_record_time" << stg->getStgUpdatePositionDetailRecordTime()
			<< "last_save_time" << stg->getStgLastSavedTime()
			<< "list_instrument_id" << BSON_ARRAY(stg->getStgInstrumentIdA() << stg->getStgInstrumentIdB()))));

		USER_PRINT("DBManager::UpdateStrategy ok");
	}
	else
	{
		cout << "Strategy ID Not Exists!" << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::getAllStrategyYesterday(list<Strategy *> *l_strategys, string traderid, string userid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();

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
			cursor = client->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid << "is_active" << true));
		}
		else {
			cursor = client->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "is_active" << true));
		}
	}
	else {
		cursor = client->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << true));
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

		stg->setStgInstrumentAScale(p.getIntField("instrument_a_scale"));
		stg->setStgInstrumentBScale(p.getIntField("instrument_b_scale"));

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
		stg->setStgUpdatePositionDetailRecordTime(p.getStringField("update_position_detail_record_time"));
		stg->setStgLastSavedTime(p.getStringField("last_save_time"));


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

	// 重新加到数据连接池队列
	this->recycleConn(client);

	USER_PRINT("DBManager::getAllYesterdayStragegy ok");
}

void DBManager::getAllStrategyYesterdayByTraderIdAndUserIdAndStrategyId(list<Strategy *> *l_strategys, string traderid, string userid, string strategyid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
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
				cursor = client->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid << "strategy_id" << strategyid << "is_active" << true));
			}
			else {
				cursor = client->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << userid << "is_active" << true));
			}
			
		}
		else {
			cursor = client->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "is_active" << true));
		}
	}
	else {
		cursor = client->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << true));
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

		stg->setStgInstrumentAScale(p.getIntField("instrument_a_scale"));
		stg->setStgInstrumentBScale(p.getIntField("instrument_b_scale"));

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
		stg->setStgUpdatePositionDetailRecordTime(p.getStringField("update_position_detail_record_time"));
		stg->setStgLastSavedTime(p.getStringField("last_save_time"));


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

	// 重新加到数据连接池队列
	this->recycleConn(client);

	USER_PRINT("DBManager::getAllStragegy ok");
}

void DBManager::getAllStrategyYesterdayByActiveUser(list<Strategy *> *l_strategys, list<User *> *l_users, string traderid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getAllStrategyYesterdayByActiveUser()");
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

			cursor = client->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("trader_id" << traderid << "user_id" << (*user_itor)->getUserID() << "is_active" << true));
		}
		else {
			cursor = client->query(DB_STRATEGY_YESTERDAY_COLLECTION, MONGO_QUERY("user_id" << (*user_itor)->getUserID() << "is_active" << true));
		}

		while (cursor->more()) {
			BSONObj p = cursor->next();
			Strategy *stg = new Strategy();
			//cout << "\t*position_a_sell_today = " << p.getIntField("position_a_sell_today") << ", ";
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

			/////*新增字段*/

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
			//cout << "trading_day = " << p.getStringField("trading_day") << ", ";

			stg->setStgAWaitPriceTick(p.getField("a_wait_price_tick").Double());
			stg->setStgBWaitPriceTick(p.getField("b_wait_price_tick").Double());
			stg->setStgBuyClose(p.getField("buy_close").Double());
			stg->setStgBuyOpen(p.getField("buy_open").Double());
			stg->setStgIsActive(p.getField("is_active").Bool());
			stg->setStgLots(p.getIntField("lots"));
			stg->setStgLotsBatch(p.getIntField("lots_batch"));

			stg->setStgInstrumentAScale(p.getIntField("instrument_a_scale"));
			stg->setStgInstrumentBScale(p.getIntField("instrument_b_scale"));

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
			stg->setStgLastSavedTime(p.getStringField("last_save_time"));

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

				// 计算策略时间
				stg->StgTimeCal();

				stg->addInstrumentToList(stg->getStgInstrumentIdA());
				stg->addInstrumentToList(stg->getStgInstrumentIdB());
				//cout << "stg->setStgInstrumentIdA(elements[0]) = " << stg->getStgInstrumentIdA() << ", ";
				//cout << "stg->setStgInstrumentIdA(elements[1]) = " << stg->getStgInstrumentIdB() << endl;
			}

			//for (vector<BSONElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
			//	cout << *it << endl;
			//}

			l_strategys->push_back(stg);
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);

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
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	string DB_MARKETCONFIG_COLLECTION;
	if (this->is_online) {

		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig";
	}
	else
	{

		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig_panhou";
	}
	int count_number = 0;

	count_number = client->count(DB_MARKETCONFIG_COLLECTION,
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

		client->insert(DB_MARKETCONFIG_COLLECTION, p);
		USER_PRINT("DBManager::CreateMarketConfig ok");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::DeleteMarketConfig(MarketConfig *mc) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	string DB_MARKETCONFIG_COLLECTION;
	if (this->is_online) {
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig";
	}
	else
	{
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig_panhou";
	}
	int count_number = 0;

	count_number = client->count(DB_MARKETCONFIG_COLLECTION,
		BSON("market_id" << mc->getMarketID()));

	if (count_number > 0) {
		client->update(DB_MARKETCONFIG_COLLECTION, BSON("market_id" << (mc->getMarketID().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeleteMarketConfig ok");
	}
	else {
		cout << "MarketConfig ID Not Exists!" << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::UpdateMarketConfig(MarketConfig *mc) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	string DB_MARKETCONFIG_COLLECTION;
	if (this->is_online) {
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig";
	}
	else
	{
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig_panhou";
	}
	int count_number = 0;

	count_number = client->count(DB_MARKETCONFIG_COLLECTION,
		BSON("market_id" << mc->getMarketID()));

	if (count_number > 0) {
		client->update(DB_MARKETCONFIG_COLLECTION, BSON("market_id" << (mc->getMarketID().c_str())), BSON("$set" << BSON("market_id" << mc->getMarketID()
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
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::getAllMarketConfig(list<MarketConfig *> *l_marketconfig) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getAllMarketConfig()");
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
			delete *market_itor;
			market_itor = l_marketconfig->erase(market_itor);
		}
	}

	int countnum = client->count(DB_MARKETCONFIG_COLLECTION);
	if (countnum == 0) {
		this->getXtsDBLogger()->info("\tDBManager::getAllMarketConfig None!");
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			client->query(DB_MARKETCONFIG_COLLECTION, MONGO_QUERY("isactive" << ISACTIVE));
		while (cursor->more()) {
			BSONObj p = cursor->next();
			this->getXtsDBLogger()->info("\t*market_id:{} market_frontAddr:{} broker_id:{} password:{}",
				p.getStringField("market_id"), p.getStringField("market_frontAddr"), p.getStringField("broker_id"), p.getStringField("password"));
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
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::getAllMarketConfigBee(list<MarketConfig *> *l_marketconfig) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getAllMarketConfigBee()");
	/// 初始化的时候，必须保证list为空
	if (l_marketconfig->size() > 0) {
		list<MarketConfig *>::iterator market_itor;
		for (market_itor = l_marketconfig->begin(); market_itor != l_marketconfig->end();) {
			delete *market_itor;
			market_itor = l_marketconfig->erase(market_itor);
		}
	}

	int countnum = client->count(DB_MARKET_BEE_COLLECTION);
	if (countnum == 0) {
		this->getXtsDBLogger()->info("\tDBManager::getAllMarketConfigBee None!");
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			client->query(DB_MARKET_BEE_COLLECTION, MONGO_QUERY("isactive" << ISACTIVE));
		while (cursor->more()) {
			BSONObj p = cursor->next();
			this->getXtsDBLogger()->info("\t*market_id:{} market_frontAddr:{} broker_id:{} password:{}",
				p.getStringField("market_id"), p.getStringField("market_frontAddr"), p.getStringField("broker_id"), p.getStringField("password"));
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
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

MarketConfig * DBManager::getOneMarketConfig() {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getOneMarketConfig()");
	USER_PRINT(this->is_online);
	string DB_MARKETCONFIG_COLLECTION;
	if (this->is_online) {
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig";
	}
	else
	{
		DB_MARKETCONFIG_COLLECTION = "CTP.marketconfig_panhou";
	}
	int countnum = client->count(DB_MARKETCONFIG_COLLECTION);
	if (countnum == 0) {
		cout << "\tDBManager::getOneMarketConfig None!" << endl;
	}
	else {
		BSONObj p = client->findOne(DB_MARKETCONFIG_COLLECTION, BSON("isactive" << ISACTIVE));
		if ((strcmp(p.getStringField("market_id"), ""))) {

			this->getXtsDBLogger()->debug("\t*market_id:{} market_frontAddr:{} broker_id:{} userid:{} password:{}",
				p.getStringField("market_id"), p.getStringField("market_frontAddr"), p.getStringField("broker_id"), 
				p.getStringField("password"));
			
			return new MarketConfig(p.getStringField("market_id"), p.getStringField("market_frontAddr"), p.getStringField("broker_id"),
				p.getStringField("user_id"), p.getStringField("password"), p.getStringField("isactive"));
		}
		else {
			USER_PRINT("DBManager::getOneMarketConfig None");
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	return NULL;
}

MarketConfig * DBManager::getOneMarketConfigBee() {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getOneMarketConfigBee()");
	
	int countnum = client->count(DB_MARKET_BEE_COLLECTION);
	if (countnum == 0) {
		cout << "\tDBManager::getOneMarketConfig None!" << endl;
	}
	else {
		BSONObj p = client->findOne(DB_MARKET_BEE_COLLECTION, BSON("isactive" << ISACTIVE));
		if ((strcmp(p.getStringField("market_id"), ""))) {

			this->getXtsDBLogger()->debug("\t*market_id:{} market_frontAddr:{} broker_id:{} userid:{} password:{}",
				p.getStringField("market_id"), p.getStringField("market_frontAddr"), p.getStringField("broker_id"),
				p.getStringField("password"));

			return new MarketConfig(p.getStringField("market_id"), p.getStringField("market_frontAddr"), p.getStringField("broker_id"),
				p.getStringField("user_id"), p.getStringField("password"), p.getStringField("isactive"));
		}
		else {
			USER_PRINT("DBManager::getOneMarketConfig None");
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	return NULL;
}

/************************************************************************/
/*  创建算法
删除算法
更新算法
获取算法															*/
/************************************************************************/
void DBManager::CreateAlgorithm(Algorithm *alg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;

	count_number = client->count(DB_ALGORITHM_COLLECTION,
		BSON("name" << alg->getAlgName()));

	if (count_number > 0) {
		cout << "Algorithm Already Exists!" << endl;
	}
	else {
		BSONObjBuilder b;
		b.append("name", alg->getAlgName());
		b.append("isactive", alg->getIsActive());
		BSONObj p = b.obj();

		client->insert(DB_ALGORITHM_COLLECTION, p);
		USER_PRINT("DBManager::CreateAlgorithm ok");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}
void DBManager::DeleteAlgorithm(Algorithm *alg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;

	count_number = client->count(DB_ALGORITHM_COLLECTION,
		BSON("name" << alg->getAlgName()));

	if (count_number > 0) {
		client->update(DB_ALGORITHM_COLLECTION, BSON("name" << (alg->getAlgName().c_str())), BSON("$set" << BSON("isactive" << ISNOTACTIVE)));
		USER_PRINT("DBManager::DeleteAlgorithm ok");
	}
	else {
		cout << "MarketConfig ID Not Exists!" << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
}
void DBManager::UpdateAlgorithm(Algorithm *alg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	int count_number = 0;

	count_number = client->count(DB_ALGORITHM_COLLECTION,
		BSON("name" << alg->getAlgName()));

	if (count_number > 0) {
		client->update(DB_ALGORITHM_COLLECTION, BSON("name" << (alg->getAlgName().c_str())), BSON("$set" << BSON("name" << alg->getAlgName() << "isactive" << alg->getIsActive())));
		USER_PRINT("DBManager::UpdateAlgorithm ok");
	}
	else
	{
		cout << "Algorithm name Not Exists!" << endl;
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
}
void DBManager::getAllAlgorithm(list<Algorithm *> *l_alg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getAllAlgorithm()");
	/// 初始化的时候，必须保证list为空
	if (l_alg->size() > 0) {
		list<Algorithm *>::iterator alg_itor;
		for (alg_itor = l_alg->begin(); alg_itor != l_alg->end();) {
			alg_itor = l_alg->erase(alg_itor);
		}
	}

	int countnum = client->count(DB_ALGORITHM_COLLECTION);
	if (countnum == 0) {
		this->getXtsDBLogger()->info("\tDBManager::getAllAlgorithm None!");
	}
	else {
		unique_ptr<DBClientCursor> cursor =
			client->query(DB_ALGORITHM_COLLECTION, MONGO_QUERY("isactive" << ISACTIVE));
		while (cursor->more()) {
			BSONObj p = cursor->next();
			this->getXtsDBLogger()->info("\t*name:{}", p.getStringField("name"));
			Algorithm *alg = new Algorithm();
			alg->setAlgName(p.getStringField("name"));
			alg->setIsActive(p.getStringField("isactive"));

			l_alg->push_back(alg);
		}
		USER_PRINT("DBManager::getAllAlgorithm ok");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
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

void DBManager::CreatePositionDetail(USER_CSgitFtdcOrderField *posd) {
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

void DBManager::DeletePositionDetail(USER_CSgitFtdcOrderField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DeletePositionDetail");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//this->conn->update(DB_POSITIONDETAIL_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));
		
		client->remove(DB_POSITIONDETAIL_COLLECTION, MONGO_QUERY("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef));


		USER_PRINT("DBManager::DeletePositionDetail ok");
	}
	else {
		cout << "删除order持仓明细,持仓明细不存在!" << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::DeletePositionDetail OK");
}


void DBManager::UpdatePositionDetail(USER_CSgitFtdcOrderField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::UpdatePositionDetail");
	std::cout << "DBManager::UpdatePositionDetail()" << std::endl;
	
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		
		std::cout << "\tposd->TradingDayRecord = " << posd->TradingDayRecord << std::endl;
		client->update(DB_POSITIONDETAIL_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
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

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::UpdatePositionDetail OK");
}



void DBManager::getAllPositionDetail(list<USER_CSgitFtdcOrderField *> *l_posd, string traderid, string userid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::getAllPositionDetail");
	this->getXtsDBLogger()->info("DBManager::getAllPositionDetail()");
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CSgitFtdcOrderField *>::iterator itor;
		for (itor = l_posd->begin(); itor != l_posd->end();) {
			delete (*itor);
			itor = l_posd->erase(itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	if (traderid.compare("")) { //如果traderid不为空

		if (userid.compare("")) { //如果userid不为空
			cursor = client->query(DB_POSITIONDETAIL_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			cursor = client->query(DB_POSITIONDETAIL_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		cursor = client->query(DB_POSITIONDETAIL_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {
		
		BSONObj p = cursor->next();

		USER_CSgitFtdcOrderField *new_pos = new USER_CSgitFtdcOrderField();

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

		/*std::cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
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
		std::cout << "new_pos->StrategyID = " << new_pos->StrategyID << std::endl;*/

		l_posd->push_back(new_pos);
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::getAllPositionDetail OK");
}

void DBManager::DropPositionDetail() {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DropPositionDetail");
	client->dropCollection(DB_POSITIONDETAIL_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetail ok");
	// 重新加到数据连接池队列
	this->recycleConn(client);
}


/************************************************************************/
/* 修改过仓位的策略的持仓明细(order) CRUD                                   */
/************************************************************************/
void DBManager::CreatePositionDetailChanged(USER_CSgitFtdcOrderField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::CreatePositionDetailChanged");

	int posd_count_num = 0;

	if (posd != NULL) {
		posd_count_num = client->count(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION,
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
			client->insert(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, p);
			USER_PRINT("DBManager::CreatePositionDetailChanged ok");
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::CreatePositionDetailYesterday OK");
}
void DBManager::DeletePositionDetailChanged(USER_CSgitFtdcOrderField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::DeletePositionDetailChanged()");
	USER_PRINT("DBManager::DeletePositionDetailChanged");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//this->conn->update(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));

		client->remove(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, MONGO_QUERY("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef));


		USER_PRINT("DBManager::DeletePositionDetailChanged ok");
	}
	else {
		this->getXtsDBLogger()->debug("\t删除昨持仓明细,持仓明细不存在!");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::DeletePositionDetailChanged OK");
}

void DBManager::UpdatePositionDetailChanged(USER_CSgitFtdcOrderField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::UpdatePositionDetailChanged");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		client->update(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
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
		USER_PRINT("DBManager::UpdatePositionDetailChanged ok");
	}
	else {
		cout << "更新昨持仓明细,持仓明细未找到!" << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::UpdatePositionDetailChanged OK");
}

void DBManager::getAllPositionDetailChanged(list<USER_CSgitFtdcOrderField *> *l_posd,
	string traderid, string userid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::getAllPositionDetailChanged");
	this->getXtsDBLogger()->info("DBManager::getAllPositionDetailChanged()");
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CSgitFtdcOrderField *>::iterator itor;
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
			cursor = client->query(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			USER_PRINT("如果userid为空");
			cursor = client->query(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		USER_PRINT("如果traderid为空");
		cursor = client->query(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();

		USER_CSgitFtdcOrderField *new_pos = new USER_CSgitFtdcOrderField();

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

		/*cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
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
		cout << "volumetradedbatch = " << p.getIntField("volumetradedbatch") << std::endl;*/

		l_posd->push_back(new_pos);
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::getAllPositionDetailChanged OK");
}

void DBManager::DropPositionDetailChanged() {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DropPositionDetailChanged");
	client->dropCollection(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetailChanged ok");
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

//根据策略,删除策略对应的order,trade持仓明细
bool DBManager::DeletePositionDetailChangedByStrategy(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DeletePositionDetailChangedByStrategy");
	//std::cout << "DBManager::DeletePositionDetailChangedByStrategy()" << std::endl;
	bool flag = true;
	int count_number = client->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str()) << "trading_day" << (stg->getStgTradingDay().c_str()) << "is_active" << true));

	if (count_number > 0) {
		/*std::cout << "\t策略存在,开始删除对应策略持仓明细" << std::endl;
		std::cout << "\t策略存在,开始删除持仓明细changed order" << std::endl;
		std::cout << "\tstrategyid = " << stg->getStgStrategyId() << std::endl;
		std::cout << "\tuserid = " << stg->getStgUserId() << std::endl;*/

		client->remove(DB_POSITIONDETAIL_ORDER_CHANGED_COLLECTION, MONGO_QUERY("strategyid" << stg->getStgStrategyId().c_str() << "userid" << stg->getStgUserId().c_str() << "is_active" << ISACTIVE));
		//std::cout << "\t策略存在,开始删除持仓明细changed trade" << std::endl;
		client->remove(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, MONGO_QUERY("strategyid" << stg->getStgStrategyId().c_str() << "userid" << stg->getStgUserId().c_str() << "is_active" << ISACTIVE));
		flag = true;
	}
	else {
		//cout << "\t策略不存在!!" << endl;
		flag = false;
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::DeletePositionDetailChangedByStrategy ok");
	return flag;
}


void DBManager::CreatePositionDetailYesterday(USER_CSgitFtdcOrderField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::CreatePositionDetailYesterday");

	int posd_count_num = 0;

	if (posd != NULL) {
		posd_count_num = client->count(DB_POSITIONDETAIL_YESTERDAY_COLLECTION,
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
			client->insert(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, p);
			USER_PRINT("DBManager::CreatePositionDetailYesterday ok");
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::CreatePositionDetailYesterday OK");
}
void DBManager::DeletePositionDetailYesterday(USER_CSgitFtdcOrderField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::DeletePositionDetailYesterday()");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_YESTERDAY_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//this->conn->update(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));
		
		client->remove(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, MONGO_QUERY("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef));

		USER_PRINT("DBManager::DeletePositionDetail ok");
	}
	else {
		this->getXtsDBLogger()->debug("\t删除昨持仓明细,持仓明细不存在!");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::DeletePositionDetailYesterday OK");
}

void DBManager::UpdatePositionDetailYesterday(USER_CSgitFtdcOrderField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::UpdatePositionDetailYesterday");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_YESTERDAY_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		client->update(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
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
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::UpdatePositionDetailYesterday OK");
}

void DBManager::getAllPositionDetailYesterday(list<USER_CSgitFtdcOrderField *> *l_posd,
	string traderid, string userid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getAllPositionDetailYesterday()");
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CSgitFtdcOrderField *>::iterator itor;
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
			cursor = client->query(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			USER_PRINT("如果userid为空");
			cursor = client->query(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		USER_PRINT("如果traderid为空");
		cursor = client->query(DB_POSITIONDETAIL_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();

		USER_CSgitFtdcOrderField *new_pos = new USER_CSgitFtdcOrderField();

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

		/*cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
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
		cout << "volumetradedbatch = " << p.getIntField("volumetradedbatch") << std::endl;*/

		l_posd->push_back(new_pos);
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::getAllPositionDetailYesterday OK");
}

void DBManager::DropPositionDetailYesterday() {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DropPositionDetailYesterday");
	client->dropCollection(DB_POSITIONDETAIL_YESTERDAY_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetailYesterday ok");
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

//根据策略,删除策略对应的order,trade持仓明细
bool DBManager::DeletePositionDetailByStrategy(Strategy *stg) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DeletePositionDetailByStrategy");
	//std::cout << "DBManager::DeletePositionDetailByStrategy()" << std::endl;
	bool flag = true;
	int count_number = client->count(DB_STRATEGY_COLLECTION,
		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str()) << "trading_day" << (stg->getStgTradingDay().c_str()) << "is_active" << true));

	if (count_number > 0) {
		/*std::cout << "\t策略存在,开始删除对应策略持仓明细" << std::endl;
		std::cout << "\t策略存在,开始删除持仓明细order" << std::endl;*/
		client->remove(DB_POSITIONDETAIL_COLLECTION, MONGO_QUERY("strategyid" << stg->getStgStrategyId().c_str() << "userid" << stg->getStgUserId().c_str() << "is_active" << ISACTIVE));
		//std::cout << "\t策略存在,开始删除持仓明细trade" << std::endl;
		client->remove(DB_POSITIONDETAIL_TRADE_COLLECTION, MONGO_QUERY("strategyid" << stg->getStgStrategyId().c_str() << "userid" << stg->getStgUserId().c_str() << "is_active" << ISACTIVE));
		flag = true;
	}
	else {
		//cout << "\t策略不存在!!" << endl;
		flag = false;
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::DeletePositionDetailByStrategy ok");
	return flag;
}

void DBManager::DeletePositionDetailTrade(USER_CSgitFtdcTradeField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DeletePositionDetailTrade");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_TRADE_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//this->conn->update(DB_POSITIONDETAIL_TRADE_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));
		
		client->remove(DB_POSITIONDETAIL_TRADE_COLLECTION, MONGO_QUERY("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDayRecord << "orderref" << posd->OrderRef));

		
		USER_PRINT("DBManager::DeletePositionDetailTrade ok");
	}
	else {
		cout << "删除trade持仓明细,持仓明细不存在!" << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::DeletePositionDetailTrade OK");
}

void DBManager::UpdatePositionDetailTrade(USER_CSgitFtdcTradeField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::UpdatePositionDetailTrade");

	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_TRADE_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		client->update(DB_POSITIONDETAIL_TRADE_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
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

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::UpdatePositionDetailTrade OK");
}

void DBManager::getAllPositionDetailTrade(list<USER_CSgitFtdcTradeField *> *l_posd, string trader_id, string userid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getAllPositionDetailTrade()");
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CSgitFtdcTradeField *>::iterator itor;
		for (itor = l_posd->begin(); itor != l_posd->end();) {
			delete (*itor);
			itor = l_posd->erase(itor);
		}
	}

	unique_ptr<DBClientCursor> cursor;

	if (trader_id.compare("")) { //如果traderid不为空

		if (userid.compare("")) { //如果userid不为空
			cursor = client->query(DB_POSITIONDETAIL_TRADE_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			cursor = client->query(DB_POSITIONDETAIL_TRADE_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		cursor = client->query(DB_POSITIONDETAIL_TRADE_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {

		BSONObj p = cursor->next();

		USER_CSgitFtdcTradeField *new_pos = new USER_CSgitFtdcTradeField();

		strcpy(new_pos->InstrumentID, p.getStringField("instrumentid"));
		strcpy(new_pos->OrderRef, p.getStringField("orderref"));
		strcpy(new_pos->UserID, p.getStringField("userid"));
		new_pos->Direction = p.getIntField("direction");
		new_pos->OffsetFlag = p.getIntField("offsetflag");
		new_pos->HedgeFlag = p.getIntField("hedgeflag");
		new_pos->Price = p.getField("price").Double();
		strcpy(new_pos->ExchangeID, p.getStringField("exchangeid"));
		strcpy(new_pos->TradingDay, p.getStringField("tradingday"));
		strcpy(new_pos->TradingDayRecord, p.getStringField("tradingdayrecord"));
		strcpy(new_pos->TradeDate, p.getStringField("tradedate"));
		strcpy(new_pos->StrategyID, p.getStringField("strategyid"));
		new_pos->Volume = p.getIntField("volume");


		/*cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
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
		std::cout << "new_pos->StrategyID = " << new_pos->StrategyID << std::endl;*/

		l_posd->push_back(new_pos);
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::getAllPositionDetailTrade OK");
}

void DBManager::DropPositionDetailTrade() {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DropPositionDetailTrade");
	client->dropCollection(DB_POSITIONDETAIL_TRADE_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetailTrade ok");
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::CreatePositionDetailTradeChanged(USER_CSgitFtdcTradeField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::CreatePositionDetailTradeChanged");

	int posd_count_num = 0;

	if (posd != NULL) {

		posd_count_num = client->count(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION,
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
			b.append("exchangeid", posd->ExchangeID);

			BSONObj p = b.obj();
			client->insert(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, p);
			USER_PRINT("DBManager::CreatePositionDetailTradeChanged ok");
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::CreatePositionDetailTradeChanged OK");
}

void DBManager::DeletePositionDetailTradeChanged(USER_CSgitFtdcTradeField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::DeletePositionDetailTradeChanged");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//this->conn->update(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));
		
		client->remove(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, MONGO_QUERY("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef));

		USER_PRINT("DBManager::DeletePositionDetailTradeYesterday ok");
	}
	else {
		this->getXtsDBLogger()->debug("\t删除昨持仓明细,持仓明细不存在!");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::DeletePositionDetailTradeChanged OK");
}

void DBManager::UpdatePositionDetailTradeChanged(USER_CSgitFtdcTradeField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::UpdatePositionDetailTradeChanged");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		client->update(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
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
		USER_PRINT("DBManager::UpdatePositionDetailTradeChanged ok");
	}
	else {
		cout << "更新昨持仓明细,持仓明细未找到!" << endl;
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::UpdatePositionDetailTradeChanged OK");
}

void DBManager::getAllPositionDetailTradeChanged(list<USER_CSgitFtdcTradeField *> *l_posd, string trader_id, string userid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::getAllPositionDetailTradeChanged");
	this->getXtsDBLogger()->info("DBManager::getAllPositionDetailTradeChanged()");
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CSgitFtdcTradeField *>::iterator itor;
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
			cursor = client->query(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			USER_PRINT("如果userid为空");
			cursor = client->query(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		USER_PRINT("如果traderid为空");
		cursor = client->query(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();

		USER_CSgitFtdcTradeField *new_pos = new USER_CSgitFtdcTradeField();

		strcpy(new_pos->InstrumentID, p.getStringField("instrumentid"));
		strcpy(new_pos->OrderRef, p.getStringField("orderref"));
		strcpy(new_pos->UserID, p.getStringField("userid"));
		new_pos->Direction = p.getIntField("direction");
		new_pos->OffsetFlag = p.getIntField("offsetflag");
		new_pos->HedgeFlag = p.getIntField("hedgeflag");
		new_pos->Price = p.getField("price").Double();
		strcpy(new_pos->ExchangeID, p.getStringField("exchangeid"));
		strcpy(new_pos->TradingDay, p.getStringField("tradingday"));
		strcpy(new_pos->TradingDayRecord, p.getStringField("tradingdayrecord"));
		strcpy(new_pos->TradeDate, p.getStringField("tradedate"));
		strcpy(new_pos->StrategyID, p.getStringField("strategyid"));
		new_pos->Volume = p.getIntField("volume");

		/*cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
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
		cout << "volume = " << p.getIntField("volume") << std::endl;*/

		l_posd->push_back(new_pos);
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::getAllPositionDetailTradeChanged OK");
}

void DBManager::DropPositionDetailTradeChanged() {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DropPositionDetailTradeChanged");
	client->dropCollection(DB_POSITIONDETAIL_TRADE_CHANGED_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetailTradeChanged ok");
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

///************************************************************************/
///* 更新修改持仓明细记录                                                    */
///************************************************************************/
//bool DBManager::UpdatePositionModifyRecord(Strategy *stg, string recordtime) {
//	std::cout << "DBManager::UpdatePositionModifyRecord()" << std::endl;
//	int count_number = 0;
//	bool flag = true;
//	count_number = this->conn->count(DB_POSITIONMODIFYRECORD_COLLECTION,
//		BSON("strategy_id" << (stg->getStgStrategyId().c_str()) << "user_id" << (stg->getStgUserId().c_str()) << "is_active" << true));
//
//	if (count_number > 0) {
//		cout << "\t策略已存在,进行更新操作!" << endl;
//
//	}
//	else {
//		cout << "\t策略不存在!" << endl;
//		flag = false;
//	}
//	return flag;
//}

void DBManager::CreatePositionDetailTradeYesterday(USER_CSgitFtdcTradeField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::CreatePositionDetailTradeYesterday");

	int posd_count_num = 0;

	if (posd != NULL) {
		
		posd_count_num = client->count(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION,
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
			b.append("exchangeid", posd->ExchangeID);

			BSONObj p = b.obj();
			client->insert(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, p);
			USER_PRINT("DBManager::CreatePositionDetailTradeYesterday ok");
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::CreatePositionDetailYesterday OK");
}

void DBManager::DeletePositionDetailTradeYesterday(USER_CSgitFtdcTradeField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::DeletePositionDetailTradeYesterday()");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		//this->conn->update(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("is_active" << ISNOTACTIVE)));
		
		client->remove(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, MONGO_QUERY("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef));

		USER_PRINT("DBManager::DeletePositionDetailTradeYesterday ok");
	}
	else {
		this->getXtsDBLogger()->debug("\t删除昨持仓明细,持仓明细不存在!");
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::DeletePositionDetailTradeYesterday OK");
}

void DBManager::UpdatePositionDetailTradeYesterday(USER_CSgitFtdcTradeField *posd) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::UpdatePositionDetailTradeYesterday");
	int count_number = 0;

	count_number = client->count(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION,
		BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE));

	if (count_number > 0) {
		client->update(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, BSON("userid" << posd->UserID << "strategyid" << posd->StrategyID << "tradingday" << posd->TradingDay << "orderref" << posd->OrderRef << "is_active" << ISACTIVE), BSON("$set" << BSON("instrumentid" << posd->InstrumentID
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
	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::UpdatePositionDetailTradeYesterday OK");
}

void DBManager::getAllPositionDetailTradeYesterday(list<USER_CSgitFtdcTradeField *> *l_posd, string trader_id, string userid) {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::getAllPositionDetailTradeYesterday()");
	/// 初始化的时候，必须保证list为空
	if (l_posd->size() > 0) {
		list<USER_CSgitFtdcTradeField *>::iterator itor;
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
			cursor = client->query(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, MONGO_QUERY("userid" << userid << "is_active" << ISACTIVE));
		}
		else {
			USER_PRINT("如果userid为空");
			cursor = client->query(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
		}
	}
	else {
		USER_PRINT("如果traderid为空");
		cursor = client->query(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION, MONGO_QUERY("is_active" << ISACTIVE));
	}

	while (cursor->more()) {
		BSONObj p = cursor->next();

		USER_CSgitFtdcTradeField *new_pos = new USER_CSgitFtdcTradeField();

		strcpy(new_pos->InstrumentID, p.getStringField("instrumentid"));
		strcpy(new_pos->OrderRef, p.getStringField("orderref"));
		strcpy(new_pos->UserID, p.getStringField("userid"));
		new_pos->Direction = p.getIntField("direction");
		new_pos->OffsetFlag = p.getIntField("offsetflag");
		new_pos->HedgeFlag = p.getIntField("hedgeflag");
		new_pos->Price = p.getField("price").Double();
		strcpy(new_pos->ExchangeID, p.getStringField("exchangeid"));
		strcpy(new_pos->TradingDay, p.getStringField("tradingday"));
		strcpy(new_pos->TradingDayRecord, p.getStringField("tradingdayrecord"));
		strcpy(new_pos->TradeDate, p.getStringField("tradedate"));
		strcpy(new_pos->StrategyID, p.getStringField("strategyid"));
		new_pos->Volume = p.getIntField("volume");

		/*cout << "\t*instrumentid = " << p.getStringField("instrumentid") << ", ";
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
		cout << "volume = " << p.getIntField("volume") << std::endl;*/

		l_posd->push_back(new_pos);
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::getAllPositionDetailYesterday OK");
}

void DBManager::DropPositionDetailTradeYesterday() {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::DropPositionDetailTradeYesterday");
	client->dropCollection(DB_POSITIONDETAIL_TRADE_YESTERDAY_COLLECTION);
	USER_PRINT("DBManager::DropPositionDetailTradeYesterday ok");
	// 重新加到数据连接池队列
	this->recycleConn(client);
}

void DBManager::UpdateSystemRunningStatus(string value) {

	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	this->getXtsDBLogger()->info("DBManager::UpdateSystemRunningStatus value = {}", value);
	this->getXtsDBLogger()->info("DBManager::UpdateSystemRunningStatus key = {}", DB_RUNNING_KEY);

	int count_number = client->count(DB_SYSTEM_RUNNING_STATUS_COLLECTION,
		BSON("key" << DB_RUNNING_KEY));

	if (count_number > 0) {
		client->update(DB_SYSTEM_RUNNING_STATUS_COLLECTION, MONGO_QUERY("key" << DB_RUNNING_KEY), BSON("$set" << BSON("status" << value)));
	}
	else {
		std::cout << "\t查找数量为0！" << std::endl;
	}

	// 重新加到数据连接池队列
	this->recycleConn(client);
	USER_PRINT("DBManager::UpdateSystemRunningStatus ok");
}

bool DBManager::GetSystemRunningStatus() {
	// 从数据连接池队列中获取连接
	mongo::DBClientConnection *client = this->getConn();
	USER_PRINT("DBManager::GetSystemRunningStatus");
	bool flag = false;;
	BSONObj obj = client->findOne(DB_SYSTEM_RUNNING_STATUS_COLLECTION, MONGO_QUERY("key" << DB_RUNNING_KEY));
	if (!obj.isEmpty()) {
		if (!strcmp("off", obj.getStringField("status"))) {
			flag = true;
		}
		else {
			flag = false;
		}
	}
	// 重新加到数据连接池队列
	this->recycleConn(client);
	return flag;
}

mongo::DBClientConnection * DBManager::getConn() {
	mongo::DBClientConnection *client = NULL;
	this->queue_DBClient.wait_dequeue(client);
	return client;
}

void DBManager::recycleConn(mongo::DBClientConnection *conn) {
	this->queue_DBClient.enqueue(conn);
}

void DBManager::setDB_Connect_Status(bool db_connect_status) {
	this->db_connect_status = db_connect_status;
}

bool DBManager::getDB_Connect_Status() {
	return this->db_connect_status;
}

std::shared_ptr<spdlog::logger> DBManager::getXtsDBLogger() {
	return this->xts_db_logger;
}
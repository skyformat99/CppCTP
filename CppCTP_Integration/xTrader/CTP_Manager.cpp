#include <errno.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "CTP_Manager.h"
#include "User.h"
#include "Utils.h"
#include "Debug.h"
#include "msg.h"

using namespace rapidjson;

static DBManager *static_dbm = new DBManager();

#define MSG_SEND_FLAG 1

int server_msg_ref = 0;

CTP_Manager::CTP_Manager() {
	this->on_off = 0;
	//this->dbm = new DBManager();
	this->dbm = new DBManager();
	this->l_user = new list<User *>();
	this->l_trader = new list<string>();
	this->l_obj_trader = new list<Trader *>();
	this->l_strategys = new list<Strategy *>();
	this->l_instrument = new list<string>();
	this->l_unsubinstrument = new list<string>();
}

bool CTP_Manager::CheckIn(Login *login) {
	
}

/// trader login
bool CTP_Manager::TraderLogin(string traderid, string password) {
	return this->dbm->FindTraderByTraderIdAndPassword(traderid, password);
}

/// admin login
bool CTP_Manager::AdminLogin(string adminid, string password) {
	return this->dbm->FindAdminByAdminIdAndPassword(adminid, password);
}

User * CTP_Manager::CreateAccount(User *user) {
	USER_PRINT("CTP_Manager::CreateAccount");
	//tcp://180.168.146.187:10030 //24H
	//tcp://180.168.146.187:10000 //实盘仿真

	if (user != NULL) {
		TdSpi *tdspi = new TdSpi();

		//User *user = new User(td_frontAddress, td_broker, td_user, td_pass, td_user, TraderID);

		/*设置api*/
		string flowpath = "conn/td/" + user->getUserID() + "/";
		int flag = Utils::CreateFolder(flowpath.c_str());
		if (flag != 0) {
			cout << "无法创建用户流文件!" << endl;
			return NULL;
		}
		else {
			CThostFtdcTraderApi *tdapi = CThostFtdcTraderApi::CreateFtdcTraderApi(flowpath.c_str());
			cout << tdapi << endl;
			if (!tdapi) {
				return NULL;
			}
			else {
				user->setUserTradeAPI(tdapi);
				user->setUserTradeSPI(tdspi);
			}
		}

		sleep(1);
		user->getUserTradeSPI()->Connect(user);
		sleep(1);
		user->getUserTradeSPI()->Login(user);
		sleep(1);
		user->getUserTradeSPI()->QrySettlementInfoConfirm(user);

		
		//tdspi->QrySettlementInfo(user);
		//sleep(6);
		//string instrument = "cu1609";
		//user->getUserTradeSPI()->OrderInsert(user, const_cast<char *>(instrument.c_str()), '0', '0', 20, 39000, "1");
	}

	return user;
}

MdSpi * CTP_Manager::CreateMd(string md_frontAddress, string md_broker, string md_user, string md_pass) {
	MdSpi *mdspi = NULL;
	CThostFtdcMdApi *mdapi = NULL;
	string conn_md_frontAddress = md_frontAddress;
	md_frontAddress = md_frontAddress.substr(6, md_frontAddress.length() - 1);
	int pos = md_frontAddress.find_first_of(':', 0);
	md_frontAddress.replace(pos, 1, "_");

	/*设置api*/
	string flowpath = "conn/md/" + md_frontAddress + "/";
	int flag = Utils::CreateFolder(flowpath.c_str());
	if (flag != 0) {
		cout << "无法创建行情流文件!" << endl;
		return NULL;
	} else {
		mdapi = CThostFtdcMdApi::CreateFtdcMdApi(flowpath.c_str());
		mdspi = new MdSpi(mdapi);
	}
	USER_PRINT(const_cast<char *>(conn_md_frontAddress.c_str()));
	mdspi->Connect(const_cast<char *>(conn_md_frontAddress.c_str()));
	sleep(1);
	mdspi->Login(const_cast<char *>(md_broker.c_str()), const_cast<char *>(md_user.c_str()), const_cast<char *>(md_pass.c_str()));

	return mdspi;
}

/// 释放
void CTP_Manager::ReleaseAccount(User *user) {
	if (user) {
		// 释放UserApi
		if (user->getUserTradeAPI())
		{
			user->getUserTradeAPI()->RegisterSpi(NULL);
			user->getUserTradeAPI()->Release();
			user->setUserTradeAPI(NULL);
		}
		// 释放UserSpi实例   
		if (user->getUserTradeSPI())
		{
			delete user->getUserTradeSPI();
			user->setUserTradeSPI(NULL);
		}
	}
}

/// 订阅行情
void CTP_Manager::SubmarketData(MdSpi *mdspi, list<string> *l_instrument) {
	if (mdspi && (l_instrument->size() > 0)) {
		mdspi->SubMarket(l_instrument);
	}
}

/// 退订合约
void CTP_Manager::UnSubmarketData(MdSpi *mdspi, string instrumentID, list<string > *l_unsubinstrument) {
	if (mdspi && (this->l_instrument->size() > 0)) {
		/// 从合约列表里删除一个
		this->delSubInstrument(instrumentID, this->l_instrument);
		/// 统计合约的个数
		int count = this->calInstrument(instrumentID, this->l_instrument);
		/// 如果合约个数为0,必须退订
		if (count == 0) {
			this->l_unsubinstrument->push_back(instrumentID);
			mdspi->UnSubMarket(this->l_unsubinstrument);
		}
		
	}
}

/// 添加订阅合约
list<string> * CTP_Manager::addSubInstrument(string instrumentID, list<string > *l_instrument) {
	l_instrument->push_back(instrumentID);
	USER_PRINT(l_instrument->size());
	return l_instrument;
}

/// 删除订阅合约
list<string> * CTP_Manager::delSubInstrument(string instrumentID, list<string > *l_instrument) {
	USER_PRINT(l_instrument->size());
	if (l_instrument->size() > 0) {
		list<string>::iterator itor;
		for (itor = l_instrument->begin(); itor != l_instrument->end();) {
			cout << *itor << endl;
			if (*itor == instrumentID) {
				itor = l_instrument->erase(itor);
				USER_PRINT("Remove OK");
				//break;
				USER_PRINT(l_instrument->size());
				USER_PRINT("CTP_Manager::delInstrument finish");
				return l_instrument;
			}
			else {
				itor++;
			}
		}
	}
	USER_PRINT(l_instrument->size());
	USER_PRINT("CTP_Manager::delInstrument finish");
	return l_instrument;
}

/// 统计合约数量
int CTP_Manager::calInstrument(string instrumentID, list<string> *l_instrument) {
	int count = 0;
	if (l_instrument->size() > 0) {
		list<string>::iterator itor;
		for (itor = l_instrument->begin(); itor != l_instrument->end(); itor++) {
			cout << *itor << endl;
			if ((*itor) == instrumentID) {
				count++;
			}
		}
	}
	else {
		count = 0;
	}
	return count;
}

/// 退订合约增加
list<string> CTP_Manager::addUnSubInstrument(string instrumentID, list<string> l_instrument) {
	if (l_instrument.size() > 0) {
		l_instrument.clear();
	}
	l_instrument.push_back(instrumentID);
	return l_instrument;
}

/// 得到l_instrument
list<string> * CTP_Manager::getL_Instrument() {
	return this->l_instrument;
}

/// 得到数据库操作对象
DBManager * CTP_Manager::getDBManager() {
	return this->dbm;
}

/// 设置l_trader
void CTP_Manager::addTraderToLTrader(string trader) {
	this->l_trader->push_back(trader);
}

/// 获取trader是否在l_trader里
bool CTP_Manager::checkInLTrader(string trader) {
	bool flag = false;
	list<string>::iterator Itor;
	for (Itor = this->l_trader->begin(); Itor != this->l_trader->end(); Itor++) {
		if (*Itor == trader) {
			flag = true;
		}
	}
	return flag;
}

/// 得到l_trader
list<string> *CTP_Manager::getL_Trader() {
	return this->l_trader;
}

/// 得到l_obj_trader
list<Trader *> * CTP_Manager::getL_Obj_Trader() {
	return this->l_obj_trader;
}

/// 移除元素
void CTP_Manager::removeFromLTrader(string trader) {
	list<string>::iterator Itor;
	for (Itor = this->l_trader->begin(); Itor != this->l_trader->end();) {
		if (*Itor == trader) {
			Itor = this->l_trader->erase(Itor);
		}
		else {
			Itor++;
		}
	}
}

/// 增加用户期货账户
void CTP_Manager::addFuturesToTrader(string traderid, User *user) {
	map<string, list<User *> *>::iterator m_itor;
	m_itor = this->m_trader.find(traderid);
	if (m_itor == (this->m_trader.end())) {
		cout << "we do not find" << traderid << endl;
		list<User *> *l_user = new list<User *>();
		this->m_trader.insert(pair<string, list<User *> *>(traderid, l_user));
	}
	else {
		cout << "we find " << traderid << endl;
		this->m_trader[traderid]->push_back(user);
	}
		
}

/// 获取期货账户map
map<string, list<User *> *> CTP_Manager::getTraderMap() {
	return this->m_trader;
}

/// 返回用户列表
list<User *> *CTP_Manager::getL_User() {
	return this->l_user;
}

/// 得到strategy_list
list<Strategy *> * CTP_Manager::getListStrategy() {
	return this->l_strategys;
}

/// 设置strategy_list
void CTP_Manager::setListStrategy(list<Strategy *> *l_strategys) {
	this->l_strategys = l_strategys;
}

/// 设置mdspi
void CTP_Manager::setMdSpi(MdSpi *mdspi) {
	this->mdspi = mdspi;
}

/// 获得mdspi
MdSpi * CTP_Manager::getMdSpi() {
	return this->mdspi;
}

/// 设置开关
int CTP_Manager::getOn_Off() {
	return this->on_off;
}

void CTP_Manager::setOn_Off(int on_off) {
	this->on_off = on_off;
}

/// 处理客户端发来的消息
void CTP_Manager::HandleMessage(int fd, char *msg_tmp) {
	//std::cout << "HandleMessage fd = " << fd << std::endl;
	std::cout << "服务端收到的数据 = " << msg_tmp << std::endl;

	const char *rsp_msg;

	rapidjson::Document doc;
	rapidjson::Document build_doc;
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<StringBuffer> writer(buffer);

	//doc.Parse(msg);

	char msg[sizeof(msg_tmp)];
	memset(msg, 0x00, sizeof(msg));
	strcpy(msg, msg_tmp);

	if (doc.ParseInsitu(msg).HasParseError()) {
		std::cout << "json parse error" << std::endl;
		return;
	}
	
	if (!doc.HasMember("MsgType")) {
		std::cout << "json had no msgtype" << std::endl;
		return;
	}

	rapidjson::Value& s = doc["MsgType"];

	int msgtype = s.GetInt();

	if (msgtype == 1) { // TraderInfo
		std::cout << "请求交易登录..." << std::endl;
		rapidjson::Value &TraderID = doc["TraderID"];
		rapidjson::Value &Password = doc["Password"];
		rapidjson::Value &MsgRef = doc["MsgRef"];
		rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
		rapidjson::Value &MsgSrc = doc["MsgSrc"];
		string s_TraderID = TraderID.GetString();
		string s_Password = Password.GetString();
		int i_MsgRef = MsgRef.GetInt();
		int i_MsgSendFlag = MsgSendFlag.GetInt();
		int i_MsgSrc = MsgSrc.GetInt();
		
		std::cout << "收到交易员ID = " << s_TraderID << std::endl;
		std::cout << "收到交易员密码 = " << s_Password << std::endl;

		bool isTraderExists = static_dbm->FindTraderByTraderIdAndPassword(s_TraderID, s_Password);
		if (isTraderExists) { // 用户存在
			std::cout << "交易员存在..." << std::endl;
			build_doc.SetObject();
			rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
			build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
			build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
			build_doc.AddMember("MsgType", 1, allocator);
			build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
			build_doc.AddMember("MsgResult", 0, allocator);
			build_doc.AddMember("MsgErrorReason", "", allocator);
			build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
		}
		else {
			std::cout << "交易员不存在..." << std::endl;
			build_doc.SetObject();
			rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();

			build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
			build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
			build_doc.AddMember("MsgType", 1, allocator);
			build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
			build_doc.AddMember("MsgResult", 1, allocator);
			build_doc.AddMember("MsgErrorReason", "No Trader Find!", allocator);
			build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
		}
	}
	else if (msgtype == 2) { // UserInfo
		std::cout << "请求账户信息..." << std::endl;
		rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
		rapidjson::Value &TraderID = doc["TraderID"];
		rapidjson::Value &MsgRef = doc["MsgRef"];
		rapidjson::Value &MsgSrc = doc["MsgSrc"];

		string s_TraderID = TraderID.GetString();
		int i_MsgRef = MsgRef.GetInt();
		int i_MsgSendFlag = MsgSendFlag.GetInt();
		int i_MsgSrc = MsgSrc.GetInt();

		std::cout << "收到交易员ID = " << s_TraderID << std::endl;
		list<FutureAccount *> l_futureaccount;
		static_dbm->SearchFutrueListByTraderID(s_TraderID, &l_futureaccount);

		/*构建UserInfo的Json*/
		build_doc.SetObject();
		rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
		build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
		build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
		build_doc.AddMember("MsgType", 2, allocator);
		build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
		build_doc.AddMember("MsgResult", 0, allocator);
		build_doc.AddMember("MsgErrorReason", "", allocator);
		//创建Info数组
		rapidjson::Value info_array(rapidjson::kArrayType);

		list<FutureAccount *>::iterator future_itor;
		for (future_itor = l_futureaccount.begin(); future_itor != l_futureaccount.end(); future_itor++) {

			rapidjson::Value info_object(rapidjson::kObjectType);
			info_object.SetObject();
			info_object.AddMember("brokerid", rapidjson::StringRef((*future_itor)->getBrokerID().c_str()), allocator);
			info_object.AddMember("traderid", rapidjson::StringRef((*future_itor)->getTraderID().c_str()), allocator);
			info_object.AddMember("password", rapidjson::StringRef((*future_itor)->getPassword().c_str()), allocator);
			info_object.AddMember("userid", rapidjson::StringRef((*future_itor)->getUserID().c_str()), allocator);
			info_object.AddMember("frontaddress", rapidjson::StringRef((*future_itor)->getFrontAddress().c_str()), allocator);

			info_array.PushBack(info_object, allocator);
		}

		build_doc.AddMember("Info", info_array, allocator);
		build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

	}
	else if (msgtype == 3) { // 创建StrategyInfo
		std::cout << "请求查询策略信息..." << std::endl;

		rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
		rapidjson::Value &TraderID = doc["TraderID"];
		rapidjson::Value &MsgRef = doc["MsgRef"];
		rapidjson::Value &MsgSrc = doc["MsgSrc"];
		rapidjson::Value &UserID = doc["UserID"];

		string s_TraderID = TraderID.GetString();
		int i_MsgRef = MsgRef.GetInt();
		int i_MsgSendFlag = MsgSendFlag.GetInt();
		int i_MsgSrc = MsgSrc.GetInt();
		string s_UserID = UserID.GetString();

		std::cout << "收到交易员ID = " << s_TraderID << std::endl;
		std::cout << "收到期货账户ID = " << s_UserID << std::endl;

		list<Strategy *> l_strategys;
		static_dbm->getAllStrategy(&l_strategys, s_TraderID, s_UserID);

		/*构建StrategyInfo的Json*/
		build_doc.SetObject();
		rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
		build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
		build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
		build_doc.AddMember("MsgType", 3, allocator);
		build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
		build_doc.AddMember("MsgResult", 0, allocator);
		build_doc.AddMember("MsgErrorReason", "", allocator);
		//创建Info数组
		rapidjson::Value info_array(rapidjson::kArrayType);

		list<Strategy *>::iterator stg_itor;
		for (stg_itor = l_strategys.begin(); stg_itor != l_strategys.end(); stg_itor++) {

			rapidjson::Value info_object(rapidjson::kObjectType);
			info_object.SetObject();
			info_object.AddMember("position_a_sell_today", (*stg_itor)->getStgPositionASellToday(), allocator);
			info_object.AddMember("position_b_sell", (*stg_itor)->getStgPositionBSell(), allocator);
			info_object.AddMember("spread_shift", (*stg_itor)->getStgSpreadShift(), allocator);
			info_object.AddMember("position_b_sell_today", (*stg_itor)->getStgPositionBSellToday(), allocator);
			info_object.AddMember("position_b_buy_today", (*stg_itor)->getStgPositionBBuyToday(), allocator);
			info_object.AddMember("position_a_sell", (*stg_itor)->getStgPositionASell(), allocator);
			info_object.AddMember("buy_close", (*stg_itor)->getStgBuyClose(), allocator);
			info_object.AddMember("stop_loss", (*stg_itor)->getStgStopLoss(), allocator);
			info_object.AddMember("position_b_buy_yesterday", (*stg_itor)->getStgPositionBBuyYesterday(), allocator);
			info_object.AddMember("is_active", (*stg_itor)->isStgIsActive(), allocator);
			info_object.AddMember("position_b_sell_yesterday", (*stg_itor)->getStgPositionBSellYesterday(), allocator);
			info_object.AddMember("strategy_id", rapidjson::StringRef((*stg_itor)->getStgStrategyId().c_str()), allocator);
			info_object.AddMember("position_b_buy", (*stg_itor)->getStgPositionBBuy(), allocator);
			info_object.AddMember("lots_batch", (*stg_itor)->getStgLotsBatch(), allocator);
			info_object.AddMember("position_a_buy", (*stg_itor)->getStgPositionABuy(), allocator);
			info_object.AddMember("sell_open", (*stg_itor)->getStgSellOpen(), allocator);
			info_object.AddMember("order_algorithm", rapidjson::StringRef((*stg_itor)->getStgOrderAlgorithm().c_str()), allocator);
			info_object.AddMember("trader_id", rapidjson::StringRef((*stg_itor)->getStgTraderId().c_str()), allocator);
			info_object.AddMember("order_action_tires_limit", (*stg_itor)->getStgOrderActionTiresLimit(), allocator);
			info_object.AddMember("sell_close", (*stg_itor)->getStgSellClose(), allocator);
			info_object.AddMember("buy_open", (*stg_itor)->getStgBuyOpen(), allocator);
			info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator);
			
			rapidjson::Value instrument_array(rapidjson::kArrayType);
			for (int j = 0; j < 2; j++) {
				rapidjson::Value instrument_object(rapidjson::kObjectType);
				instrument_object.SetObject();
				if (j == 0) {
					instrument_object.SetString(rapidjson::StringRef((*stg_itor)->getStgInstrumentIdA().c_str()));
				}
				else if (j == 1) {
					instrument_object.SetString(rapidjson::StringRef((*stg_itor)->getStgInstrumentIdB().c_str()));
				}

				instrument_array.PushBack(instrument_object, allocator);
			}
			info_object.AddMember("list_instrument_id", instrument_array, allocator);
			info_object.AddMember("position_a_buy_yesterday", (*stg_itor)->getStgPositionABuyYesterday(), allocator);
			info_object.AddMember("user_id", rapidjson::StringRef((*stg_itor)->getStgUserId().c_str()), allocator);
			info_object.AddMember("position_a_buy_today", (*stg_itor)->getStgPositionABuyToday(), allocator);
			info_object.AddMember("position_a_sell_yesterday", (*stg_itor)->getStgPositionASellYesterday(), allocator);
			info_object.AddMember("lots", (*stg_itor)->getStgLots(), allocator);
			info_object.AddMember("a_wait_price_tick", (*stg_itor)->getStgAWaitPriceTick(), allocator);
			info_object.AddMember("b_wait_price_tick", (*stg_itor)->getStgBWaitPriceTick(), allocator);

			info_array.PushBack(info_object, allocator);
		}

		build_doc.AddMember("Info", info_array, allocator);
		build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
		
	}
	else if (msgtype == 4) { // market行情接口参数信息
		std::cout << "请求行情接口配置信息..." << std::endl;

		rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
		rapidjson::Value &TraderID = doc["TraderID"];
		rapidjson::Value &MsgRef = doc["MsgRef"];
		rapidjson::Value &MsgSrc = doc["MsgSrc"];

		string s_TraderID = TraderID.GetString();
		int i_MsgRef = MsgRef.GetInt();
		int i_MsgSendFlag = MsgSendFlag.GetInt();
		int i_MsgSrc = MsgSrc.GetInt();

		std::cout << "收到交易员ID = " << s_TraderID << std::endl;

		list<MarketConfig *> l_marketconfig;
		static_dbm->getAllMarketConfig(&l_marketconfig);

		/*构建MarketInfo的Json*/
		build_doc.SetObject();
		rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
		build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
		build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
		build_doc.AddMember("MsgType", 4, allocator);
		build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
		build_doc.AddMember("MsgResult", 0, allocator);
		build_doc.AddMember("MsgErrorReason", "", allocator);
		//创建Info数组
		rapidjson::Value info_array(rapidjson::kArrayType);

		list<MarketConfig *>::iterator market_itor;
		for (market_itor = l_marketconfig.begin(); market_itor != l_marketconfig.end(); market_itor++) {

			rapidjson::Value info_object(rapidjson::kObjectType);
			info_object.SetObject();
			info_object.AddMember("brokerid", rapidjson::StringRef((*market_itor)->getMarketID().c_str()), allocator);
			info_object.AddMember("traderid", rapidjson::StringRef((*market_itor)->getMarketFrontAddr().c_str()), allocator);
			info_object.AddMember("password", rapidjson::StringRef((*market_itor)->getBrokerID().c_str()), allocator);
			info_object.AddMember("userid", rapidjson::StringRef((*market_itor)->getUserID().c_str()), allocator);
			info_object.AddMember("frontaddress", rapidjson::StringRef((*market_itor)->getPassword().c_str()), allocator);

			info_array.PushBack(info_object, allocator);
		}

		build_doc.AddMember("Info", info_array, allocator);
		build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

		
	}
	else if (msgtype == 5) { // 修改Strategy（修改Strategy参数）
		std::cout << "请求策略修改..." << std::endl;

		rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
		rapidjson::Value &TraderID = doc["TraderID"];
		rapidjson::Value &MsgRef = doc["MsgRef"];
		rapidjson::Value &MsgSrc = doc["MsgSrc"];
		rapidjson::Value &UserID = doc["UserID"];
		rapidjson::Value &infoArray = doc["Info"];

		string s_TraderID = TraderID.GetString();
		int i_MsgRef = MsgRef.GetInt();
		int i_MsgSendFlag = MsgSendFlag.GetInt();
		int i_MsgSrc = MsgSrc.GetInt();
		string s_UserID = UserID.GetString();

		std::cout << "收到交易员ID = " << s_TraderID << std::endl;
		std::cout << "收到期货账户ID = " << s_UserID << std::endl;

		if (infoArray.IsArray()) {
			for (int i = 0; i < infoArray.Size(); i++) {
				const Value& object = infoArray[i];
				std::string q_user_id = object["user_id"].GetString();
				std::string q_strategy_id = object["strategy_id"].GetString();
			}
		}


	}
	else if (msgtype == 6) { // 新建Strategy（修改Strategy参数）
		std::cout << "新建策略..." << std::endl;


	}
	else if (msgtype == 7) { // 删除Strategy（修改Strategy参数）
		std::cout << "删除策略..." << std::endl;
	}
	else {
		std::cout << "请求类型错误!" << endl;
		//return;
	}

	
	build_doc.Accept(writer);
	//rsp_msg = const_cast<char *>(buffer.GetString());
	std::cout << "服务端响应数据 = " << buffer.GetString() << std::endl;
	if (strlen(buffer.GetString()) < 0) {
		std::cout << "服务端收到错误类型!" << std::endl;
	}
	else {
		if (write_msg(fd, const_cast<char *>(buffer.GetString()), strlen(buffer.GetString())) < 0) {
			printf("先前客户端已断开!!!\n");
			//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
			if (errno == EPIPE) {
				std::cout << "EPIPE" << std::endl;
				//break;
			}
			perror("protocal error");
		}
	}
	
}

/// 初始化
void CTP_Manager::init() {
	/// 数据库查询所有的Trader
	this->dbm->getAllTrader(this->l_trader);

	/// 查询所有的策略
	this->dbm->getAllStrategy(this->l_strategys);

	/// 查询所有的期货账户
	this->dbm->getAllFutureAccount(this->l_user);

	/// 绑定操作,策略绑定到对应期货账户下
	list<User *>::iterator user_itor;
	list<Strategy *>::iterator stg_itor;
	USER_PRINT("Iterator USERS");
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		
		USER_PRINT((*user_itor)->getUserID());

		USER_PRINT("Iterator STRATEGIES");
		for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) { // 遍历Strategy
			USER_PRINT((*stg_itor)->getStgUserId());
			if ((*stg_itor)->getStgUserId() == (*user_itor)->getUserID()) {
				USER_PRINT("Strategy Bind To User");
				(*user_itor)->addStrategyToList((*stg_itor));
				(*stg_itor)->setStgUser((*user_itor));
				USER_PRINT("(*stg_itor)->setStgUser((*user_itor))");
				USER_PRINT((*stg_itor)->getStgUser());
			}
		}
		/// 遍历设值后进行TD初始化
		this->CreateAccount((*user_itor));
		sleep(5);
		std::cout << "Account : " << (*user_itor)->getUserID() << " Init OK!" << std::endl;
	}


	/// 遍历User,对TdSpi进行Strategy_List赋值
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		USER_PRINT((*user_itor)->getUserID());
		(*user_itor)->getUserTradeSPI()->setListStrategy(this->l_strategys);
	}

	/// 第一个元素 this->l_user->front()

	/// 查询合约价格最小跳
	/// 遍历User,对TdSpi进行Strategy_List赋值
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		USER_PRINT((*user_itor)->getUserID());
		(*user_itor)->getUserTradeSPI()->QryInstrument();
	}

	/// 对策略合约价格最小跳赋值
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		(*user_itor)->setStgInstrumnetPriceTick();
	}

	/// 行情初始化
	MarketConfig *mc = this->dbm->getOneMarketConfig();
	if (mc != NULL) {
		this->mdspi = this->CreateMd(mc->getMarketFrontAddr(), mc->getBrokerID(), mc->getUserID(), mc->getPassword());
		if (this->mdspi != NULL) {
			/// 向mdspi赋值strategys
			this->mdspi->setListStrategy(this->l_strategys);
			/// 订阅合约
			for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) { // 遍历Strategy
				USER_PRINT((*stg_itor)->getStgInstrumentIdA());
				USER_PRINT((*stg_itor)->getStgInstrumentIdB());
				/// 添加策略的合约到l_instrument
				this->addSubInstrument((*stg_itor)->getStgInstrumentIdA(), this->l_instrument);
				this->addSubInstrument((*stg_itor)->getStgInstrumentIdB(), this->l_instrument);
			}
			/// 订阅合约
			this->SubmarketData(this->mdspi, this->l_instrument);
		}
	}
}
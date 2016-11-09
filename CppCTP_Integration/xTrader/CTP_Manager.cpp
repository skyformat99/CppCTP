#include <errno.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "CTP_Manager.h"
#include "User.h"
#include "Utils.h"
#include "Debug.h"
#include "msg.h"

#define MSG_SEND_FLAG 1
using namespace rapidjson;

static DBManager *static_dbm = new DBManager();
static Trader *op = new Trader();

int server_msg_ref = 0;

CTP_Manager::CTP_Manager() {
	this->on_off = 0;
	//this->dbm = new DBManager();
	this->dbm = new DBManager();
	this->l_user = new list<User *>();
	this->l_trader = new list<string>();
	this->l_obj_trader = new list<Trader *>();
	this->l_strategys = new list<Strategy *>();
	this->l_strategys_yesterday = new list<Strategy *>();
	this->l_instrument = new list<string>();
	this->l_unsubinstrument = new list<string>();
}

bool CTP_Manager::CheckIn(Login *login) {
	
}

/// trader login
bool CTP_Manager::TraderLogin(string traderid, string password, Trader *op) {
	return this->dbm->FindTraderByTraderIdAndPassword(traderid, password, op);
}

/// admin login
bool CTP_Manager::AdminLogin(string adminid, string password) {
	return this->dbm->FindAdminByAdminIdAndPassword(adminid, password);
}

User * CTP_Manager::CreateAccount(User *user, list<Strategy *> *l_strategys) {
	USER_PRINT("CTP_Manager::CreateAccount");
	//tcp://180.168.146.187:10030 //24H
	//tcp://180.168.146.187:10000 //实盘仿真

	if (user != NULL) {
		TdSpi *tdspi = new TdSpi();

		tdspi->setListStrategy(l_strategys); // 初始化策略给到位

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
			cout << "TdApi初始化完成... tdapi = " << tdapi << endl;
			if (!tdapi) {
				return NULL;
			}
			else {
				user->setUserTradeAPI(tdapi);
				user->setUserTradeSPI(tdspi);
			}
		}

		sleep(1);
		user->getUserTradeSPI()->Connect(user); // 连接
		sleep(1);
		user->getUserTradeSPI()->Login(user); // 登陆
		sleep(1);
		user->getUserTradeSPI()->QrySettlementInfoConfirm(user); // 确认交易结算

	}

	return user;
}

MdSpi * CTP_Manager::CreateMd(string md_frontAddress, string md_broker, string md_user, string md_pass, list<Strategy *> *l_strategys) {
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
		mdspi->setListStrategy(l_strategys); // 初始化策略给到位
	}
	USER_PRINT(const_cast<char *>(conn_md_frontAddress.c_str()));
	mdspi->Connect(const_cast<char *>(conn_md_frontAddress.c_str())); // 连接
	sleep(1);
	mdspi->Login(const_cast<char *>(md_broker.c_str()), const_cast<char *>(md_user.c_str()), const_cast<char *>(md_pass.c_str())); // 登陆
	sleep(1);

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

/// 得到strategy_list
list<Strategy *> * CTP_Manager::getListStrategyYesterday() {
	return this->l_strategys_yesterday;
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
void CTP_Manager::HandleMessage(int fd, char *msg_tmp, CTP_Manager *ctp_m) {
	//std::cout << "HandleMessage fd = " << fd << std::endl;
	std::cout << "服务端收到的数据 = " << msg_tmp << std::endl;
	std::cout << "CTP管理类地址 = " << ctp_m << std::endl;

	const char *rsp_msg;

	rapidjson::Document doc;
	rapidjson::Document build_doc;
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<StringBuffer> writer(buffer);

	//doc.Parse(msg);

	char msg[sizeof(msg_tmp)];
	memset(msg, 0x00, sizeof(msg));
	strcpy(msg, msg_tmp);

	if (doc.ParseInsitu(msg).HasParseError()) { // 解析出错
		std::cout << "json parse error" << std::endl;
		//return;
		build_doc.SetObject();
		rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
		build_doc.AddMember("MsgType", 99, allocator);
		build_doc.AddMember("MsgResult", 1, allocator);
		build_doc.AddMember("MsgErrorReason", "json解析出错", allocator);
	}
	else {
		if (!doc.HasMember("MsgType")) { // json不包含MsgType key
			std::cout << "json had no msgtype" << std::endl;
			//return;
			build_doc.SetObject();
			rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
			build_doc.AddMember("MsgType", 99, allocator);
			build_doc.AddMember("MsgResult", 1, allocator);
			build_doc.AddMember("MsgErrorReason", "json未包含消息类型", allocator);
		}
		else {
			rapidjson::Value& s = doc["MsgType"];

			int msgtype = s.GetInt();

			std::cout << "msgtype = " << msgtype << std::endl;

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

				bool isTraderExists = static_dbm->FindTraderByTraderIdAndPassword(s_TraderID, s_Password, op);
				if (isTraderExists) { // 用户存在
					std::cout << "交易员存在..." << std::endl;
					build_doc.SetObject();
					rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
					build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
					build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
					build_doc.AddMember("MsgType", 1, allocator);
					build_doc.AddMember("TraderName", rapidjson::StringRef(op->getTraderName().c_str()), allocator);
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
					build_doc.AddMember("TraderName", "", allocator);
					if (s_TraderID == "") {
						std::cout << "s_TraderID == ''" << std::endl;
						build_doc.AddMember("TraderID", "", allocator);
					}
					else {
						std::cout << "!s_TraderID == ''" << std::endl;
						build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
					}

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
					//info_object.AddMember("is_active", (*stg_itor)->isStgIsActive(), allocator);
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
					//info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator);

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
					info_object.AddMember("StrategyOnoff", (*stg_itor)->getOn_Off(), allocator);

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
					info_object.AddMember("brokerid", rapidjson::StringRef((*market_itor)->getBrokerID().c_str()), allocator);
					info_object.AddMember("password", rapidjson::StringRef((*market_itor)->getPassword().c_str()), allocator);
					info_object.AddMember("userid", rapidjson::StringRef((*market_itor)->getUserID().c_str()), allocator);
					info_object.AddMember("frontaddress", rapidjson::StringRef((*market_itor)->getMarketFrontAddr().c_str()), allocator);

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

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 5, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("MsgResult", 0, allocator);
				build_doc.AddMember("MsgErrorReason", "", allocator);
				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				/*1:进行策略的修改,更新到数据库*/
				if (infoArray.IsArray()) {
					std::cout << "info is array" << std::endl;
					for (int i = 0; i < infoArray.Size(); i++) {
						const Value& object = infoArray[i];
						std::string q_user_id = object["user_id"].GetString();
						std::string q_strategy_id = object["strategy_id"].GetString();
						std::cout << "q_user_id = " << q_user_id << std::endl;
						std::cout << "q_strategy_id = " << q_strategy_id << std::endl;
						list<Strategy *>::iterator stg_itor;
						for (stg_itor = ctp_m->getListStrategy()->begin(); stg_itor != ctp_m->getListStrategy()->end(); stg_itor++) {
							std::cout << "(*stg_itor)->getStgUserId() = " << (*stg_itor)->getStgUserId() << std::endl;
							std::cout << "(*stg_itor)->getStgStrategyId() = " << (*stg_itor)->getStgStrategyId() << std::endl;
							if (((*stg_itor)->getStgUserId() == q_user_id) && ((*stg_itor)->getStgStrategyId() == q_strategy_id)) {
								std::cout << "找到即将修改的Strategy" << std::endl;

								(*stg_itor)->setStgPositionASellToday(object["position_a_sell_today"].GetInt());
								(*stg_itor)->setStgPositionBSell(object["position_b_sell"].GetInt());
								(*stg_itor)->setStgSpreadShift(object["spread_shift"].GetDouble());
								(*stg_itor)->setStgPositionBSellToday(object["position_b_sell_today"].GetInt());
								(*stg_itor)->setStgPositionBBuyToday(object["position_b_buy_today"].GetInt());
								(*stg_itor)->setStgPositionASell(object["position_a_sell"].GetInt());
								(*stg_itor)->setStgBuyClose(object["buy_close"].GetDouble());
								(*stg_itor)->setStgStopLoss(object["stop_loss"].GetDouble());
								(*stg_itor)->setStgPositionBBuyYesterday(object["position_b_buy_yesterday"].GetInt());
								//(*stg_itor)->setStgIsActive(object["is_active"].GetBool());
								(*stg_itor)->setStgPositionBSellYesterday(object["position_b_sell_yesterday"].GetInt());
								(*stg_itor)->setStgStrategyId(object["strategy_id"].GetString());
								(*stg_itor)->setStgPositionBBuy(object["position_b_buy"].GetInt());
								(*stg_itor)->setStgLotsBatch(object["lots_batch"].GetInt());
								(*stg_itor)->setStgPositionABuy(object["position_a_buy"].GetInt());
								(*stg_itor)->setStgSellOpen(object["sell_open"].GetDouble());
								(*stg_itor)->setStgOrderAlgorithm(object["order_algorithm"].GetString());
								(*stg_itor)->setStgTraderId(object["trader_id"].GetString());
								(*stg_itor)->setStgOrderActionTiresLimit(object["order_action_tires_limit"].GetInt());
								(*stg_itor)->setStgSellClose(object["sell_close"].GetDouble());
								(*stg_itor)->setStgBuyOpen(object["buy_open"].GetDouble());
								//(*stg_itor)->setStgOnlyClose(object["only_close"].GetBool());


								//遍历list_instrument_id
								const Value& info_object = object["list_instrument_id"];
								if (info_object.IsArray()) {
									for (int j = 0; j < info_object.Size(); j++) {
										std::string instrument = info_object[j].GetString();
										std::cout << "instrument[" << j << "] = " << instrument << std::endl;
										if (j == 0) {
											(*stg_itor)->setStgInstrumentIdA(instrument);
										}
										else if (j == 1) {
											(*stg_itor)->setStgInstrumentIdB(instrument);
										}
									}
								}

								(*stg_itor)->setStgPositionABuyYesterday(object["position_a_buy_yesterday"].GetInt());
								(*stg_itor)->setStgUserId(object["user_id"].GetString());
								(*stg_itor)->setStgPositionABuyToday(object["position_a_buy_today"].GetInt());
								(*stg_itor)->setStgPositionASellYesterday(object["position_a_sell_yesterday"].GetInt());
								(*stg_itor)->setStgLots(object["lots"].GetInt());
								(*stg_itor)->setStgAWaitPriceTick(object["a_wait_price_tick"].GetDouble());
								(*stg_itor)->setStgBWaitPriceTick(object["b_wait_price_tick"].GetDouble());
								(*stg_itor)->setOn_Off(object["StrategyOnoff"].GetInt());

								std::cout << "Strategy修改完成!" << std::endl;
								static_dbm->UpdateStrategy((*stg_itor));

								/*构造内容json*/
								rapidjson::Value create_info_object(rapidjson::kObjectType);
								create_info_object.SetObject();
								create_info_object.AddMember("position_a_sell_today", (*stg_itor)->getStgPositionASellToday(), allocator);
								create_info_object.AddMember("position_b_sell", (*stg_itor)->getStgPositionBSell(), allocator);
								create_info_object.AddMember("spread_shift", (*stg_itor)->getStgSpreadShift(), allocator);
								create_info_object.AddMember("position_b_sell_today", (*stg_itor)->getStgPositionBSellToday(), allocator);
								create_info_object.AddMember("position_b_buy_today", (*stg_itor)->getStgPositionBBuyToday(), allocator);
								create_info_object.AddMember("position_a_sell", (*stg_itor)->getStgPositionASell(), allocator);
								create_info_object.AddMember("buy_close", (*stg_itor)->getStgBuyClose(), allocator);
								create_info_object.AddMember("stop_loss", (*stg_itor)->getStgStopLoss(), allocator);
								create_info_object.AddMember("position_b_buy_yesterday", (*stg_itor)->getStgPositionBBuyYesterday(), allocator);
								//create_info_object.AddMember("is_active", (*stg_itor)->isStgIsActive(), allocator);
								create_info_object.AddMember("position_b_sell_yesterday", (*stg_itor)->getStgPositionBSellYesterday(), allocator);
								create_info_object.AddMember("strategy_id", rapidjson::StringRef((*stg_itor)->getStgStrategyId().c_str()), allocator);
								create_info_object.AddMember("position_b_buy", (*stg_itor)->getStgPositionBBuy(), allocator);
								create_info_object.AddMember("lots_batch", (*stg_itor)->getStgLotsBatch(), allocator);
								create_info_object.AddMember("position_a_buy", (*stg_itor)->getStgPositionABuy(), allocator);
								create_info_object.AddMember("sell_open", (*stg_itor)->getStgSellOpen(), allocator);
								create_info_object.AddMember("order_algorithm", rapidjson::StringRef((*stg_itor)->getStgOrderAlgorithm().c_str()), allocator);
								create_info_object.AddMember("trader_id", rapidjson::StringRef((*stg_itor)->getStgTraderId().c_str()), allocator);
								create_info_object.AddMember("order_action_tires_limit", (*stg_itor)->getStgOrderActionTiresLimit(), allocator);
								create_info_object.AddMember("sell_close", (*stg_itor)->getStgSellClose(), allocator);
								create_info_object.AddMember("buy_open", (*stg_itor)->getStgBuyOpen(), allocator);
								//info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator);

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
								create_info_object.AddMember("list_instrument_id", instrument_array, allocator);
								create_info_object.AddMember("position_a_buy_yesterday", (*stg_itor)->getStgPositionABuyYesterday(), allocator);
								create_info_object.AddMember("user_id", rapidjson::StringRef((*stg_itor)->getStgUserId().c_str()), allocator);
								create_info_object.AddMember("position_a_buy_today", (*stg_itor)->getStgPositionABuyToday(), allocator);
								create_info_object.AddMember("position_a_sell_yesterday", (*stg_itor)->getStgPositionASellYesterday(), allocator);
								create_info_object.AddMember("lots", (*stg_itor)->getStgLots(), allocator);
								create_info_object.AddMember("a_wait_price_tick", (*stg_itor)->getStgAWaitPriceTick(), allocator);
								create_info_object.AddMember("b_wait_price_tick", (*stg_itor)->getStgBWaitPriceTick(), allocator);
								create_info_object.AddMember("StrategyOnoff", (*stg_itor)->getOn_Off(), allocator);

								create_info_array.PushBack(create_info_object, allocator);

							}
							else {
								std::cout << "未能找到修改的Strategy" << std::endl;
							}
						}
					}
				}
				else {
					std::cout << "未收到修改策略信息" << std::endl;
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

			}
			else if (msgtype == 6) { // 新建Strategy（修改Strategy参数）
				std::cout << "新建策略..." << std::endl;

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

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 5, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("MsgResult", 0, allocator);
				build_doc.AddMember("MsgErrorReason", "", allocator);
				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				/*1:进行策略的新建*/
				if (infoArray.IsArray()) {
					std::cout << "info is array" << std::endl;
					for (int i = 0; i < infoArray.Size(); i++) {
						const Value& object = infoArray[i];
						std::cout << "开始新建Strategy" << std::endl;

						Strategy *new_stg = new Strategy();

						new_stg->setStgPositionASellToday(object["position_a_sell_today"].GetInt());
						new_stg->setStgPositionBSell(object["position_b_sell"].GetInt());
						new_stg->setStgSpreadShift(object["spread_shift"].GetDouble());
						new_stg->setStgPositionBSellToday(object["position_b_sell_today"].GetInt());
						new_stg->setStgPositionBBuyToday(object["position_b_buy_today"].GetInt());
						new_stg->setStgPositionASell(object["position_a_sell"].GetInt());
						new_stg->setStgBuyClose(object["buy_close"].GetDouble());
						new_stg->setStgStopLoss(object["stop_loss"].GetDouble());
						new_stg->setStgPositionBBuyYesterday(object["position_b_buy_yesterday"].GetInt());
						//new_stg->setStgIsActive(object["is_active"].GetBool());
						new_stg->setStgPositionBSellYesterday(object["position_b_sell_yesterday"].GetInt());
						new_stg->setStgStrategyId(object["strategy_id"].GetString());
						new_stg->setStgPositionBBuy(object["position_b_buy"].GetInt());
						new_stg->setStgLotsBatch(object["lots_batch"].GetInt());
						new_stg->setStgPositionABuy(object["position_a_buy"].GetInt());
						new_stg->setStgSellOpen(object["sell_open"].GetDouble());
						new_stg->setStgOrderAlgorithm(object["order_algorithm"].GetString());
						new_stg->setStgTraderId(object["trader_id"].GetString());
						new_stg->setStgOrderActionTiresLimit(object["order_action_tires_limit"].GetInt());
						new_stg->setStgSellClose(object["sell_close"].GetDouble());
						new_stg->setStgBuyOpen(object["buy_open"].GetDouble());
						//new_stg->setStgOnlyClose(object["only_close"].GetBool());


						//遍历list_instrument_id
						const Value& info_object = object["list_instrument_id"];
						if (info_object.IsArray()) {
							for (int j = 0; j < info_object.Size(); j++) {
								std::string instrument = info_object[j].GetString();
								std::cout << "instrument[" << j << "] = " << instrument << std::endl;
								if (j == 0) {
									new_stg->setStgInstrumentIdA(instrument);
								}
								else if (j == 1) {
									new_stg->setStgInstrumentIdB(instrument);
								}
							}
						}

						new_stg->setStgPositionABuyYesterday(object["position_a_buy_yesterday"].GetInt());
						new_stg->setStgUserId(object["user_id"].GetString());
						new_stg->setStgPositionABuyToday(object["position_a_buy_today"].GetInt());
						new_stg->setStgPositionASellYesterday(object["position_a_sell_yesterday"].GetInt());
						new_stg->setStgLots(object["lots"].GetInt());
						new_stg->setStgAWaitPriceTick(object["a_wait_price_tick"].GetDouble());
						new_stg->setStgBWaitPriceTick(object["b_wait_price_tick"].GetDouble());
						new_stg->setOn_Off(object["StrategyOnoff"].GetInt());

						static_dbm->CreateStrategy(new_stg);
						std::cout << "Strategy新建完成!" << std::endl;
						ctp_m->getListStrategy()->push_back(new_stg);

						/*构造内容json*/
						rapidjson::Value create_info_object(rapidjson::kObjectType);
						create_info_object.SetObject();
						create_info_object.AddMember("position_a_sell_today", new_stg->getStgPositionASellToday(), allocator);
						create_info_object.AddMember("position_b_sell", new_stg->getStgPositionBSell(), allocator);
						create_info_object.AddMember("spread_shift", new_stg->getStgSpreadShift(), allocator);
						create_info_object.AddMember("position_b_sell_today", new_stg->getStgPositionBSellToday(), allocator);
						create_info_object.AddMember("position_b_buy_today", new_stg->getStgPositionBBuyToday(), allocator);
						create_info_object.AddMember("position_a_sell", new_stg->getStgPositionASell(), allocator);
						create_info_object.AddMember("buy_close", new_stg->getStgBuyClose(), allocator);
						create_info_object.AddMember("stop_loss", new_stg->getStgStopLoss(), allocator);
						create_info_object.AddMember("position_b_buy_yesterday", new_stg->getStgPositionBBuyYesterday(), allocator);
						//create_info_object.AddMember("is_active", new_stg->isStgIsActive(), allocator);
						create_info_object.AddMember("position_b_sell_yesterday", new_stg->getStgPositionBSellYesterday(), allocator);
						create_info_object.AddMember("strategy_id", rapidjson::StringRef(new_stg->getStgStrategyId().c_str()), allocator);
						create_info_object.AddMember("position_b_buy", new_stg->getStgPositionBBuy(), allocator);
						create_info_object.AddMember("lots_batch", new_stg->getStgLotsBatch(), allocator);
						create_info_object.AddMember("position_a_buy", new_stg->getStgPositionABuy(), allocator);
						create_info_object.AddMember("sell_open", new_stg->getStgSellOpen(), allocator);
						create_info_object.AddMember("order_algorithm", rapidjson::StringRef(new_stg->getStgOrderAlgorithm().c_str()), allocator);
						create_info_object.AddMember("trader_id", rapidjson::StringRef(new_stg->getStgTraderId().c_str()), allocator);
						create_info_object.AddMember("order_action_tires_limit", new_stg->getStgOrderActionTiresLimit(), allocator);
						create_info_object.AddMember("sell_close", new_stg->getStgSellClose(), allocator);
						create_info_object.AddMember("buy_open", new_stg->getStgBuyOpen(), allocator);
						//info_object.AddMember("only_close", new_stg->isStgOnlyClose(), allocator);

						rapidjson::Value instrument_array(rapidjson::kArrayType);
						for (int j = 0; j < 2; j++) {
							rapidjson::Value instrument_object(rapidjson::kObjectType);
							instrument_object.SetObject();
							if (j == 0) {
								instrument_object.SetString(rapidjson::StringRef(new_stg->getStgInstrumentIdA().c_str()));
							}
							else if (j == 1) {
								instrument_object.SetString(rapidjson::StringRef(new_stg->getStgInstrumentIdB().c_str()));
							}

							instrument_array.PushBack(instrument_object, allocator);
						}
						create_info_object.AddMember("list_instrument_id", instrument_array, allocator);
						create_info_object.AddMember("position_a_buy_yesterday", new_stg->getStgPositionABuyYesterday(), allocator);
						create_info_object.AddMember("user_id", rapidjson::StringRef(new_stg->getStgUserId().c_str()), allocator);
						create_info_object.AddMember("position_a_buy_today", new_stg->getStgPositionABuyToday(), allocator);
						create_info_object.AddMember("position_a_sell_yesterday", new_stg->getStgPositionASellYesterday(), allocator);
						create_info_object.AddMember("lots", new_stg->getStgLots(), allocator);
						create_info_object.AddMember("a_wait_price_tick", new_stg->getStgAWaitPriceTick(), allocator);
						create_info_object.AddMember("b_wait_price_tick", new_stg->getStgBWaitPriceTick(), allocator);
						create_info_object.AddMember("StrategyOnoff", new_stg->getOn_Off(), allocator);

						create_info_array.PushBack(create_info_object, allocator);
					}
				}
				else {
					std::cout << "未收到新建策略信息" << std::endl;
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 7) { // 删除Strategy（修改Strategy参数）
				std::cout << "删除策略..." << std::endl;

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

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 5, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("MsgResult", 0, allocator);
				build_doc.AddMember("MsgErrorReason", "", allocator);
				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				/*1:进行策略的修改,更新到数据库*/
				if (infoArray.IsArray()) {
					std::cout << "info is array" << std::endl;
					for (int i = 0; i < infoArray.Size(); i++) {
						const Value& object = infoArray[i];
						std::string q_user_id = object["user_id"].GetString();
						std::string q_strategy_id = object["strategy_id"].GetString();
						std::cout << "q_user_id = " << q_user_id << std::endl;
						std::cout << "q_strategy_id = " << q_strategy_id << std::endl;
						std::cout << "ctp_m->getListStrategy() size() = " << ctp_m->getListStrategy()->size() << std::endl;
						list<Strategy *>::iterator stg_itor;
						for (stg_itor = ctp_m->getListStrategy()->begin(); stg_itor != ctp_m->getListStrategy()->end();) {
							std::cout << "(*stg_itor)->getStgUserId() = " << (*stg_itor)->getStgUserId() << std::endl;
							std::cout << "(*stg_itor)->getStgStrategyId() = " << (*stg_itor)->getStgStrategyId() << std::endl;
							if (((*stg_itor)->getStgUserId() == q_user_id) && ((*stg_itor)->getStgStrategyId() == q_strategy_id)) {
								std::cout << "找到即将删除的Strategy" << std::endl;
								/*构造内容json*/
								rapidjson::Value create_info_object(rapidjson::kObjectType);
								create_info_object.SetObject();
								create_info_object.AddMember("position_a_sell_today", (*stg_itor)->getStgPositionASellToday(), allocator);
								create_info_object.AddMember("position_b_sell", (*stg_itor)->getStgPositionBSell(), allocator);
								create_info_object.AddMember("spread_shift", (*stg_itor)->getStgSpreadShift(), allocator);
								create_info_object.AddMember("position_b_sell_today", (*stg_itor)->getStgPositionBSellToday(), allocator);
								create_info_object.AddMember("position_b_buy_today", (*stg_itor)->getStgPositionBBuyToday(), allocator);
								create_info_object.AddMember("position_a_sell", (*stg_itor)->getStgPositionASell(), allocator);
								create_info_object.AddMember("buy_close", (*stg_itor)->getStgBuyClose(), allocator);
								create_info_object.AddMember("stop_loss", (*stg_itor)->getStgStopLoss(), allocator);
								create_info_object.AddMember("position_b_buy_yesterday", (*stg_itor)->getStgPositionBBuyYesterday(), allocator);
								//create_info_object.AddMember("is_active", (*stg_itor)->isStgIsActive(), allocator);
								create_info_object.AddMember("position_b_sell_yesterday", (*stg_itor)->getStgPositionBSellYesterday(), allocator);
								create_info_object.AddMember("strategy_id", rapidjson::StringRef((*stg_itor)->getStgStrategyId().c_str()), allocator);
								create_info_object.AddMember("position_b_buy", (*stg_itor)->getStgPositionBBuy(), allocator);
								create_info_object.AddMember("lots_batch", (*stg_itor)->getStgLotsBatch(), allocator);
								create_info_object.AddMember("position_a_buy", (*stg_itor)->getStgPositionABuy(), allocator);
								create_info_object.AddMember("sell_open", (*stg_itor)->getStgSellOpen(), allocator);
								create_info_object.AddMember("order_algorithm", rapidjson::StringRef((*stg_itor)->getStgOrderAlgorithm().c_str()), allocator);
								create_info_object.AddMember("trader_id", rapidjson::StringRef((*stg_itor)->getStgTraderId().c_str()), allocator);
								create_info_object.AddMember("order_action_tires_limit", (*stg_itor)->getStgOrderActionTiresLimit(), allocator);
								create_info_object.AddMember("sell_close", (*stg_itor)->getStgSellClose(), allocator);
								create_info_object.AddMember("buy_open", (*stg_itor)->getStgBuyOpen(), allocator);
								//info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator);

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
								create_info_object.AddMember("list_instrument_id", instrument_array, allocator);
								create_info_object.AddMember("position_a_buy_yesterday", (*stg_itor)->getStgPositionABuyYesterday(), allocator);
								create_info_object.AddMember("user_id", rapidjson::StringRef((*stg_itor)->getStgUserId().c_str()), allocator);
								create_info_object.AddMember("position_a_buy_today", (*stg_itor)->getStgPositionABuyToday(), allocator);
								create_info_object.AddMember("position_a_sell_yesterday", (*stg_itor)->getStgPositionASellYesterday(), allocator);
								create_info_object.AddMember("lots", (*stg_itor)->getStgLots(), allocator);
								create_info_object.AddMember("a_wait_price_tick", (*stg_itor)->getStgAWaitPriceTick(), allocator);
								create_info_object.AddMember("b_wait_price_tick", (*stg_itor)->getStgBWaitPriceTick(), allocator);
								create_info_object.AddMember("StrategyOnoff", (*stg_itor)->getOn_Off(), allocator);

								create_info_array.PushBack(create_info_object, allocator);

								static_dbm->DeleteStrategy((*stg_itor));

								delete (*stg_itor);
								stg_itor = ctp_m->getListStrategy()->erase(stg_itor);

								std::cout << "Strategy删除完成!" << std::endl;

							}
							else {
								std::cout << "未能找到修改的Strategy" << std::endl;
								stg_itor++;
							}
						}
					}
				}
				else {
					std::cout << "未收到修改策略信息" << std::endl;
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 8) { // 交易员开关
				std::cout << "交易员开关..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &OnOff = doc["OnOff"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();
				int i_OnOff = OnOff.GetInt();

				bool bFind = false;

				std::cout << "收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "收到交易员账户开关 = " << i_OnOff << std::endl;

				/*构建交易员开关的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 8, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

				list<Trader *>::iterator trader_itor;
				for (trader_itor = ctp_m->getL_Obj_Trader()->begin(); trader_itor != ctp_m->getL_Obj_Trader()->end(); trader_itor++) {
					if ((*trader_itor)->getTraderID() == s_TraderID) {
						std::cout << "找到需要开关的交易员ID" << std::endl;
						(*trader_itor)->setOn_Off(i_OnOff);
						bFind = true;
					}
				}

				if (bFind) {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
					build_doc.AddMember("OnOff", i_OnOff, allocator);
				}
				else {
					build_doc.AddMember("MsgResult", 1, allocator);
					build_doc.AddMember("MsgErrorReason", "未找到该交易员!", allocator);
					build_doc.AddMember("OnOff", 0, allocator);
				}

			}
			else if (msgtype == 9) { // 期货账户开关
				std::cout << "期货账户开关..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];
				rapidjson::Value &OnOff = doc["OnOff"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();
				string s_UserID = UserID.GetString();
				int i_OnOff = OnOff.GetInt();

				bool bFind = false;

				std::cout << "收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "收到期货账户ID = " << s_UserID << std::endl;
				std::cout << "收到期货账户开关 = " << i_OnOff << std::endl;

				/*构建期货账户开关的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 9, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

				list<User *>::iterator user_itor;
				for (user_itor = ctp_m->getL_User()->begin(); user_itor != ctp_m->getL_User()->end(); user_itor++) {
					if ((*user_itor)->getUserID() == s_UserID) {
						std::cout << "找到需要开关的期货账户ID" << std::endl;
						(*user_itor)->setOn_Off(i_OnOff);
						bFind = true;
					}
				}

				if (bFind) {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
					build_doc.AddMember("OnOff", i_OnOff, allocator);
				}
				else {
					build_doc.AddMember("MsgResult", 1, allocator);
					build_doc.AddMember("MsgErrorReason", "未找到该期货账户!", allocator);
					build_doc.AddMember("OnOff", 0, allocator);
				}

			}
			else if (msgtype == 10) { // 查询策略昨仓
				std::cout << "查询策略昨仓..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];
				rapidjson::Value &StrategyID = doc["StrategyID"];

				string s_TraderID = TraderID.GetString();
				string s_UserID = UserID.GetString();
				string s_StrategyID = StrategyID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				bool bFind = false;

				std::cout << "收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "收到期货账户ID = " << s_UserID << std::endl;

				/*构建策略昨仓Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 10, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);
				build_doc.AddMember("StrategyID", rapidjson::StringRef(s_StrategyID.c_str()), allocator);

				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				list<Strategy *>::iterator stg_itor;
				for (stg_itor = ctp_m->getListStrategyYesterday()->begin(); stg_itor != ctp_m->getListStrategyYesterday()->end(); stg_itor++) {
					if (((*stg_itor)->getStgUserId() == s_UserID) && ((*stg_itor)->getStgStrategyId() == s_StrategyID) && ((*stg_itor)->getStgTraderId() == s_TraderID)) {
						std::cout << "找到需要查询的昨仓" << std::endl;

						/*构造内容json*/
						rapidjson::Value create_info_object(rapidjson::kObjectType);
						create_info_object.SetObject();
						create_info_object.AddMember("position_a_sell_today", (*stg_itor)->getStgPositionASellToday(), allocator);
						create_info_object.AddMember("position_b_sell", (*stg_itor)->getStgPositionBSell(), allocator);
						create_info_object.AddMember("spread_shift", (*stg_itor)->getStgSpreadShift(), allocator);
						create_info_object.AddMember("position_b_sell_today", (*stg_itor)->getStgPositionBSellToday(), allocator);
						create_info_object.AddMember("position_b_buy_today", (*stg_itor)->getStgPositionBBuyToday(), allocator);
						create_info_object.AddMember("position_a_sell", (*stg_itor)->getStgPositionASell(), allocator);
						create_info_object.AddMember("buy_close", (*stg_itor)->getStgBuyClose(), allocator);
						create_info_object.AddMember("stop_loss", (*stg_itor)->getStgStopLoss(), allocator);
						create_info_object.AddMember("position_b_buy_yesterday", (*stg_itor)->getStgPositionBBuyYesterday(), allocator);
						//create_info_object.AddMember("is_active", (*stg_itor)->isStgIsActive(), allocator);
						create_info_object.AddMember("position_b_sell_yesterday", (*stg_itor)->getStgPositionBSellYesterday(), allocator);
						create_info_object.AddMember("strategy_id", rapidjson::StringRef((*stg_itor)->getStgStrategyId().c_str()), allocator);
						create_info_object.AddMember("position_b_buy", (*stg_itor)->getStgPositionBBuy(), allocator);
						create_info_object.AddMember("lots_batch", (*stg_itor)->getStgLotsBatch(), allocator);
						create_info_object.AddMember("position_a_buy", (*stg_itor)->getStgPositionABuy(), allocator);
						create_info_object.AddMember("sell_open", (*stg_itor)->getStgSellOpen(), allocator);
						create_info_object.AddMember("order_algorithm", rapidjson::StringRef((*stg_itor)->getStgOrderAlgorithm().c_str()), allocator);
						create_info_object.AddMember("trader_id", rapidjson::StringRef((*stg_itor)->getStgTraderId().c_str()), allocator);
						create_info_object.AddMember("order_action_tires_limit", (*stg_itor)->getStgOrderActionTiresLimit(), allocator);
						create_info_object.AddMember("sell_close", (*stg_itor)->getStgSellClose(), allocator);
						create_info_object.AddMember("buy_open", (*stg_itor)->getStgBuyOpen(), allocator);
						//info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator);

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
						create_info_object.AddMember("list_instrument_id", instrument_array, allocator);
						create_info_object.AddMember("position_a_buy_yesterday", (*stg_itor)->getStgPositionABuyYesterday(), allocator);
						create_info_object.AddMember("user_id", rapidjson::StringRef((*stg_itor)->getStgUserId().c_str()), allocator);
						create_info_object.AddMember("position_a_buy_today", (*stg_itor)->getStgPositionABuyToday(), allocator);
						create_info_object.AddMember("position_a_sell_yesterday", (*stg_itor)->getStgPositionASellYesterday(), allocator);
						create_info_object.AddMember("lots", (*stg_itor)->getStgLots(), allocator);
						create_info_object.AddMember("a_wait_price_tick", (*stg_itor)->getStgAWaitPriceTick(), allocator);
						create_info_object.AddMember("b_wait_price_tick", (*stg_itor)->getStgBWaitPriceTick(), allocator);
						create_info_object.AddMember("StrategyOnoff", (*stg_itor)->getOn_Off(), allocator);

						create_info_array.PushBack(create_info_object, allocator);

						bFind = true;
					}
				}

				if (bFind) {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
				}
				else {
					build_doc.AddMember("MsgResult", 1, allocator);
					build_doc.AddMember("MsgErrorReason", "未找到该策略昨仓信息", allocator);
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

			}
			else {
				std::cout << "请求类型错误!" << endl;
				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 99, allocator);
				build_doc.AddMember("MsgResult", 1, allocator);
				build_doc.AddMember("MsgErrorReason", "请求类型不存在", allocator);
			}
		}
	}
	
	

	build_doc.Accept(writer);
	//rsp_msg = const_cast<char *>(buffer.GetString());
	//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
	std::cout << "服务端响应数据 = " << buffer.GetString() << std::endl;

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

/// 初始化
void CTP_Manager::init() {
	/// 数据库查询所有的Trader
	this->dbm->getAllTrader(this->l_trader);

	this->dbm->getAllObjTrader(this->l_obj_trader);

	/// 查询所有的期货账户
	this->dbm->getAllFutureAccount(this->l_user);

	/// 查询所有的策略
	this->dbm->getAllStrategy(this->l_strategys);

	/// 查询昨仓持仓
	this->dbm->getAllStrategyYesterday(this->l_strategys_yesterday);
	

	/// 绑定操作,策略绑定到对应期货账户下
	list<User *>::iterator user_itor;
	list<Strategy *>::iterator stg_itor;
	USER_PRINT("Iterator USERS");
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		
		USER_PRINT((*user_itor)->getUserID());
		
		(*user_itor)->setCTP_Manager(this); //每个user对象设置CTP_Manager对象

		for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) { // 遍历Strategy
			USER_PRINT((*stg_itor)->getStgUserId());
			if ((*stg_itor)->getStgUserId() == (*user_itor)->getUserID()) {
				USER_PRINT("Strategy Bind To User");
				(*user_itor)->addStrategyToList((*stg_itor)); // 将策略添加到期货账户策略列表里
				(*stg_itor)->setStgUser((*user_itor)); // 策略设置自己的期货账户对象
				USER_PRINT("(*stg_itor)->setStgUser((*user_itor))");
				USER_PRINT((*stg_itor)->getStgUser());
			}
		}
		/// 遍历设值后进行TD初始化
		this->CreateAccount((*user_itor), this->l_strategys);
		std::cout << "账户 : " << (*user_itor)->getUserID() << " 初始化完成!" << std::endl;
		sleep(3);
	}


	/// 合约查询后对策略合约最小跳进行赋值
	this->l_user->front()->getUserTradeSPI()->QryInstrument();
	sleep(3);

	list<CThostFtdcInstrumentField *> *l_instruments_info = this->l_user->front()->getUserTradeSPI()->getL_Instruments_Info();

	if (l_instruments_info != NULL) {
		for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
			(*user_itor)->getUserTradeSPI()->setL_Instruments_Info(l_instruments_info);
			(*user_itor)->setStgInstrumnetPriceTick(); // 对策略合约价格最小跳赋值
		}
	}
	else {
		std::cout << "策略最小跳价格获取失败!!!" << std::endl;
	}

	/// 行情初始化
	MarketConfig *mc = this->dbm->getOneMarketConfig();
	if (mc != NULL) {
		this->mdspi = this->CreateMd(mc->getMarketFrontAddr(), mc->getBrokerID(), mc->getUserID(), mc->getPassword(), this->l_strategys);
		if (this->mdspi != NULL) {
			/// 向mdspi赋值strategys
			//this->mdspi->setListStrategy(this->l_strategys);
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
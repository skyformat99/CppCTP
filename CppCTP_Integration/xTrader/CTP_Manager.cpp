#include <errno.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "CTP_Manager.h"
#include "User.h"
#include "Algorithm.h"
#include "Utils.h"
#include "Debug.h"
#include "msg.h"

#define MSG_SEND_FLAG 1
using namespace rapidjson;

static DBManager *static_dbm = new DBManager();
static Trader *op = new Trader();
static const int MAX_THREAD_COUNT = 100;

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

	this->l_posdetail_trade = new list<USER_CThostFtdcTradeField *>();
	this->l_posdetail_trade_yesterday = new list<USER_CThostFtdcTradeField *>();

	this->l_posdetail = new list<USER_CThostFtdcOrderField *>();
	this->l_posdetail_yesterday = new list<USER_CThostFtdcOrderField *>();

	this->l_instrument = new list<string>();
	this->l_unsubinstrument = new list<string>();
	this->l_sessions = new list<Session *>();
	this->isClosingSaved = false;
	this->system_init_flag = false;
	this->trading_day = "";
	this->isMdLogin = false;
	this->isCTPFinishedPositionInit = false;
	this->is_market_close = true;
	this->is_start_end_task = false;
	this->is_market_close_done = false;
	//this->ten_min_flag = false;
	//this->one_min_flag = false;
	//this->one_second_flag = false;
}

bool CTP_Manager::ten_min_flag = false;
bool CTP_Manager::one_min_flag = false;
bool CTP_Manager::one_second_flag = false;

//bool CTP_Manager::CheckIn(Login *login) {
//	
//}

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
		tdspi->setCtpManager(this);			 // 设置CTP_Manager

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
			USER_PRINT("TdApi初始化完成...");
			USER_PRINT(tdapi);
			if (!tdapi) {
				return NULL;
			}
			else {
				user->setUserTradeAPI(tdapi);
				user->setUserTradeSPI(tdspi);
			}
		}

		sleep(1);
		std::cout << "CTP_Manager::CreateAccount()" << std::endl;
		std::cout << "\t期货账户 = " << user->getUserID() << endl;
		std::cout << "\tBrokerID = " << user->getBrokerID() << endl;
		std::cout << "\t期货密码 = " << user->getPassword() << endl;
		user->getUserTradeSPI()->Connect(user, this->system_init_flag); // 连接
		sleep(1);
		user->getUserTradeSPI()->Login(user); // 登陆
		sleep(1);
		user->getUserTradeSPI()->QrySettlementInfoConfirm(user); // 确认交易结算
		/// 设置初始化状态完成
		user->setThread_Init_Status(true);

		std::cout << "In Thread = " << std::this_thread::get_id() << ", UserID = " << user->getUserID() << std::endl;

		/// 等待结束
		user->getUserTradeAPI()->Join();
	}

	/*while (true)
	{
		std::cout << "In Thread = " << std::this_thread::get_id() << ", UserID = " << user->getUserID() << std::endl;
		sleep(5);
	}*/
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
		mdspi->setCtpManager(this);
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

/// 得到l_unsubinstrument
list<string> * CTP_Manager::getL_UnsubInstrument() {
	return this->l_unsubinstrument;
}

/// 得到数据库操作对象
DBManager * CTP_Manager::getDBManager() {
	return this->dbm;
}

/// 设置l_trader
void CTP_Manager::addTraderToLTrader(string trader) {
	this->l_trader->push_back(trader);
}

/// 更新系统交易状态
void CTP_Manager::updateSystemFlag() {
	string status = "off";
	this->dbm->UpdateSystemRunningStatus(status);
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

bool CTP_Manager::getTenMinFlag() {
	return this->ten_min_flag;
}

void CTP_Manager::setTenMinFlag(bool ten_min_flag) {
	this->ten_min_flag = ten_min_flag;
}

bool CTP_Manager::getOneMinFlag() {
	return this->one_min_flag;
}

void CTP_Manager::setOneMinFlag(bool one_min_flag) {
	this->one_min_flag = one_min_flag;
}

bool CTP_Manager::getOneSecondFlag() {
	return this->one_second_flag;
}

void CTP_Manager::setOneSecondFlag(bool one_second_flag) {
	this->one_second_flag = one_second_flag;
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

/// 设置定时器
void CTP_Manager::setCalTimer(Timer *cal_timer) {
	this->cal_timer = cal_timer;
}

/// 获取定时器
Timer * CTP_Manager::getCalTimer() {
	return this->cal_timer;
}

/// 收盘时保存strategy_list
void CTP_Manager::saveStrategy() {
	/************************************************************************/
	/* 1:盘后保存工作
		a:将持仓明细表全部清空(把内存里最新的持仓明细存储进去)
		a:保存策略参数
		b:保存持仓明细*/
	/************************************************************************/
	USER_PRINT("CTP_Manager::saveStrategy()");
	if (!this->isClosingSaved) {

		// 删除持仓明细所有数据
		this->dbm->DropPositionDetail();

		list<Strategy *>::iterator stg_itor;
		list<USER_CThostFtdcOrderField *>::iterator posd_itor;
		list<USER_CThostFtdcTradeField *>::iterator posd_itor_trade;
		USER_PRINT("CTP_Manager::saveStrategy");
		std::cout << "CTP_Manager::saveStrategy()" << std::endl;
		for (stg_itor = this->l_strategys->begin(); 
			stg_itor != this->l_strategys->end(); stg_itor++) { // 遍历Strategy
			
			
			std::cout << "\t保存期货账户 = " << (*stg_itor)->getStgUserId() << std::endl;
			std::cout << "\t保存策略ID = " << (*stg_itor)->getStgStrategyId() << std::endl;

			//this->dbm->UpdateStrategy((*stg_itor));
			(*stg_itor)->UpdateStrategy((*stg_itor));

			// 遍历strategy持仓明细并保存(order)
			for (posd_itor = (*stg_itor)->getStg_List_Position_Detail_From_Order()->begin();
				posd_itor != (*stg_itor)->getStg_List_Position_Detail_From_Order()->end();
				posd_itor++) {

				//this->dbm->CreatePositionDetail((*posd_itor));
				std::cout << "\t================持仓明细输出BEGIN===================" << std::endl;
				std::cout << "\tinstrumentid = " << (*posd_itor)->InstrumentID << std::endl;
				std::cout << "\torderref = " << (*posd_itor)->OrderRef << std::endl;
				std::cout << "\tuserid = " << (*posd_itor)->UserID << std::endl;
				std::cout << "\tdirection = " << (*posd_itor)->Direction << std::endl;
				std::cout << "\tcomboffsetflag = " << (*posd_itor)->CombOffsetFlag << std::endl;
				std::cout << "\tcombhedgeflag = " << (*posd_itor)->CombHedgeFlag << std::endl;
				std::cout << "\tlimitprice = " << (*posd_itor)->LimitPrice << std::endl;
				std::cout << "\tvolumetotaloriginal = " << (*posd_itor)->VolumeTotalOriginal << std::endl;
				std::cout << "\ttradingday = " << (*posd_itor)->TradingDay << std::endl;
				std::cout << "\ttradingdayrecord = " << (*posd_itor)->TradingDayRecord << std::endl;
				std::cout << "\torderstatus = " << (*posd_itor)->OrderStatus << std::endl;
				std::cout << "\tvolumetraded = " << (*posd_itor)->VolumeTraded << std::endl;
				std::cout << "\tvolumetotal = " << (*posd_itor)->VolumeTotal << std::endl;
				std::cout << "\tinsertdate = " << (*posd_itor)->InsertDate << std::endl;
				std::cout << "\tinserttime = " << (*posd_itor)->InsertTime << std::endl;
				std::cout << "\tstrategyid = " << (*posd_itor)->StrategyID << std::endl;
				std::cout << "\tvolumetradedbatch = " << (*posd_itor)->VolumeTradedBatch << std::endl;
				std::cout << "\t================持仓明细输出 END ===================" << std::endl;

				(*stg_itor)->CreatePositionDetail((*posd_itor));
			}

			// 遍历strategy持仓明细并保存(trade)
			for (posd_itor_trade = (*stg_itor)->getStg_List_Position_Detail_From_Trade()->begin();
				posd_itor_trade != (*stg_itor)->getStg_List_Position_Detail_From_Trade()->end();
				posd_itor_trade++) {

				//this->dbm->CreatePositionDetail((*posd_itor));
				std::cout << "\t================持仓明细输出BEGIN===================" << std::endl;
				std::cout << "\tinstrumentid = " << (*posd_itor_trade)->InstrumentID << std::endl;
				std::cout << "\torderref = " << (*posd_itor_trade)->OrderRef << std::endl;
				std::cout << "\tuserid = " << (*posd_itor_trade)->UserID << std::endl;
				std::cout << "\tdirection = " << (*posd_itor_trade)->Direction << std::endl;
				std::cout << "\toffsetflag = " << (*posd_itor_trade)->OffsetFlag << std::endl;
				std::cout << "\thedgeflag = " << (*posd_itor_trade)->HedgeFlag << std::endl;
				std::cout << "\tprice = " << (*posd_itor_trade)->Price << std::endl;
				std::cout << "\ttradingday = " << (*posd_itor_trade)->TradingDay << std::endl;
				std::cout << "\ttradingdayrecord = " << (*posd_itor_trade)->TradingDayRecord << std::endl;
				std::cout << "\ttradedate = " << (*posd_itor_trade)->TradeDate << std::endl;
				std::cout << "\tstrategyid = " << (*posd_itor_trade)->StrategyID << std::endl;
				std::cout << "\tvolume = " << (*posd_itor_trade)->Volume << std::endl;
				std::cout << "\t================持仓明细输出 END ===================" << std::endl;

				(*stg_itor)->CreatePositionTradeDetail((*posd_itor_trade));
			}
		}
		this->isClosingSaved = true;
	}
}

/// 保存所有策略持仓明细
void CTP_Manager::saveAllStrategyPositionDetail() {

	list<Strategy *>::iterator stg_itor;
	list<USER_CThostFtdcOrderField *>::iterator posd_itor;
	list<USER_CThostFtdcTradeField *>::iterator posd_itor_trade;
	USER_PRINT("CTP_Manager::saveAllStrategyPositionDetail()");
	std::cout << "CTP_Manager::saveAllStrategyPositionDetail()" << std::endl;
	for (stg_itor = this->l_strategys->begin();
		stg_itor != this->l_strategys->end(); stg_itor++) { // 遍历Strategy

		//this->dbm->UpdateStrategy((*stg_itor));
		//(*stg_itor)->setStgUpdatePositionDetailRecordTime(Utils::getDate());
		(*stg_itor)->setStgLastSavedTime(Utils::getDate());
		(*stg_itor)->UpdateStrategy((*stg_itor));
		std::cout << "\t关闭服务端,保存期货账户 = " << (*stg_itor)->getStgUserId() << std::endl;
		std::cout << "\t关闭服务端,保存策略ID = " << (*stg_itor)->getStgStrategyId() << std::endl;
		std::cout << "\t关闭服务端,Order明细长度 = " << (*stg_itor)->getStg_List_Position_Detail_From_Order()->size() << std::endl;
		std::cout << "\t关闭服务端,Trade明细长度 = " << (*stg_itor)->getStg_List_Position_Detail_From_Trade()->size() << std::endl;

		// 遍历strategy持仓明细并保存
		for (posd_itor = (*stg_itor)->getStg_List_Position_Detail_From_Order()->begin();
			posd_itor != (*stg_itor)->getStg_List_Position_Detail_From_Order()->end();
			posd_itor++) {

			//this->dbm->CreatePositionDetail((*posd_itor));
			/*std::cout << "\t\tinstrumentid = " << (*posd_itor)->InstrumentID << std::endl;
			std::cout << "\t\torderref = " << (*posd_itor)->OrderRef << std::endl;
			std::cout << "\t\tuserid = " << (*posd_itor)->UserID << std::endl;
			std::cout << "\t\tdirection = " << (*posd_itor)->Direction << std::endl;*/
			/*std::cout << "\tcomboffsetflag = " << string(1, (*posd_itor)->CombOffsetFlag[0]) << std::endl;
			std::cout << "\tcombhedgeflag = " << string(1, (*posd_itor)->CombHedgeFlag[0]) << std::endl;*/

			/*std::cout << "\t\tcomboffsetflag = " << (*posd_itor)->CombOffsetFlag << std::endl;
			std::cout << "\t\tcombhedgeflag = " <<  (*posd_itor)->CombHedgeFlag << std::endl;
			std::cout << "\t\tlimitprice = " << (*posd_itor)->LimitPrice << std::endl;
			std::cout << "\t\tvolumetotaloriginal = " << (*posd_itor)->VolumeTotalOriginal << std::endl;
			std::cout << "\t\ttradingday = " << (*posd_itor)->TradingDay << std::endl;
			std::cout << "\t\ttradingdayrecord = " << (*posd_itor)->TradingDayRecord << std::endl;
			std::cout << "\t\torderstatus = " << (*posd_itor)->OrderStatus << std::endl;
			std::cout << "\t\tvolumetraded = " << (*posd_itor)->VolumeTraded << std::endl;
			std::cout << "\t\tvolumetotal = " << (*posd_itor)->VolumeTotal << std::endl;
			std::cout << "\t\tinsertdate = " << (*posd_itor)->InsertDate << std::endl;
			std::cout << "\t\tinserttime = " << (*posd_itor)->InsertTime << std::endl;
			std::cout << "\t\tstrategyid = " << (*posd_itor)->StrategyID << std::endl;
			std::cout << "\t\tvolumetradedbatch = " << (*posd_itor)->VolumeTradedBatch << std::endl;*/

			(*stg_itor)->Update_Position_Detail_To_DB((*posd_itor));
		}

		// 遍历strategy持仓明细并保存
		for (posd_itor_trade = (*stg_itor)->getStg_List_Position_Detail_From_Trade()->begin();
			posd_itor_trade != (*stg_itor)->getStg_List_Position_Detail_From_Trade()->end();
			posd_itor_trade++) {

			//this->dbm->CreatePositionDetail((*posd_itor));
			/*std::cout << "\t\tinstrumentid = " << (*posd_itor_trade)->InstrumentID << std::endl;
			std::cout << "\t\torderref = " << (*posd_itor_trade)->OrderRef << std::endl;
			std::cout << "\t\tuserid = " << (*posd_itor_trade)->UserID << std::endl;
			std::cout << "\t\tdirection = " << (*posd_itor_trade)->Direction << std::endl;*/
			/*std::cout << "\tcomboffsetflag = " << string(1, (*posd_itor)->CombOffsetFlag[0]) << std::endl;
			std::cout << "\tcombhedgeflag = " << string(1, (*posd_itor)->CombHedgeFlag[0]) << std::endl;*/

			/*std::cout << "\t\toffsetflag = " << (*posd_itor_trade)->OffsetFlag << std::endl;
			std::cout << "\t\thedgeflag = " << (*posd_itor_trade)->HedgeFlag << std::endl;
			std::cout << "\t\tprice = " << (*posd_itor_trade)->Price << std::endl;
			std::cout << "\t\ttradingday = " << (*posd_itor_trade)->TradingDay << std::endl;
			std::cout << "\t\ttradingdayrecord = " << (*posd_itor_trade)->TradingDayRecord << std::endl;
			std::cout << "\t\ttradingdate = " << (*posd_itor_trade)->TradeDate << std::endl;
			std::cout << "\t\tstrategyid = " << (*posd_itor_trade)->StrategyID << std::endl;
			std::cout << "\t\tvolume = " << (*posd_itor_trade)->Volume << std::endl;*/

			(*stg_itor)->Update_Position_Trade_Detail_To_DB((*posd_itor_trade));
		}
	}
}

/// 保存一个策略持仓明细
void CTP_Manager::saveStrategyPositionDetail(Strategy *stg) {
	list<USER_CThostFtdcOrderField *>::iterator posd_itor;
	list<USER_CThostFtdcTradeField *>::iterator posd_itor_trade;
	USER_PRINT("CTP_Manager::saveStrategyPositionDetail()");
	std::cout << "CTP_Manager::saveStrategyPositionDetail()" << std::endl;
	//this->dbm->UpdateStrategy((*stg_itor));
	stg->setStgUpdatePositionDetailRecordTime(Utils::getDate());
	//stg->setStgLastSavedTime(Utils::getDate());
	stg->UpdateStrategy(stg);
	std::cout << "\t保存期货账户 = " << stg->getStgUserId() << std::endl;
	std::cout << "\t保存策略ID = " << stg->getStgStrategyId() << std::endl;
	std::cout << "\t保存策略order持仓明细长度 = " << stg->getStg_List_Position_Detail_From_Order()->size() << std::endl;
	std::cout << "\t保存策略trade持仓明细长度 = " << stg->getStg_List_Position_Detail_From_Trade()->size() << std::endl;

	// 遍历strategy持仓明细并保存
	for (posd_itor = stg->getStg_List_Position_Detail_From_Order()->begin();
		posd_itor != stg->getStg_List_Position_Detail_From_Order()->end();
		posd_itor++) {

		//this->dbm->CreatePositionDetail((*posd_itor));
		//std::cout << "\t\tinstrumentid = " << (*posd_itor)->InstrumentID << std::endl;
		//std::cout << "\t\torderref = " << (*posd_itor)->OrderRef << std::endl;
		//std::cout << "\t\tuserid = " << (*posd_itor)->UserID << std::endl;
		//std::cout << "\t\tdirection = " << (*posd_itor)->Direction << std::endl;
		///*std::cout << "\tcomboffsetflag = " << string(1, (*posd_itor)->CombOffsetFlag[0]) << std::endl;
		//std::cout << "\tcombhedgeflag = " << string(1, (*posd_itor)->CombHedgeFlag[0]) << std::endl;*/

		//std::cout << "\t\tcomboffsetflag = " << (*posd_itor)->CombOffsetFlag << std::endl;
		//std::cout << "\t\tcombhedgeflag = " << (*posd_itor)->CombHedgeFlag << std::endl;
		//std::cout << "\t\tlimitprice = " << (*posd_itor)->LimitPrice << std::endl;
		//std::cout << "\t\tvolumetotaloriginal = " << (*posd_itor)->VolumeTotalOriginal << std::endl;
		//std::cout << "\t\ttradingday = " << (*posd_itor)->TradingDay << std::endl;
		//strcpy((*posd_itor)->TradingDayRecord, this->getTradingDay().c_str());
		//std::cout << "\t\ttradingdayrecord = " << (*posd_itor)->TradingDayRecord << std::endl;
		//std::cout << "\t\torderstatus = " << (*posd_itor)->OrderStatus << std::endl;
		//std::cout << "\t\tvolumetraded = " << (*posd_itor)->VolumeTraded << std::endl;
		//std::cout << "\t\tvolumetotal = " << (*posd_itor)->VolumeTotal << std::endl;
		//std::cout << "\t\tinsertdate = " << (*posd_itor)->InsertDate << std::endl;
		//std::cout << "\t\tinserttime = " << (*posd_itor)->InsertTime << std::endl;
		//std::cout << "\t\tstrategyid = " << (*posd_itor)->StrategyID << std::endl;
		//std::cout << "\t\tvolumetradedbatch = " << (*posd_itor)->VolumeTradedBatch << std::endl;

		stg->Update_Position_Detail_To_DB((*posd_itor));
	}

	// 遍历strategy持仓明细并保存
	for (posd_itor_trade = stg->getStg_List_Position_Detail_From_Trade()->begin();
		posd_itor_trade != stg->getStg_List_Position_Detail_From_Trade()->end();
		posd_itor_trade++) {

		//this->dbm->CreatePositionDetail((*posd_itor));
		//std::cout << "\t\tinstrumentid = " << (*posd_itor_trade)->InstrumentID << std::endl;
		//std::cout << "\t\torderref = " << (*posd_itor_trade)->OrderRef << std::endl;
		//std::cout << "\t\tuserid = " << (*posd_itor_trade)->UserID << std::endl;
		//std::cout << "\t\tdirection = " << (*posd_itor_trade)->Direction << std::endl;
		///*std::cout << "\tcomboffsetflag = " << string(1, (*posd_itor)->CombOffsetFlag[0]) << std::endl;
		//std::cout << "\tcombhedgeflag = " << string(1, (*posd_itor)->CombHedgeFlag[0]) << std::endl;*/

		//std::cout << "\t\toffsetflag = " << (*posd_itor_trade)->OffsetFlag << std::endl;
		//std::cout << "\t\thedgeflag = " << (*posd_itor_trade)->HedgeFlag << std::endl;
		//std::cout << "\t\tprice = " << (*posd_itor_trade)->Price << std::endl;
		//std::cout << "\t\ttradingday = " << (*posd_itor_trade)->TradingDay << std::endl;
		//strcpy((*posd_itor_trade)->TradingDayRecord, this->getTradingDay().c_str());
		//std::cout << "\t\ttradingdayrecord = " << (*posd_itor_trade)->TradingDayRecord << std::endl;
		//std::cout << "\t\ttradingdate = " << (*posd_itor_trade)->TradeDate << std::endl;
		//std::cout << "\t\tstrategyid = " << (*posd_itor_trade)->StrategyID << std::endl;
		//std::cout << "\t\tvolume = " << (*posd_itor_trade)->Volume << std::endl;

		stg->Update_Position_Trade_Detail_To_DB((*posd_itor_trade));
	}
}

/// 保存一个策略持仓明细修改过的
void CTP_Manager::saveStrategyChangedPositionDetail(Strategy *stg) {
	list<USER_CThostFtdcOrderField *>::iterator posd_itor;
	list<USER_CThostFtdcTradeField *>::iterator posd_itor_trade;
	USER_PRINT("CTP_Manager::saveStrategyChangedPositionDetail()");
	std::cout << "CTP_Manager::saveStrategyChangedPositionDetail()" << std::endl;
	//this->dbm->UpdateStrategy((*stg_itor));
	stg->setStgUpdatePositionDetailRecordTime(Utils::getDate());
	//stg->setStgLastSavedTime(Utils::getDate());
	stg->UpdateStrategy(stg);
	std::cout << "\t保存 修改的策略 期货账户 = " << stg->getStgUserId() << std::endl;
	std::cout << "\t保存 修改的策略 ID = " << stg->getStgStrategyId() << std::endl;
	std::cout << "\t保存 修改的策略 order持仓明细长度 = " << stg->getStg_List_Position_Detail_From_Order()->size() << std::endl;
	std::cout << "\t保存 修改的策略 trade持仓明细长度 = " << stg->getStg_List_Position_Detail_From_Trade()->size() << std::endl;

	// 遍历strategy持仓明细并保存
	for (posd_itor = stg->getStg_List_Position_Detail_From_Order()->begin();
		posd_itor != stg->getStg_List_Position_Detail_From_Order()->end();
		posd_itor++) {

		//this->dbm->CreatePositionDetail((*posd_itor));
		//std::cout << "\t\tinstrumentid = " << (*posd_itor)->InstrumentID << std::endl;
		//std::cout << "\t\torderref = " << (*posd_itor)->OrderRef << std::endl;
		//std::cout << "\t\tuserid = " << (*posd_itor)->UserID << std::endl;
		//std::cout << "\t\tdirection = " << (*posd_itor)->Direction << std::endl;
		///*std::cout << "\tcomboffsetflag = " << string(1, (*posd_itor)->CombOffsetFlag[0]) << std::endl;
		//std::cout << "\tcombhedgeflag = " << string(1, (*posd_itor)->CombHedgeFlag[0]) << std::endl;*/

		//std::cout << "\t\tcomboffsetflag = " << (*posd_itor)->CombOffsetFlag << std::endl;
		//std::cout << "\t\tcombhedgeflag = " << (*posd_itor)->CombHedgeFlag << std::endl;
		//std::cout << "\t\tlimitprice = " << (*posd_itor)->LimitPrice << std::endl;
		//std::cout << "\t\tvolumetotaloriginal = " << (*posd_itor)->VolumeTotalOriginal << std::endl;
		//std::cout << "\t\ttradingday = " << (*posd_itor)->TradingDay << std::endl;
		//strcpy((*posd_itor)->TradingDayRecord, this->getTradingDay().c_str());
		//std::cout << "\t\ttradingdayrecord = " << (*posd_itor)->TradingDayRecord << std::endl;
		//std::cout << "\t\torderstatus = " << (*posd_itor)->OrderStatus << std::endl;
		//std::cout << "\t\tvolumetraded = " << (*posd_itor)->VolumeTraded << std::endl;
		//std::cout << "\t\tvolumetotal = " << (*posd_itor)->VolumeTotal << std::endl;
		//std::cout << "\t\tinsertdate = " << (*posd_itor)->InsertDate << std::endl;
		//std::cout << "\t\tinserttime = " << (*posd_itor)->InsertTime << std::endl;
		//std::cout << "\t\tstrategyid = " << (*posd_itor)->StrategyID << std::endl;
		//std::cout << "\t\tvolumetradedbatch = " << (*posd_itor)->VolumeTradedBatch << std::endl;

		stg->Update_Position_Changed_Detail_To_DB((*posd_itor));
	}

	// 遍历strategy持仓明细并保存
	for (posd_itor_trade = stg->getStg_List_Position_Detail_From_Trade()->begin();
		posd_itor_trade != stg->getStg_List_Position_Detail_From_Trade()->end();
		posd_itor_trade++) {

		//this->dbm->CreatePositionDetail((*posd_itor));
		std::cout << "\t\tinstrumentid = " << (*posd_itor_trade)->InstrumentID << std::endl;
		std::cout << "\t\torderref = " << (*posd_itor_trade)->OrderRef << std::endl;
		std::cout << "\t\tuserid = " << (*posd_itor_trade)->UserID << std::endl;
		std::cout << "\t\tdirection = " << (*posd_itor_trade)->Direction << std::endl;
		/*std::cout << "\tcomboffsetflag = " << string(1, (*posd_itor)->CombOffsetFlag[0]) << std::endl;
		std::cout << "\tcombhedgeflag = " << string(1, (*posd_itor)->CombHedgeFlag[0]) << std::endl;*/

		std::cout << "\t\toffsetflag = " << (*posd_itor_trade)->OffsetFlag << std::endl;
		std::cout << "\t\thedgeflag = " << (*posd_itor_trade)->HedgeFlag << std::endl;
		std::cout << "\t\tprice = " << (*posd_itor_trade)->Price << std::endl;
		std::cout << "\t\ttradingday = " << (*posd_itor_trade)->TradingDay << std::endl;
		strcpy((*posd_itor_trade)->TradingDayRecord, this->getTradingDay().c_str());
		std::cout << "\t\ttradingdayrecord = " << (*posd_itor_trade)->TradingDayRecord << std::endl;
		std::cout << "\t\ttradingdate = " << (*posd_itor_trade)->TradeDate << std::endl;
		std::cout << "\t\tstrategyid = " << (*posd_itor_trade)->StrategyID << std::endl;
		std::cout << "\t\tvolume = " << (*posd_itor_trade)->Volume << std::endl;

		stg->Update_Position_Trade_Changed_Detail_To_DB((*posd_itor_trade));
	}
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

/// 设置行情是否登陆完成
bool CTP_Manager::getMdLogin() {
	return this->isMdLogin;
}
void CTP_Manager::setMdLogin(bool isMdLogin) {
	this->isMdLogin = isMdLogin;
}

/// 设置ctp仓位是否初始化完成
bool CTP_Manager::getCTPFinishedPositionInit() {
	return this->isCTPFinishedPositionInit;
}
void CTP_Manager::setCTPFinishedPositionInit(bool isCTPFinishedPositionInit) {
	this->isCTPFinishedPositionInit = isCTPFinishedPositionInit;
}

/// 设置交易日
void CTP_Manager::setTradingDay(string trading_day) {
	this->trading_day = trading_day;
}

string CTP_Manager::getTradingDay() {
	return this->trading_day;
}

/// 初始化数据发送
void CTP_Manager::InitClientData(int fd, CTP_Manager *ctp_m, string s_TraderID, int i_MsgRef, int i_MsgSrc, string s_UserID, string s_StrategyID) {
	std::cout << "CTP_Manager::InitClientData()" << std::endl;
	rapidjson::Document build_doc;
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<StringBuffer> writer(buffer);

	std::cout << "\t服务端发送客户端初始化数据" << std::endl;

	/************************************************************************/
	/*请求行情接口配置信息     msgtype == 4                                                       */
	/************************************************************************/
	std::cout << "\t请求行情接口配置信息..." << std::endl;

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
	build_doc.Accept(writer);
	//rsp_msg = const_cast<char *>(buffer.GetString());
	//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
	std::cout << "\t服务端响应数据 = " << buffer.GetString() << std::endl;

	if (write_msg(fd, const_cast<char *>(buffer.GetString()), strlen(buffer.GetString())) < 0) {
		printf("\t客户端已断开!!!\n");
		//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
		if (errno == EPIPE) {
			std::cout << "\tEPIPE" << std::endl;
			//break;
		}
		perror("\tprotocal error");
	}


	/************************************************************************/
	/*请求账户信息        msgtype == 2                                                    */
	/************************************************************************/
	std::cout << "\t请求账户信息..." << std::endl;
	//list<FutureAccount *> l_futureaccount;
	//static_dbm->SearchFutrueListByTraderID(s_TraderID, &l_futureaccount);

	rapidjson::Document build_doc2;
	rapidjson::StringBuffer buffer2;
	rapidjson::Writer<StringBuffer> writer2(buffer2);


	/*构建UserInfo的Json*/
	build_doc2.SetObject();
	rapidjson::Document::AllocatorType& allocator1 = build_doc2.GetAllocator();
	build_doc2.AddMember("MsgRef", server_msg_ref++, allocator1);
	build_doc2.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator1);
	build_doc2.AddMember("MsgType", 2, allocator1);
	build_doc2.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator1);
	build_doc2.AddMember("MsgResult", 0, allocator1);
	build_doc2.AddMember("MsgErrorReason", "", allocator1);
	//创建Info数组
	rapidjson::Value info_array1(rapidjson::kArrayType);

	list<User *>::iterator future_itor;
	for (future_itor = ctp_m->getL_User()->begin(); future_itor != ctp_m->getL_User()->end(); future_itor++) {

		rapidjson::Value info_object(rapidjson::kObjectType);
		info_object.SetObject();
		info_object.AddMember("brokerid", rapidjson::StringRef((*future_itor)->getBrokerID().c_str()), allocator1);
		info_object.AddMember("traderid", rapidjson::StringRef((*future_itor)->getTraderID().c_str()), allocator1);
		info_object.AddMember("password", rapidjson::StringRef((*future_itor)->getPassword().c_str()), allocator1);
		info_object.AddMember("userid", rapidjson::StringRef((*future_itor)->getUserID().c_str()), allocator1);
		info_object.AddMember("frontaddress", rapidjson::StringRef((*future_itor)->getFrontAddress().c_str()), allocator1);
		info_object.AddMember("on_off", (*future_itor)->getOn_Off(), allocator1);
		if ((*future_itor)->getIsPositionRight())
		{
			info_object.AddMember("positionflag", 1, allocator1);
		}
		else
		{
			info_object.AddMember("positionflag", 0, allocator1);
		}

		info_array1.PushBack(info_object, allocator1);
		ctp_m->addSocketFD((*future_itor)->getUserID(), fd);
	}

	build_doc2.AddMember("Info", info_array1, allocator1);
	build_doc2.AddMember("MsgSrc", i_MsgSrc, allocator1);
	build_doc2.Accept(writer2);
	//rsp_msg = const_cast<char *>(buffer2.GetString());
	//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
	std::cout << "\t服务端响应数据 = " << buffer2.GetString() << std::endl;

	if (write_msg(fd, const_cast<char *>(buffer2.GetString()), strlen(buffer2.GetString())) < 0) {
		printf("\t客户端已断开!!!\n");
		//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
		if (errno == EPIPE) {
			std::cout << "\tEPIPE" << std::endl;
			//break;
		}
		perror("protocal error");
	}


	/************************************************************************/
	/*请求查询算法             msgtype == 11                                            */
	/************************************************************************/
	std::cout << "\t请求查询算法..." << std::endl;


	rapidjson::Document build_doc3;
	rapidjson::StringBuffer buffer3;
	rapidjson::Writer<StringBuffer> writer3(buffer3);

	list<Algorithm *> l_alg;
	static_dbm->getAllAlgorithm(&l_alg);

	/*构建MarketInfo的Json*/
	build_doc3.SetObject();
	rapidjson::Document::AllocatorType& allocator2 = build_doc3.GetAllocator();
	build_doc3.AddMember("MsgRef", server_msg_ref++, allocator2);
	build_doc3.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator2);
	build_doc3.AddMember("MsgType", 11, allocator2);
	build_doc3.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator2);
	build_doc3.AddMember("MsgResult", 0, allocator2);
	build_doc3.AddMember("MsgErrorReason", "", allocator2);
	//创建Info数组
	rapidjson::Value info_array2(rapidjson::kArrayType);

	list<Algorithm *>::iterator alg_itor;
	for (alg_itor = l_alg.begin(); alg_itor != l_alg.end(); alg_itor++) {

		rapidjson::Value info_object(rapidjson::kObjectType);
		info_object.SetObject();
		info_object.AddMember("name", rapidjson::StringRef((*alg_itor)->getAlgName().c_str()), allocator2);
		info_array2.PushBack(info_object, allocator2);
	}

	build_doc3.AddMember("Info", info_array2, allocator2);
	build_doc3.AddMember("MsgSrc", i_MsgSrc, allocator2);
	build_doc3.Accept(writer3);
	//rsp_msg = const_cast<char *>(buffer.GetString());
	//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
	std::cout << "\t服务端响应数据 = " << buffer3.GetString() << std::endl;

	if (write_msg(fd, const_cast<char *>(buffer3.GetString()), strlen(buffer3.GetString())) < 0) {
		printf("\t客户端已断开!!!\n");
		//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
		if (errno == EPIPE) {
			std::cout << "\tEPIPE" << std::endl;
			//break;
		}
		perror("\tprotocal error");
	}




	/************************************************************************/
	/*请求查询策略信息      msgtype == 3                                                   */
	/************************************************************************/
	std::cout << "\t请求查询策略信息..." << std::endl;


	rapidjson::Document build_doc4;
	rapidjson::StringBuffer buffer4;
	rapidjson::Writer<StringBuffer> writer4(buffer4);
	list<Strategy *> l_strategys;
	//static_dbm->getAllStrategy(&l_strategys, s_TraderID, s_UserID);
	static_dbm->getAllStrategyByActiveUser(&l_strategys, ctp_m->getL_User(), s_TraderID);

	/*构建StrategyInfo的Json*/
	build_doc4.SetObject();
	rapidjson::Document::AllocatorType& allocator3 = build_doc4.GetAllocator();
	build_doc4.AddMember("MsgRef", server_msg_ref++, allocator3);
	build_doc4.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator3);
	build_doc4.AddMember("MsgType", 3, allocator3);
	build_doc4.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator3);
	build_doc4.AddMember("MsgResult", 0, allocator3);
	build_doc4.AddMember("MsgErrorReason", "", allocator3);
	build_doc4.AddMember("StrategyID", rapidjson::StringRef(s_StrategyID.c_str()), allocator3);
	//创建Info数组
	rapidjson::Value info_array3(rapidjson::kArrayType);

	list<Strategy *>::iterator stg_itor;
	for (stg_itor = l_strategys.begin(); stg_itor != l_strategys.end(); stg_itor++) {

		rapidjson::Value info_object(rapidjson::kObjectType);
		info_object.SetObject();
		info_object.AddMember("position_a_sell_today", (*stg_itor)->getStgPositionASellToday(), allocator3);
		info_object.AddMember("position_b_sell", (*stg_itor)->getStgPositionBSell(), allocator3);
		info_object.AddMember("spread_shift", (*stg_itor)->getStgSpreadShift(), allocator3);
		info_object.AddMember("position_b_sell_today", (*stg_itor)->getStgPositionBSellToday(), allocator3);
		info_object.AddMember("position_b_buy_today", (*stg_itor)->getStgPositionBBuyToday(), allocator3);
		info_object.AddMember("position_a_sell", (*stg_itor)->getStgPositionASell(), allocator3);
		info_object.AddMember("buy_close", (*stg_itor)->getStgBuyClose(), allocator3);
		info_object.AddMember("stop_loss", (*stg_itor)->getStgStopLoss(), allocator3);
		info_object.AddMember("position_b_buy_yesterday", (*stg_itor)->getStgPositionBBuyYesterday(), allocator3);
		//info_object.AddMember("is_active", (*stg_itor)->isStgIsActive(), allocator3);
		info_object.AddMember("position_b_sell_yesterday", (*stg_itor)->getStgPositionBSellYesterday(), allocator3);
		info_object.AddMember("strategy_id", rapidjson::StringRef((*stg_itor)->getStgStrategyId().c_str()), allocator3);
		info_object.AddMember("position_b_buy", (*stg_itor)->getStgPositionBBuy(), allocator3);
		info_object.AddMember("lots_batch", (*stg_itor)->getStgLotsBatch(), allocator3);
		info_object.AddMember("position_a_buy", (*stg_itor)->getStgPositionABuy(), allocator3);
		info_object.AddMember("sell_open", (*stg_itor)->getStgSellOpen(), allocator3);
		info_object.AddMember("order_algorithm", rapidjson::StringRef((*stg_itor)->getStgOrderAlgorithm().c_str()), allocator3);
		info_object.AddMember("trader_id", rapidjson::StringRef((*stg_itor)->getStgTraderId().c_str()), allocator3);
		info_object.AddMember("a_order_action_limit", (*stg_itor)->getStgAOrderActionTiresLimit(), allocator3);
		info_object.AddMember("b_order_action_limit", (*stg_itor)->getStgBOrderActionTiresLimit(), allocator3);
		info_object.AddMember("sell_close", (*stg_itor)->getStgSellClose(), allocator3);
		info_object.AddMember("buy_open", (*stg_itor)->getStgBuyOpen(), allocator3);

		/*开关字段*/
		info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator3);
		info_object.AddMember("strategy_on_off", (*stg_itor)->getOn_Off(), allocator3);
		info_object.AddMember("sell_open_on_off", (*stg_itor)->getStgSellOpenOnOff(), allocator3);
		info_object.AddMember("buy_close_on_off", (*stg_itor)->getStgBuyCloseOnOff(), allocator3);
		info_object.AddMember("sell_close_on_off", (*stg_itor)->getStgSellCloseOnOff(), allocator3);
		info_object.AddMember("buy_open_on_off", (*stg_itor)->getStgBuyOpenOnOff(), allocator3);


		/*新增字段*/
		info_object.AddMember("trade_model", rapidjson::StringRef((*stg_itor)->getStgTradeModel().c_str()), allocator3);
		info_object.AddMember("update_position_detail_record_time", rapidjson::StringRef((*stg_itor)->getStgUpdatePositionDetailRecordTime().c_str()), allocator3);
		info_object.AddMember("hold_profit", (*stg_itor)->getStgHoldProfit(), allocator3);
		info_object.AddMember("close_profit", (*stg_itor)->getStgCloseProfit(), allocator3);
		info_object.AddMember("commission", (*stg_itor)->getStgCommission(), allocator3);
		info_object.AddMember("position", (*stg_itor)->getStgPosition(), allocator3);
		info_object.AddMember("position_buy", (*stg_itor)->getStgPositionBuy(), allocator3);
		info_object.AddMember("position_sell", (*stg_itor)->getStgPositionSell(), allocator3);
		info_object.AddMember("trade_volume", (*stg_itor)->getStgTradeVolume(), allocator3);
		info_object.AddMember("amount", (*stg_itor)->getStgAmount(), allocator3);
		info_object.AddMember("average_shift", (*stg_itor)->getStgAverageShift(), allocator3);
		info_object.AddMember("a_limit_price_shift", (*stg_itor)->getStgALimitPriceShift(), allocator3);
		info_object.AddMember("b_limit_price_shift", (*stg_itor)->getStgBLimitPriceShift(), allocator3);

		/*2017.03.03新增参数*/
		info_object.AddMember("on_off", (*stg_itor)->getOn_Off(), allocator3);
		info_object.AddMember("a_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdA().c_str()), allocator3);
		info_object.AddMember("b_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdB().c_str()), allocator3);
		info_object.AddMember("a_limit_price_shift", (*stg_itor)->getStgALimitPriceShift(), allocator3);
		info_object.AddMember("b_limit_price_shift", (*stg_itor)->getStgBLimitPriceShift(), allocator3);
		info_object.AddMember("sell_open_on_off", (*stg_itor)->getStgSellOpenOnOff(), allocator3);
		info_object.AddMember("buy_close_on_off", (*stg_itor)->getStgBuyCloseOnOff(), allocator3);
		info_object.AddMember("buy_open_on_off", (*stg_itor)->getStgBuyOpenOnOff(), allocator3);
		info_object.AddMember("sell_close_on_off", (*stg_itor)->getStgSellCloseOnOff(), allocator3);



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

			instrument_array.PushBack(instrument_object, allocator3);
		}
		info_object.AddMember("list_instrument_id", instrument_array, allocator3);
		info_object.AddMember("position_a_buy_yesterday", (*stg_itor)->getStgPositionABuyYesterday(), allocator3);
		info_object.AddMember("user_id", rapidjson::StringRef((*stg_itor)->getStgUserId().c_str()), allocator3);
		info_object.AddMember("position_a_buy_today", (*stg_itor)->getStgPositionABuyToday(), allocator3);
		info_object.AddMember("position_a_sell_yesterday", (*stg_itor)->getStgPositionASellYesterday(), allocator3);
		info_object.AddMember("lots", (*stg_itor)->getStgLots(), allocator3);
		info_object.AddMember("a_wait_price_tick", (*stg_itor)->getStgAWaitPriceTick(), allocator3);
		info_object.AddMember("b_wait_price_tick", (*stg_itor)->getStgBWaitPriceTick(), allocator3);
		//info_object.AddMember("StrategyOnoff", (*stg_itor)->getOn_Off(), allocator3);

		info_array3.PushBack(info_object, allocator3);
	}

	build_doc4.AddMember("Info", info_array3, allocator3);
	build_doc4.AddMember("MsgSrc", i_MsgSrc, allocator3);
	build_doc4.Accept(writer4);
	//rsp_msg = const_cast<char *>(buffer.GetString());
	//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
	std::cout << "\t服务端响应数据 = " << buffer4.GetString() << std::endl;

	if (write_msg(fd, const_cast<char *>(buffer4.GetString()), strlen(buffer4.GetString())) < 0) {
		printf("\t客户端已断开!!!\n");
		//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
		if (errno == EPIPE) {
			std::cout << "\tEPIPE" << std::endl;
			//break;
		}
		perror("\tprotocal error");
	}


	/************************************************************************/
	/* 查询期货账户昨日持仓明细(order)    msgtype == 15                                                                 */
	/************************************************************************/
	// 查询期货账户昨日持仓明细
	std::cout << "\t查询期货账户昨日持仓明细..." << std::endl;


	rapidjson::Document build_doc5;
	rapidjson::StringBuffer buffer5;
	rapidjson::Writer<StringBuffer> writer5(buffer5);

	bool bFind = false;

	list<USER_CThostFtdcOrderField *> l_posd;
	static_dbm->getAllPositionDetailYesterday(&l_posd, s_TraderID, s_UserID);

	/*构建策略昨仓Json*/
	build_doc5.SetObject();
	rapidjson::Document::AllocatorType& allocator4 = build_doc5.GetAllocator();
	build_doc5.AddMember("MsgRef", server_msg_ref++, allocator4);
	build_doc5.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator4);
	build_doc5.AddMember("MsgType", 15, allocator4);
	build_doc5.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator4);
	build_doc5.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator4);

	//创建Info数组
	rapidjson::Value create_info_array4(rapidjson::kArrayType);

	list<USER_CThostFtdcOrderField *>::iterator pod_itor;
	for (pod_itor = l_posd.begin(); pod_itor != l_posd.end(); pod_itor++) {

		/*构造内容json*/
		rapidjson::Value create_info_object(rapidjson::kObjectType);
		create_info_object.SetObject();

		create_info_object.AddMember("InstrumentID", rapidjson::StringRef((*pod_itor)->InstrumentID), allocator4);
		create_info_object.AddMember("OrderRef", rapidjson::StringRef((*pod_itor)->OrderRef), allocator4);
		create_info_object.AddMember("UserID", rapidjson::StringRef((*pod_itor)->UserID), allocator4);
		//create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);
		
		/*string direction;
		std::cout << "msgtype == 15 (*pod_itor)->Direction = " << (*pod_itor)->Direction << std::endl;
		direction = (*pod_itor)->Direction;*/
		create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator4);
		
		/*string CombOffsetFlag;
		CombOffsetFlag = (*pod_itor)->CombOffsetFlag[0];*/
		create_info_object.AddMember("CombOffsetFlag", (*pod_itor)->CombOffsetFlag[0], allocator4);
		
		/*string CombHedgeFlag;
		CombHedgeFlag = (*pod_itor)->CombHedgeFlag[0];*/
		create_info_object.AddMember("CombHedgeFlag", (*pod_itor)->CombHedgeFlag[0], allocator4);

		create_info_object.AddMember("LimitPrice", (*pod_itor)->LimitPrice, allocator4);
		create_info_object.AddMember("VolumeTotalOriginal", (*pod_itor)->VolumeTotalOriginal, allocator4);
		create_info_object.AddMember("TradingDay", rapidjson::StringRef((*pod_itor)->TradingDay), allocator4);
		create_info_object.AddMember("TradingDayRecord", rapidjson::StringRef((*pod_itor)->TradingDayRecord), allocator4);

		/*string OrderStatus;
		OrderStatus = (*pod_itor)->OrderStatus;*/
		create_info_object.AddMember("OrderStatus", (*pod_itor)->OrderStatus, allocator4);

		create_info_object.AddMember("VolumeTraded", (*pod_itor)->VolumeTraded, allocator4);
		create_info_object.AddMember("VolumeTotal", (*pod_itor)->VolumeTotal, allocator4);
		create_info_object.AddMember("InsertDate", rapidjson::StringRef((*pod_itor)->InsertDate), allocator4);
		create_info_object.AddMember("InsertTime", rapidjson::StringRef((*pod_itor)->InsertTime), allocator4);
		create_info_object.AddMember("StrategyID", rapidjson::StringRef((*pod_itor)->StrategyID), allocator4);
		create_info_object.AddMember("VolumeTradedBatch", (*pod_itor)->VolumeTradedBatch, allocator4);

		create_info_array4.PushBack(create_info_object, allocator4);

		bFind = true;
	}

	if (bFind) {
		build_doc5.AddMember("MsgResult", 0, allocator4);
		build_doc5.AddMember("MsgErrorReason", "", allocator4);
	}
	else {
		build_doc5.AddMember("MsgResult", 0, allocator4);
		build_doc5.AddMember("MsgErrorReason", "未找到该策略昨仓明细(order)", allocator4);
	}

	build_doc5.AddMember("Info", create_info_array4, allocator4);
	build_doc5.AddMember("MsgSrc", i_MsgSrc, allocator4);
	build_doc5.Accept(writer5);
	//rsp_msg = const_cast<char *>(buffer5.GetString());
	//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
	std::cout << "\t服务端响应数据 = " << buffer5.GetString() << std::endl;

	if (write_msg(fd, const_cast<char *>(buffer5.GetString()), strlen(buffer5.GetString())) < 0) {
		printf("\t客户端已断开!!!\n");
		//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
		if (errno == EPIPE) {
			std::cout << "\tEPIPE" << std::endl;
			//break;
		}
		perror("\tprotocal error");
	}





	/************************************************************************/
	/*查询期货账户昨日持仓明细(trade) msgtype == 17                                                                      */
	/************************************************************************/
	// 查询期货账户昨日持仓明细(trade)
	std::cout << "\t查询期货账户昨日持仓明细..." << std::endl;

	bFind = false;

	rapidjson::Document build_doc6;
	rapidjson::StringBuffer buffer6;
	rapidjson::Writer<StringBuffer> writer6(buffer6);

	list<USER_CThostFtdcTradeField *> l_posd_trade;
	static_dbm->getAllPositionDetailTradeYesterday(&l_posd_trade, s_TraderID, s_UserID);

	/*构建策略昨仓Json*/
	build_doc6.SetObject();
	rapidjson::Document::AllocatorType& allocator5 = build_doc6.GetAllocator();
	build_doc6.AddMember("MsgRef", server_msg_ref++, allocator5);
	build_doc6.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator5);
	build_doc6.AddMember("MsgType", 17, allocator5);
	build_doc6.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator5);
	build_doc6.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator5);

	//创建Info数组
	rapidjson::Value create_info_array5(rapidjson::kArrayType);

	list<USER_CThostFtdcTradeField *>::iterator pod_itor_trade;
	for (pod_itor_trade = l_posd_trade.begin(); pod_itor_trade != l_posd_trade.end(); pod_itor_trade++) {

		/*构造内容json*/
		rapidjson::Value create_info_object(rapidjson::kObjectType);
		create_info_object.SetObject();

		create_info_object.AddMember("InstrumentID", rapidjson::StringRef((*pod_itor_trade)->InstrumentID), allocator5);
		create_info_object.AddMember("OrderRef", rapidjson::StringRef((*pod_itor_trade)->OrderRef), allocator5);
		create_info_object.AddMember("UserID", rapidjson::StringRef((*pod_itor_trade)->UserID), allocator5);

		/*string direction;
		if ((*pod_itor_trade)->Direction == '0')
		{
			direction = "0";
		}
		else if ((*pod_itor_trade)->Direction == '1')
		{
			direction = "1";
		}*/
		create_info_object.AddMember("Direction", (*pod_itor_trade)->Direction, allocator5);

		/*string OffsetFlag;
		if ((*pod_itor_trade)->OffsetFlag == '0')
		{
			OffsetFlag = "0";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '1')
		{
			OffsetFlag = "1";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '2')
		{
			OffsetFlag = "2";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '3')
		{
			OffsetFlag = "3";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '4')
		{
			OffsetFlag = "4";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '5')
		{
			OffsetFlag = "5";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '6')
		{
			OffsetFlag = "6";
		}*/
		create_info_object.AddMember("OffsetFlag", (*pod_itor_trade)->OffsetFlag, allocator5);

		/*string HedgeFlag;
		if ((*pod_itor_trade)->HedgeFlag == '1')
		{
		HedgeFlag = "1";
		}
		else if ((*pod_itor_trade)->HedgeFlag == '2')
		{
		HedgeFlag = "2";
		}
		else if ((*pod_itor_trade)->HedgeFlag == '3')
		{
		HedgeFlag = "3";
		}
		else if ((*pod_itor_trade)->HedgeFlag == '5')
		{
		HedgeFlag = "5";
		}*/
		create_info_object.AddMember("HedgeFlag", (*pod_itor_trade)->HedgeFlag, allocator5);

		//create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator5);
		//create_info_object.AddMember("OffsetFlag", (*pod_itor)->OffsetFlag, allocator5);
		//create_info_object.AddMember("HedgeFlag", (*pod_itor)->HedgeFlag, allocator5);
		create_info_object.AddMember("Price", (*pod_itor_trade)->Price, allocator5);
		create_info_object.AddMember("TradingDay", rapidjson::StringRef((*pod_itor_trade)->TradingDay), allocator5);
		create_info_object.AddMember("TradingDayRecord", rapidjson::StringRef((*pod_itor_trade)->TradingDayRecord), allocator5);
		create_info_object.AddMember("TradeDate", rapidjson::StringRef((*pod_itor_trade)->TradeDate), allocator5);
		create_info_object.AddMember("StrategyID", rapidjson::StringRef((*pod_itor_trade)->StrategyID), allocator5);
		create_info_object.AddMember("Volume", (*pod_itor_trade)->Volume, allocator5);

		create_info_array5.PushBack(create_info_object, allocator5);

		bFind = true;
	}

	if (bFind) {
		build_doc6.AddMember("MsgResult", 0, allocator5);
		build_doc6.AddMember("MsgErrorReason", "", allocator5);
	}
	else {
		build_doc6.AddMember("MsgResult", 0, allocator5);
		build_doc6.AddMember("MsgErrorReason", "未找到该策略昨仓明细(trade)", allocator5);
	}

	build_doc6.AddMember("Info", create_info_array5, allocator5);
	build_doc6.AddMember("MsgSrc", i_MsgSrc, allocator5);
	build_doc6.Accept(writer6);
	//rsp_msg = const_cast<char *>(buffer6.GetString());
	//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
	std::cout << "\t服务端响应数据 = " << buffer6.GetString() << std::endl;

	if (write_msg(fd, const_cast<char *>(buffer6.GetString()), strlen(buffer6.GetString())) < 0) {
		printf("\t客户端已断开!!!\n");
		//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
		if (errno == EPIPE) {
			std::cout << "\tEPIPE" << std::endl;
			//break;
		}
		perror("\tprotocal error");
	}

	/************************************************************************/
	/* 查询期货账户今日持仓明细(order)已修改过的    msgtype == 20                                                                 */
	/************************************************************************/
	// 查询期货账户今日持仓明细
	std::cout << "\t查询期货账户今日持仓明细order(已修改过的)..." << std::endl;
	

	rapidjson::Document build_doc20;
	rapidjson::StringBuffer buffer20;
	rapidjson::Writer<StringBuffer> writer20(buffer20);

	bFind = false;
	static_dbm->getAllPositionDetailChanged(&l_posd, s_TraderID, s_UserID);

	std::cout << "\torder size = " << l_posd.size() << std::endl;

	/*构建策略昨仓Json*/
	build_doc20.SetObject();
	rapidjson::Document::AllocatorType& allocator20 = build_doc20.GetAllocator();
	build_doc20.AddMember("MsgRef", server_msg_ref++, allocator20);
	build_doc20.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator20);
	build_doc20.AddMember("MsgType", 20, allocator20);
	build_doc20.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator20);
	build_doc20.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator20);

	//创建Info数组
	rapidjson::Value create_info_array20(rapidjson::kArrayType);
	for (pod_itor = l_posd.begin(); pod_itor != l_posd.end(); pod_itor++) {

		/*构造内容json*/
		rapidjson::Value create_info_object(rapidjson::kObjectType);
		create_info_object.SetObject();

		create_info_object.AddMember("InstrumentID", rapidjson::StringRef((*pod_itor)->InstrumentID), allocator20);
		create_info_object.AddMember("OrderRef", rapidjson::StringRef((*pod_itor)->OrderRef), allocator20);
		create_info_object.AddMember("UserID", rapidjson::StringRef((*pod_itor)->UserID), allocator20);
		//create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);

		/*string direction;
		std::cout << "msgtype == 15 (*pod_itor)->Direction = " << (*pod_itor)->Direction << std::endl;
		direction = (*pod_itor)->Direction;*/
		create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator20);

		/*string CombOffsetFlag;
		CombOffsetFlag = (*pod_itor)->CombOffsetFlag[0];*/
		create_info_object.AddMember("CombOffsetFlag", (*pod_itor)->CombOffsetFlag[0], allocator20);

		/*string CombHedgeFlag;
		CombHedgeFlag = (*pod_itor)->CombHedgeFlag[0];*/
		create_info_object.AddMember("CombHedgeFlag", (*pod_itor)->CombHedgeFlag[0], allocator20);

		create_info_object.AddMember("LimitPrice", (*pod_itor)->LimitPrice, allocator20);
		create_info_object.AddMember("VolumeTotalOriginal", (*pod_itor)->VolumeTotalOriginal, allocator20);
		create_info_object.AddMember("TradingDay", rapidjson::StringRef((*pod_itor)->TradingDay), allocator20);
		create_info_object.AddMember("TradingDayRecord", rapidjson::StringRef((*pod_itor)->TradingDayRecord), allocator20);

		/*string OrderStatus;
		OrderStatus = (*pod_itor)->OrderStatus;*/
		create_info_object.AddMember("OrderStatus", (*pod_itor)->OrderStatus, allocator20);

		create_info_object.AddMember("VolumeTraded", (*pod_itor)->VolumeTraded, allocator20);
		create_info_object.AddMember("VolumeTotal", (*pod_itor)->VolumeTotal, allocator20);
		create_info_object.AddMember("InsertDate", rapidjson::StringRef((*pod_itor)->InsertDate), allocator20);
		create_info_object.AddMember("InsertTime", rapidjson::StringRef((*pod_itor)->InsertTime), allocator20);
		create_info_object.AddMember("StrategyID", rapidjson::StringRef((*pod_itor)->StrategyID), allocator20);
		create_info_object.AddMember("VolumeTradedBatch", (*pod_itor)->VolumeTradedBatch, allocator20);

		create_info_array20.PushBack(create_info_object, allocator20);

		bFind = true;
	}

	if (bFind) {
		build_doc20.AddMember("MsgResult", 0, allocator20);
		build_doc20.AddMember("MsgErrorReason", "", allocator20);
	}
	else {
		build_doc20.AddMember("MsgResult", 0, allocator20);
		build_doc20.AddMember("MsgErrorReason", "未找到该策略今仓明细(order)", allocator20);
	}

	build_doc20.AddMember("Info", create_info_array20, allocator20);
	build_doc20.AddMember("MsgSrc", i_MsgSrc, allocator20);
	build_doc20.Accept(writer20);
	//rsp_msg = const_cast<char *>(buffer5.GetString());
	//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
	std::cout << "\t服务端响应数据 = " << buffer20.GetString() << std::endl;

	if (write_msg(fd, const_cast<char *>(buffer20.GetString()), strlen(buffer20.GetString())) < 0) {
		printf("\t客户端已断开!!!\n");
		//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
		if (errno == EPIPE) {
			std::cout << "\tEPIPE" << std::endl;
			//break;
		}
		perror("\tprotocal error");
	}





	/************************************************************************/
	/*查询期货账户昨日持仓明细(trade) msgtype == 21                                                                      */
	/************************************************************************/
	// 查询期货账户昨日持仓明细(trade)
	std::cout << "\t查询期货账户今日持仓明细trade(已修改过的)..." << std::endl;

	bFind = false;

	rapidjson::Document build_doc21;
	rapidjson::StringBuffer buffer21;
	rapidjson::Writer<StringBuffer> writer21(buffer21);

	static_dbm->getAllPositionDetailTradeChanged(&l_posd_trade, s_TraderID, s_UserID);

	std::cout << "\ttrade size = " << l_posd_trade.size() << std::endl;

	/*构建策略昨仓Json*/
	build_doc21.SetObject();
	rapidjson::Document::AllocatorType& allocator21 = build_doc21.GetAllocator();
	build_doc21.AddMember("MsgRef", server_msg_ref++, allocator21);
	build_doc21.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator21);
	build_doc21.AddMember("MsgType", 21, allocator21);
	build_doc21.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator21);
	build_doc21.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator21);

	//创建Info数组
	rapidjson::Value create_info_array21(rapidjson::kArrayType);
	for (pod_itor_trade = l_posd_trade.begin(); pod_itor_trade != l_posd_trade.end(); pod_itor_trade++) {

		/*构造内容json*/
		rapidjson::Value create_info_object(rapidjson::kObjectType);
		create_info_object.SetObject();

		create_info_object.AddMember("InstrumentID", rapidjson::StringRef((*pod_itor_trade)->InstrumentID), allocator21);
		create_info_object.AddMember("OrderRef", rapidjson::StringRef((*pod_itor_trade)->OrderRef), allocator21);
		create_info_object.AddMember("UserID", rapidjson::StringRef((*pod_itor_trade)->UserID), allocator21);

		/*string direction;
		if ((*pod_itor_trade)->Direction == '0')
		{
		direction = "0";
		}
		else if ((*pod_itor_trade)->Direction == '1')
		{
		direction = "1";
		}*/
		create_info_object.AddMember("Direction", (*pod_itor_trade)->Direction, allocator21);

		/*string OffsetFlag;
		if ((*pod_itor_trade)->OffsetFlag == '0')
		{
		OffsetFlag = "0";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '1')
		{
		OffsetFlag = "1";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '2')
		{
		OffsetFlag = "2";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '3')
		{
		OffsetFlag = "3";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '4')
		{
		OffsetFlag = "4";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '5')
		{
		OffsetFlag = "5";
		}
		else if ((*pod_itor_trade)->OffsetFlag == '6')
		{
		OffsetFlag = "6";
		}*/
		create_info_object.AddMember("OffsetFlag", (*pod_itor_trade)->OffsetFlag, allocator21);

		/*string HedgeFlag;
		if ((*pod_itor_trade)->HedgeFlag == '1')
		{
		HedgeFlag = "1";
		}
		else if ((*pod_itor_trade)->HedgeFlag == '2')
		{
		HedgeFlag = "2";
		}
		else if ((*pod_itor_trade)->HedgeFlag == '3')
		{
		HedgeFlag = "3";
		}
		else if ((*pod_itor_trade)->HedgeFlag == '5')
		{
		HedgeFlag = "5";
		}*/
		create_info_object.AddMember("HedgeFlag", (*pod_itor_trade)->HedgeFlag, allocator21);

		//create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator21);
		//create_info_object.AddMember("OffsetFlag", (*pod_itor)->OffsetFlag, allocator21);
		//create_info_object.AddMember("HedgeFlag", (*pod_itor)->HedgeFlag, allocator21);
		create_info_object.AddMember("Price", (*pod_itor_trade)->Price, allocator21);
		create_info_object.AddMember("TradingDay", rapidjson::StringRef((*pod_itor_trade)->TradingDay), allocator21);
		create_info_object.AddMember("TradingDayRecord", rapidjson::StringRef((*pod_itor_trade)->TradingDayRecord), allocator21);
		create_info_object.AddMember("TradeDate", rapidjson::StringRef((*pod_itor_trade)->TradeDate), allocator21);
		create_info_object.AddMember("StrategyID", rapidjson::StringRef((*pod_itor_trade)->StrategyID), allocator21);
		create_info_object.AddMember("Volume", (*pod_itor_trade)->Volume, allocator21);

		create_info_array21.PushBack(create_info_object, allocator21);

		bFind = true;
	}

	if (bFind) {
		build_doc21.AddMember("MsgResult", 0, allocator21);
		build_doc21.AddMember("MsgErrorReason", "", allocator21);
	}
	else {
		build_doc21.AddMember("MsgResult", 0, allocator21);
		build_doc21.AddMember("MsgErrorReason", "未找到该策略昨仓明细(trade)", allocator21);
	}

	build_doc21.AddMember("Info", create_info_array21, allocator21);
	build_doc21.AddMember("MsgSrc", i_MsgSrc, allocator21);
	build_doc21.Accept(writer21);
	//rsp_msg = const_cast<char *>(buffer21.GetString());
	//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
	std::cout << "\t服务端响应数据 = " << buffer21.GetString() << std::endl;

	if (write_msg(fd, const_cast<char *>(buffer21.GetString()), strlen(buffer21.GetString())) < 0) {
		printf("\t客户端已断开!!!\n");
		//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
		if (errno == EPIPE) {
			std::cout << "\tEPIPE" << std::endl;
			//break;
		}
		perror("\tprotocal error");
	}
}

/// 处理客户端发来的消息
void CTP_Manager::HandleMessage(int fd, char *msg_tmp, CTP_Manager *ctp_m) {
	std::cout << "CTP_Manager::HandleMessage" << std::endl;
	std::cout << "\t服务端收到的数据 = " << msg_tmp << std::endl;
	std::cout << "\tCTP管理类地址 = " << ctp_m << std::endl;
	std::cout << "\t通信文件流标识 = " << fd << std::endl;
	static_dbm->setIs_Online(ctp_m->getDBManager()->getIs_Online());

	//const char *rsp_msg;
	int global_msg_type = 0;

	rapidjson::Document doc;
	rapidjson::Document build_doc;
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<StringBuffer> writer(buffer);

	//doc.Parse(msg);

	char msg[sizeof(msg_tmp)];
	memset(msg, 0x00, sizeof(msg));
	strcpy(msg, msg_tmp);

	if (doc.ParseInsitu(msg).HasParseError()) { // 解析出错
		std::cout << "\tjson parse error" << std::endl;
		//return;
		build_doc.SetObject();
		rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
		build_doc.AddMember("MsgType", 99, allocator);
		build_doc.AddMember("MsgResult", 1, allocator);
		build_doc.AddMember("MsgErrorReason", "json解析出错", allocator);
	}
	else {
		if (!doc.HasMember("MsgType")) { // json不包含MsgType key
			std::cout << "\tjson had no msgtype" << std::endl;
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
			global_msg_type = msgtype;

			std::cout << "\tmsgtype = " << msgtype << std::endl;

			if (msgtype == 1) { // TraderInfo
				std::cout << "\t请求交易登录..." << std::endl;
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

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到交易员密码 = " << s_Password << std::endl;

				bool isTraderExists = static_dbm->FindTraderByTraderIdAndPassword(s_TraderID, s_Password, op);
				if (isTraderExists) { // 用户存在
					std::cout << "\t交易员存在..." << std::endl;
					build_doc.SetObject();
					rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
					build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
					build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
					build_doc.AddMember("MsgType", 1, allocator);
					build_doc.AddMember("TraderName", rapidjson::StringRef(op->getTraderName().c_str()), allocator);
					build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
					build_doc.AddMember("OnOff", op->getOn_Off(), allocator);
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
					build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

					build_doc.Accept(writer);
					//rsp_msg = const_cast<char *>(buffer.GetString());
					//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
					std::cout << "\t服务端响应数据 = " << buffer.GetString() << std::endl;

					if (write_msg(fd, const_cast<char *>(buffer.GetString()), strlen(buffer.GetString())) < 0) {
						printf("\t客户端已断开!!!\n");
						//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
						if (errno == EPIPE) {
							std::cout << "\tEPIPE" << std::endl;
							//break;
						}
						perror("\tprotocal error");
					}

					InitClientData(fd, ctp_m, s_TraderID, server_msg_ref, i_MsgSrc);

				}
				else {
					std::cout << "\t交易员不存在..." << std::endl;
					build_doc.SetObject();
					rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
					build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
					build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
					build_doc.AddMember("MsgType", 1, allocator);
					build_doc.AddMember("TraderName", "", allocator);
					if (s_TraderID == "") {
						std::cout << "\ts_TraderID == ''" << std::endl;
						build_doc.AddMember("TraderID", "", allocator);
					}
					else {
						std::cout << "\ts_TraderID == ''" << std::endl;
						build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
					}
					build_doc.AddMember("OnOff", 0, allocator);
					build_doc.AddMember("MsgResult", 1, allocator);
					build_doc.AddMember("MsgErrorReason", "No Trader Find!", allocator);
					build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

					build_doc.Accept(writer);
					//rsp_msg = const_cast<char *>(buffer.GetString());
					//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
					std::cout << "\t服务端响应数据 = " << buffer.GetString() << std::endl;

					if (write_msg(fd, const_cast<char *>(buffer.GetString()), strlen(buffer.GetString())) < 0) {
						printf("\t客户端已断开!!!\n");
						//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
						if (errno == EPIPE) {
							std::cout << "EPIPE" << std::endl;
							//break;
						}
						perror("\tprotocal error");
					}
				}
			}
			else if (msgtype == 2) { // UserInfo
				std::cout << "\t请求账户信息..." << std::endl;
				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				//list<FutureAccount *> l_futureaccount;
				//static_dbm->SearchFutrueListByTraderID(s_TraderID, &l_futureaccount);

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

				list<User *>::iterator future_itor;
				for (future_itor = ctp_m->getL_User()->begin(); future_itor != ctp_m->getL_User()->end(); future_itor++) {

					rapidjson::Value info_object(rapidjson::kObjectType);
					info_object.SetObject();
					info_object.AddMember("brokerid", rapidjson::StringRef((*future_itor)->getBrokerID().c_str()), allocator);
					info_object.AddMember("traderid", rapidjson::StringRef((*future_itor)->getTraderID().c_str()), allocator);
					info_object.AddMember("password", rapidjson::StringRef((*future_itor)->getPassword().c_str()), allocator);
					info_object.AddMember("userid", rapidjson::StringRef((*future_itor)->getUserID().c_str()), allocator);
					info_object.AddMember("frontaddress", rapidjson::StringRef((*future_itor)->getFrontAddress().c_str()), allocator);
					info_object.AddMember("on_off", (*future_itor)->getOn_Off(), allocator);
					if ((*future_itor)->getIsPositionRight())
					{
						info_object.AddMember("positionflag", 1, allocator);
					} 
					else
					{
						info_object.AddMember("positionflag", 0, allocator);
					}

					info_array.PushBack(info_object, allocator);

					ctp_m->addSocketFD((*future_itor)->getUserID(), fd);
				}

				build_doc.AddMember("Info", info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

			}
			else if (msgtype == 3) { // 请求查询策略信息
				std::cout << "\t请求查询策略信息..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];
				rapidjson::Value &StrategyID = doc["StrategyID"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();
				string s_UserID = UserID.GetString();
				string s_StrategyID = StrategyID.GetString();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				//list<Strategy *> l_strategys;
				//static_dbm->getAllStrategy(&l_strategys, s_TraderID, s_UserID);
				//static_dbm->getAllStrategyByActiveUser(&l_strategys, ctp_m->getL_User(), s_TraderID);

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 3, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("MsgResult", 0, allocator);
				build_doc.AddMember("MsgErrorReason", "", allocator);
				build_doc.AddMember("StrategyID", rapidjson::StringRef(s_StrategyID.c_str()), allocator);
				//创建Info数组
				rapidjson::Value info_array(rapidjson::kArrayType);

				list<Strategy *>::iterator stg_itor;
				for (stg_itor = ctp_m->getListStrategy()->begin(); stg_itor != ctp_m->getListStrategy()->end(); stg_itor++) {

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
					info_object.AddMember("a_order_action_limit", (*stg_itor)->getStgAOrderActionTiresLimit(), allocator);
					info_object.AddMember("b_order_action_limit", (*stg_itor)->getStgBOrderActionTiresLimit(), allocator);
					info_object.AddMember("sell_close", (*stg_itor)->getStgSellClose(), allocator);
					info_object.AddMember("buy_open", (*stg_itor)->getStgBuyOpen(), allocator);

					/*开关字段*/
					info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator);
					info_object.AddMember("strategy_on_off", (*stg_itor)->getOn_Off(), allocator);
					info_object.AddMember("sell_open_on_off", (*stg_itor)->getStgSellOpenOnOff(), allocator);
					info_object.AddMember("buy_close_on_off", (*stg_itor)->getStgBuyCloseOnOff(), allocator);
					info_object.AddMember("sell_close_on_off", (*stg_itor)->getStgSellCloseOnOff(), allocator);
					info_object.AddMember("buy_open_on_off", (*stg_itor)->getStgBuyOpenOnOff(), allocator);


					/*新增字段*/
					info_object.AddMember("trade_model", rapidjson::StringRef((*stg_itor)->getStgTradeModel().c_str()), allocator);
					info_object.AddMember("update_position_detail_record_time", rapidjson::StringRef((*stg_itor)->getStgUpdatePositionDetailRecordTime().c_str()), allocator);
					info_object.AddMember("hold_profit", (*stg_itor)->getStgHoldProfit(), allocator);
					info_object.AddMember("close_profit", (*stg_itor)->getStgCloseProfit(), allocator);
					info_object.AddMember("commission", (*stg_itor)->getStgCommission(), allocator);
					info_object.AddMember("position", (*stg_itor)->getStgPosition(), allocator);
					info_object.AddMember("position_buy", (*stg_itor)->getStgPositionBuy(), allocator);
					info_object.AddMember("position_sell", (*stg_itor)->getStgPositionSell(), allocator);
					info_object.AddMember("trade_volume", (*stg_itor)->getStgTradeVolume(), allocator);
					info_object.AddMember("amount", (*stg_itor)->getStgAmount(), allocator);
					info_object.AddMember("average_shift", (*stg_itor)->getStgAverageShift(), allocator);
					info_object.AddMember("a_limit_price_shift", (*stg_itor)->getStgALimitPriceShift(), allocator);
					info_object.AddMember("b_limit_price_shift", (*stg_itor)->getStgBLimitPriceShift(), allocator);

					/*2017.03.03新增参数*/
					info_object.AddMember("on_off", (*stg_itor)->getOn_Off(), allocator);
					info_object.AddMember("a_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdA().c_str()), allocator);
					info_object.AddMember("b_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdB().c_str()), allocator);
					info_object.AddMember("a_limit_price_shift", (*stg_itor)->getStgALimitPriceShift(), allocator);
					info_object.AddMember("b_limit_price_shift", (*stg_itor)->getStgBLimitPriceShift(), allocator);
					info_object.AddMember("sell_open_on_off", (*stg_itor)->getStgSellOpenOnOff(), allocator);
					info_object.AddMember("buy_close_on_off", (*stg_itor)->getStgBuyCloseOnOff(), allocator);
					info_object.AddMember("buy_open_on_off", (*stg_itor)->getStgBuyOpenOnOff(), allocator);
					info_object.AddMember("sell_close_on_off", (*stg_itor)->getStgSellCloseOnOff(), allocator);



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
					//info_object.AddMember("StrategyOnoff", (*stg_itor)->getOn_Off(), allocator);

					info_array.PushBack(info_object, allocator);
				}

				build_doc.AddMember("Info", info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

			}
			else if (msgtype == 4) { // market行情接口参数信息
				std::cout << "\t请求行情接口配置信息..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;

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
			else if (msgtype == 5) { // 修改Strategy（修改Strategy参数,不带修改持仓信息）
				std::cout << "\t请求策略修改..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];
				rapidjson::Value &infoArray = doc["Info"];
				rapidjson::Value &StrategyID = doc["StrategyID"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();
				string s_UserID = UserID.GetString();
				string s_StrategyID = StrategyID.GetString();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 5, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);
				build_doc.AddMember("StrategyID", rapidjson::StringRef(s_StrategyID.c_str()), allocator);
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

								//(*stg_itor)->setStgPositionASellToday(object["position_a_sell_today"].GetInt());
								//(*stg_itor)->setStgPositionBSell(object["position_b_sell"].GetInt());
								//(*stg_itor)->setStgPositionBSellToday(object["position_b_sell_today"].GetInt());
								//(*stg_itor)->setStgPositionBBuyToday(object["position_b_buy_today"].GetInt());
								//(*stg_itor)->setStgPositionASell(object["position_a_sell"].GetInt());
								//(*stg_itor)->setStgPositionBBuyYesterday(object["position_b_buy_yesterday"].GetInt());
								//(*stg_itor)->setStgIsActive(object["is_active"].GetBool());
								//(*stg_itor)->setStgPositionBSellYesterday(object["position_b_sell_yesterday"].GetInt());
								//(*stg_itor)->setStgPositionBBuy(object["position_b_buy"].GetInt());
								//(*stg_itor)->setStgPositionABuy(object["position_a_buy"].GetInt());
								
								/* PyQt更新参数如下 */
								(*stg_itor)->setStgUserId(object["user_id"].GetString());
								(*stg_itor)->setStgStrategyId(object["strategy_id"].GetString());
								(*stg_itor)->setStgTradeModel(object["trade_model"].GetString());
								(*stg_itor)->setStgOrderAlgorithm(object["order_algorithm"].GetString());
								(*stg_itor)->setStgLots(object["lots"].GetInt());
								(*stg_itor)->setStgLotsBatch(object["lots_batch"].GetInt());
								(*stg_itor)->setStgStopLoss(object["stop_loss"].GetDouble());
								//(*stg_itor)->setOn_Off(object["on_off"].GetInt());
								(*stg_itor)->setStgSpreadShift(object["spread_shift"].GetDouble());
								//(*stg_itor)->setStgInstrumentIdA(object["a_instrument_id"].GetString());
								//(*stg_itor)->setStgInstrumentIdB(object["b_instrument_id"].GetString());
								(*stg_itor)->setStgALimitPriceShift(object["a_limit_price_shift"].GetInt());
								(*stg_itor)->setStgBLimitPriceShift(object["b_limit_price_shift"].GetInt());
								(*stg_itor)->setStgAWaitPriceTick(object["a_wait_price_tick"].GetDouble());
								(*stg_itor)->setStgBWaitPriceTick(object["b_wait_price_tick"].GetDouble());
								(*stg_itor)->setStgAOrderActionTiresLimit(object["a_order_action_limit"].GetInt());
								(*stg_itor)->setStgBOrderActionTiresLimit(object["b_order_action_limit"].GetInt());
								(*stg_itor)->setStgSellOpen(object["sell_open"].GetDouble());
								(*stg_itor)->setStgSellClose(object["sell_close"].GetDouble());
								(*stg_itor)->setStgBuyClose(object["buy_close"].GetDouble());
								(*stg_itor)->setStgBuyOpen(object["buy_open"].GetDouble());
								(*stg_itor)->setStgSellOpenOnOff(object["sell_open_on_off"].GetInt());
								(*stg_itor)->setStgBuyCloseOnOff(object["buy_close_on_off"].GetInt());
								(*stg_itor)->setStgSellCloseOnOff(object["sell_close_on_off"].GetInt());
								(*stg_itor)->setStgBuyOpenOnOff(object["buy_open_on_off"].GetInt());



								//(*stg_itor)->setStgTraderId(object["trader_id"].GetString());

								/*(*stg_itor)->setStgBuyOpen(object["only_close"].GetInt());
								(*stg_itor)->setOn_Off(object["strategy_on_off"].GetInt());*/
								
								
								//(*stg_itor)->setStgOnlyClose(object["only_close"].GetInt());
								//(*stg_itor)->setStgHoldProfit(object["hold_profit"].GetDouble());
								//(*stg_itor)->setStgCloseProfit(object["close_profit"].GetDouble());
								//(*stg_itor)->setStgCommisstion(object["commission"].GetDouble());
								////(*stg_itor)->setStgPosition(object["position"].GetInt());
								////(*stg_itor)->setStgPositionBuy(object["position_buy"].GetInt());
								////(*stg_itor)->setStgPositionSell(object["position_sell"].GetInt());
								//(*stg_itor)->setStgTradeVolume(object["trade_volume"].GetInt());
								//(*stg_itor)->setStgAmount(object["amount"].GetDouble());
								//(*stg_itor)->setStgAverageShift(object["average_shift"].GetDouble());
								//(*stg_itor)->setOn_Off(object["StrategyOnoff"].GetInt());


								////遍历list_instrument_id
								//const Value& info_object = object["list_instrument_id"];
								//if (info_object.IsArray()) {
								//	for (int j = 0; j < info_object.Size(); j++) {
								//		std::string instrument = info_object[j].GetString();
								//		std::cout << "instrument[" << j << "] = " << instrument << std::endl;
								//		if (j == 0) {
								//			(*stg_itor)->setStgInstrumentIdA(instrument);
								//		}
								//		else if (j == 1) {
								//			(*stg_itor)->setStgInstrumentIdB(instrument);
								//		}
								//	}
								//}
								//(*stg_itor)->setStgPositionABuyYesterday(object["position_a_buy_yesterday"].GetInt());
								//(*stg_itor)->setStgPositionABuyToday(object["position_a_buy_today"].GetInt());
								//(*stg_itor)->setStgPositionASellYesterday(object["position_a_sell_yesterday"].GetInt());
								

								std::cout << "Strategy修改完成!" << std::endl;
								static_dbm->UpdateStrategy((*stg_itor));

								/*构造内容json*/
								rapidjson::Value create_info_object(rapidjson::kObjectType);
								create_info_object.SetObject();

								create_info_object.AddMember("user_id", rapidjson::StringRef((*stg_itor)->getStgUserId().c_str()), allocator);
								create_info_object.AddMember("strategy_id", rapidjson::StringRef((*stg_itor)->getStgStrategyId().c_str()), allocator);
								create_info_object.AddMember("trade_model", rapidjson::StringRef((*stg_itor)->getStgTradeModel().c_str()), allocator);
								create_info_object.AddMember("order_algorithm", rapidjson::StringRef((*stg_itor)->getStgOrderAlgorithm().c_str()), allocator);
								create_info_object.AddMember("lots", (*stg_itor)->getStgLots(), allocator);
								create_info_object.AddMember("lots_batch", (*stg_itor)->getStgLotsBatch(), allocator);
								create_info_object.AddMember("stop_loss", (*stg_itor)->getStgStopLoss(), allocator);
								create_info_object.AddMember("on_off", (*stg_itor)->getOn_Off(), allocator);
								create_info_object.AddMember("spread_shift", (*stg_itor)->getStgSpreadShift(), allocator);
								create_info_object.AddMember("a_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdA().c_str()), allocator);
								create_info_object.AddMember("b_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdB().c_str()), allocator);
								create_info_object.AddMember("a_limit_price_shift", (*stg_itor)->getStgALimitPriceShift(), allocator);
								create_info_object.AddMember("b_limit_price_shift", (*stg_itor)->getStgBLimitPriceShift(), allocator);
								create_info_object.AddMember("a_wait_price_tick", (*stg_itor)->getStgAWaitPriceTick(), allocator);
								create_info_object.AddMember("b_wait_price_tick", (*stg_itor)->getStgBWaitPriceTick(), allocator);
								create_info_object.AddMember("a_order_action_limit", (*stg_itor)->getStgAOrderActionTiresLimit(), allocator);
								create_info_object.AddMember("b_order_action_limit", (*stg_itor)->getStgBOrderActionTiresLimit(), allocator);
								create_info_object.AddMember("sell_close", (*stg_itor)->getStgSellClose(), allocator);
								create_info_object.AddMember("buy_open", (*stg_itor)->getStgBuyOpen(), allocator);
								create_info_object.AddMember("sell_open", (*stg_itor)->getStgSellOpen(), allocator);
								create_info_object.AddMember("buy_close", (*stg_itor)->getStgBuyClose(), allocator);
								create_info_object.AddMember("strategy_on_off", (*stg_itor)->getOn_Off(), allocator);
								create_info_object.AddMember("sell_open_on_off", (*stg_itor)->getStgSellOpenOnOff(), allocator);
								create_info_object.AddMember("buy_close_on_off", (*stg_itor)->getStgBuyCloseOnOff(), allocator);
								create_info_object.AddMember("sell_close_on_off", (*stg_itor)->getStgSellCloseOnOff(), allocator);
								create_info_object.AddMember("buy_open_on_off", (*stg_itor)->getStgBuyOpenOnOff(), allocator);


								//create_info_object.AddMember("position_a_sell_today", (*stg_itor)->getStgPositionASellToday(), allocator);
								//create_info_object.AddMember("position_b_sell", (*stg_itor)->getStgPositionBSell(), allocator);
								
								
								//create_info_object.AddMember("position_b_sell_today", (*stg_itor)->getStgPositionBSellToday(), allocator);
								//create_info_object.AddMember("position_b_buy_today", (*stg_itor)->getStgPositionBBuyToday(), allocator);
								//create_info_object.AddMember("position_a_sell", (*stg_itor)->getStgPositionASell(), allocator);
								
								
								//create_info_object.AddMember("position_b_buy_yesterday", (*stg_itor)->getStgPositionBBuyYesterday(), allocator);
								//create_info_object.AddMember("is_active", (*stg_itor)->isStgIsActive(), allocator);
								//create_info_object.AddMember("position_b_sell_yesterday", (*stg_itor)->getStgPositionBSellYesterday(), allocator);
								
								//create_info_object.AddMember("position_b_buy", (*stg_itor)->getStgPositionBBuy(), allocator);
								
								//create_info_object.AddMember("position_a_buy", (*stg_itor)->getStgPositionABuy(), allocator);
								
								
								create_info_object.AddMember("trader_id", rapidjson::StringRef((*stg_itor)->getStgTraderId().c_str()), allocator);
								create_info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator);
						
								


								/*新增字段*/
								
								create_info_object.AddMember("hold_profit", (*stg_itor)->getStgHoldProfit(), allocator);
								create_info_object.AddMember("close_profit", (*stg_itor)->getStgCloseProfit(), allocator);
								create_info_object.AddMember("commission", (*stg_itor)->getStgCommission(), allocator);
								//create_info_object.AddMember("position", (*stg_itor)->getStgPosition(), allocator);
								//create_info_object.AddMember("position_buy", (*stg_itor)->getStgPositionBuy(), allocator);
								//create_info_object.AddMember("position_sell", (*stg_itor)->getStgPositionSell(), allocator);
								create_info_object.AddMember("trade_volume", (*stg_itor)->getStgTradeVolume(), allocator);
								create_info_object.AddMember("amount", (*stg_itor)->getStgAmount(), allocator);
								create_info_object.AddMember("average_shift", (*stg_itor)->getStgAverageShift(), allocator);
								create_info_object.AddMember("update_position_detail_record_time", rapidjson::StringRef((*stg_itor)->getStgUpdatePositionDetailRecordTime().c_str()), allocator);

								/*rapidjson::Value instrument_array(rapidjson::kArrayType);
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
								create_info_object.AddMember("list_instrument_id", instrument_array, allocator);*/
								//create_info_object.AddMember("position_a_buy_yesterday", (*stg_itor)->getStgPositionABuyYesterday(), allocator);
								
								//create_info_object.AddMember("position_a_buy_today", (*stg_itor)->getStgPositionABuyToday(), allocator);
								//create_info_object.AddMember("position_a_sell_yesterday", (*stg_itor)->getStgPositionASellYesterday(), allocator);
								
								
								//create_info_object.AddMember("StrategyOnoff", (*stg_itor)->getOn_Off(), allocator);

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
			else if (msgtype == 6) { // 新建Strategy新建策略
				std::cout << "\t新建Strategy..." << std::endl;

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

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 6, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				
				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				list<User *>::iterator user_itor;
				

				/*1:进行策略的新建*/
				if (infoArray.IsArray()) {
					std::cout << "info is array" << std::endl;
					for (int i = 0; i < infoArray.Size(); i++) {
						User *user_tmp = NULL;
						bool is_user_exists = false;
						const Value& object = infoArray[i];
						
						for (user_itor = ctp_m->getL_User()->begin(); user_itor != ctp_m->getL_User()->end(); user_itor++) { // 遍历User

							USER_PRINT((*user_itor)->getUserID());

							//(*user_itor)->setCTP_Manager(this); //每个user对象设置CTP_Manager对象

							if (object["user_id"].GetString() == (*user_itor)->getUserID()) {
								user_tmp = (*user_itor);
								is_user_exists = true;
							}
						}

						if (is_user_exists == true) {
							USER_PRINT(user_tmp);
							Strategy *new_stg = new Strategy(user_tmp);

							new_stg->setStgTraderId(object["trader_id"].GetString());
							new_stg->setStgUserId(object["user_id"].GetString());
							new_stg->setStgStrategyId(object["strategy_id"].GetString());
							new_stg->setStgInstrumentIdA(object["a_instrument_id"].GetString());
							new_stg->setStgInstrumentIdB(object["b_instrument_id"].GetString());
							new_stg->setStgTradingDay(ctp_m->getTradingDay());
							// 最新修改时间
							new_stg->setStgUpdatePositionDetailRecordTime(Utils::getDate());
							new_stg->setStgIsPositionRight(false);

							//new_stg->setStgPositionASellToday(object["position_a_sell_today"].GetInt());
							//new_stg->setStgPositionBSell(object["position_b_sell"].GetInt());
							//new_stg->setStgSpreadShift(object["spread_shift"].GetDouble());
							//new_stg->setStgALimitPriceShift(object["a_limit_price_shift"].GetInt());
							//new_stg->setStgBLimitPriceShift(object["b_limit_price_shift"].GetInt());
							//new_stg->setStgPositionBSellToday(object["position_b_sell_today"].GetInt());
							//new_stg->setStgPositionBBuyToday(object["position_b_buy_today"].GetInt());
							//new_stg->setStgPositionASell(object["position_a_sell"].GetInt());
							//new_stg->setStgBuyClose(object["buy_close"].GetDouble());
							//new_stg->setStgStopLoss(object["stop_loss"].GetDouble());
							//new_stg->setStgPositionBBuyYesterday(object["position_b_buy_yesterday"].GetInt());
							////new_stg->setStgIsActive(object["is_active"].GetBool());
							//new_stg->setStgPositionBSellYesterday(object["position_b_sell_yesterday"].GetInt());
							//
							//new_stg->setStgPositionBBuy(object["position_b_buy"].GetInt());
							//new_stg->setStgLotsBatch(object["lots_batch"].GetInt());
							//new_stg->setStgPositionABuy(object["position_a_buy"].GetInt());
							//new_stg->setStgSellOpen(object["sell_open"].GetDouble());
							//new_stg->setStgOrderAlgorithm(object["order_algorithm"].GetString());
							//
							//new_stg->setStgAOrderActionTiresLimit(object["a_order_action_limit"].GetInt());
							//new_stg->setStgBOrderActionTiresLimit(object["b_order_action_limit"].GetInt());
							//new_stg->setStgSellClose(object["sell_close"].GetDouble());
							//new_stg->setStgBuyOpen(object["buy_open"].GetDouble());


							/*new_stg->setStgOnlyClose(object["only_close"].GetInt());
							new_stg->setOn_Off(object["strategy_on_off"].GetInt());
							new_stg->setStgSellOpenOnOff(object["sell_open_on_off"].GetInt());
							new_stg->setStgBuyCloseOnOff(object["buy_close_on_off"].GetInt());
							new_stg->setStgSellCloseOnOff(object["sell_close_on_off"].GetInt());
							new_stg->setStgBuyOpenOnOff(object["buy_open_on_off"].GetInt());*/

							/*新增字段*/
							/*new_stg->setStgTradeModel(object["trade_model"].GetString());
							new_stg->setStgHoldProfit(object["hold_profit"].GetDouble());
							new_stg->setStgCloseProfit(object["close_profit"].GetDouble());
							new_stg->setStgCommisstion(object["commission"].GetDouble());
							new_stg->setStgPosition(object["position"].GetInt());
							new_stg->setStgPositionBuy(object["position_buy"].GetInt());
							new_stg->setStgPositionSell(object["position_sell"].GetInt());
							new_stg->setStgTradeVolume(object["trade_volume"].GetInt());
							new_stg->setStgAmount(object["amount"].GetDouble());
							new_stg->setStgAverageShift(object["average_shift"].GetDouble());*/


							////遍历list_instrument_id
							//const Value& info_object = object["list_instrument_id"];
							//if (info_object.IsArray()) {
							//	for (int j = 0; j < info_object.Size(); j++) {
							//		std::string instrument = info_object[j].GetString();
							//		std::cout << "instrument[" << j << "] = " << instrument << std::endl;
							//		if (j == 0) {
							//			new_stg->setStgInstrumentIdA(instrument);
							//		}
							//		else if (j == 1) {
							//			new_stg->setStgInstrumentIdB(instrument);
							//		}
							//	}
							//}


							//new_stg->setStgPositionABuyYesterday(object["position_a_buy_yesterday"].GetInt());
							//
							//new_stg->setStgPositionABuyToday(object["position_a_buy_today"].GetInt());
							//new_stg->setStgPositionASellYesterday(object["position_a_sell_yesterday"].GetInt());
							//new_stg->setStgLots(object["lots"].GetInt());
							//new_stg->setStgAWaitPriceTick(object["a_wait_price_tick"].GetDouble());
							//new_stg->setStgBWaitPriceTick(object["b_wait_price_tick"].GetDouble());

							int flag = static_dbm->CreateStrategy(new_stg);
							int flag1 = static_dbm->CreateStrategyYesterday(new_stg);

							if (flag) {
								std::cout << "Strategy已存在无需新建!" << std::endl;
								build_doc.AddMember("MsgResult", 1, allocator);
								build_doc.AddMember("MsgErrorReason", "策略已存在,不能重复创建!", allocator);

								/*构造内容json*/
								rapidjson::Value create_info_object(rapidjson::kObjectType);
								create_info_object.SetObject();
								create_info_object.AddMember("position_a_sell_today", new_stg->getStgPositionASellToday(), allocator);
								create_info_object.AddMember("position_b_sell", new_stg->getStgPositionBSell(), allocator);
								create_info_object.AddMember("spread_shift", new_stg->getStgSpreadShift(), allocator);
								create_info_object.AddMember("a_limit_price_shift", new_stg->getStgALimitPriceShift(), allocator);
								create_info_object.AddMember("b_limit_price_shift", new_stg->getStgBLimitPriceShift(), allocator);
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
								create_info_object.AddMember("a_order_action_limit", new_stg->getStgAOrderActionTiresLimit(), allocator);
								create_info_object.AddMember("b_order_action_limit", new_stg->getStgBOrderActionTiresLimit(), allocator);
								create_info_object.AddMember("sell_close", new_stg->getStgSellClose(), allocator);
								create_info_object.AddMember("buy_open", new_stg->getStgBuyOpen(), allocator);
								create_info_object.AddMember("update_position_detail_record_time", rapidjson::StringRef(new_stg->getStgUpdatePositionDetailRecordTime().c_str()), allocator);


								create_info_object.AddMember("only_close", new_stg->isStgOnlyClose(), allocator);
								create_info_object.AddMember("strategy_on_off", new_stg->getOn_Off(), allocator);
								create_info_object.AddMember("sell_open_on_off", new_stg->getStgSellOpenOnOff(), allocator);
								create_info_object.AddMember("buy_close_on_off", new_stg->getStgBuyCloseOnOff(), allocator);
								create_info_object.AddMember("sell_close_on_off", new_stg->getStgSellCloseOnOff(), allocator);
								create_info_object.AddMember("buy_open_on_off", new_stg->getStgBuyOpenOnOff(), allocator);


								create_info_object.AddMember("trade_model", rapidjson::StringRef(new_stg->getStgTradeModel().c_str()), allocator);

								create_info_object.AddMember("hold_profit", new_stg->getStgHoldProfit(), allocator);
								create_info_object.AddMember("close_profit", new_stg->getStgCloseProfit(), allocator);
								create_info_object.AddMember("commission", new_stg->getStgCommission(), allocator);
								create_info_object.AddMember("position", new_stg->getStgPosition(), allocator);
								create_info_object.AddMember("position_buy", new_stg->getStgPositionBuy(), allocator);
								create_info_object.AddMember("position_sell", new_stg->getStgPositionSell(), allocator);
								create_info_object.AddMember("trade_volume", new_stg->getStgTradeVolume(), allocator);
								create_info_object.AddMember("amount", new_stg->getStgAmount(), allocator);
								create_info_object.AddMember("average_shift", new_stg->getStgAverageShift(), allocator);

								create_info_object.AddMember("a_instrument_id", rapidjson::StringRef(new_stg->getStgInstrumentIdA().c_str()), allocator);
								create_info_object.AddMember("b_instrument_id", rapidjson::StringRef(new_stg->getStgInstrumentIdB().c_str()), allocator);

								/*rapidjson::Value instrument_array(rapidjson::kArrayType);
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
								create_info_object.AddMember("list_instrument_id", instrument_array, allocator);*/
								create_info_object.AddMember("position_a_buy_yesterday", new_stg->getStgPositionABuyYesterday(), allocator);
								create_info_object.AddMember("user_id", rapidjson::StringRef(new_stg->getStgUserId().c_str()), allocator);
								create_info_object.AddMember("position_a_buy_today", new_stg->getStgPositionABuyToday(), allocator);
								create_info_object.AddMember("position_a_sell_yesterday", new_stg->getStgPositionASellYesterday(), allocator);
								create_info_object.AddMember("lots", new_stg->getStgLots(), allocator);
								create_info_object.AddMember("a_wait_price_tick", new_stg->getStgAWaitPriceTick(), allocator);
								create_info_object.AddMember("b_wait_price_tick", new_stg->getStgBWaitPriceTick(), allocator);

								create_info_array.PushBack(create_info_object, allocator);

								delete new_stg;
							}
							else {
								std::cout << "Strategy新建完成!" << std::endl;
								/// 将策略加到维护列表里
								ctp_m->getListStrategy()->push_back(new_stg);
								/// 将策略添加到User策略列表里
								ctp_m->syncStrategyAddToUsers(new_stg);

								/// 订阅合约
								/// 添加策略的合约到l_instrument
								ctp_m->addSubInstrument(new_stg->getStgInstrumentIdA(), ctp_m->getL_Instrument());
								ctp_m->addSubInstrument(new_stg->getStgInstrumentIdB(), ctp_m->getL_Instrument());
								/// 订阅合约
								ctp_m->SubmarketData(ctp_m->getMdSpi(), ctp_m->getL_Instrument());

								std::cout << "CTP_Manager包含策略个数 = " << ctp_m->getListStrategy()->size() << std::endl;
								build_doc.AddMember("MsgResult", 0, allocator);
								build_doc.AddMember("MsgErrorReason", "", allocator);

								/*构造内容json*/
								rapidjson::Value create_info_object(rapidjson::kObjectType);
								create_info_object.SetObject();
								create_info_object.AddMember("position_a_sell_today", new_stg->getStgPositionASellToday(), allocator);
								create_info_object.AddMember("position_b_sell", new_stg->getStgPositionBSell(), allocator);
								create_info_object.AddMember("spread_shift", new_stg->getStgSpreadShift(), allocator);
								create_info_object.AddMember("a_limit_price_shift", new_stg->getStgALimitPriceShift(), allocator);
								create_info_object.AddMember("b_limit_price_shift", new_stg->getStgBLimitPriceShift(), allocator);
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
								create_info_object.AddMember("a_order_action_limit", new_stg->getStgAOrderActionTiresLimit(), allocator);
								create_info_object.AddMember("b_order_action_limit", new_stg->getStgBOrderActionTiresLimit(), allocator);
								create_info_object.AddMember("sell_close", new_stg->getStgSellClose(), allocator);
								create_info_object.AddMember("buy_open", new_stg->getStgBuyOpen(), allocator);
								create_info_object.AddMember("update_position_detail_record_time", rapidjson::StringRef(new_stg->getStgUpdatePositionDetailRecordTime().c_str()), allocator);


								create_info_object.AddMember("only_close", new_stg->isStgOnlyClose(), allocator);
								create_info_object.AddMember("strategy_on_off", new_stg->getOn_Off(), allocator);
								create_info_object.AddMember("sell_open_on_off", new_stg->getStgSellOpenOnOff(), allocator);
								create_info_object.AddMember("buy_close_on_off", new_stg->getStgBuyCloseOnOff(), allocator);
								create_info_object.AddMember("sell_close_on_off", new_stg->getStgSellCloseOnOff(), allocator);
								create_info_object.AddMember("buy_open_on_off", new_stg->getStgBuyOpenOnOff(), allocator);


								create_info_object.AddMember("trade_model", rapidjson::StringRef(new_stg->getStgTradeModel().c_str()), allocator);

								create_info_object.AddMember("hold_profit", new_stg->getStgHoldProfit(), allocator);
								create_info_object.AddMember("close_profit", new_stg->getStgCloseProfit(), allocator);
								create_info_object.AddMember("commission", new_stg->getStgCommission(), allocator);
								create_info_object.AddMember("position", new_stg->getStgPosition(), allocator);
								create_info_object.AddMember("position_buy", new_stg->getStgPositionBuy(), allocator);
								create_info_object.AddMember("position_sell", new_stg->getStgPositionSell(), allocator);
								create_info_object.AddMember("trade_volume", new_stg->getStgTradeVolume(), allocator);
								create_info_object.AddMember("amount", new_stg->getStgAmount(), allocator);
								create_info_object.AddMember("average_shift", new_stg->getStgAverageShift(), allocator);

								create_info_object.AddMember("a_instrument_id", rapidjson::StringRef(new_stg->getStgInstrumentIdA().c_str()), allocator);
								create_info_object.AddMember("b_instrument_id", rapidjson::StringRef(new_stg->getStgInstrumentIdB().c_str()), allocator);

								/*rapidjson::Value instrument_array(rapidjson::kArrayType);
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
								create_info_object.AddMember("list_instrument_id", instrument_array, allocator);*/
								create_info_object.AddMember("position_a_buy_yesterday", new_stg->getStgPositionABuyYesterday(), allocator);
								create_info_object.AddMember("user_id", rapidjson::StringRef(new_stg->getStgUserId().c_str()), allocator);
								create_info_object.AddMember("position_a_buy_today", new_stg->getStgPositionABuyToday(), allocator);
								create_info_object.AddMember("position_a_sell_yesterday", new_stg->getStgPositionASellYesterday(), allocator);
								create_info_object.AddMember("lots", new_stg->getStgLots(), allocator);
								create_info_object.AddMember("a_wait_price_tick", new_stg->getStgAWaitPriceTick(), allocator);
								create_info_object.AddMember("b_wait_price_tick", new_stg->getStgBWaitPriceTick(), allocator);

								create_info_array.PushBack(create_info_object, allocator);
							}
						}
						else
						{
							std::cout << "未找到策略对应账户信息" << std::endl;
							build_doc.AddMember("MsgResult", 1, allocator);
							build_doc.AddMember("MsgErrorReason", "未找到策略对应账户信息!", allocator);
							break;
						}
					}
				}
				else {
					std::cout << "未收到新建策略信息" << std::endl;
					build_doc.AddMember("MsgResult", 1, allocator);
					build_doc.AddMember("MsgErrorReason", "未收到新建策略信息!", allocator);
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 7) { // 删除Strategy（修改Strategy参数）
				std::cout << "\t删除策略..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];
				rapidjson::Value &StrategyID = doc["StrategyID"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();
				string s_UserID = UserID.GetString();
				string s_StrategyID = StrategyID.GetString();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 7, allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("StrategyID", rapidjson::StringRef(s_StrategyID.c_str()), allocator);

				std::cout << "收到要删除的策略ID = " << s_StrategyID << std::endl;

				/*1:进行策略的删除,更新到数据库*/
				std::cout << "CTP_Manager包含策略个数 = " << ctp_m->getListStrategy()->size() << std::endl;
				list<Strategy *>::iterator stg_itor;
				for (stg_itor = ctp_m->getListStrategy()->begin(); stg_itor != ctp_m->getListStrategy()->end();) {

					std::cout << "(*stg_itor)->getStgUserId() = " << (*stg_itor)->getStgUserId() << std::endl;
					std::cout << "(*stg_itor)->getStgStrategyId() = " << (*stg_itor)->getStgStrategyId() << std::endl;

					if (((*stg_itor)->getStgUserId() == s_UserID) && ((*stg_itor)->getStgStrategyId() == s_StrategyID)) {
						std::cout << "找到即将删除的Strategy" << std::endl;
						int flag = static_dbm->DeleteStrategy((*stg_itor));
						int flag_1 = static_dbm->DeleteStrategyYesterday((*stg_itor));

						if (flag || flag_1) {
							std::cout << "Strategy未找到删除项!" << std::endl;
							build_doc.AddMember("MsgResult", 1, allocator);
							build_doc.AddMember("MsgErrorReason", "未找到删除的策略!", allocator);
						}
						else {
							std::cout << "Strategy删除完成!" << std::endl;
							build_doc.AddMember("MsgResult", 0, allocator);
							build_doc.AddMember("MsgErrorReason", "", allocator);
						}

						// 退订删除的合约
						ctp_m->UnSubmarketData(ctp_m->getMdSpi(), (*stg_itor)->getStgInstrumentIdA(), ctp_m->getL_UnsubInstrument());
						sleep(1);
						ctp_m->UnSubmarketData(ctp_m->getMdSpi(), (*stg_itor)->getStgInstrumentIdB(), ctp_m->getL_UnsubInstrument());

						//从数据库删除
						list<USER_CThostFtdcOrderField *>::iterator order_itor;
						for (order_itor = (*stg_itor)->getStg_List_Position_Detail_From_Order()->begin();
							order_itor != (*stg_itor)->getStg_List_Position_Detail_From_Order()->end();
							order_itor++)
						{
							ctp_m->getDBManager()->DeletePositionDetail((*order_itor));
							ctp_m->getDBManager()->DeletePositionDetailYesterday((*order_itor));
						}

						list<USER_CThostFtdcTradeField *>::iterator trade_itor;
						for (trade_itor = (*stg_itor)->getStg_List_Position_Detail_From_Trade()->begin(); 
							trade_itor != (*stg_itor)->getStg_List_Position_Detail_From_Trade()->end();
							trade_itor++) {
							ctp_m->getDBManager()->DeletePositionDetailTrade((*trade_itor));
							ctp_m->getDBManager()->DeletePositionDetailTradeYesterday((*trade_itor));
						}

						// 析构策略对应的持仓明细(order, trade)
						(*stg_itor)->clearStgPositionDetail();

						// 删除策略
						//delete (*stg_itor);
						stg_itor = ctp_m->getListStrategy()->erase(stg_itor);
						// 同步删除到User策略列表
						ctp_m->syncStrategyDeleteToUsers(s_TraderID, s_UserID, s_StrategyID);
					}
					else {
						std::cout << "未能找到修改的Strategy" << std::endl;
						stg_itor++;
					}
				}

				//build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 8) { // 交易员开关
				std::cout << "\t交易员开关..." << std::endl;

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

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
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
						// 更新数据库
						static_dbm->UpdateTrader((*trader_itor)->getTraderID(), (*trader_itor));
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
				std::cout << "\t期货账户开关..." << std::endl;

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

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;
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
						//更新数据库
						static_dbm->UpdateFutureAccount((*user_itor));
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
				std::cout << "\t查询策略昨仓..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];

				string s_TraderID = TraderID.GetString();
				string s_UserID = UserID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				bool bFind = false;

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				list<Strategy *> l_strategys;
				//static_dbm->getAllStrategyYesterday(&l_strategys, s_TraderID, s_UserID);
				static_dbm->getAllStrategyYesterdayByActiveUser(&l_strategys, ctp_m->getL_User(), s_TraderID);

				/*构建策略昨仓Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 10, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);

				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				list<Strategy *>::iterator stg_itor;
				for (stg_itor = l_strategys.begin(); stg_itor != l_strategys.end(); stg_itor++) {
					if (((*stg_itor)->getStgTraderId() == s_TraderID)) {
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
						
						create_info_object.AddMember("sell_close", (*stg_itor)->getStgSellClose(), allocator);
						create_info_object.AddMember("buy_open", (*stg_itor)->getStgBuyOpen(), allocator);
						create_info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator);

						/*新增字段*/
						create_info_object.AddMember("trade_model", rapidjson::StringRef((*stg_itor)->getStgTradeModel().c_str()), allocator);
						create_info_object.AddMember("hold_profit", (*stg_itor)->getStgHoldProfit(), allocator);
						create_info_object.AddMember("close_profit", (*stg_itor)->getStgCloseProfit(), allocator);
						create_info_object.AddMember("commission", (*stg_itor)->getStgCommission(), allocator);
						create_info_object.AddMember("position", (*stg_itor)->getStgPosition(), allocator);
						create_info_object.AddMember("position_buy", (*stg_itor)->getStgPositionBuy(), allocator);
						create_info_object.AddMember("position_sell", (*stg_itor)->getStgPositionSell(), allocator);
						create_info_object.AddMember("trade_volume", (*stg_itor)->getStgTradeVolume(), allocator);
						create_info_object.AddMember("amount", (*stg_itor)->getStgAmount(), allocator);
						create_info_object.AddMember("average_shift", (*stg_itor)->getStgAverageShift(), allocator);
						
						create_info_object.AddMember("on_off", (*stg_itor)->getOn_Off(), allocator);
						create_info_object.AddMember("a_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdA().c_str()), allocator);
						create_info_object.AddMember("b_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdB().c_str()), allocator);
						create_info_object.AddMember("a_limit_price_shift", (*stg_itor)->getStgALimitPriceShift(), allocator);
						create_info_object.AddMember("b_limit_price_shift", (*stg_itor)->getStgBLimitPriceShift(), allocator);
						create_info_object.AddMember("a_order_action_limit", (*stg_itor)->getStgAOrderActionTiresLimit(), allocator);
						create_info_object.AddMember("b_order_action_limit", (*stg_itor)->getStgBOrderActionTiresLimit(), allocator);
						create_info_object.AddMember("sell_open_on_off", (*stg_itor)->getStgSellOpenOnOff(), allocator);
						create_info_object.AddMember("buy_close_on_off", (*stg_itor)->getStgBuyCloseOnOff(), allocator);
						create_info_object.AddMember("buy_open_on_off", (*stg_itor)->getStgBuyOpenOnOff(), allocator);
						create_info_object.AddMember("sell_close_on_off", (*stg_itor)->getStgSellCloseOnOff(), allocator);

						/*rapidjson::Value instrument_array(rapidjson::kArrayType);
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

						create_info_object.AddMember("list_instrument_id", instrument_array, allocator);*/


						create_info_object.AddMember("position_a_buy_yesterday", (*stg_itor)->getStgPositionABuyYesterday(), allocator);
						create_info_object.AddMember("user_id", rapidjson::StringRef((*stg_itor)->getStgUserId().c_str()), allocator);
						create_info_object.AddMember("position_a_buy_today", (*stg_itor)->getStgPositionABuyToday(), allocator);
						create_info_object.AddMember("position_a_sell_yesterday", (*stg_itor)->getStgPositionASellYesterday(), allocator);
						create_info_object.AddMember("lots", (*stg_itor)->getStgLots(), allocator);
						create_info_object.AddMember("a_wait_price_tick", (*stg_itor)->getStgAWaitPriceTick(), allocator);
						create_info_object.AddMember("b_wait_price_tick", (*stg_itor)->getStgBWaitPriceTick(), allocator);
						//create_info_object.AddMember("StrategyOnoff", (*stg_itor)->getOn_Off(), allocator);

						create_info_array.PushBack(create_info_object, allocator);

						bFind = true;
					}
				}

				if (bFind) {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
				}
				else {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "未找到该策略昨仓信息", allocator);
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

			}
			else if (msgtype == 11) { //查询算法
				std::cout << "\t请求查询算法..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;

				list<Algorithm *> l_alg;
				static_dbm->getAllAlgorithm(&l_alg);

				/*构建MarketInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 11, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("MsgResult", 0, allocator);
				build_doc.AddMember("MsgErrorReason", "", allocator);
				//创建Info数组
				rapidjson::Value info_array(rapidjson::kArrayType);

				list<Algorithm *>::iterator alg_itor;
				for (alg_itor = l_alg.begin(); alg_itor != l_alg.end(); alg_itor++) {

					rapidjson::Value info_object(rapidjson::kObjectType);
					info_object.SetObject();
					info_object.AddMember("name", rapidjson::StringRef((*alg_itor)->getAlgName().c_str()), allocator);
					info_array.PushBack(info_object, allocator);
				}

				build_doc.AddMember("Info", info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 12) { // 修改Strategy（修改Strategy参数,带修改持仓信息）
				std::cout << "\t请求策略持仓修改..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];
				rapidjson::Value &infoArray = doc["Info"];
				rapidjson::Value &StrategyID = doc["StrategyID"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();
				string s_UserID = UserID.GetString();
				string s_StrategyID = StrategyID.GetString();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 12, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);
				build_doc.AddMember("StrategyID", rapidjson::StringRef(s_StrategyID.c_str()), allocator);
				
				
				// 是否找到要修改的策略
				bool isFindStrategy = false;
				bool isStgTradeTasking = false;
				bool isPositionMeetChanged = true;
				
				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				/*1:进行策略的修改,更新到数据库*/
				if (infoArray.IsArray()) {
					USER_PRINT("infoArray.IsArray()");
					for (int i = 0; i < infoArray.Size(); i++) {
						const Value& object = infoArray[i];
						std::string q_user_id = object["user_id"].GetString();
						std::string q_strategy_id = object["strategy_id"].GetString();
						std::cout << "q_user_id = " << q_user_id << std::endl;
						std::cout << "q_strategy_id = " << q_strategy_id << std::endl;
						list<Strategy *>::iterator stg_itor;

						if (!isFindStrategy) // 没找到对应的策略
						{
							for (stg_itor = ctp_m->getListStrategy()->begin(); stg_itor != ctp_m->getListStrategy()->end(); stg_itor++) {
								std::cout << "\t(*stg_itor)->getStgUserId() = " << (*stg_itor)->getStgUserId() << std::endl;
								std::cout << "\t(*stg_itor)->getStgStrategyId() = " << (*stg_itor)->getStgStrategyId() << std::endl;
								if (((*stg_itor)->getStgUserId() == q_user_id) && ((*stg_itor)->getStgStrategyId() == q_strategy_id)) {
									std::cout << "\t找到即将修改的Strategy" << std::endl;
									isFindStrategy = true;
									USER_PRINT("判断修改持仓...");
									/// 交易执行中无法修改持仓
									if ((*stg_itor)->isStgTradeTasking())
									{
										USER_PRINT("处于交易中...");
										Utils::printRedColor("处于交易中无法修改持仓");
										isStgTradeTasking = true;
										break;
									}

									// 如果持仓量有小于0的置为0
									//(*stg_itor)->calibrate_position();

									/// 判断仓位数量,界面发送的修改数量大于系统中的数量,判断为出错
									if ((((*stg_itor)->getStgPositionABuy() < object["position_a_buy"].GetInt()) && (object["position_a_buy"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionABuyYesterday() < object["position_a_buy_yesterday"].GetInt()) && (object["position_a_buy"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionABuyToday() < object["position_a_buy_today"].GetInt()) && (object["position_a_buy_today"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionBBuy() < object["position_b_buy"].GetInt()) && (object["position_b_buy"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionBBuyToday() < object["position_b_buy_today"].GetInt()) && (object["position_b_buy_today"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionBBuyYesterday() < object["position_b_buy_yesterday"].GetInt()) && (object["position_b_buy_yesterday"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionASell() < object["position_a_sell"].GetInt()) && (object["position_a_sell"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionASellToday() < object["position_a_sell_today"].GetInt()) && (object["position_a_sell_today"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionASellYesterday() < object["position_a_sell_yesterday"].GetInt()) && (object["position_a_sell_yesterday"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionBSell() < object["position_b_sell"].GetInt()) && (object["position_b_sell"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionBSellToday() < object["position_b_sell_today"].GetInt()) && (object["position_b_sell_today"].GetInt() != 0))
										|| (((*stg_itor)->getStgPositionBSellYesterday() < object["position_b_sell_yesterday"].GetInt())) && (object["position_b_sell_yesterday"].GetInt() != 0))
									{
										USER_PRINT("界面发送的修改数量大于系统中的数量,出错...");
										Utils::printRedColor("界面发送的修改数量大于系统中的数量,出错...");
										isPositionMeetChanged = false;
										break;
									}

									list<USER_CThostFtdcOrderField *>::iterator position_itor;			//order持仓明细
									list<USER_CThostFtdcTradeField *>::iterator position_trade_itor;	//trade持仓明细
									
									std::cout << "\t修改之前的仓位:" << std::endl;
									std::cout << "\tA卖(" << (*stg_itor)->getStgPositionASell() << ", " << (*stg_itor)->getStgPositionASellYesterday() << ")" << std::endl;
									std::cout << "\tB买(" << (*stg_itor)->getStgPositionBBuy() << ", " << (*stg_itor)->getStgPositionBBuyYesterday() << ")" << std::endl;
									std::cout << "\tA买(" << (*stg_itor)->getStgPositionABuy() << ", " << (*stg_itor)->getStgPositionABuyYesterday() << ")" << std::endl;
									std::cout << "\tB卖(" << (*stg_itor)->getStgPositionBSell() << ", " << (*stg_itor)->getStgPositionBSellYesterday() << ")" << std::endl;

									std::cout << "\t构造假的order,trade平仓单" << std::endl;
									//A昨卖
									Utils::printGreenColor("A昨卖");
									// Order
									USER_CThostFtdcOrderField *order_ASellYesterdayClose = new USER_CThostFtdcOrderField();
									(*stg_itor)->createFakeOrderPositionDetail(order_ASellYesterdayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdA(), '1', '0', '4',
										(*stg_itor)->getStgPositionASellYesterday() - object["position_a_sell_yesterday"].GetInt());

									// Trade
									USER_CThostFtdcTradeField *trade_ASellYesterdayClose = new USER_CThostFtdcTradeField();
									(*stg_itor)->createFakeTradePositionDetail(trade_ASellYesterdayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdA(), '1', '0', '4',
										(*stg_itor)->getStgPositionASellYesterday() - object["position_a_sell_yesterday"].GetInt());

									(*stg_itor)->update_position_detail(order_ASellYesterdayClose);
									(*stg_itor)->update_position_detail(trade_ASellYesterdayClose);

									//A昨买
									Utils::printGreenColor("A昨买");
									// Order
									USER_CThostFtdcOrderField *order_ABuyYesterdayClose = new USER_CThostFtdcOrderField();
									(*stg_itor)->createFakeOrderPositionDetail(order_ABuyYesterdayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdA(), '1', '1', '4',
										(*stg_itor)->getStgPositionABuyYesterday() - object["position_a_buy_yesterday"].GetInt());

									// Trade
									USER_CThostFtdcTradeField *trade_ABuyYesterdayClose = new USER_CThostFtdcTradeField();
									(*stg_itor)->createFakeTradePositionDetail(trade_ABuyYesterdayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdA(), '1', '1', '4',
										(*stg_itor)->getStgPositionABuyYesterday() - object["position_a_buy_yesterday"].GetInt());

									(*stg_itor)->update_position_detail(order_ABuyYesterdayClose);
									(*stg_itor)->update_position_detail(trade_ABuyYesterdayClose);

									//B昨卖
									Utils::printGreenColor("B昨卖");
									// Order
									USER_CThostFtdcOrderField *order_BSellYesterdayClose = new USER_CThostFtdcOrderField();
									(*stg_itor)->createFakeOrderPositionDetail(order_BSellYesterdayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdB(), '1', '0', '4',
										(*stg_itor)->getStgPositionBSellYesterday() - object["position_b_sell_yesterday"].GetInt());

									// Trade
									USER_CThostFtdcTradeField *trade_BSellYesterdayClose = new USER_CThostFtdcTradeField();
									(*stg_itor)->createFakeTradePositionDetail(trade_BSellYesterdayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdB(), '1', '0', '4',
										(*stg_itor)->getStgPositionBSellYesterday() - object["position_b_sell_yesterday"].GetInt());

									(*stg_itor)->update_position_detail(order_BSellYesterdayClose);
									(*stg_itor)->update_position_detail(trade_BSellYesterdayClose);


									//B昨买
									Utils::printGreenColor("B昨买");
									USER_CThostFtdcOrderField *order_BBuyYesterdayClose = new USER_CThostFtdcOrderField();
									(*stg_itor)->createFakeOrderPositionDetail(order_BBuyYesterdayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdB(), '1', '1', '4',
										(*stg_itor)->getStgPositionBBuyYesterday() - object["position_b_buy_yesterday"].GetInt());

									// Trade
									USER_CThostFtdcTradeField *trade_BBuyYesterdayClose = new USER_CThostFtdcTradeField();
									(*stg_itor)->createFakeTradePositionDetail(trade_BBuyYesterdayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdB(), '1', '1', '4',
										(*stg_itor)->getStgPositionBBuyYesterday() - object["position_b_buy_yesterday"].GetInt());

									(*stg_itor)->update_position_detail(order_BBuyYesterdayClose);
									(*stg_itor)->update_position_detail(trade_BBuyYesterdayClose);


									//A今卖
									Utils::printGreenColor("A今卖");
									// Order
									USER_CThostFtdcOrderField *order_ASellTodayClose = new USER_CThostFtdcOrderField();
									(*stg_itor)->createFakeOrderPositionDetail(order_ASellTodayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdA(), '1', '0', '3',
										(*stg_itor)->getStgPositionASellToday() - object["position_a_sell_today"].GetInt());

									// Trade
									USER_CThostFtdcTradeField *trade_ASellTodayClose = new USER_CThostFtdcTradeField();
									(*stg_itor)->createFakeTradePositionDetail(trade_ASellTodayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdA(), '1', '0', '3',
										(*stg_itor)->getStgPositionASellToday() - object["position_a_sell_today"].GetInt());

									(*stg_itor)->update_position_detail(order_ASellTodayClose);
									(*stg_itor)->update_position_detail(trade_ASellTodayClose);

									//A今买
									Utils::printGreenColor("A今买");
									// Order
									USER_CThostFtdcOrderField *order_ABuyTodayClose = new USER_CThostFtdcOrderField();
									(*stg_itor)->createFakeOrderPositionDetail(order_ABuyTodayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdA(), '1', '1', '3',
										(*stg_itor)->getStgPositionABuyToday() - object["position_a_buy_today"].GetInt());

									// Trade
									USER_CThostFtdcTradeField *trade_ABuyTodayClose = new USER_CThostFtdcTradeField();
									(*stg_itor)->createFakeTradePositionDetail(trade_ABuyTodayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdA(), '1', '1', '3',
										(*stg_itor)->getStgPositionABuyToday() - object["position_a_buy_today"].GetInt());

									(*stg_itor)->update_position_detail(order_ABuyTodayClose);
									(*stg_itor)->update_position_detail(trade_ABuyTodayClose);

									//B今卖
									Utils::printGreenColor("B今卖");
									// Order
									USER_CThostFtdcOrderField *order_BSellTodayClose = new USER_CThostFtdcOrderField();
									(*stg_itor)->createFakeOrderPositionDetail(order_BSellTodayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdB(), '1', '0', '3',
										(*stg_itor)->getStgPositionBSellToday() - object["position_b_sell_today"].GetInt());

									// Trade
									USER_CThostFtdcTradeField *trade_BSellTodayClose = new USER_CThostFtdcTradeField();
									(*stg_itor)->createFakeTradePositionDetail(trade_BSellTodayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdB(), '1', '0', '3',
										(*stg_itor)->getStgPositionBSellToday() - object["position_b_sell_today"].GetInt());

									(*stg_itor)->update_position_detail(order_BSellTodayClose);
									(*stg_itor)->update_position_detail(trade_BSellTodayClose);

									//B今买
									Utils::printGreenColor("B今买");
									// Order
									USER_CThostFtdcOrderField *order_BBuyTodayClose = new USER_CThostFtdcOrderField();
									(*stg_itor)->createFakeOrderPositionDetail(order_BBuyTodayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdB(), '1', '1', '3',
										(*stg_itor)->getStgPositionBBuyToday() - object["position_b_buy_today"].GetInt());

									// Trade
									USER_CThostFtdcTradeField *trade_BBuyTodayClose = new USER_CThostFtdcTradeField();
									(*stg_itor)->createFakeTradePositionDetail(trade_BBuyTodayClose, ctp_m->getTradingDay(),
										(*stg_itor)->getStgInstrumentIdB(), '1', '1', '3',
										(*stg_itor)->getStgPositionBBuyToday() - object["position_b_buy_today"].GetInt());

									(*stg_itor)->update_position_detail(order_BBuyTodayClose);
									(*stg_itor)->update_position_detail(trade_BBuyTodayClose);

									// 赋值
									(*stg_itor)->setStgPositionABuy(object["position_a_buy"].GetInt());
									(*stg_itor)->setStgPositionABuyYesterday(object["position_a_buy_yesterday"].GetInt());
									(*stg_itor)->setStgPositionABuyToday(object["position_a_buy_today"].GetInt());
									(*stg_itor)->setStgPositionBBuy(object["position_b_buy"].GetInt());
									(*stg_itor)->setStgPositionBBuyYesterday(object["position_b_buy_yesterday"].GetInt());
									(*stg_itor)->setStgPositionBBuyToday(object["position_b_buy_today"].GetInt());
									(*stg_itor)->setStgPositionASell(object["position_a_sell"].GetInt());
									(*stg_itor)->setStgPositionASellYesterday(object["position_a_sell_yesterday"].GetInt());
									(*stg_itor)->setStgPositionASellToday(object["position_a_sell_today"].GetInt());
									(*stg_itor)->setStgPositionBSell(object["position_b_sell"].GetInt());
									(*stg_itor)->setStgPositionBSellYesterday(object["position_b_sell_yesterday"].GetInt());
									(*stg_itor)->setStgPositionBSellToday(object["position_b_sell_today"].GetInt());


									std::cout << "\t修改之后的仓位:" << std::endl;
									std::cout << "\tA卖(" << (*stg_itor)->getStgPositionASell() << ", " << (*stg_itor)->getStgPositionASellYesterday() << ")" << std::endl;
									std::cout << "\tB买(" << (*stg_itor)->getStgPositionBBuy() << ", " << (*stg_itor)->getStgPositionBBuyYesterday() << ")" << std::endl;
									std::cout << "\tA买(" << (*stg_itor)->getStgPositionABuy() << ", " << (*stg_itor)->getStgPositionABuyYesterday() << ")" << std::endl;
									std::cout << "\tB卖(" << (*stg_itor)->getStgPositionBSell() << ", " << (*stg_itor)->getStgPositionBSellYesterday() << ")" << std::endl;

									// 最新修改时间
									(*stg_itor)->setStgUpdatePositionDetailRecordTime(Utils::getDate());
									(*stg_itor)->setStgIsPositionRight(false);

									//static_dbm->UpdateStrategy((*stg_itor));
									// 存储之前先清空集合
									//ctp_m->getDBManager()->DropPositionDetail();
									//ctp_m->getDBManager()->DropPositionDetailTrade();
									//ctp_m->saveStrategyPositionDetail((*stg_itor));
									//ctp_m->saveAllStrategyPositionDetail();
									/************************************************************************/
									/* 删除今日持仓明细(order,trade)，今日持仓明细changed(order,trade)
									   保存修改的策略到今日持仓明细，今日持仓明细changed*/
									/************************************************************************/
									ctp_m->getDBManager()->DeletePositionDetailByStrategy((*stg_itor));
									ctp_m->getDBManager()->DeletePositionDetailChangedByStrategy((*stg_itor));

									ctp_m->saveStrategyPositionDetail((*stg_itor));
									ctp_m->saveStrategyChangedPositionDetail((*stg_itor));

									/************************************************************************/
									/* 校准仓位之后,更新所有的运行标志位
									a:期货账户开关打开
									b:更新交易执行状态标志位task_status
									c:更新tick锁stg_select_order_algorithm_flag为释放*/
									/************************************************************************/
									(*stg_itor)->getStgUser()->setOn_Off(1);
									(*stg_itor)->setStgTradeTaskingRecovery();
									(*stg_itor)->setStgSelectOrderAlgorithmFlag("CTP_Manager::HandleMessage() msgtype == 12", false);

									std::cout << "\tStrategy修改持仓完成!" << std::endl;

									/*构造内容json*/
									rapidjson::Value create_info_object(rapidjson::kObjectType);
									create_info_object.SetObject();

									create_info_object.AddMember("trader_id", rapidjson::StringRef((*stg_itor)->getStgTraderId().c_str()), allocator);
									create_info_object.AddMember("user_id", rapidjson::StringRef((*stg_itor)->getStgUserId().c_str()), allocator);
									create_info_object.AddMember("strategy_id", rapidjson::StringRef((*stg_itor)->getStgStrategyId().c_str()), allocator);
									create_info_object.AddMember("position_a_buy", (*stg_itor)->getStgPositionABuy(), allocator);
									create_info_object.AddMember("position_a_buy_today", (*stg_itor)->getStgPositionABuyToday(), allocator);
									create_info_object.AddMember("position_a_buy_yesterday", (*stg_itor)->getStgPositionABuyYesterday(), allocator);
									create_info_object.AddMember("position_a_sell", (*stg_itor)->getStgPositionASell(), allocator);
									create_info_object.AddMember("position_a_sell_today", (*stg_itor)->getStgPositionASellToday(), allocator);
									create_info_object.AddMember("position_a_sell_yesterday", (*stg_itor)->getStgPositionASellYesterday(), allocator);
									create_info_object.AddMember("position_b_buy", (*stg_itor)->getStgPositionBBuy(), allocator);
									create_info_object.AddMember("position_b_buy_today", (*stg_itor)->getStgPositionBBuyToday(), allocator);
									create_info_object.AddMember("position_b_buy_yesterday", (*stg_itor)->getStgPositionBBuyYesterday(), allocator);
									create_info_object.AddMember("position_b_sell", (*stg_itor)->getStgPositionBSell(), allocator);
									create_info_object.AddMember("position_b_sell_today", (*stg_itor)->getStgPositionBSellToday(), allocator);
									create_info_object.AddMember("position_b_sell_yesterday", (*stg_itor)->getStgPositionBSellYesterday(), allocator);

									create_info_array.PushBack(create_info_object, allocator);

									//析构
									delete order_ASellYesterdayClose;
									delete trade_ASellYesterdayClose;
									delete order_ABuyYesterdayClose;
									delete trade_ABuyYesterdayClose;

									delete order_BSellYesterdayClose;
									delete trade_BSellYesterdayClose;
									delete order_BBuyYesterdayClose;
									delete trade_BBuyYesterdayClose;

									delete order_ASellTodayClose;
									delete trade_ASellTodayClose;
									delete order_ABuyTodayClose;
									delete trade_ABuyTodayClose;

									delete order_BSellTodayClose;
									delete trade_BSellTodayClose;
									delete order_BBuyTodayClose;
									delete trade_BBuyTodayClose;

								}
								else {
									std::cout << "\t未能找到修改的Strategy" << std::endl;
								}
							}
						}
					}
				}
				else {
					USER_PRINT("infoArray.Is Not Array()");
					std::cout << "\t未收到修改策略信息" << std::endl;
				}
				
				/// 如果找到修改的策略
				if (isFindStrategy) {
					if (isStgTradeTasking) //策略正在交易
					{
						build_doc.AddMember("MsgResult", 1, allocator);
						build_doc.AddMember("MsgErrorReason", "策略正在交易,无法修改持仓!", allocator);
					}
					else {

						if (!isPositionMeetChanged) //数量不满足
						{
							build_doc.AddMember("MsgResult", 1, allocator);
							build_doc.AddMember("MsgErrorReason", "修改持仓数量大于当前持仓数量!", allocator);
						}
						else {
							build_doc.AddMember("MsgResult", 0, allocator);
							build_doc.AddMember("MsgErrorReason", "", allocator);
						}
					}
				}
				else {
					build_doc.AddMember("MsgResult", 1, allocator);
					build_doc.AddMember("MsgErrorReason", "未能找到修改的Strategy!", allocator);
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);

			}
			else if (msgtype == 13) { // 策略开关
				std::cout << "\t策略开关..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];
				rapidjson::Value &StrategyID = doc["StrategyID"];
				rapidjson::Value &OnOff = doc["OnOff"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();
				string s_UserID = UserID.GetString();
				string s_StrategyID = StrategyID.GetString();
				int i_OnOff = OnOff.GetInt();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 13, allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("StrategyID", rapidjson::StringRef(s_StrategyID.c_str()), allocator);
				build_doc.AddMember("OnOff", i_OnOff, allocator);

				/*1:进行策略的删除,更新到数据库*/
				std::cout << "ctp_m->getListStrategy() size() = " << ctp_m->getListStrategy()->size() << std::endl;
				list<Strategy *>::iterator stg_itor;
				for (stg_itor = ctp_m->getListStrategy()->begin(); stg_itor != ctp_m->getListStrategy()->end();) {

					std::cout << "(*stg_itor)->getStgUserId() = " << (*stg_itor)->getStgUserId() << std::endl;
					std::cout << "(*stg_itor)->getStgStrategyId() = " << (*stg_itor)->getStgStrategyId() << std::endl;

					if (((*stg_itor)->getStgUserId() == s_UserID) && ((*stg_itor)->getStgStrategyId() == s_StrategyID)) {

						std::cout << "找到即将更新的Strategy" << std::endl;

						(*stg_itor)->setOn_Off(i_OnOff);

						int flag = static_dbm->UpdateStrategyOnOff((*stg_itor));

						if (flag) {
							std::cout << "策略未找到!" << std::endl;
							build_doc.AddMember("MsgResult", 1, allocator);
							build_doc.AddMember("MsgErrorReason", "未找到删除的策略!", allocator);

						}
						else {
							std::cout << "策略开关更新完成!" << std::endl;
							build_doc.AddMember("MsgResult", 0, allocator);
							build_doc.AddMember("MsgErrorReason", "", allocator);

						}
						/*delete (*stg_itor);
						stg_itor = ctp_m->getListStrategy()->erase(stg_itor);*/
						break;
					}
					else {
						std::cout << "未能找到修改的Strategy" << std::endl;
						stg_itor++;
					}
				}

				//build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 14) {// 策略只平开关
				std::cout << "\t策略只平开关..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];
				rapidjson::Value &StrategyID = doc["StrategyID"];
				rapidjson::Value &OnOff = doc["OnOff"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();
				string s_UserID = UserID.GetString();
				string s_StrategyID = StrategyID.GetString();
				int i_OnOff = OnOff.GetInt();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 14, allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("StrategyID", rapidjson::StringRef(s_StrategyID.c_str()), allocator);
				build_doc.AddMember("OnOff", i_OnOff, allocator);

				/*1:进行策略的删除,更新到数据库*/
				std::cout << "ctp_m->getListStrategy() size() = " << ctp_m->getListStrategy()->size() << std::endl;
				list<Strategy *>::iterator stg_itor;
				for (stg_itor = ctp_m->getListStrategy()->begin(); stg_itor != ctp_m->getListStrategy()->end();) {
					std::cout << "(*stg_itor)->getStgUserId() = " << (*stg_itor)->getStgUserId() << std::endl;
					std::cout << "(*stg_itor)->getStgStrategyId() = " << (*stg_itor)->getStgStrategyId() << std::endl;
					if (((*stg_itor)->getStgUserId() == s_UserID) && ((*stg_itor)->getStgStrategyId() == s_StrategyID)) {
						
						std::cout << "找到即将更新的Strategy" << std::endl;

						(*stg_itor)->setStgOnlyClose(i_OnOff);

						int flag = static_dbm->UpdateStrategyOnlyCloseOnOff((*stg_itor));

						if (flag) {
							std::cout << "策略未找到!" << std::endl;
							build_doc.AddMember("MsgResult", 1, allocator);
							build_doc.AddMember("MsgErrorReason", "未找到删除的策略!", allocator);

						}
						else {
							std::cout << "策略开关更新完成!" << std::endl;
							build_doc.AddMember("MsgResult", 0, allocator);
							build_doc.AddMember("MsgErrorReason", "", allocator);

						}
						/*delete (*stg_itor);
						stg_itor = ctp_m->getListStrategy()->erase(stg_itor);*/
						break;
					}
					else {
						std::cout << "未能找到修改的Strategy" << std::endl;
						stg_itor++;
					}
				}

				//build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 15) { // 查询期货账户昨日持仓明细
				std::cout << "\t查询期货账户昨日持仓明细..." << std::endl;
				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];

				string s_TraderID = TraderID.GetString();
				string s_UserID = UserID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				bool bFind = false;

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				list<USER_CThostFtdcOrderField *> l_posd;
				static_dbm->getAllPositionDetailYesterday(&l_posd, s_TraderID, s_UserID);

				/*构建策略昨仓Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 15, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);

				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				list<USER_CThostFtdcOrderField *>::iterator pod_itor;
				for (pod_itor = l_posd.begin(); pod_itor != l_posd.end(); pod_itor++) {

					/*构造内容json*/
					rapidjson::Value create_info_object(rapidjson::kObjectType);
					create_info_object.SetObject();

					create_info_object.AddMember("InstrumentID", rapidjson::StringRef((*pod_itor)->InstrumentID), allocator);
					create_info_object.AddMember("OrderRef", rapidjson::StringRef((*pod_itor)->OrderRef), allocator);
					create_info_object.AddMember("UserID", rapidjson::StringRef((*pod_itor)->UserID), allocator);
					//create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);

					/*string direction;
					direction = (*pod_itor)->Direction;*/
					create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);

					/*string CombOffsetFlag;
					CombOffsetFlag = (*pod_itor)->CombOffsetFlag[0];*/
					create_info_object.AddMember("CombOffsetFlag", (*pod_itor)->CombOffsetFlag[0], allocator);

					/*string CombHedgeFlag;
					CombHedgeFlag = (*pod_itor)->CombHedgeFlag[0];*/
					create_info_object.AddMember("CombHedgeFlag", (*pod_itor)->CombHedgeFlag[0], allocator);

					create_info_object.AddMember("LimitPrice", (*pod_itor)->LimitPrice, allocator);
					create_info_object.AddMember("VolumeTotalOriginal", (*pod_itor)->VolumeTotalOriginal, allocator);
					create_info_object.AddMember("TradingDay", rapidjson::StringRef((*pod_itor)->TradingDay), allocator);
					create_info_object.AddMember("TradingDayRecord", rapidjson::StringRef((*pod_itor)->TradingDayRecord), allocator);

					/*string OrderStatus;
					OrderStatus = (*pod_itor)->OrderStatus;*/
					create_info_object.AddMember("OrderStatus", (*pod_itor)->OrderStatus, allocator);

					create_info_object.AddMember("VolumeTraded", (*pod_itor)->VolumeTraded, allocator);
					create_info_object.AddMember("VolumeTotal", (*pod_itor)->VolumeTotal, allocator);
					create_info_object.AddMember("InsertDate", rapidjson::StringRef((*pod_itor)->InsertDate), allocator);
					create_info_object.AddMember("InsertTime", rapidjson::StringRef((*pod_itor)->InsertTime), allocator);
					create_info_object.AddMember("StrategyID", rapidjson::StringRef((*pod_itor)->StrategyID), allocator);
					create_info_object.AddMember("VolumeTradedBatch", (*pod_itor)->VolumeTradedBatch, allocator);

					create_info_array.PushBack(create_info_object, allocator);

					bFind = true;
				}

				if (bFind) {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
				}
				else {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "未找到该策略昨仓明细(order)", allocator);
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 16) { // 查询服务端sessions
				std::cout << "\t请求服务端sessions信息..." << std::endl;
				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"]; 
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];

				string s_TraderID = TraderID.GetString();
				string s_UserID = UserID.GetString();

				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "收到UserID = " << s_UserID << std::endl;
				
				list<Session *>::iterator sid_itor;
				list<User *>::iterator user_itor;

				/*构建UserInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 16, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);
				build_doc.AddMember("MsgResult", 0, allocator);
				build_doc.AddMember("MsgErrorReason", "", allocator);
				//创建Info数组
				rapidjson::Value info_array(rapidjson::kArrayType);

				/// session维护，如果不是本交易日的session，就要删除
				if (s_UserID == "") {
					for (user_itor = ctp_m->getL_User()->begin(); user_itor != ctp_m->getL_User()->end();
						user_itor++) {
						USER_PRINT((*user_itor)->getUserID());
						for (sid_itor = (*user_itor)->getL_Sessions()->begin(); sid_itor != (*user_itor)->getL_Sessions()->end(); sid_itor++) {
							USER_PRINT("Get Sessions Info");
							rapidjson::Value info_object(rapidjson::kObjectType);
							info_object.SetObject();
							info_object.AddMember("userid", rapidjson::StringRef((*sid_itor)->getUserID().c_str()), allocator);
							info_object.AddMember("sessionid", (*sid_itor)->getSessionID(), allocator);
							info_object.AddMember("frontid", (*sid_itor)->getFrontID(), allocator);
							info_object.AddMember("trading_day", rapidjson::StringRef((*sid_itor)->getTradingDay().c_str()), allocator);
							info_array.PushBack(info_object, allocator);
						}
					}
				}
				else {
					for (user_itor = ctp_m->getL_User()->begin(); user_itor != ctp_m->getL_User()->end();
						user_itor++) {
						USER_PRINT((*user_itor)->getUserID());
						if (s_UserID == (*user_itor)->getUserID()) { // 如果用户名一致
							for (sid_itor = (*user_itor)->getL_Sessions()->begin(); sid_itor != (*user_itor)->getL_Sessions()->end(); sid_itor++) {
								USER_PRINT("Get Sessions Info");
								rapidjson::Value info_object(rapidjson::kObjectType);
								info_object.SetObject();
								info_object.AddMember("userid", rapidjson::StringRef((*sid_itor)->getUserID().c_str()), allocator);
								info_object.AddMember("sessionid", (*sid_itor)->getSessionID(), allocator);
								info_object.AddMember("frontid", (*sid_itor)->getFrontID(), allocator);
								info_object.AddMember("trading_day", rapidjson::StringRef((*sid_itor)->getTradingDay().c_str()), allocator);
								info_array.PushBack(info_object, allocator);
							}
						}
					}
				}
				

				build_doc.AddMember("Info", info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 17) { // 查询期货账户昨日持仓明细(trade)
				std::cout << "\t查询期货账户昨日持仓明细..." << std::endl;
				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];

				string s_TraderID = TraderID.GetString();
				string s_UserID = UserID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				bool bFind = false;

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				list<USER_CThostFtdcTradeField *> l_posd;
				static_dbm->getAllPositionDetailTradeYesterday(&l_posd, s_TraderID, s_UserID);

				/*构建策略昨仓Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 17, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);

				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				list<USER_CThostFtdcTradeField *>::iterator pod_itor;
				for (pod_itor = l_posd.begin(); pod_itor != l_posd.end(); pod_itor++) {

					/*构造内容json*/
					rapidjson::Value create_info_object(rapidjson::kObjectType);
					create_info_object.SetObject();

					create_info_object.AddMember("InstrumentID", rapidjson::StringRef((*pod_itor)->InstrumentID), allocator);
					create_info_object.AddMember("OrderRef", rapidjson::StringRef((*pod_itor)->OrderRef), allocator);
					create_info_object.AddMember("UserID", rapidjson::StringRef((*pod_itor)->UserID), allocator);

					/*string direction;
					if ((*pod_itor)->Direction == '0')
					{
					direction = "0";
					}
					else if ((*pod_itor)->Direction == '1')
					{
					direction = "1";
					}*/
					create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);

					/*string OffsetFlag;
					if ((*pod_itor)->OffsetFlag == '0')
					{
					OffsetFlag = "0";
					}
					else if ((*pod_itor)->OffsetFlag == '1')
					{
					OffsetFlag = "1";
					}
					else if ((*pod_itor)->OffsetFlag == '2')
					{
					OffsetFlag = "2";
					}
					else if ((*pod_itor)->OffsetFlag == '3')
					{
					OffsetFlag = "3";
					}
					else if ((*pod_itor)->OffsetFlag == '4')
					{
					OffsetFlag = "4";
					}
					else if ((*pod_itor)->OffsetFlag == '5')
					{
					OffsetFlag = "5";
					}
					else if ((*pod_itor)->OffsetFlag == '6')
					{
					OffsetFlag = "6";
					}*/
					create_info_object.AddMember("OffsetFlag", (*pod_itor)->OffsetFlag, allocator);

					/*string HedgeFlag;
					if ((*pod_itor)->HedgeFlag == '1')
					{
						HedgeFlag = "1";
					}
					else if ((*pod_itor)->HedgeFlag == '2')
					{
						HedgeFlag = "2";
					}
					else if ((*pod_itor)->HedgeFlag == '3')
					{
						HedgeFlag = "3";
					}
					else if ((*pod_itor)->HedgeFlag == '5')
					{
						HedgeFlag = "5";
					}*/
					create_info_object.AddMember("HedgeFlag", (*pod_itor)->HedgeFlag, allocator);

					//create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);
					//create_info_object.AddMember("OffsetFlag", (*pod_itor)->OffsetFlag, allocator);
					//create_info_object.AddMember("HedgeFlag", (*pod_itor)->HedgeFlag, allocator);
					create_info_object.AddMember("Price", (*pod_itor)->Price, allocator);
					create_info_object.AddMember("TradingDay", rapidjson::StringRef((*pod_itor)->TradingDay), allocator);
					create_info_object.AddMember("TradingDayRecord", rapidjson::StringRef((*pod_itor)->TradingDayRecord), allocator);
					create_info_object.AddMember("TradeDate", rapidjson::StringRef((*pod_itor)->TradeDate), allocator);
					create_info_object.AddMember("StrategyID", rapidjson::StringRef((*pod_itor)->StrategyID), allocator);
					create_info_object.AddMember("Volume", (*pod_itor)->Volume, allocator);

					create_info_array.PushBack(create_info_object, allocator);

					bFind = true;
				}

				if (bFind) {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
				}
				else {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "未找到该策略昨仓明细(trade)", allocator);
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 20) { // 查询期货账户今日持仓明细(order)
				std::cout << "\t查询期货账户今日持仓明细(order)..." << std::endl;
				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];

				string s_TraderID = TraderID.GetString();
				string s_UserID = UserID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				bool bFind = false;

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				list<USER_CThostFtdcOrderField *> l_posd;
				static_dbm->getAllPositionDetailChanged(&l_posd, s_TraderID, s_UserID);

				/*构建策略昨仓Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 20, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);

				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				list<USER_CThostFtdcOrderField *>::iterator pod_itor;
				for (pod_itor = l_posd.begin(); pod_itor != l_posd.end(); pod_itor++) {

					/*构造内容json*/
					rapidjson::Value create_info_object(rapidjson::kObjectType);
					create_info_object.SetObject();

					create_info_object.AddMember("InstrumentID", rapidjson::StringRef((*pod_itor)->InstrumentID), allocator);
					create_info_object.AddMember("OrderRef", rapidjson::StringRef((*pod_itor)->OrderRef), allocator);
					create_info_object.AddMember("UserID", rapidjson::StringRef((*pod_itor)->UserID), allocator);
					//create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);

					/*string direction;
					direction = (*pod_itor)->Direction;*/
					create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);

					/*string CombOffsetFlag;
					CombOffsetFlag = (*pod_itor)->CombOffsetFlag[0];*/
					create_info_object.AddMember("CombOffsetFlag", (*pod_itor)->CombOffsetFlag[0], allocator);

					/*string CombHedgeFlag;
					CombHedgeFlag = (*pod_itor)->CombHedgeFlag[0];*/
					create_info_object.AddMember("CombHedgeFlag", (*pod_itor)->CombHedgeFlag[0], allocator);

					create_info_object.AddMember("LimitPrice", (*pod_itor)->LimitPrice, allocator);
					create_info_object.AddMember("VolumeTotalOriginal", (*pod_itor)->VolumeTotalOriginal, allocator);
					create_info_object.AddMember("TradingDay", rapidjson::StringRef((*pod_itor)->TradingDay), allocator);
					create_info_object.AddMember("TradingDayRecord", rapidjson::StringRef((*pod_itor)->TradingDayRecord), allocator);

					/*string OrderStatus;
					OrderStatus = (*pod_itor)->OrderStatus;*/
					create_info_object.AddMember("OrderStatus", (*pod_itor)->OrderStatus, allocator);

					create_info_object.AddMember("VolumeTraded", (*pod_itor)->VolumeTraded, allocator);
					create_info_object.AddMember("VolumeTotal", (*pod_itor)->VolumeTotal, allocator);
					create_info_object.AddMember("InsertDate", rapidjson::StringRef((*pod_itor)->InsertDate), allocator);
					create_info_object.AddMember("InsertTime", rapidjson::StringRef((*pod_itor)->InsertTime), allocator);
					create_info_object.AddMember("StrategyID", rapidjson::StringRef((*pod_itor)->StrategyID), allocator);
					create_info_object.AddMember("VolumeTradedBatch", (*pod_itor)->VolumeTradedBatch, allocator);

					create_info_array.PushBack(create_info_object, allocator);

					bFind = true;
				}

				if (bFind) {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
				}
				else {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "未找到该策略昨仓明细(order)", allocator);
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 21) { // 查询期货账户今日持仓明细(trade)
				std::cout << "\t查询期货账户今日持仓明细(trade)..." << std::endl;
				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];

				string s_TraderID = TraderID.GetString();
				string s_UserID = UserID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();

				bool bFind = false;

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				list<USER_CThostFtdcTradeField *> l_posd;
				static_dbm->getAllPositionDetailTradeChanged(&l_posd, s_TraderID, s_UserID);

				/*构建策略昨仓Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 21, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);

				//创建Info数组
				rapidjson::Value create_info_array(rapidjson::kArrayType);

				list<USER_CThostFtdcTradeField *>::iterator pod_itor;
				for (pod_itor = l_posd.begin(); pod_itor != l_posd.end(); pod_itor++) {

					/*构造内容json*/
					rapidjson::Value create_info_object(rapidjson::kObjectType);
					create_info_object.SetObject();

					create_info_object.AddMember("InstrumentID", rapidjson::StringRef((*pod_itor)->InstrumentID), allocator);
					create_info_object.AddMember("OrderRef", rapidjson::StringRef((*pod_itor)->OrderRef), allocator);
					create_info_object.AddMember("UserID", rapidjson::StringRef((*pod_itor)->UserID), allocator);

					/*string direction;
					if ((*pod_itor)->Direction == '0')
					{
					direction = "0";
					}
					else if ((*pod_itor)->Direction == '1')
					{
					direction = "1";
					}*/
					create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);

					/*string OffsetFlag;
					if ((*pod_itor)->OffsetFlag == '0')
					{
					OffsetFlag = "0";
					}
					else if ((*pod_itor)->OffsetFlag == '1')
					{
					OffsetFlag = "1";
					}
					else if ((*pod_itor)->OffsetFlag == '2')
					{
					OffsetFlag = "2";
					}
					else if ((*pod_itor)->OffsetFlag == '3')
					{
					OffsetFlag = "3";
					}
					else if ((*pod_itor)->OffsetFlag == '4')
					{
					OffsetFlag = "4";
					}
					else if ((*pod_itor)->OffsetFlag == '5')
					{
					OffsetFlag = "5";
					}
					else if ((*pod_itor)->OffsetFlag == '6')
					{
					OffsetFlag = "6";
					}*/
					create_info_object.AddMember("OffsetFlag", (*pod_itor)->OffsetFlag, allocator);

					/*string HedgeFlag;
					if ((*pod_itor)->HedgeFlag == '1')
					{
					HedgeFlag = "1";
					}
					else if ((*pod_itor)->HedgeFlag == '2')
					{
					HedgeFlag = "2";
					}
					else if ((*pod_itor)->HedgeFlag == '3')
					{
					HedgeFlag = "3";
					}
					else if ((*pod_itor)->HedgeFlag == '5')
					{
					HedgeFlag = "5";
					}*/
					create_info_object.AddMember("HedgeFlag", (*pod_itor)->HedgeFlag, allocator);

					//create_info_object.AddMember("Direction", (*pod_itor)->Direction, allocator);
					//create_info_object.AddMember("OffsetFlag", (*pod_itor)->OffsetFlag, allocator);
					//create_info_object.AddMember("HedgeFlag", (*pod_itor)->HedgeFlag, allocator);
					create_info_object.AddMember("Price", (*pod_itor)->Price, allocator);
					create_info_object.AddMember("TradingDay", rapidjson::StringRef((*pod_itor)->TradingDay), allocator);
					create_info_object.AddMember("TradingDayRecord", rapidjson::StringRef((*pod_itor)->TradingDayRecord), allocator);
					create_info_object.AddMember("TradeDate", rapidjson::StringRef((*pod_itor)->TradeDate), allocator);
					create_info_object.AddMember("StrategyID", rapidjson::StringRef((*pod_itor)->StrategyID), allocator);
					create_info_object.AddMember("Volume", (*pod_itor)->Volume, allocator);

					create_info_array.PushBack(create_info_object, allocator);

					bFind = true;
				}

				if (bFind) {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
				}
				else {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "未找到该策略昨仓明细(trade)", allocator);
				}

				build_doc.AddMember("Info", create_info_array, allocator);
				build_doc.AddMember("MsgSrc", i_MsgSrc, allocator);
			}
			else if (msgtype == 22) { // 请求查询策略信息(单条策略信息)
				std::cout << "\t请求查询策略信息..." << std::endl;

				rapidjson::Value &MsgSendFlag = doc["MsgSendFlag"];
				rapidjson::Value &TraderID = doc["TraderID"];
				rapidjson::Value &MsgRef = doc["MsgRef"];
				rapidjson::Value &MsgSrc = doc["MsgSrc"];
				rapidjson::Value &UserID = doc["UserID"];
				rapidjson::Value &StrategyID = doc["StrategyID"];

				string s_TraderID = TraderID.GetString();
				int i_MsgRef = MsgRef.GetInt();
				int i_MsgSendFlag = MsgSendFlag.GetInt();
				int i_MsgSrc = MsgSrc.GetInt();
				string s_UserID = UserID.GetString();
				string s_StrategyID = StrategyID.GetString();

				std::cout << "\t收到交易员ID = " << s_TraderID << std::endl;
				std::cout << "\t收到期货账户ID = " << s_UserID << std::endl;

				//list<Strategy *> l_strategys;
				//static_dbm->getAllStrategy(&l_strategys, s_TraderID, s_UserID);
				//static_dbm->getAllStrategyByActiveUser(&l_strategys, ctp_m->getL_User(), s_TraderID);

				/*构建StrategyInfo的Json*/
				build_doc.SetObject();
				rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
				build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
				build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
				build_doc.AddMember("MsgType", 22, allocator);
				build_doc.AddMember("TraderID", rapidjson::StringRef(s_TraderID.c_str()), allocator);
				build_doc.AddMember("UserID", rapidjson::StringRef(s_UserID.c_str()), allocator);
				build_doc.AddMember("StrategyID", rapidjson::StringRef(s_StrategyID.c_str()), allocator);
				
				bool isFind = false; //是否找到修改的策略

				//创建Info数组
				rapidjson::Value info_array(rapidjson::kArrayType);

				list<Strategy *>::iterator stg_itor;
				for (stg_itor = ctp_m->getListStrategy()->begin(); stg_itor != ctp_m->getListStrategy()->end(); stg_itor++) {

					//策略id，期货账户id，交易员id均相同
					if (((*stg_itor)->getStgStrategyId() == s_StrategyID) &&
						((*stg_itor)->getStgUserId() == s_UserID) &&
						((*stg_itor)->getStgTraderId() == s_TraderID)) {
						// 找到对应的strategy
						isFind = true;

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
						info_object.AddMember("a_order_action_limit", (*stg_itor)->getStgAOrderActionTiresLimit(), allocator);
						info_object.AddMember("b_order_action_limit", (*stg_itor)->getStgBOrderActionTiresLimit(), allocator);
						info_object.AddMember("sell_close", (*stg_itor)->getStgSellClose(), allocator);
						info_object.AddMember("buy_open", (*stg_itor)->getStgBuyOpen(), allocator);

						/*开关字段*/
						info_object.AddMember("only_close", (*stg_itor)->isStgOnlyClose(), allocator);
						info_object.AddMember("strategy_on_off", (*stg_itor)->getOn_Off(), allocator);
						info_object.AddMember("sell_open_on_off", (*stg_itor)->getStgSellOpenOnOff(), allocator);
						info_object.AddMember("buy_close_on_off", (*stg_itor)->getStgBuyCloseOnOff(), allocator);
						info_object.AddMember("sell_close_on_off", (*stg_itor)->getStgSellCloseOnOff(), allocator);
						info_object.AddMember("buy_open_on_off", (*stg_itor)->getStgBuyOpenOnOff(), allocator);


						/*新增字段*/
						info_object.AddMember("trade_model", rapidjson::StringRef((*stg_itor)->getStgTradeModel().c_str()), allocator);
						info_object.AddMember("update_position_detail_record_time", rapidjson::StringRef((*stg_itor)->getStgUpdatePositionDetailRecordTime().c_str()), allocator);
						info_object.AddMember("hold_profit", (*stg_itor)->getStgHoldProfit(), allocator);
						info_object.AddMember("close_profit", (*stg_itor)->getStgCloseProfit(), allocator);
						info_object.AddMember("commission", (*stg_itor)->getStgCommission(), allocator);
						info_object.AddMember("position", (*stg_itor)->getStgPosition(), allocator);
						info_object.AddMember("position_buy", (*stg_itor)->getStgPositionBuy(), allocator);
						info_object.AddMember("position_sell", (*stg_itor)->getStgPositionSell(), allocator);
						info_object.AddMember("trade_volume", (*stg_itor)->getStgTradeVolume(), allocator);
						info_object.AddMember("amount", (*stg_itor)->getStgAmount(), allocator);
						info_object.AddMember("average_shift", (*stg_itor)->getStgAverageShift(), allocator);
						info_object.AddMember("a_limit_price_shift", (*stg_itor)->getStgALimitPriceShift(), allocator);
						info_object.AddMember("b_limit_price_shift", (*stg_itor)->getStgBLimitPriceShift(), allocator);

						/*2017.03.03新增参数*/
						info_object.AddMember("on_off", (*stg_itor)->getOn_Off(), allocator);
						info_object.AddMember("a_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdA().c_str()), allocator);
						info_object.AddMember("b_instrument_id", rapidjson::StringRef((*stg_itor)->getStgInstrumentIdB().c_str()), allocator);
						info_object.AddMember("a_limit_price_shift", (*stg_itor)->getStgALimitPriceShift(), allocator);
						info_object.AddMember("b_limit_price_shift", (*stg_itor)->getStgBLimitPriceShift(), allocator);
						info_object.AddMember("sell_open_on_off", (*stg_itor)->getStgSellOpenOnOff(), allocator);
						info_object.AddMember("buy_close_on_off", (*stg_itor)->getStgBuyCloseOnOff(), allocator);
						info_object.AddMember("buy_open_on_off", (*stg_itor)->getStgBuyOpenOnOff(), allocator);
						info_object.AddMember("sell_close_on_off", (*stg_itor)->getStgSellCloseOnOff(), allocator);


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
						//info_object.AddMember("StrategyOnoff", (*stg_itor)->getOn_Off(), allocator);

						info_array.PushBack(info_object, allocator);

						(*stg_itor)->printStrategyInfo("msgtype == 22");
						(*stg_itor)->printStrategyInfoPosition();
					}
					if (isFind)
					{
						break;
					}
				}

				if (isFind) {
					build_doc.AddMember("MsgResult", 0, allocator);
					build_doc.AddMember("MsgErrorReason", "", allocator);
				}
				else {
					build_doc.AddMember("MsgResult", 1, allocator);
					build_doc.AddMember("MsgErrorReason", "未找到该策略!", allocator);
				}
				
				build_doc.AddMember("Info", info_array, allocator);
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
				build_doc.AddMember("MsgSrc", 0, allocator);
			}
		}
	}
	
	
	if (global_msg_type != 1) {
		build_doc.Accept(writer);
		//rsp_msg = const_cast<char *>(buffer.GetString());
		//std::cout << "yyyyyyyyyyyyyyyyyyyyyyy" << std::endl;
		std::cout << "\t服务端响应数据 = " << buffer.GetString() << std::endl;

		if (write_msg(fd, const_cast<char *>(buffer.GetString()), strlen(buffer.GetString())) < 0) {
			printf("\t先前客户端已断开!!!\n");
			//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
			if (errno == EPIPE) {
				std::cout << "\tEPIPE信号" << std::endl;
				//break;
			}
			
			perror("\tprotocal error\n");
		}
	}
}

#if 0
/// 初始化昨仓
bool CTP_Manager::initYesterdayPosition() {
	bool flag = true;
	list<Strategy *>::iterator stg_itor;
	list<Strategy *>::iterator stg_itor_yesterday;
	std::cout << "系统交易日 = " << this->getTradingDay() << std::endl;

	for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) {
		// 遍历Strategy
		std::cout << "策略最后更新时间 = " << (*stg_itor)->getStgTradingDay() << std::endl;
		bool flag = Utils::compareTradingDay((*stg_itor)->getStgTradingDay().c_str(), this->getTradingDay().c_str());

		std::cout << "对比结果 = " << flag << std::endl;
		std::cout << "今仓 userid = " << (*stg_itor)->getStgUserId() << std::endl;
		std::cout << "今仓 strategy_id = " << (*stg_itor)->getStgStrategyId() << std::endl;

		if (flag == false) { // 如果时间晚于最新交易日，那么将策略新建到昨仓，并且更新仓位

			this->dbm->UpdateFutureAccountOrderRef((*stg_itor)->getStgUser(), "1000000001");

			std::cout << "时间晚于最新交易日，策略新建到昨仓，并且更新仓位" << std::endl;

			// 昨仓变量初始化给今仓
			(*stg_itor)->setStgPositionABuy((*stg_itor)->getStgPositionABuy());
			(*stg_itor)->setStgPositionABuyToday(0);
			(*stg_itor)->setStgPositionABuyYesterday((*stg_itor)->getStgPositionABuy());
			(*stg_itor)->setStgPositionASell((*stg_itor)->getStgPositionASell());
			(*stg_itor)->setStgPositionASellToday(0);
			(*stg_itor)->setStgPositionASellYesterday((*stg_itor)->getStgPositionASell());
			(*stg_itor)->setStgPositionBBuy((*stg_itor)->getStgPositionBBuy());
			(*stg_itor)->setStgPositionBBuyToday(0);
			(*stg_itor)->setStgPositionBBuyYesterday((*stg_itor)->getStgPositionBBuy());
			(*stg_itor)->setStgPositionBSell((*stg_itor)->getStgPositionBSell());
			(*stg_itor)->setStgPositionBSellToday(0);
			(*stg_itor)->setStgPositionBSellYesterday((*stg_itor)->getStgPositionBSell());

			// 更新今仓最新交易时间
			(*stg_itor)->setStgTradingDay(this->getTradingDay());

			// 更新今仓到数据库
			this->getDBManager()->UpdateStrategy((*stg_itor));

			if (this->l_strategys_yesterday->size() > 0) {
				for (stg_itor_yesterday = this->l_strategys_yesterday->begin(); stg_itor_yesterday != this->l_strategys_yesterday->end(); stg_itor_yesterday++) {
					// 遍历Strategy
					std::cout << "昨仓 userid = " << (*stg_itor_yesterday)->getStgUserId() << std::endl;
					std::cout << "昨仓 strategy_id = " << (*stg_itor_yesterday)->getStgStrategyId() << std::endl;

					if (((*stg_itor)->getStgUserId() == (*stg_itor_yesterday)->getStgUserId()) && ((*stg_itor)->getStgStrategyId() == (*stg_itor_yesterday)->getStgStrategyId())) {
						std::cout << "昨仓里找到相同的策略" << std::endl;

						// 删除昨仓
						this->getDBManager()->DeleteStrategyYesterday((*stg_itor_yesterday));

						// 将初始化的今仓新建为昨仓
						this->getDBManager()->CreateStrategyYesterday((*stg_itor));
					}
					else {
						//std::cout << "昨仓里 未 找到相同的策略" << std::endl;
						// 将初始化的今仓新建为昨仓
						this->getDBManager()->CreateStrategyYesterday((*stg_itor));
					}

				}
			}
			else {
				std::cout << "昨仓策略为空,将初始化今仓更新到昨仓" << std::endl;
				// 将初始化的今仓新建为昨仓
				this->getDBManager()->CreateStrategyYesterday((*stg_itor));
			}
			
		}
		else { // 如果时间等于最新交易日,从昨仓取出相同的进行初始化
			std::cout << "策略时间等于最新交易日,从昨仓取出相同的进行初始化" << std::endl;
			bool is_exists = false;
			if (this->l_strategys_yesterday->size() > 0) {
				for (stg_itor_yesterday = this->l_strategys_yesterday->begin(); stg_itor_yesterday != this->l_strategys_yesterday->end(); stg_itor_yesterday++) {
					// 遍历Strategy
					std::cout << "昨仓 userid = " << (*stg_itor_yesterday)->getStgUserId() << std::endl;
					std::cout << "昨仓 strategy_id = " << (*stg_itor_yesterday)->getStgStrategyId() << std::endl;

					if (((*stg_itor)->getStgUserId() == (*stg_itor_yesterday)->getStgUserId()) && ((*stg_itor)->getStgStrategyId() == (*stg_itor_yesterday)->getStgStrategyId())) {
						std::cout << "昨仓里找到相同的策略" << std::endl;

						// 昨仓变量初始化给今仓
						(*stg_itor)->setStgPositionABuy((*stg_itor_yesterday)->getStgPositionABuy());
						(*stg_itor)->setStgPositionABuyToday(0);
						(*stg_itor)->setStgPositionABuyYesterday((*stg_itor_yesterday)->getStgPositionABuy());
						(*stg_itor)->setStgPositionASell((*stg_itor_yesterday)->getStgPositionASell());
						(*stg_itor)->setStgPositionASellToday(0);
						(*stg_itor)->setStgPositionASellYesterday((*stg_itor_yesterday)->getStgPositionASell());
						(*stg_itor)->setStgPositionBBuy((*stg_itor_yesterday)->getStgPositionBBuy());
						(*stg_itor)->setStgPositionBBuyToday(0);
						(*stg_itor)->setStgPositionBBuyYesterday((*stg_itor_yesterday)->getStgPositionBBuy());
						(*stg_itor)->setStgPositionBSell((*stg_itor_yesterday)->getStgPositionBSell());
						(*stg_itor)->setStgPositionBSellToday(0);
						(*stg_itor)->setStgPositionBSellYesterday((*stg_itor_yesterday)->getStgPositionBSell());

						is_exists = true;
					}
					else {
						//std::cout << "循环中...昨仓里 未 找到相同的策略,初始化有误!" << std::endl;
					}

				}
				/// 是否找到昨仓里策略,未找到就出错
				if (!is_exists) {
					std::cout << "昨仓里 未 找到相同的策略,初始化有误!" << std::endl;
					flag = false;
					return flag;
				}
			}
			else {
				std::cout << "昨仓里 未 找到相同的策略,初始化有误!" << std::endl;
				flag = false;
				return flag;
			}
		}
	}

	return flag;
}

#endif

/// 初始化策略(策略更新trading_day, 期货账户更新order_ref)
bool CTP_Manager::initStrategyAndFutureAccount() {
	USER_PRINT("CTP_Manager::initStrategyAndFutureAccount()");
	std::cout << "CTP_Manager::initStrategyAndFutureAccount()" << std::endl;
	/// 如果ctp_manager交易日为空,说明未初始化完成
	if (this->trading_day == "")
	{
		std::cout << "\tctp_manager交易日为空,初始化失败!" << std::endl;
		this->setCTPFinishedPositionInit(false);
		return false;
	}
	else {
		std::cout << "\tctp_manager交易日 不 为空" << std::endl;
		if (this->getMdLogin()) {
			std::cout << "\t行情 已 登陆成功,初始化成功" << std::endl;
			this->setCTPFinishedPositionInit(true);
		}
		else {
			std::cout << "\t行情 未 登陆成功,初始化失败!" << std::endl;
			this->setCTPFinishedPositionInit(false);
		}
	}
	bool flag = true;
	bool is_position_right = true; //策略是否有过修改持仓,默认无,仓位都是正确的

	list<Strategy *>::iterator stg_itor;
	list<Strategy *>::iterator stg_itor_yesterday;
	list<USER_CThostFtdcOrderField *>::iterator position_itor;
	list<USER_CThostFtdcTradeField *>::iterator position_trade_itor;

	std::cout << "\tCTP_Manager 系统交易日 = " << this->getTradingDay() << std::endl;
	// 判断策略
	bool is_equal = false;
	for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) {
		// 遍历Strategy
		std::cout << "\t策略最后更新时间 = " << (*stg_itor)->getStgTradingDay() << std::endl;
		
		is_equal = Utils::compareTradingDay((*stg_itor)->getStgTradingDay().c_str(), this->getTradingDay().c_str());

		std::cout << "\t对比结果 = " << is_equal << std::endl;
		std::cout << "\t今仓 userid = " << (*stg_itor)->getStgUserId() << std::endl;
		std::cout << "\t今仓 strategy_id = " << (*stg_itor)->getStgStrategyId() << std::endl;

		/************************************************************************/
		/* 判断持仓明细的trading_day
			1:如果trading_day等于CTP系统交易日,那么持仓明细已经更新过了，毋需更新
			2:如果trading_day小于CTP系统交易日,那么过期的持仓明细进行更新，将过期的今仓数据更新为昨仓数据，并且更新trading_day*/
		/************************************************************************/

		if (is_equal == false) { // 如果时间晚于最新交易日，那么将策略新建到昨仓，并且更新仓位

			std::cout << "\t时间晚于最新交易日，更新策略持仓变量,昨仓变量初始化" << std::endl;
			// 上个交易日的今仓作为当天交易日的昨仓,当天交易日的今仓全部初始化为0
			(*stg_itor)->setStgPositionABuy((*stg_itor)->getStgPositionABuy());
			(*stg_itor)->setStgPositionABuyToday(0);
			(*stg_itor)->setStgPositionABuyYesterday((*stg_itor)->getStgPositionABuy());
			(*stg_itor)->setStgPositionASell((*stg_itor)->getStgPositionASell());
			(*stg_itor)->setStgPositionASellToday(0);
			(*stg_itor)->setStgPositionASellYesterday((*stg_itor)->getStgPositionASell());
			(*stg_itor)->setStgPositionBBuy((*stg_itor)->getStgPositionBBuy());
			(*stg_itor)->setStgPositionBBuyToday(0);
			(*stg_itor)->setStgPositionBBuyYesterday((*stg_itor)->getStgPositionBBuy());
			(*stg_itor)->setStgPositionBSell((*stg_itor)->getStgPositionBSell());
			(*stg_itor)->setStgPositionBSellToday(0);
			(*stg_itor)->setStgPositionBSellYesterday((*stg_itor)->getStgPositionBSell());

			
			// 新的交易日，把曾经的记录时间清掉
			(*stg_itor)->setStgUpdatePositionDetailRecordTime("");
			(*stg_itor)->setStgLastSavedTime("");
			this->getDBManager()->DeletePositionDetailChangedByStrategy((*stg_itor));
			(*stg_itor)->setStgIsPositionRight(true);

			// 更新今仓最新交易时间
			(*stg_itor)->setStgTradingDay(this->getTradingDay());
			// 更新今仓到数据库
			this->dbm->UpdateStrategy((*stg_itor));
			// 更新对应期货账户报单引用
			this->dbm->UpdateFutureAccountOrderRef(this->dbm->getConn(), (*stg_itor)->getStgUser(), "1000000001");

		} else { // 如果时间等于最新交易日,说明今天已经更新过策略
			std::cout << "\t策略时间等于最新交易日,无需初始化" << std::endl;
		}
	}

	/***************************Order**********************************/
	/* 遍历今仓列表(order),对比TradingDayRecord,
	a:小于本交易日TradingDay,覆盖到昨仓持仓明细列表
	b:等于本交易日,直接从昨仓持仓明细列表进行初始化*/
	/************************************************************************/
	//是否拥有昨持仓明细，默认为false
	bool is_own_yesterday_order_position_detail = false;

	if (this->l_posdetail->size() > 0) { // 如果今持仓明细有记录
		std::cout << "\tCTP_Manager::initStrategyAndFutureAccount() 今持仓明细有记录(order)" << std::endl;
		for (position_itor = this->l_posdetail->begin(); position_itor != this->l_posdetail->end(); position_itor++) { // 遍历今持仓明细列表
			std::cout << "\t(*position_itor)->TradingDayRecord = " << (*position_itor)->TradingDayRecord << std::endl;
			//日期不等,说明存在昨仓持仓明细
			if (strcmp((*position_itor)->TradingDayRecord, this->getTradingDay().c_str())) {
				is_own_yesterday_order_position_detail = true;
			}
		}
	}

	if (is_own_yesterday_order_position_detail) { //当今持仓明细有数据记录时间与CTP TradingDay时间不相等，保存持仓明细到昨持仓明细，并且清空今持仓明细
		// 清空数据库
		this->dbm->DropPositionDetail();
		this->dbm->DropPositionDetailYesterday();

		for (position_itor = this->l_posdetail->begin(); position_itor != this->l_posdetail->end(); position_itor++) { // 遍历今持仓明细列表
			std::cout << "\t(*position_itor)->TradingDayRecord = " << (*position_itor)->TradingDayRecord << std::endl;
			//日期不等
			if (strcmp((*position_itor)->TradingDayRecord, this->getTradingDay().c_str())) {
				// 删除昨持仓明细对象,如果存在就删除,没有跳出
					//this->dbm->DeletePositionDetailYesterday((*position_itor));
				// 创建新的策略昨仓
				// 记录更新时间为本交易日
				strcpy((*position_itor)->TradingDayRecord, this->getTradingDay().c_str());
				this->dbm->CreatePositionDetailYesterday((*position_itor));
				/*strcpy((*position_itor)->TradingDayRecord, this->getTradingDay().c_str());
				this->dbm->UpdatePositionDetail((*position_itor));*/
			}
		}
	}

	// 获取昨持仓明细
	this->dbm->getAllPositionDetailYesterday(this->l_posdetail_yesterday);

	// 将昨持仓明细添加到策略的持仓明细里(order)
	for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) {
		// 一旦有策略当天有保存过,那么从今持仓明细开始初始化
		if ((*stg_itor)->getStgLastSavedTime() != "")
		{
			for (position_itor = this->l_posdetail->begin(); position_itor != this->l_posdetail->end(); position_itor++) {

				USER_PRINT("将今持仓明细添加到策略的持仓明细里(order)");

				if ((!strcmp((*stg_itor)->getStgStrategyId().c_str(), (*position_itor)->StrategyID)) &&
					(!strcmp((*stg_itor)->getStgUserId().c_str(), (*position_itor)->UserID))) { //策略id相同 && 用户ID相同
					strcpy((*position_itor)->TradingDayRecord, this->getTradingDay().c_str());
					//添加到对应策略的持仓明细列表里
					(*stg_itor)->getStg_List_Position_Detail_From_Order()->push_back((*position_itor));
				}
			}
		}
		else { // 策略未修改过持仓变量，从昨持仓明细初始化
			for (position_itor = this->l_posdetail_yesterday->begin(); position_itor != this->l_posdetail_yesterday->end(); position_itor++) {

				USER_PRINT("将昨持仓明细添加到策略的持仓明细里(order)_1");

				if ((!strcmp((*stg_itor)->getStgStrategyId().c_str(), (*position_itor)->StrategyID)) &&
					(!strcmp((*stg_itor)->getStgUserId().c_str(), (*position_itor)->UserID))) { //策略id相同 && 用户ID相同
					strcpy((*position_itor)->TradingDayRecord, this->getTradingDay().c_str());
					USER_PRINT("将昨持仓明细添加到策略的持仓明细里(order)_2");
					//添加到对应策略的持仓明细列表里
					(*stg_itor)->getStg_List_Position_Detail_From_Order()->push_back((*position_itor));
				}
			}
		}
	}
	USER_PRINT("order持仓明细添加完成");

	/*******************************Trade******************************/
	/* 遍历今仓列表(trade),对比TradingDayRecord,
	a:小于本交易日TradingDay,覆盖到昨仓持仓明细列表
	b:等于本交易日,直接从昨仓持仓明细列表进行初始化*/
	/************************************************************************/
	//是否拥有昨持仓明细，默认为false
	bool is_own_yesterday_trade_position_detail = false;

	for (position_trade_itor = this->l_posdetail_trade->begin(); position_trade_itor != this->l_posdetail_trade->end(); position_trade_itor++) { // 遍历今持仓明细列表
		// 有日期日期不同
		if (strcmp((*position_trade_itor)->TradingDayRecord, this->getTradingDay().c_str())) //日期不等
		{
			is_own_yesterday_trade_position_detail = true;
		}
	}

	if (is_own_yesterday_trade_position_detail) { //当今持仓明细第一条数据记录时间与CTP TradingDay时间不相等，清空操作昨持仓明细集合
		// 清空数据库
		this->dbm->DropPositionDetailTrade();
		this->dbm->DropPositionDetailTradeYesterday();
		//USER_PRINT("dop position detail trade");
		for (position_trade_itor = this->l_posdetail_trade->begin(); position_trade_itor != this->l_posdetail_trade->end(); position_trade_itor++) { // 遍历今持仓明细列表

			if (strcmp((*position_trade_itor)->TradingDayRecord, this->getTradingDay().c_str())) //日期不等
			{
				// 删除昨持仓明细对象,如果存在就删除,没有跳出
				// this->dbm->DeletePositionDetailYesterday((*position_itor));
				// 创建新的策略昨仓
				// 记录更新时间为本交易日
				strcpy((*position_trade_itor)->TradingDayRecord, this->getTradingDay().c_str());
				this->dbm->CreatePositionDetailTradeYesterday((*position_trade_itor));
				/*strcpy((*position_trade_itor)->TradingDayRecord, this->getTradingDay().c_str());
				this->dbm->UpdatePositionDetailTrade((*position_trade_itor));*/
			}
		}
	}
	USER_PRINT("初始化持仓明细(trade)");

	// 获取昨持仓明细
	this->dbm->getAllPositionDetailTradeYesterday(this->l_posdetail_trade_yesterday);

	// 将昨持仓明细添加到策略的持仓明细里(trade)
	for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) {
		// 一旦有策略当天有保存过,那么从今持仓明细开始初始化
		if ((*stg_itor)->getStgLastSavedTime() != "") {
			for (position_trade_itor = this->l_posdetail_trade->begin();
				position_trade_itor != this->l_posdetail_trade->end();
				position_trade_itor++) {

				USER_PRINT("将今持仓明细添加到策略的持仓明细里(trade)");

				if ((!strcmp((*stg_itor)->getStgStrategyId().c_str(), (*position_trade_itor)->StrategyID)) &&
					(!strcmp((*stg_itor)->getStgUserId().c_str(), (*position_trade_itor)->UserID))) { //策略id相同 && 用户ID相同
					
					strcpy((*position_trade_itor)->TradingDayRecord, this->getTradingDay().c_str());
					//添加到对应策略的持仓明细列表里
					(*stg_itor)->getStg_List_Position_Detail_From_Trade()->push_back((*position_trade_itor));

				}
			}
		}
		else { // 仓位未修改从昨持仓明细初始化(trade)
			for (position_trade_itor = this->l_posdetail_trade_yesterday->begin();
				position_trade_itor != this->l_posdetail_trade_yesterday->end();
				position_trade_itor++) {

				USER_PRINT("将昨持仓明细添加到策略的持仓明细里(trade)");

				if ((!strcmp((*stg_itor)->getStgStrategyId().c_str(), (*position_trade_itor)->StrategyID)) &&
					(!strcmp((*stg_itor)->getStgUserId().c_str(), (*position_trade_itor)->UserID))) { //策略id相同 && 用户ID相同
					strcpy((*position_trade_itor)->TradingDayRecord, this->getTradingDay().c_str());
					//添加到对应策略的持仓明细列表里
					(*stg_itor)->getStg_List_Position_Detail_From_Trade()->push_back((*position_trade_itor));

				}

			}
		}
	}

	//统计每个策略的持仓量
	for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) {
		(*stg_itor)->calPosition();
	}

	USER_PRINT("finish CTP_Manager::initStrategyAndFutureAccount()");

	return flag;
}

/// 统计本地order,trade持仓明细
void CTP_Manager::initPositionDetailDataFromLocalOrderAndTrade() {
	list<USER_CThostFtdcOrderField *>::iterator position_itor;
	list<USER_CThostFtdcTradeField *>::iterator position_trade_itor;
	list<User *>::iterator user_itor;

	/// 遍历User,绑定对应持仓明细(Order)
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		for (position_itor = this->l_posdetail->begin(); position_itor != this->l_posdetail->end(); position_itor++) { // 遍历今持仓明细列表

			if (!strcmp((*position_itor)->UserID, (*user_itor)->getUserID().c_str()))
			{
				(*user_itor)->addL_Position_Detail_From_Local_Order((*position_itor));
			}

		}
	}

	/// 遍历User,绑定对应持仓明细(Trade)
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		for (position_trade_itor = this->l_posdetail_trade->begin(); position_trade_itor != this->l_posdetail_trade->end(); position_trade_itor++) { // 遍历今持仓明细列表

			if (!strcmp((*position_trade_itor)->UserID, (*user_itor)->getUserID().c_str())) // 日期相等
			{
				(*user_itor)->addL_Position_Detail_From_Local_Trade((*position_trade_itor));
			}
		}
	}	
}

/// 增加SOCKET FD
void CTP_Manager::addSocketFD(string user_id, int fd) {
	//查找交易员
	map<string, list<int> *>::iterator m_itor;
	m_itor = this->m_socket_fds.find(user_id);
	if (m_itor == (this->m_socket_fds.end())) {
		std::cout << "m_socket_fds 未找到 该期货账户 = " << user_id << std::endl;
		list<int> * l_fds = new list<int>();
		l_fds->push_back(fd);
		this->m_socket_fds.insert(pair<string, list<int> *>(user_id, l_fds));
	}
	else {
		std::cout << "m_socket_fds 找到 该期货账户 = " << user_id << std::endl;
		bool is_find = false;
		/// 遍历该交易员的list集合，如果存在该fd，直接跳过，不存在直接存入
		list<int>::iterator int_itor;
		for (int_itor = m_itor->second->begin(); int_itor != m_itor->second->end(); int_itor++)
		{
			if ((*int_itor) == fd) {
				is_find = true;
			}
		}
		if (!is_find)
		{
			m_itor->second->push_back(fd);
		}
	}
}

/// 删除SOCKET FD
void CTP_Manager::delSocketFD(string user_id, int fd) {
	//查找交易员
	map<string, list<int> *>::iterator m_itor;
	m_itor = this->m_socket_fds.find(user_id);
	if (m_itor == (this->m_socket_fds.end())) {
		std::cout << "m_socket_fds 未找到 该期货账户 = " << user_id << std::endl;
		// this->stg_map_instrument_action_counter->insert(pair<string, int>(instrument_id, 0));
		// this->init_instrument_id_action_counter(string(pOrder->InstrumentID));
		//return 0;
	}
	else {
		std::cout << "m_socket_fds 找到 该期货账户 = " << user_id << std::endl;
		/// 遍历该交易员的list集合，如果存在该fd，直接跳过，不存在直接存入
		list<int>::iterator int_itor;
		for (int_itor = m_itor->second->begin(); int_itor != m_itor->second->end(); int_itor++)
		{
			if ((*int_itor) == fd) {
				int_itor = m_itor->second->erase(int_itor);
			}
		}
	}
}

/// 发送行情断线通知
void CTP_Manager::sendMarketOffLineMessage() {
	std::cout << "CTP_Manager::sendMarketOffLineMessage()" << std::endl;

	std::cout << "服务端发送发送行情断线通知" << std::endl;

	/************************************************************************/
	/*发送行情断线通知     msgtype == 18                                                       */
	/************************************************************************/
	
	map<string, list<int> *>::iterator m_itor;
	for (m_itor = this->m_socket_fds.begin(); m_itor != this->m_socket_fds.end(); m_itor++) {

		rapidjson::Document build_doc;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<StringBuffer> writer(buffer);

		/*构建MarketInfo的Json*/
		build_doc.SetObject();
		rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
		build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
		build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
		build_doc.AddMember("MsgType", 18, allocator);
		build_doc.AddMember("MsgSrc", 1, allocator);
		build_doc.AddMember("MsgResult", 1, allocator);
		build_doc.AddMember("MsgErrorReason", "CTP行情已断开连接", allocator);
		build_doc.Accept(writer);

		list<int>::iterator int_itor;
		for (int_itor = m_itor->second->begin(); int_itor != m_itor->second->end(); int_itor++)
		{
			if (write_msg((*int_itor), const_cast<char *>(buffer.GetString()), strlen(buffer.GetString())) < 0) {
				printf("\t客户端已断开!!!\n");
				//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
				if (errno == EPIPE) {
					std::cout << "EPIPE" << std::endl;
					//break;
				}
				perror("\tprotocal error");
			}
		}
	}
}

/// 发送交易断线通知
void CTP_Manager::sendTradeOffLineMessage(string user_id) {
	std::cout << "CTP_Manager::sendTradeOffLineMessage()" << std::endl;
	std::cout << "\t发送交易断线通知" << std::endl;

	/************************************************************************/
	/*发送交易断线通知     msgtype == 19                                       */
	/************************************************************************/
	//查找交易员
	map<string, list<int> *>::iterator m_itor;
	m_itor = this->m_socket_fds.find(user_id);
	if (m_itor == (this->m_socket_fds.end())) {
		std::cout << "m_socket_fds 未找到 该期货账户 = " << user_id << std::endl;
	}
	else {
		std::cout << "m_socket_fds 找到 该期货账户 = " << user_id << std::endl;
		/// 遍历该交易员的list集合，如果存在该fd，直接跳过，不存在直接存入
		list<int>::iterator int_itor;
		for (int_itor = m_itor->second->begin(); int_itor != m_itor->second->end(); int_itor++)
		{
			rapidjson::Document build_doc;
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<StringBuffer> writer(buffer);

			/*构建MarketInfo的Json*/
			build_doc.SetObject();
			rapidjson::Document::AllocatorType& allocator = build_doc.GetAllocator();
			build_doc.AddMember("MsgRef", server_msg_ref++, allocator);
			build_doc.AddMember("MsgSendFlag", MSG_SEND_FLAG, allocator);
			build_doc.AddMember("MsgType", 19, allocator);
			build_doc.AddMember("UserID", rapidjson::StringRef(user_id.c_str()), allocator);
			build_doc.AddMember("MsgSrc", 1, allocator);
			build_doc.AddMember("MsgResult", 1, allocator);
			build_doc.AddMember("MsgErrorReason", "CTP交易已断开连接", allocator);
			build_doc.Accept(writer);

			list<int>::iterator int_itor;
			for (int_itor = m_itor->second->begin(); int_itor != m_itor->second->end(); int_itor++)
			{
				if (write_msg((*int_itor), const_cast<char *>(buffer.GetString()), strlen(buffer.GetString())) < 0) {
					printf("先前客户端已断开!!!\n");
					//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
					if (errno == EPIPE) {
						std::cout << "\tEPIPE" << std::endl;
						//break;
					}
					perror("\tprotocal error");
				}
			}
		}
	}
}

/// 策略增加后同步到Users
bool CTP_Manager::syncStrategyAddToUsers(Strategy *stg) {
	USER_PRINT("CTP_Manager::syncStrategyAddToUsers");
	std::cout << "CTP_Manager::syncStrategyAddToUsers()" << std::endl;
	bool isAddSuccess = false;
	bool isFindUser = false;

	list<User *>::iterator user_itor;

	/// 添加策略到user列表里
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		std::cout << "\t系统存在账户 = " << (*user_itor)->getUserID() << std::endl;
		std::cout << "\t增加策略账户 = " << stg->getStgUserId() << std::endl;
		if ((stg->getStgUserId() == (*user_itor)->getUserID())
			&& (stg->getStgTraderId() == (*user_itor)->getTraderID())) {
			std::cout << "\t期货账户,交易员均相同,开始同步..." << std::endl;
			isFindUser = true;
			(*user_itor)->addStrategyToList(stg);
			isAddSuccess = true;
		}
	}

	if (isFindUser && isAddSuccess) {
		std::cout << "\t新增策略已同步到期货账户!" << std::endl;
		return true;
	}
	else {
		std::cout << "\t新增策略 未 同步到期货账户!" << std::endl;
		return false;
	}

}

/// 策略删除后同步到Users
bool CTP_Manager::syncStrategyDeleteToUsers(string d_traderid, string d_userid, string d_strategyid) {
	USER_PRINT("CTP_Manager::syncStrategyAddToUsers");
	std::cout << "CTP_Manager::syncStrategyAddToUsers()" << std::endl;
	bool isDeleteSuccess = false;
	bool isFindUser = false;

	list<User *>::iterator user_itor;
	list<Strategy *>::iterator stg_itor;

	/// 添加策略到user列表里
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		std::cout << "\t系统存在账户 = " << (*user_itor)->getUserID() << std::endl;
		std::cout << "\t欲删除策略账户id = " << d_userid << std::endl;
		std::cout << "\t欲删除策略id = " << d_strategyid << std::endl;
		if ((d_userid == (*user_itor)->getUserID())
			&& (d_traderid == (*user_itor)->getTraderID())) {
			std::cout << "\t期货账户,交易员均相同,开始匹配策略id..." << std::endl;
			isFindUser = true;
			for (stg_itor = (*user_itor)->getListStrategy()->begin(); stg_itor != (*user_itor)->getListStrategy()->end(); ) {
				if ((*stg_itor)->getStgStrategyId() == d_strategyid)
				{
					std::cout << "\t策略id匹配成功..." << std::endl;
					delete (*stg_itor);
					stg_itor = (*user_itor)->getListStrategy()->erase(stg_itor);
					isDeleteSuccess = true;
					break;
				}
				else {
					stg_itor++;
				}
			}
		}
	}

	if (isFindUser && isDeleteSuccess) {
		std::cout << "\t删除策略已同步到期货账户!" << std::endl;
		return true;
	}
	else {
		std::cout << "\t删除策略 未 同步到期货账户!" << std::endl;
		return false;
	}
}

//休盘
void CTP_Manager::setIsMarketClose(bool is_market_close) {
	std::cout << "CTP_Manager::setIsMarketClose()" << std::endl;
	this->is_market_close = is_market_close;
	if (is_market_close)
	{
		std::cout << "\t设置为休盘状态" << std::endl;
	}
	else {
		std::cout << "\t设置为盘中状态" << std::endl;
	}
}

bool CTP_Manager::getIsMarketClose() {
	return this->is_market_close;
}

//系统收盘最后5秒内要完成的收尾工作标志位
void CTP_Manager::setIsStartEndTask(bool is_start_end_task) {
	this->is_start_end_task = is_start_end_task;
}

bool CTP_Manager::getIsStartEndTask() {
	return this->is_start_end_task;
}

//已经收盘标志位
void CTP_Manager::setIsMarketCloseDone(bool is_market_close_done) {
	this->is_market_close_done = is_market_close_done;
}

bool CTP_Manager::getIsMarketCloseDone() {
	return this->is_market_close_done;
}

///// 初始化昨仓明细
//bool CTP_Manager::initYesterdayPositionDetail() {
//	bool flag = false;
//	list<USER_CThostFtdcOrderField *>::iterator posd_itor;
//	list<Strategy *>::iterator stg_itor;
//	
//	std::cout << "CTP_Manager::initYesterdayPositionDetail()" << std::endl;
//	std::cout << "\tCTP_Manager 系统交易日 = " << this->getTradingDay() << std::endl;
//	/// 遍历策略，遍历昨仓明细，进行绑定
//	for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++)
//	{
//		for (posd_itor = this->l_posdetail->begin(); posd_itor != this->l_posdetail->end(); posd_itor++) {
//
//			// 绑定策略对应的持仓明细
//			if ((!strcmp((*stg_itor)->getStgUserId().c_str(), (*posd_itor)->UserID)) &&
//				(!strcmp((*stg_itor)->getStgStrategyId().c_str(), (*posd_itor)->StrategyID))) {
//
//				/************************************************************************/
//				/* 判断持仓明细的trading_day
//					1:如果trading_day等于CTP系统交易日,那么持仓明细已经更新过了，毋需更新
//					2:如果trading_day小于CTP系统交易日,那么过期的持仓明细进行更新，将过期的今仓数据更新为昨仓数据，并且更新trading_day*/
//				/************************************************************************/
//				
//				if (!Utils::compareTradingDay((*posd_itor)->TradingDay, this->getTradingDay().c_str())) { // 持仓明细的trading_day不等于ctp系统交易日
//					//trading_day小于CTP系统交易日,那么过期的持仓明细进行更新，将过期的今仓数据更新为昨仓数据，并且更新trading_day
//				}
//
//				(*stg_itor)->printStrategyInfo("CTP_Manager::initYesterdayPositionDetail() 添加昨仓明细到各自策略持仓明细列表");
//				// 将持仓明细添加到对应策略持仓明细列表
//				(*stg_itor)->add_position_detail((*posd_itor));
//				flag = true;
//			}
//
//		}
//	}
//
//	if ((this->l_posdetail->size() > 0) && flag) {
//		flag = true;
//	}
//	else if ((this->l_posdetail->size() == 0) && (flag == false)) {
//		flag = true;
//	}
//	else
//	{
//		flag = false;
//	}
//	return flag;
//}

///// 初始化昨仓明细
//bool CTP_Manager::initYesterdayPositionDetail() {
//	bool flag = false;
//	list<USER_CThostFtdcOrderField *>::iterator posd_itor;
//	list<Strategy *>::iterator stg_itor;
//
//	std::cout << "CTP_Manager::initYesterdayPositionDetail()" << std::endl;
//	std::cout << "\tCTP_Manager 系统交易日 = " << this->getTradingDay() << std::endl;
//	/// 遍历策略，遍历昨仓明细，进行绑定
//	for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++)
//	{
//		for (posd_itor = this->l_posdetail->begin(); posd_itor != this->l_posdetail->end(); posd_itor++) {
//
//			// 绑定策略对应的持仓明细
//			if ((!strcmp((*stg_itor)->getStgUserId().c_str(), (*posd_itor)->UserID)) &&
//				(!strcmp((*stg_itor)->getStgStrategyId().c_str(), (*posd_itor)->StrategyID))) {
//
//				/************************************************************************/
//				/* 判断持仓明细的trading_day
//				1:如果trading_day等于CTP系统交易日,那么持仓明细已经更新过了，毋需更新
//				2:如果trading_day小于CTP系统交易日,那么过期的持仓明细进行更新，将过期的今仓数据更新为昨仓数据，并且更新trading_day*/
//				/************************************************************************/
//
//				if (!Utils::compareTradingDay((*posd_itor)->TradingDay, this->getTradingDay().c_str())) { // 持仓明细的trading_day不等于ctp系统交易日
//					//trading_day小于CTP系统交易日,那么过期的持仓明细进行更新，将过期的今仓数据更新为昨仓数据，并且更新trading_day
//
//				}
//
//				(*stg_itor)->printStrategyInfo("CTP_Manager::initYesterdayPositionDetail() 添加昨仓明细到各自策略持仓明细列表");
//				// 将持仓明细添加到对应策略持仓明细列表
//				(*stg_itor)->add_position_detail((*posd_itor));
//				flag = true;
//			}
//
//		}
//	}
//
//	if ((this->l_posdetail->size() > 0) && flag) {
//		flag = true;
//	}
//	else if ((this->l_posdetail->size() == 0) && (flag == false)) {
//		flag = true;
//	}
//	else
//	{
//		flag = false;
//	}
//	return flag;
//}

/// 初始化
bool CTP_Manager::init(bool is_online) {

	bool init_flag = true;

	std::cout << "Trade Server开始初始化..." << std::endl;

	/************************************************************************/
	/* 初始化工作按顺序执行
	1:查询所有Trader
	2:查询所有期货账户
	3:初始化昨仓
	4:初始化交易SPI
	5:初始化策略列表合约最小跳
	6:初始化今仓
	7:初始化行情SPI*/
	/************************************************************************/
	//设置盘中还是盘后
	this->dbm->setIs_Online(is_online);

	std::cout << "\t初始化网络环境完成..." << std::endl;
	if (!this->dbm->getDBConnection()) {
		std::cout << "\tMongoDB数据库连接有问题..." << std::endl;
		init_flag = false;
		return init_flag;
	}

	/// 数据库查询所有的Trader
	this->dbm->getAllTrader(this->l_trader);
	this->dbm->getAllObjTrader(this->l_obj_trader);
	std::cout << "\t初始化交易员完成..." << std::endl;

	/// 查询所有的期货账户
	this->dbm->getAllFutureAccount(this->l_user);
	std::cout << "\t初始化期货账户完成..." << std::endl;

	if ((this->l_user->size() <= 0) || (this->l_obj_trader->size() <= 0)) {
		USER_PRINT("期货账户或者交易员账户为空，未能初始化!");
		std::cout << "\t期货账户或者交易员账户为空，未能初始化!请检查数据库!" << std::endl;
		init_flag = false;
		return init_flag;
	}

	/// 查询所有期货账户的sessionid,完成绑定
	//this->dbm->getAllSession(this->l_sessions);
	
	/// 查询策略
	//this->dbm->getAllStrategyYesterday(this->l_strategys);
	//this->dbm->getAllStrategy(this->l_strategys);
	this->dbm->getAllStrategyByActiveUser(this->l_strategys, this->l_user);

	/// 查询昨仓策略
	//this->dbm->getAllStrategyYesterday(this->l_strategys_yesterday);
	this->dbm->getAllStrategyYesterdayByActiveUser(this->l_strategys_yesterday, this->l_user);
	
	std::cout << "\t初始化策略完成..." << std::endl;

	/// 查询上次系统结束是否正常结束
	//this->dbm->CheckSystemStartFlag();

	this->system_init_flag = this->dbm->GetSystemRunningStatus();
	std::cout << "\t系统初始化状态 = " << this->system_init_flag << std::endl;

	/// 查询昨仓持仓明细(order)
	this->dbm->getAllPositionDetail(this->l_posdetail);

	/// 查询昨仓持仓明细(trade)
	this->dbm->getAllPositionDetailTrade(this->l_posdetail_trade);
	

	/// 绑定操作：账户绑定到对应交易员名下
	/// 绑定操作：策略绑定到对应期货账户下
	list<User *>::iterator user_itor;
	list<Strategy *>::iterator stg_itor;
	list<Trader *>::iterator trader_itor;
	list<Session *>::iterator sid_itor;

	/// 绑定sessionid到每个期货账户名下
	/*for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) {
		USER_PRINT((*user_itor)->getUserID());
		for (sid_itor = this->l_sessions->begin(); sid_itor != this->l_sessions->end(); sid_itor++) {
			USER_PRINT((*sid_itor)->getUserID());
			if ((*sid_itor)->getUserID() == (*user_itor)->getUserID()) {
				USER_PRINT("(*sid_itor)->getUserID() == (*user_itor)->getUserID()");
				(*user_itor)->getL_Sessions()->push_back((*sid_itor));
			}
		}
	}*/

	/// 绑定交易员和期货账户
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) {
		USER_PRINT((*user_itor)->getUserID());
		USER_PRINT((*user_itor)->getTraderID());
		(*user_itor)->setCTP_Manager(this); //每个user对象设置CTP_Manager对象
		(*user_itor)->setDBManager(this->dbm); //每个user对象设置DBManager对象

		for (trader_itor = this->l_obj_trader->begin(); trader_itor != this->l_obj_trader->end(); trader_itor++) {
			USER_PRINT((*trader_itor)->getTraderID());
			if ((*trader_itor)->getTraderID() == (*user_itor)->getTraderID()) {
				(*user_itor)->setTrader((*trader_itor));
			}
		}
	}
	std::cout << "\t初始化交易员,期货账户绑定完成..." << std::endl;

	/// 绑定期货账户和策略
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User

		USER_PRINT((*user_itor)->getUserID());

		for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) { // 遍历Strategy

			if ((*stg_itor)->getStgUserId() == (*user_itor)->getUserID()) {
				USER_PRINT("Strategy Bind To User");
				(*user_itor)->addStrategyToList((*stg_itor)); // 将策略添加到期货账户策略列表里
				(*stg_itor)->setStgUser((*user_itor)); // 策略设置自己的期货账户对象
				//(*stg_itor)->setStgTradingDay((*user_itor)->getTradingDay()); // 更新时间
			}
		}
	}
	std::cout << "\t初始化期货账户,策略绑定完成..." << std::endl;

	/*/// 设置时间
	this->setTradingDay(this->l_user->front()->getUserTradeSPI()->getTradingDay());*/

	/// 行情初始化
	MarketConfig *mc = this->dbm->getOneMarketConfig();
	if (mc != NULL) {
		this->mdspi = this->CreateMd(mc->getMarketFrontAddr(), mc->getBrokerID(), mc->getUserID(), mc->getPassword(), this->l_strategys);
	}
	else {
		std::cout << "\t行情初始化失败!" << std::endl;
		init_flag = false;
		return init_flag;
	}
	std::cout << "\t初始化行情完成..." << std::endl;

	/// 如果是新的交易日，更新策略，今昨持仓量以及期货账户报单引用
	if (!(this->initStrategyAndFutureAccount())) {
		USER_PRINT("初始化策略，仓位失败...");
		std::cout << "\t初始化策略，仓位失败..." << std::endl;
		init_flag = false;
		return init_flag;
	}
	else {
		USER_PRINT("策略，仓位完成...");
		std::cout << "\t初始化策略,仓位完成..." << std::endl;
	}

	
	//std::thread* thr[MAX_THREAD_COUNT] = { nullptr };

	/// 绑定期货账户和策略
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		
		USER_PRINT((*user_itor)->getUserID());
		/// 遍历设值后进行TD初始化，初始化绑定对应的策略列表
		//this->CreateAccount((*user_itor), (*user_itor)->getListStrategy());

		/************************************************************************/
		/* 改为多线程创建Account                                                                     */
		/************************************************************************/
		
		this->user_threads.push_back(std::thread(&CTP_Manager::CreateAccount, this, (*user_itor), (*user_itor)->getListStrategy()));

		/*thr[thread_index] = new std::thread(&CTP_Manager::CreateAccount, this, (*user_itor), (*user_itor)->getListStrategy());
		thr[thread_index]->detach();*/
		//std::thread(&CTP_Manager::CreateAccount, this,  (*user_itor), (*user_itor)->getListStrategy()).detach();

		//std::cout << "\t账户 : " << (*user_itor)->getUserID() << " 初始化完成!" << std::endl;
		//sleep(3);
	}

	std::cout << "\tIn Main Thread = " << std::this_thread::get_id() << std::endl;
	std::cout << "\t主线程设置子线程分离..." << std::endl;
	/// 设置线程分离
	for (auto& t: this->user_threads)
	{
		t.detach();
	}

	bool is_all_user_finished_init = true;
	while (true)
	{
		/// 初始值为true,一旦有账户不满足初始化完成条件，置false操作
		is_all_user_finished_init = true;
		/// 检查USER初始化是否完成
		std::cout << "\tIn Main Thread = " << std::this_thread::get_id() << std::endl;
		std::cout << "\t检查期货账户登录状态" << std::endl;

		for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User

			if (!(*user_itor)->getThread_Init_Status()) {
				std::cout << "\t" << (*user_itor)->getUserID() << "账户还在初始化中..." << std::endl;
				is_all_user_finished_init = false;
			}
			
		}

		if (is_all_user_finished_init) {
			std::cout << "\t账户初始化完成,进入登录检查..." << std::endl;
			break;
		}

		sleep(2);
	}
	
	/// 进行登录检查,把登录失败的user从当前账户列表移出去
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end();) { // 遍历User
		//已经连接+已经登录+登录无错误
		if ((*user_itor)->getIsConnected() &&
			(*user_itor)->getIsLogged() &&
			(!(*user_itor)->getIsLoggedError()))
		{
			user_itor++;
		} 
		else
		{
			delete (*user_itor);
			user_itor = this->l_user->erase(user_itor);
		}
	}

	if ((this->l_user->size() <= 0)) {
		USER_PRINT("可用期货账户为空!");
		std::cout << "\t期货账户登录均失败,请检查id与密码" << std::endl;
		init_flag = false;
		return init_flag;
	}
	else {
		std::cout << "\t初始化期货账户和策略绑定完成..." << std::endl;
	}



	/// 查询投资者持仓
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		USER_PRINT((*user_itor)->getUserID());
		sleep(1);
		(*user_itor)->getUserTradeSPI()->QryInvestorPosition();
		sleep(2);
		(*user_itor)->getUserTradeSPI()->QryInvestorPositionDetail();
		//sleep(2);
	}
	
	/// 进行仓位数据核对

	/// 统计本地仓位
	this->initPositionDetailDataFromLocalOrderAndTrade();

	/// 打印输出各自结果
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		std::cout << "\t=====================================================" << std::endl;
		std::cout << "\t=\033[32m从CTP API查询统计结果\033[0m" << std::endl;
		(*user_itor)->getL_Position_Detail_Data((*user_itor)->getL_Position_Detail_From_CTP());
		std::cout << "\t=\033[32m从本地Order查询统计结果\033[0m" << std::endl;
		(*user_itor)->getL_Position_Detail_Data((*user_itor)->getL_Position_Detail_From_Local_Order());
		std::cout << "\t=\033[32m从本地Trade查询统计结果\033[0m" << std::endl;
		(*user_itor)->getL_Position_Detail_Data((*user_itor)->getL_Position_Detail_From_Local_Trade());
		std::cout << "\t=====================================================" << std::endl;
	}

	/// 判断输出仓位对比，如果不正确，关闭该期货账户开关
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		if (!(*user_itor)->ComparePositionTotal()) {
			(*user_itor)->setOn_Off(0);
		}
	}

#if 0
	/// 将仓位出现问题的user从列表里移出去
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end();) { // 遍历User
		/// 仓位问题出错
		if (!((*user_itor)->getIsPositionRight()))
		{
			delete (*user_itor);
			user_itor = this->l_user->erase(user_itor);
		}
		else
		{
			user_itor++;
		}
	}

	/// 把失效的user对应的策略移除掉
	for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end();) { // 遍历Strategy

		if (!(*stg_itor)->getStgUser()) {
			delete (*stg_itor);
			stg_itor = this->l_strategys->erase(stg_itor);
		}
		else {
			stg_itor++;
		}
	}
#endif

	if ((this->l_user->size() <= 0)) {
		USER_PRINT("可用期货账户为空!");
		std::cout << "\t期货账户仓位全部不一致!全部均初始化失败!" << std::endl;
		init_flag = false;
		return init_flag;
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
		std::cout << "\t策略最小跳价格获取失败!!!" << std::endl;
		init_flag = false;
		return init_flag;
	}
	std::cout << "\t初始化合约最小跳价格完成..." << std::endl;

	

	/// session维护，如果不是本交易日的session，就要删除
	/*for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) {
		USER_PRINT((*user_itor)->getUserID());
		for (sid_itor = (*user_itor)->getL_Sessions()->begin(); sid_itor != (*user_itor)->getL_Sessions()->end();) {
			USER_PRINT((*sid_itor)->getTradingDay());
			USER_PRINT(this->getTradingDay());
			if ((*sid_itor)->getTradingDay() != this->getTradingDay()) {
				USER_PRINT("日期不等,删除sesson");
				this->dbm->DeleteSession((*sid_itor));
				delete (*sid_itor);
				sid_itor = (*user_itor)->getL_Sessions()->erase(sid_itor);
			}
			else
			{
				sid_itor++;
			}
		}
	}*/


	

	/// 昨仓持仓明细初始化
	/*if (!(this->initYesterdayPositionDetail())) {
		USER_PRINT("初始化昨仓明细失败...");
		init_flag = false;
		return init_flag;
	}
	else {
		USER_PRINT("初始化昨仓明细成功...");
	}*/

	list<CThostFtdcTradeField *> *l_query_trade;
	/// 初始化今仓(trade)
	//for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
	//	(*user_itor)->getUserTradeSPI()->QryTrade();
	//	sleep(1);
	//	l_query_trade = (*user_itor)->getUserTradeSPI()->getL_query_trade();
	//	for (stg_itor = (*user_itor)->getListStrategy()->begin(); stg_itor != (*user_itor)->getListStrategy()->end(); stg_itor++) { // 遍历Strategy
	//		(*stg_itor)->setL_query_trade((*user_itor)->getUserTradeSPI()->getL_query_trade());
	//	}
	//}


	/************************************************************************/
	/* 初始化今持仓明细，通过QryOrder回调进行计算(此法不可靠，由于返回值乱序，无法维护)                                                                     */
	/************************************************************************/
#if 0
	USER_PRINT("初始化持仓明细");
	list<CThostFtdcOrderField *> *l_query_order;
	list<CThostFtdcOrderField *>::iterator order_itor;
	/// 初始化今仓(order)
	for (user_itor = this->l_user->begin(); user_itor != this->l_user->end(); user_itor++) { // 遍历User
		(*user_itor)->getUserTradeSPI()->QryOrder();
		sleep(3);
		l_query_order = (*user_itor)->getUserTradeSPI()->getL_query_order();
		for (stg_itor = (*user_itor)->getListStrategy()->begin(); stg_itor != (*user_itor)->getListStrategy()->end(); stg_itor++) { // 遍历Strategy

			for (order_itor = l_query_order->begin(); order_itor != l_query_order->end(); order_itor++) {

				string temp((*order_itor)->OrderRef);
				USER_PRINT(temp);
				USER_PRINT(temp.substr(temp.length() - 2, 2));
				if (temp.substr(temp.length() - 2, 2) == ((*stg_itor)->getStgStrategyId())) {
					(*stg_itor)->addOrderToListQueryOrder((*order_itor));
				}
			}	
		}
	}
	std::cout << "t初始化今仓持仓明细完成..." << std::endl;
#endif
	
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
		std::cout << "\t行情订阅已初始化!" << std::endl;
	}

	std::cout << "\033[32m===========恭喜============\033[0m" << std::endl;
	std::cout << "\033[32m|=!xTrader系统初始化完成!=|\033[0m" << std::endl;
	std::cout << "\033[32m===========================\033[0m" << std::endl;

	//初始化完毕,开启系统全局开关
	this->setOn_Off(1);
	this->dbm->UpdateSystemRunningStatus("on");

	return init_flag;
}
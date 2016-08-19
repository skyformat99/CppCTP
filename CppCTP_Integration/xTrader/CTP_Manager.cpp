#include "CTP_Manager.h"
#include "User.h"
#include "Utils.h"
#include "Debug.h"


CTP_Manager::CTP_Manager() {
	this->dbm = new DBManager();
	this->l_user = new list<User *>();
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

User * CTP_Manager::CreateAccount(string td_frontAddress, string td_broker, string td_user, string td_pass, Trader *trader) {
	USER_PRINT("CTP_Manager::CreateAccount");
	//tcp://180.168.146.187:10030 //24H
	//tcp://180.168.146.187:10000 //实盘仿真

	TdSpi *tdspi = new TdSpi();

	User *user = new User(td_frontAddress, td_broker, td_user, td_pass, td_user, trader);

	/*设置api*/
	string flowpath = "conn/td/" + user->getUserID() + "/";
	int flag = Utils::CreateFolder(flowpath.c_str());
	if (flag != 0) {
		cout << "无法创建用户流文件!" << endl;
		return NULL;
	} else {
		CThostFtdcTraderApi *tdapi = CThostFtdcTraderApi::CreateFtdcTraderApi(flowpath.c_str());
		cout << tdapi << endl;
		if (!tdapi) {
			return NULL;
		} else {
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

	this->l_user->push_back(user);
	//tdspi->QrySettlementInfo(user);
	//sleep(6);
	//string instrument = "cu1609";
	//user->getUserTradeSPI()->OrderInsert(user, const_cast<char *>(instrument.c_str()), '0', '0', 20, 39000, "1");

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

///释放
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

///订阅行情
void CTP_Manager::SubmarketData(MdSpi *mdspi, list<string > l_instrument) {
	if (mdspi && (l_instrument.size() > 0)) {
		mdspi->SubMarket(l_instrument);
	}
}

/// 退订合约
void CTP_Manager::UnSubmarketData(MdSpi *mdspi, list<string > l_instrument) {
	if (mdspi && (l_instrument.size() > 0)) {
		mdspi->UnSubMarket(l_instrument);
	}
}

/// 添加订阅合约
list<string> CTP_Manager::addSubInstrument(string instrumentID, list<string > l_instrument) {
	l_instrument.push_back(instrumentID);
	USER_PRINT(l_instrument.size());
	return l_instrument;
}

/// 删除订阅合约
list<string> CTP_Manager::delSubInstrument(string instrumentID, list<string > l_instrument) {
	USER_PRINT(l_instrument.size());
	if (l_instrument.size() > 0) {
		list<string>::iterator itor;
		for (itor = l_instrument.begin(); itor != l_instrument.end(); ) {
			cout << *itor << endl;
			if (*itor == instrumentID) {
				itor = l_instrument.erase(itor);
				USER_PRINT("Remove OK");
				//break;
				USER_PRINT(l_instrument.size());
				USER_PRINT("CTP_Manager::delInstrument finish");
				return l_instrument;
			}
			else {
				itor++;
			}
		}
	}
	USER_PRINT(l_instrument.size());
	USER_PRINT("CTP_Manager::delInstrument finish");
	return l_instrument;
}

/// 统计合约数量
int CTP_Manager::calInstrument(string instrumentID, list<string> l_instrument) {
	int count = 0;
	if (l_instrument.size() > 0) {
		list<string>::iterator itor;
		for (itor = l_instrument.begin(); itor != l_instrument.end(); itor++) {
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
list<string> CTP_Manager::getL_Instrument() {
	return this->l_instrument;
}

/// 得到数据库操作对象
DBManager * CTP_Manager::getDBManager() {
	return this->dbm;
}

/// 设置l_trader
void CTP_Manager::addTraderToLTrader(string trader) {
	this->l_trader.push_back(trader);
}

/// 获取trader是否在l_trader里
bool CTP_Manager::checkInLTrader(string trader) {
	bool flag = false;
	list<string>::iterator Itor;
	for (Itor = this->l_trader.begin(); Itor != this->l_trader.end(); Itor++) {
		if (*Itor == trader) {
			flag = true;
		}
	}
	return flag;
}

/// 得到l_trader
list<string> CTP_Manager::getL_Trader() {
	return this->l_trader;
}

/// 移除元素
void CTP_Manager::removeFromLTrader(string trader) {
	list<string>::iterator Itor;
	for (Itor = this->l_trader.begin(); Itor != this->l_trader.end();) {
		if (*Itor == trader) {
			Itor = this->l_trader.erase(Itor);
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
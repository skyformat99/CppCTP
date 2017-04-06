#include <sstream>
#include "User.h"
#include "DBManager.h"
#include "Debug.h"
#include "Utils.h"

/// Order相关的集合
#define DB_ORDERINSERT_COLLECTION         "CTP.orderinsert"
#define DB_ONRTNORDER_COLLECTION          "CTP.onrtnorder"
#define DB_ONRTNTRADE_COLLECTION          "CTP.onrtntrade"
#define DB_ORDERACTION_COLLECTION         "CTP.orderaction"
#define DB_ORDERCOMBINE_COLLECTION        "CTP.ordercombine"
#define DB_ONRSPORDERACTION_COLLECTION    "CTP.onrsporderaction"
#define DB_ONERRRTNORDERACTION_COLLECTION "CTP.onerrrtnorderaction"
#define DB_ONRSPORDERINSERT_COLLECTION    "CTP.onrsporderinsert"
#define DB_ONERRRTNORDERINSERT_COLLECTION "CTP.onerrrtnorderinsert"
#define DB_ONRSPQRYINVESTORPOSITION       "CTP.onrspqryinvestorposition"


//转码数组
char codeDst_2[90] = { 0 };

User::User(string frontAddress, string BrokerID, string UserID, string Password, string nRequestID, int on_off, string TraderID, string stg_order_ref_base) {
	this->on_off = on_off;
	this->BrokerID = BrokerID;
	this->UserID = UserID;
	this->Password = Password;
	this->frontAddress = frontAddress;
	this->nRequestID = atoi(UserID.c_str());
	this->isConfirmSettlement = false;
	this->TradeConn = DBManager::getDBConnection();
	this->PositionConn = DBManager::getDBConnection();
	this->OrderConn = DBManager::getDBConnection();
	this->OrderRefConn = DBManager::getDBConnection();
	this->OrderActionConn = DBManager::getDBConnection();
	this->TraderID = TraderID;
	this->l_strategys = new list<Strategy *>();
	this->l_sessions = new list<Session *>();
	this->stg_map_instrument_action_counter = new map<string, int>();
	this->stg_order_ref_base = Utils::strtolonglong(stg_order_ref_base);
	this->isLogged = true;
	this->isConnected = true;
	this->isLoggedError = false;
	this->isPositionRight = true;
	this->l_position_detail_from_ctp = new list<USER_INSTRUMENT_POSITION *>();
	this->l_position_detail_from_local_order = new list<USER_INSTRUMENT_POSITION *>();
	this->l_position_detail_from_local_trade = new list<USER_INSTRUMENT_POSITION *>();
	this->thread_init_status = false;
	USER_PRINT(this->stg_order_ref_base);
}

User::User(string BrokerID, string UserID, int nRequestID, string stg_order_ref_base) {
	this->on_off = 0;
	this->BrokerID = BrokerID;
	this->UserID = UserID;
	this->nRequestID = atoi(UserID.c_str());
	this->isConfirmSettlement = false;
	this->TradeConn = DBManager::getDBConnection();
	this->PositionConn = DBManager::getDBConnection();
	this->OrderConn = DBManager::getDBConnection();
	this->OrderRefConn = DBManager::getDBConnection();
	this->OrderActionConn = DBManager::getDBConnection();
	this->l_strategys = new list<Strategy *>();
	this->l_sessions = new list<Session *>();
	this->stg_map_instrument_action_counter = new map<string, int>();
	this->stg_order_ref_base = Utils::strtolonglong(stg_order_ref_base);
	this->isLogged = true;
	this->isConnected = true;
	this->isLoggedError = false;
	this->isPositionRight = true;
	this->thread_init_status = false;
	this->l_position_detail_from_ctp = new list<USER_INSTRUMENT_POSITION *>();
	this->l_position_detail_from_local_order = new list<USER_INSTRUMENT_POSITION *>();
	this->l_position_detail_from_local_trade = new list<USER_INSTRUMENT_POSITION *>();
}

User::~User() {

}

string User::getBrokerID() {
	return this->BrokerID;
}
string User::getUserID() {
	return this->UserID;
}
string User::getPassword() {
	return this->Password;
}

int User::getRequestID() {
	return this->nRequestID;
}
bool User::getIsLogged() {
	return this->isLogged;
}

bool User::getIsLoggedError() {
	return this->isLoggedError;
}

bool User::getIsConnected() {
	return this->isConnected;
}

bool User::getIsFirstTimeLogged() {
	return this->isFirstTimeLogged;
}
bool User::getIsConfirmSettlement() {
	return this->isConfirmSettlement;
}
int User::getLoginRequestID() {
	return this->loginRequestID;
}

CThostFtdcTraderApi * User::getUserTradeAPI() {
	return this->UserTradeAPI;
}

TdSpi * User::getUserTradeSPI() {
	return this->UserTradeSPI;
}

string User::getFrontAddress() {
	return this->frontAddress;
}

void User::setBrokerID(string BrokerID) {
	this->BrokerID = BrokerID;
}
void User::setUserID(string UserID) {
	this->UserID = UserID;
}
void User::setPassword(string Password) {
	this->Password = Password;
}

void User::setRequestID(int nRequestID) {
	this->nRequestID = nRequestID;
}
void User::setIsLogged(bool isLogged) {
	this->isLogged = isLogged;
}

void User::setIsLoggedError(bool isLoggedError) {
	this->isLoggedError = isLoggedError;
}

void User::setIsConnected(bool isConnected) {
	this->isConnected = isConnected;
}

void User::setIsPositionRight(bool isPositionRight) {
	this->isPositionRight = isPositionRight;
}
bool User::getIsPositionRight() {
	return this->isPositionRight;
}

void User::setIsFirstTimeLogged(bool isFirstTimeLogged) {
	this->isFirstTimeLogged = isFirstTimeLogged;
}
void User::setIsConfirmSettlement(bool isConfirmSettlement) {
	this->isConfirmSettlement = isConfirmSettlement;
}
void User::setLoginRequestID(int loginRequestID) {
	this->loginRequestID = loginRequestID;
}

void User::setUserTradeAPI(CThostFtdcTraderApi *UserTradeAPI) {
	this->UserTradeAPI = UserTradeAPI;
}

void User::setUserTradeSPI(TdSpi *UserTradeSPI) {
	this->UserTradeSPI = UserTradeSPI;
}

void User::setFrontAddress(string frontAddress) {
	this->frontAddress = frontAddress;
}

Trader * User::GetTrader() {
	return this->trader;
}
void User::setTrader(Trader *trader) {
	this->trader = trader;
}

string User::getTraderID() {
	return this->TraderID;
}


void User::setTraderID(string TraderID) {
	this->TraderID = TraderID;
}

/// 得到strategy_list
list<Strategy *> * User::getListStrategy() {
	return this->l_strategys;
}

/// 设置strategy_list
void User::setListStrategy(list<Strategy *> * l_strategys) {
	this->l_strategys = l_strategys;
}

/// 添加strategy到list
void User::addStrategyToList(Strategy *stg) {
	this->l_strategys->push_back(stg);
}

/// 拷贝持仓明细数据
void User::CopyPositionDetailData(CThostFtdcInvestorPositionDetailField *dst, CThostFtdcInvestorPositionDetailField *src) {
	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);
	///经纪公司代码
	strcpy(dst->BrokerID, src->BrokerID);
	///投资者代码
	strcpy(dst->InvestorID, src->InvestorID);
	///投机套保标志
	dst->HedgeFlag = src->HedgeFlag;
	///买卖
	dst->Direction = src->Direction;
	///开仓日期
	strcpy(dst->OpenDate, src->OpenDate);
	///成交编号
	strcpy(dst->TradeID, src->TradeID);
	///数量
	dst->Volume = src->Volume;
	///开仓价
	dst->OpenPrice = src->OpenPrice;
	///交易日
	strcpy(dst->TradingDay, src->TradingDay);
	///结算编号
	dst->SettlementID = src->SettlementID;
	///成交类型
	dst->TradeType = src->TradeType;
	///组合合约代码
	strcpy(dst->CombInstrumentID, src->CombInstrumentID);
	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);
	///逐日盯市平仓盈亏
	dst->CloseProfitByDate = src->CloseProfitByDate;
	///逐笔对冲平仓盈亏
	dst->CloseProfitByTrade = src->CloseProfitByTrade;
	///逐日盯市持仓盈亏
	dst->PositionProfitByDate = src->PositionProfitByDate;
	///逐笔对冲持仓盈亏
	dst->PositionProfitByTrade = src->PositionProfitByTrade;
	///投资者保证金
	dst->Margin = src->Margin;
	///交易所保证金
	dst->ExchMargin = src->ExchMargin;
	///保证金率
	dst->MarginRateByMoney = src->MarginRateByMoney;
	///保证金率(按手数)
	dst->MarginRateByVolume = src->MarginRateByVolume;
	///昨结算价
	dst->LastSettlementPrice = src->LastSettlementPrice;
	///结算价
	dst->SettlementPrice = src->SettlementPrice;
	///平仓量
	dst->CloseVolume = src->CloseVolume;
	///平仓金额
	dst->CloseAmount = src->CloseAmount;
}

/// 初始化合约撤单次数,例如"cu1601":0 "cu1701":0
void User::init_instrument_id_action_counter(string instrument_id) {
	//初始化为0
	//this->stg_map_instrument_action_counter->insert(map < string, int >::value_type(instrument_id, 0)); 第一种map插入方法
	this->stg_map_instrument_action_counter->insert(pair<string, int>(instrument_id, 0)); //第二种map插入方法
}

/// 获得合约撤单次数,例如"cu1710":0
int User::get_instrument_id_action_counter(string instrument_id) {
	USER_PRINT("User::get_instrument_id_action_counter");
	//对某个合约进行加1操作
	map<string, int>::iterator m_itor;
	m_itor = this->stg_map_instrument_action_counter->find(instrument_id);
	if (m_itor == (this->stg_map_instrument_action_counter->end())) {
		cout << "撤单列表里不存在该合约 = " << instrument_id << endl;
		// this->stg_map_instrument_action_counter->insert(pair<string, int>(instrument_id, 0));
		// this->init_instrument_id_action_counter(string(pOrder->InstrumentID));
		return 0;
	}
	else {
		cout << "撤单列表里找到该合约 = " << instrument_id << endl;
		return m_itor->second;
	}
}

/// 添加对应合约撤单次数计数器,例如"cu1602":1 "cu1701":1
void User::add_instrument_id_action_counter(CThostFtdcOrderField *pOrder) {
	USER_PRINT("User::add_instrument_id_action_counter");

	if (strlen(pOrder->OrderSysID) == 0) { //只统计针对交易所编码的order
		return;
	}
	if (pOrder->OrderStatus != '5') { //撤单
		return;
	}

	if (pOrder->OrderStatus == '5') { // 当收到撤单
		//对某个合约进行加1操作
		map<string, int>::iterator m_itor;
		m_itor = this->stg_map_instrument_action_counter->find(string(pOrder->InstrumentID));
		if (m_itor == (this->stg_map_instrument_action_counter->end())) {
			cout << "撤单列表里不存在该合约 = " << pOrder->InstrumentID << endl;
			cout << "添加 " << pOrder->InstrumentID << " 合约到列表中..." << endl;
			//this->stg_map_instrument_action_counter->insert(pair<string, int>(instrument_id, 0));
			this->init_instrument_id_action_counter(string(pOrder->InstrumentID));
		}
		else {
			cout << "撤单列表里找到该合约 = " << pOrder->InstrumentID << endl;
			m_itor->second += 1;
		}

		// 将撤单次数赋予该账户下所有的策略
		list<Strategy *>::iterator stg_itor;
		for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) { // 遍历Strategy

			USER_PRINT((*stg_itor)->getStgInstrumentIdA());
			USER_PRINT((*stg_itor)->getStgInstrumentIdB());
			USER_PRINT(pOrder->InstrumentID);

			if (!strcmp((*stg_itor)->getStgInstrumentIdA().c_str(), pOrder->InstrumentID)) {
				(*stg_itor)->setStgAOrderActionCount(this->get_instrument_id_action_counter(string(pOrder->InstrumentID)));
			}

			if (!(strcmp((*stg_itor)->getStgInstrumentIdB().c_str(), pOrder->InstrumentID))) {
				(*stg_itor)->setStgBOrderActionCount(this->get_instrument_id_action_counter(string(pOrder->InstrumentID)));
			}
		}

	}
}

void User::setStgOrderRefBase(long long stg_order_ref_base) {
	this->stg_order_ref_base = stg_order_ref_base;
}

long long User::getStgOrderRefBase() {
	return this->stg_order_ref_base;
}

/// 设置策略内合约最小跳价格
void User::setStgInstrumnetPriceTick() {
	USER_PRINT("User::setStgInstrumnetPriceTick");
	//1:遍历strategy list
	list<Strategy *>::iterator stg_itor;
	list<CThostFtdcInstrumentField *>::iterator instrument_info_itor;
	for (stg_itor = this->l_strategys->begin(); stg_itor != this->l_strategys->end(); stg_itor++) {
		//2:遍历合约信息列表,找到对应合约的最小跳
		for (instrument_info_itor = this->getUserTradeSPI()->getL_Instruments_Info()->begin();
			instrument_info_itor != this->getUserTradeSPI()->getL_Instruments_Info()->end();
			instrument_info_itor++) {
			if (!strcmp((*stg_itor)->getStgInstrumentIdA().c_str(), (*instrument_info_itor)->InstrumentID)) {
				USER_PRINT("(*stg_itor)->getStgInstrumentIdA()");
				USER_PRINT((*stg_itor)->getStgInstrumentIdA());
				(*stg_itor)->setStgAPriceTick((*instrument_info_itor)->PriceTick);
			}
			if (!strcmp((*stg_itor)->getStgInstrumentIdB().c_str(), (*instrument_info_itor)->InstrumentID)) {
				USER_PRINT("(*stg_itor)->getStgInstrumentIdB()");
				USER_PRINT((*stg_itor)->getStgInstrumentIdB());
				(*stg_itor)->setStgBPriceTick((*instrument_info_itor)->PriceTick);
			}
		}
	}
}

void User::setL_Position_Detail_From_CTP(list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_ctp) {
	this->l_position_detail_from_ctp = l_position_detail_from_ctp;
}

list<USER_INSTRUMENT_POSITION *> * User::getL_Position_Detail_From_CTP() {
	return this->l_position_detail_from_ctp;
}


void User::addL_Position_Detail_From_CTP(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail) {

	list<USER_INSTRUMENT_POSITION *>::iterator itor;
	bool isFind = false;

	for (itor = this->l_position_detail_from_ctp->begin(); itor != this->l_position_detail_from_ctp->end(); itor++)
	{

		if ((!strcmp((*itor)->InstrumentID, pInvestorPositionDetail->InstrumentID))) // 如果相等
		{
			// 找到标志位赋值
			isFind = true;

			if ((!strcmp(this->getTradingDay().c_str(), pInvestorPositionDetail->TradingDay))) //今天的持仓
			{
				if (pInvestorPositionDetail->Direction == '0') { // 今买
					(*itor)->Buy_Today += pInvestorPositionDetail->Volume;
				}
				else if (pInvestorPositionDetail->Direction == '1') // 今卖
				{
					(*itor)->Sell_Today += pInvestorPositionDetail->Volume;
				}
			}
			else // 昨天的持仓
			{
				if (pInvestorPositionDetail->Direction == '0') { // 昨买
					(*itor)->Buy_Yesterday += pInvestorPositionDetail->Volume;
				}
				else if (pInvestorPositionDetail->Direction == '1') // 昨卖
				{
					(*itor)->Sell_Yesterday += pInvestorPositionDetail->Volume;
				}
			}
		}

		
	}

	if (!isFind) // 如果未找到,直接添加到统计列表里
	{
		USER_INSTRUMENT_POSITION *new_position_from_ctp = new USER_INSTRUMENT_POSITION();
		memset(new_position_from_ctp, 0, sizeof(USER_INSTRUMENT_POSITION));
		strcpy(new_position_from_ctp->InstrumentID, pInvestorPositionDetail->InstrumentID);
		new_position_from_ctp->Is_Same = false;

		if ((!strcmp(this->getTradingDay().c_str(), pInvestorPositionDetail->OpenDate))) //今天的持仓
		{
			if (pInvestorPositionDetail->Direction == '0') { // 今买
				new_position_from_ctp->Buy_Today += pInvestorPositionDetail->Volume;
			}
			else if (pInvestorPositionDetail->Direction == '1') // 今卖
			{
				new_position_from_ctp->Sell_Today += pInvestorPositionDetail->Volume;
			}
		}
		else // 昨天的持仓
		{
			if (pInvestorPositionDetail->Direction == '0') { // 昨买
				new_position_from_ctp->Buy_Yesterday += pInvestorPositionDetail->Volume;
			}
			else if (pInvestorPositionDetail->Direction == '1') // 昨卖
			{
				new_position_from_ctp->Sell_Yesterday += pInvestorPositionDetail->Volume;
			}
		}
		this->l_position_detail_from_ctp->push_back(new_position_from_ctp);
	}

}

void User::setL_Position_Detail_From_Local_Order(list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_order) {
	this->l_position_detail_from_local_order = l_position_detail_from_local_order;
}

list<USER_INSTRUMENT_POSITION *> * User::getL_Position_Detail_From_Local_Order() {
	return this->l_position_detail_from_local_order;
}

void User::addL_Position_Detail_From_Local_Order(USER_CThostFtdcOrderField *order) {
	list<USER_INSTRUMENT_POSITION *>::iterator itor;
	bool isFind = false;

	for (itor = this->l_position_detail_from_local_order->begin(); itor != this->l_position_detail_from_local_order->end(); itor++)
	{

		if ((!strcmp((*itor)->InstrumentID, order->InstrumentID))) // 如果相等
		{
			// 找到标志位赋值
			isFind = true;

			if ((!strcmp(this->getTradingDay().c_str(), order->InsertDate))) //今天的持仓
			{
				if (order->Direction == '0') { // 今买
					(*itor)->Buy_Today += order->VolumeTradedBatch;
				}
				else if (order->Direction == '1') // 今卖
				{
					(*itor)->Sell_Today += order->VolumeTradedBatch;
				}
			}
			else // 昨天的持仓
			{
				if (order->Direction == '0') { // 昨买
					(*itor)->Buy_Yesterday += order->VolumeTradedBatch;
				}
				else if (order->Direction == '1') // 昨卖
				{
					(*itor)->Sell_Yesterday += order->VolumeTradedBatch;
				}
			}
		}


	}

	if (!isFind) // 如果未找到,直接添加到统计列表里
	{
		USER_INSTRUMENT_POSITION *new_position_from_local_order = new USER_INSTRUMENT_POSITION();
		memset(new_position_from_local_order, 0, sizeof(USER_INSTRUMENT_POSITION));
		strcpy(new_position_from_local_order->InstrumentID, order->InstrumentID);
		new_position_from_local_order->Is_Same = false;

		if ((!strcmp(this->getTradingDay().c_str(), order->InsertDate))) //今天的持仓
		{
			if (order->Direction == '0') { // 今买
				new_position_from_local_order->Buy_Today += order->VolumeTradedBatch;
			}
			else if (order->Direction == '1') // 今卖
			{
				new_position_from_local_order->Sell_Today += order->VolumeTradedBatch;
			}
		}
		else // 昨天的持仓
		{
			if (order->Direction == '0') { // 昨买
				new_position_from_local_order->Buy_Yesterday += order->VolumeTradedBatch;
			}
			else if (order->Direction == '1') // 昨卖
			{
				new_position_from_local_order->Sell_Yesterday += order->VolumeTradedBatch;
			}
		}
		this->l_position_detail_from_local_order->push_back(new_position_from_local_order);
	}

}

void User::setL_Position_Detail_From_Local_Trade(list<USER_INSTRUMENT_POSITION *> *l_position_detail_from_local_trade) {
	this->l_position_detail_from_local_trade = l_position_detail_from_local_trade;
}

list<USER_INSTRUMENT_POSITION *> * User::getL_Position_Detail_From_Local_Trade() {
	return this->l_position_detail_from_local_trade;
}

void User::addL_Position_Detail_From_Local_Trade(USER_CThostFtdcTradeField *trade) {
	list<USER_INSTRUMENT_POSITION *>::iterator itor;
	bool isFind = false;

	for (itor = this->l_position_detail_from_local_trade->begin(); itor != this->l_position_detail_from_local_trade->end(); itor++)
	{

		if ((!strcmp((*itor)->InstrumentID, trade->InstrumentID))) // 如果相等
		{
			// 找到标志位赋值
			isFind = true;

			if ((!strcmp(this->getTradingDay().c_str(), trade->TradeDate))) //今天的持仓
			{
				if (trade->Direction == '0') { // 今买
					(*itor)->Buy_Today += trade->Volume;
				}
				else if (trade->Direction == '1') // 今卖
				{
					(*itor)->Sell_Today += trade->Volume;
				}
			}
			else // 昨天的持仓
			{
				if (trade->Direction == '0') { // 昨买
					(*itor)->Buy_Yesterday += trade->Volume;
				}
				else if (trade->Direction == '1') // 昨卖
				{
					(*itor)->Sell_Yesterday += trade->Volume;
				}
			}
		}


	}

	if (!isFind) // 如果未找到,直接添加到统计列表里
	{
		USER_INSTRUMENT_POSITION *new_position_from_local_trade = new USER_INSTRUMENT_POSITION();
		memset(new_position_from_local_trade, 0, sizeof(USER_INSTRUMENT_POSITION));
		strcpy(new_position_from_local_trade->InstrumentID, trade->InstrumentID);
		new_position_from_local_trade->Is_Same = false;

		if ((!strcmp(this->getTradingDay().c_str(), trade->TradeDate))) //今天的持仓
		{
			if (trade->Direction == '0') { // 今买
				new_position_from_local_trade->Buy_Today += trade->Volume;
			}
			else if (trade->Direction == '1') // 今卖
			{
				new_position_from_local_trade->Sell_Today += trade->Volume;
			}
		}
		else // 昨天的持仓
		{
			if (trade->Direction == '0') { // 昨买
				new_position_from_local_trade->Buy_Yesterday += trade->Volume;
			}
			else if (trade->Direction == '1') // 昨卖
			{
				new_position_from_local_trade->Sell_Yesterday += trade->Volume;
			}
		}
		this->l_position_detail_from_local_trade->push_back(new_position_from_local_trade);
	}
}

/// 获取统计持仓明细结果
void User::getL_Position_Detail_Data(list<USER_INSTRUMENT_POSITION *> *l_position_detail_cal) {
	std::cout << "\t=\tUser::getL_Position_Detail_Data()" << std::endl;
	std::cout << "\t=\t期货账户 = " << this->getUserID() << std::endl;
	list<USER_INSTRUMENT_POSITION *>::iterator itor;
	for (itor = l_position_detail_cal->begin(); itor != l_position_detail_cal->end(); itor++)
	{
		std::cout << "\t=\t\t**********" << std::endl;
		std::cout << "\t=\t\t*合约ID = " << (*itor)->InstrumentID << std::endl;
		std::cout << "\t=\t\t*今买 = " << (*itor)->Buy_Today << std::endl;
		std::cout << "\t=\t\t*今卖 = " << (*itor)->Sell_Today << std::endl;
		std::cout << "\t=\t\t*昨买 = " << (*itor)->Buy_Yesterday << std::endl;
		std::cout << "\t=\t\t*昨卖 = " << (*itor)->Sell_Yesterday << std::endl;
		std::cout << "\t=\t\t**********" << std::endl;
	}
}

/************************************************************************/
/* 获取数据库连接                                                         */
/************************************************************************/
mongo::DBClientConnection * User::GetPositionConn() {
	return this->PositionConn;
}
mongo::DBClientConnection * User::GetTradeConn() {
	return this->TradeConn;
}
mongo::DBClientConnection * User::GetOrderConn() {
	return this->OrderConn;
}
mongo::DBClientConnection * User::GetOrderActionConn() {
	return this->OrderActionConn;
}

/************************************************************************/
/* 完成Order的MongoDB操作                                                 */
/************************************************************************/
void User::DB_OrderInsert(mongo::DBClientConnection *conn, CThostFtdcInputOrderField *pInputOrder) {
	USER_PRINT("User::DB_OrderInsert DB Connection!");
	USER_PRINT(conn);
	BSONObjBuilder b;

	/// 交易员id
	b.append("OperatorID", this->getTraderID());

	///经纪公司代码
	b.append("BrokerID", pInputOrder->BrokerID);
	///投资者代码
	b.append("InvestorID", pInputOrder->InvestorID);
	///合约代码
	b.append("InstrumentID", pInputOrder->InstrumentID);
	///报单引用
	b.append("OrderRef", pInputOrder->OrderRef);
	///用户代码
	b.append("UserID", pInputOrder->UserID);
	///报单价格条件
	b.append("OrderPriceType", pInputOrder->OrderPriceType);
	///买卖方向
	b.append("Direction", pInputOrder->Direction);
	///组合开平标志
	b.append("CombOffsetFlag", pInputOrder->CombOffsetFlag);
	///组合投机套保标志
	b.append("CombHedgeFlag", pInputOrder->CombHedgeFlag);
	///价格
	b.append("LimitPrice", pInputOrder->LimitPrice);
	///数量
	b.append("VolumeTotalOriginal", pInputOrder->VolumeTotalOriginal);
	///有效期类型
	b.append("TimeCondition", pInputOrder->TimeCondition);
	///GTD日期
	b.append("GTDDate", pInputOrder->GTDDate);
	///成交量类型
	b.append("VolumeCondition", pInputOrder->VolumeCondition);
	///最小成交量
	b.append("MinVolume", pInputOrder->MinVolume);
	///触发条件
	b.append("ContingentCondition", pInputOrder->ContingentCondition);
	///止损价
	b.append("StopPrice", pInputOrder->StopPrice);
	///强平原因
	b.append("ForceCloseReason", pInputOrder->ForceCloseReason);
	///自动挂起标志
	b.append("IsAutoSuspend", pInputOrder->IsAutoSuspend);
	///业务单元
	b.append("BusinessUnit", pInputOrder->BusinessUnit);
	///请求编号
	b.append("RequestID", pInputOrder->RequestID);
	///用户强评标志
	b.append("UserForceClose", pInputOrder->UserForceClose);
	///互换单标志
	b.append("IsSwapOrder", pInputOrder->IsSwapOrder);
	///交易所代码
	b.append("ExchangeID", pInputOrder->ExchangeID);
	///投资单元代码
	b.append("InvestUnitID", pInputOrder->InvestUnitID);
	///资金账号
	b.append("AccountID", pInputOrder->AccountID);
	///币种代码
	b.append("CurrencyID", pInputOrder->CurrencyID);
	///交易编码
	b.append("ClientID", pInputOrder->ClientID);
	///IP地址
	b.append("IPAddress", pInputOrder->IPAddress);
	///Mac地址
	b.append("MacAddress", pInputOrder->MacAddress);

	string time_str = string(Utils::getNowTimeMs());
	int pos1 = time_str.find_first_of('_');
	int pos = time_str.find_last_of('_');
	USER_PRINT(time_str);
	USER_PRINT(time_str.length());
	USER_PRINT(pos1);
	USER_PRINT(pos);
	//std::cout << pos1 << '\n';
	//std::cout << pos << '\n';
	string SendOrderTime = time_str.substr(pos1 + 1, pos - 1);
	string SendOrderMicrosecond = time_str.substr(pos + 1);
	/// 插入时间
	b.append("SendOrderTime", SendOrderTime);
	/// 插入时间微秒
	b.append("SendOrderMicrosecond", SendOrderMicrosecond);

	/// 客户端账号（也能区分用户身份或交易员身份）:OperatorID
	b.append("OperatorID", this->getTraderID());

	string temp(pInputOrder->OrderRef);
	int len_order_ref = temp.length();
	string result = temp.substr(len_order_ref - 2, 2);
	/// 交易策略编号：StrategyID
	b.append("StrategyID", result);

	BSONObj p = b.obj();
	//conn->insert(DB_ORDERINSERT_COLLECTION, p);
	USER_PRINT("Begin Insert!");

	this->dbm->getConn()->insert(DB_ORDERINSERT_COLLECTION, p);

	/*string temp_order_ref_base = temp.substr(0, 10);
	USER_PRINT(temp_order_ref_base);
	this->DB_UpdateOrderRef(temp_order_ref_base);*/

	USER_PRINT("DBManager::DB_OrderInsert ok");
}

// 更新报单引用
void User::DB_UpdateOrderRef(string order_ref_base) {
	USER_PRINT("User::DB_UpdateOrderRef!");
	int len_order_ref_base = order_ref_base.length();
	USER_PRINT(order_ref_base);
	USER_PRINT(len_order_ref_base);
	this->dbm->UpdateFutureAccountOrderRef(this->OrderRefConn, this, order_ref_base);
}

void User::DB_OnRtnOrder(mongo::DBClientConnection *conn, CThostFtdcOrderField *pOrder){
	USER_PRINT("User::DB_OnRtnOrder DB Connection!");
	USER_PRINT(conn);
	BSONObjBuilder b;
	/// 交易员id
	b.append("OperatorID", this->getTraderID());
	///经纪公司代码
	b.append("BrokerID", pOrder->BrokerID);
	///投资者代码
	b.append("InvestorID", pOrder->InvestorID);
	///合约代码
	b.append("InstrumentID", pOrder->InstrumentID);
	///报单引用
	b.append("OrderRef", pOrder->OrderRef);
	///用户代码
	b.append("UserID", pOrder->UserID);
	///报单价格条件
	b.append("OrderPriceType", pOrder->OrderPriceType);
	///买卖方向
	b.append("Direction", pOrder->Direction);
	///组合开平标志
	b.append("CombOffsetFlag", pOrder->CombOffsetFlag);
	///组合投机套保标志
	b.append("CombHedgeFlag", pOrder->CombHedgeFlag);
	///价格
	b.append("LimitPrice", pOrder->LimitPrice);
	///数量
	b.append("VolumeTotalOriginal", pOrder->VolumeTotalOriginal);
	///有效期类型
	b.append("TimeCondition", pOrder->TimeCondition);
	///GTD日期
	b.append("GTDDate", pOrder->GTDDate);
	///成交量类型
	b.append("VolumeCondition", pOrder->VolumeCondition);
	///最小成交量
	b.append("MinVolume", pOrder->MinVolume);
	///触发条件
	b.append("ContingentCondition", pOrder->ContingentCondition);
	///止损价
	b.append("StopPrice", pOrder->StopPrice);
	///强平原因
	b.append("ForceCloseReason", pOrder->ForceCloseReason);
	///自动挂起标志
	b.append("IsAutoSuspend", pOrder->IsAutoSuspend);
	///业务单元
	b.append("BusinessUnit", pOrder->BusinessUnit);
	///请求编号
	b.append("RequestID", pOrder->RequestID);
	///本地报单编号
	b.append("OrderLocalID", pOrder->OrderLocalID);
	///交易所代码
	b.append("ExchangeID", pOrder->ExchangeID);
	///会员代码
	b.append("ParticipantID", pOrder->ParticipantID);
	///客户代码
	b.append("ClientID", pOrder->ClientID);
	///合约在交易所的代码
	b.append("ExchangeInstID", pOrder->ExchangeInstID);
	///交易所交易员代码
	b.append("TraderID", pOrder->TraderID);
	///安装编号
	b.append("InstallID", pOrder->InstallID);
	///报单提交状态
	b.append("OrderSubmitStatus", pOrder->OrderSubmitStatus);
	///报单提示序号
	b.append("NotifySequence", pOrder->NotifySequence);
	///交易日
	b.append("TradingDay", pOrder->TradingDay);
	///结算编号
	b.append("SettlementID", pOrder->SettlementID);
	///报单编号
	b.append("OrderSysID", pOrder->OrderSysID);
	///报单来源
	b.append("OrderSource", pOrder->OrderSource);
	///报单状态
	stringstream ss;
	string OrderStatus;
	ss << pOrder->OrderStatus;
	ss >> OrderStatus;
	b.append("OrderStatus", OrderStatus);
	///报单类型
	b.append("OrderType", pOrder->OrderType);
	///今成交数量
	b.append("VolumeTraded", pOrder->VolumeTraded);
	///剩余数量
	b.append("VolumeTotal", pOrder->VolumeTotal);
	///报单日期
	b.append("InsertDate", pOrder->InsertDate);
	///委托时间
	b.append("InsertTime", pOrder->InsertTime);
	///激活时间
	b.append("ActiveTime", pOrder->ActiveTime);
	///挂起时间
	b.append("SuspendTime", pOrder->SuspendTime);
	///最后修改时间
	b.append("UpdateTime", pOrder->UpdateTime);
	///撤销时间
	b.append("CancelTime", pOrder->CancelTime);
	///最后修改交易所交易员代码
	b.append("ActiveTraderID", pOrder->ActiveTraderID);
	///结算会员编号
	b.append("ClearingPartID", pOrder->ClearingPartID);
	///序号
	b.append("SequenceNo", pOrder->SequenceNo);
	///前置编号
	b.append("FrontID", pOrder->FrontID);
	///会话编号
	b.append("SessionID", pOrder->SessionID);
	///用户端产品信息
	b.append("UserProductInfo", pOrder->UserProductInfo);
	///状态信息
	///状态信息
	codeDst_2[90] = { 0 };
	Utils::Gb2312ToUtf8(codeDst_2, 90, pOrder->StatusMsg, strlen(pOrder->StatusMsg)); // Gb2312ToUtf8
	b.append("StatusMsg", codeDst_2);
	///用户强评标志
	b.append("UserForceClose", pOrder->UserForceClose);
	///操作用户代码
	b.append("ActiveUserID", pOrder->ActiveUserID);
	///经纪公司报单编号
	b.append("BrokerOrderSeq", pOrder->BrokerOrderSeq);
	///相关报单
	b.append("RelativeOrderSysID", pOrder->RelativeOrderSysID);
	///郑商所成交数量
	b.append("ZCETotalTradedVolume", pOrder->ZCETotalTradedVolume);
	///互换单标志
	b.append("IsSwapOrder", pOrder->IsSwapOrder);
	///营业部编号
	b.append("BranchID", pOrder->BranchID);
	///投资单元代码
	b.append("InvestUnitID", pOrder->InvestUnitID);
	///资金账号
	b.append("AccountID", pOrder->AccountID);
	///币种代码
	b.append("CurrencyID", pOrder->CurrencyID);
	///IP地址
	b.append("IPAddress", pOrder->IPAddress);
	///Mac地址
	b.append("MacAddress", pOrder->MacAddress);

	string time_str = Utils::getNowTimeMs();
	int pos1 = time_str.find_first_of('_');
	int pos = time_str.find_last_of('_');
	//std::cout << pos1 << '\n';
	//std::cout << pos << '\n';
	string CtpRtnOrderTime = time_str.substr(pos1 + 1, pos - 1);
	string CtpRtnOrderMicrosecond = time_str.substr(pos + 1);
	/// 插入时间
	b.append("CtpRtnOrderTime", CtpRtnOrderTime);
	/// 插入时间微秒
	b.append("CtpRtnOrderMicrosecond", CtpRtnOrderMicrosecond);

	/// 插入时间
	b.append("ExchRtnOrderTime", CtpRtnOrderTime);
	/// 插入时间微秒
	b.append("ExchRtnOrderMicrosecond", CtpRtnOrderMicrosecond);

	/// 客户端账号（也能区分用户身份或交易员身份）:OperatorID
	b.append("OperatorID", this->getTraderID());

	string temp(pOrder->OrderRef);
	USER_PRINT(temp);
	int len_order_ref = temp.length();
	string result = temp.substr(len_order_ref - 2, 2);
	/// 交易策略编号：StrategyID
	b.append("StrategyID", result);

	BSONObj p = b.obj();
	//conn->insert(DB_ONRTNORDER_COLLECTION, p);
	USER_PRINT("Begin Insert!");

	//this->dbm->getConn()->insert(DB_ONRTNORDER_COLLECTION, p);
	this->OrderConn->insert(DB_ONRTNORDER_COLLECTION, p);

	/*string temp_order_ref_base = temp.substr(0, 10);
	USER_PRINT(temp_order_ref_base);
	this->DB_UpdateOrderRef(temp_order_ref_base);*/
	USER_PRINT("DBManager::DB_OnRtnOrder ok");
	
}

void User::DB_OnRtnTrade(mongo::DBClientConnection *conn, CThostFtdcTradeField *pTrade){
	USER_PRINT(DB_ONRTNORDER_COLLECTION"User::DB_OnRtnTrade DB Connection!");
	BSONObjBuilder b;
	/// 交易员id
	b.append("OperatorID", this->getTraderID());
	///经纪公司代码
	b.append("BrokerID", pTrade->BrokerID);
	///投资者代码
	b.append("InvestorID", pTrade->InvestorID);
	///合约代码
	b.append("InstrumentID", pTrade->InstrumentID);
	///报单引用
	b.append("OrderRef", pTrade->OrderRef);
	///用户代码
	b.append("UserID", pTrade->UserID);
	///交易所代码
	b.append("ExchangeID", pTrade->ExchangeID);
	///成交编号
	b.append("TradeID", pTrade->TradeID);
	///买卖方向
	b.append("Direction", pTrade->Direction);
	///报单编号
	b.append("OrderSysID", pTrade->OrderSysID);
	///会员代码
	b.append("ParticipantID", pTrade->ParticipantID);
	///客户代码
	b.append("ClientID", pTrade->ClientID);
	///交易角色
	b.append("TradingRole", pTrade->TradingRole);
	///合约在交易所的代码
	b.append("ExchangeInstID", pTrade->ExchangeInstID);
	///开平标志
	b.append("OffsetFlag", pTrade->OffsetFlag);
	///投机套保标志
	b.append("HedgeFlag", pTrade->HedgeFlag);
	///价格
	b.append("Price", pTrade->Price);
	///数量
	b.append("Volume", pTrade->Volume);
	///成交时期
	b.append("TradeDate", pTrade->TradeDate);
	///成交时间
	b.append("TradeTime", pTrade->TradeTime);
	///成交类型
	b.append("TradeType", pTrade->TradeType);
	///成交价来源
	b.append("PriceSource", pTrade->PriceSource);
	///交易所交易员代码
	b.append("TraderID", pTrade->TraderID);
	///本地报单编号
	b.append("OrderLocalID", pTrade->OrderLocalID);
	///结算会员编号
	b.append("ClearingPartID", pTrade->ClearingPartID);
	///业务单元
	b.append("BusinessUnit", pTrade->BusinessUnit);
	///序号
	b.append("SequenceNo", pTrade->SequenceNo);
	///交易日
	b.append("TradingDay", pTrade->TradingDay);
	///结算编号
	b.append("SettlementID", pTrade->SettlementID);
	///经纪公司报单编号
	b.append("BrokerOrderSeq", pTrade->BrokerOrderSeq);
	///成交来源
	b.append("TradeSource", pTrade->TradeSource);


	string time_str = string(Utils::getNowTimeMs());
	int pos1 = time_str.find_first_of('_');
	int pos = time_str.find_last_of('_');
	//std::cout << "pos1" << pos1 << '\n';
	//std::cout << "pos" << pos << '\n';
	USER_PRINT(time_str);
	USER_PRINT(time_str.length());
	USER_PRINT(pos1);
	USER_PRINT(pos);
	string RecTradeTime = time_str.substr(pos1 + 1, pos - 1);
	USER_PRINT(RecTradeTime);
	USER_PRINT(time_str);
	string RecTradeMicrosecond = time_str.substr(21);
	USER_PRINT(RecTradeMicrosecond);
	/// 插入时间
	b.append("RecTradeTime", RecTradeTime);
	/// 插入时间微秒
	b.append("RecTradeMicrosecond", RecTradeMicrosecond);

	/// 客户端账号（也能区分用户身份或交易员身份）:OperatorID
	b.append("OperatorID", this->getTraderID());

	string temp(pTrade->OrderRef);
	USER_PRINT(pTrade->OrderRef);
	USER_PRINT(temp);
	USER_PRINT(temp.length());
	int len_order_ref = temp.length();
	string result = temp.substr(len_order_ref - 2, 2);
	USER_PRINT(result);
	/// 交易策略编号：StrategyID
	b.append("StrategyID", result);

	BSONObj p = b.obj();
	//conn->insert(DB_ONRTNTRADE_COLLECTION, p);

	//this->dbm->getConn()->insert(DB_ONRTNTRADE_COLLECTION, p);
	this->TradeConn->insert(DB_ONRTNTRADE_COLLECTION, p);
	
	USER_PRINT("DBManager::DB_OnRtnTrade ok");
}

void User::DB_OrderAction(mongo::DBClientConnection *conn, CThostFtdcInputOrderActionField *pOrderAction){
	USER_PRINT("User::DB_OrderAction DB Connection!");
	USER_PRINT(conn);
	BSONObjBuilder b;
	/// 交易员id
	b.append("OperatorID", this->getTraderID());
	///经纪公司代码
	b.append("BrokerID", pOrderAction->BrokerID);
	///投资者代码
	b.append("InvestorID", pOrderAction->InvestorID);
	///报单操作引用
	b.append("OrderActionRef", pOrderAction->OrderActionRef);
	///报单引用
	b.append("OrderRef", pOrderAction->OrderRef);
	///请求编号
	b.append("RequestID", pOrderAction->RequestID);
	///前置编号
	b.append("FrontID", pOrderAction->FrontID);
	///会话编号
	b.append("SessionID", pOrderAction->SessionID);
	///交易所代码
	b.append("ExchangeID", pOrderAction->ExchangeID);
	///报单编号
	b.append("OrderSysID", pOrderAction->OrderSysID);
	///操作标志
	b.append("ActionFlag", pOrderAction->ActionFlag);
	///价格
	b.append("LimitPrice", pOrderAction->LimitPrice);
	///数量变化
	b.append("VolumeChange", pOrderAction->VolumeChange);
	///用户代码
	b.append("UserID", pOrderAction->UserID);
	///合约代码
	b.append("InstrumentID", pOrderAction->InstrumentID);
	///投资单元代码
	b.append("InvestUnitID", pOrderAction->InvestUnitID);
	///IP地址
	b.append("IPAddress", pOrderAction->IPAddress);
	///Mac地址
	b.append("MacAddress", pOrderAction->MacAddress);
	BSONObj p = b.obj();
	this->OrderActionConn->insert(DB_ORDERACTION_COLLECTION, p);

	USER_PRINT("DBManager::DB_OrderAction ok");
}

void User::DB_OrderCombine(mongo::DBClientConnection *conn, CThostFtdcOrderField *pOrder){
	USER_PRINT("User::DB_OrderCombine DB Connection!");
	USER_PRINT(conn);
	BSONObjBuilder b;
	/// 交易员id
	b.append("OperatorID", this->getTraderID());
	///经纪公司代码
	b.append("BrokerID", pOrder->BrokerID);
	///投资者代码
	b.append("InvestorID", pOrder->InvestorID);
	///合约代码
	b.append("InstrumentID", pOrder->InstrumentID);
	///报单引用
	b.append("OrderRef", pOrder->OrderRef);
	///用户代码
	b.append("UserID", pOrder->UserID);
	///报单价格条件
	b.append("OrderPriceType", pOrder->OrderPriceType);
	///买卖方向
	b.append("Direction", pOrder->Direction);
	///组合开平标志
	b.append("CombOffsetFlag", pOrder->CombOffsetFlag);
	///组合投机套保标志
	b.append("CombHedgeFlag", pOrder->CombHedgeFlag);
	///价格
	b.append("LimitPrice", pOrder->LimitPrice);
	///数量
	b.append("VolumeTotalOriginal", pOrder->VolumeTotalOriginal);
	///有效期类型
	b.append("TimeCondition", pOrder->TimeCondition);
	///GTD日期
	b.append("GTDDate", pOrder->GTDDate);
	///成交量类型
	b.append("VolumeCondition", pOrder->VolumeCondition);
	///最小成交量
	b.append("MinVolume", pOrder->MinVolume);
	///触发条件
	b.append("ContingentCondition", pOrder->ContingentCondition);
	///止损价
	b.append("StopPrice", pOrder->StopPrice);
	///强平原因
	b.append("ForceCloseReason", pOrder->ForceCloseReason);
	///自动挂起标志
	b.append("IsAutoSuspend", pOrder->IsAutoSuspend);
	///业务单元
	b.append("BusinessUnit", pOrder->BusinessUnit);
	///请求编号
	b.append("RequestID", pOrder->RequestID);
	///本地报单编号
	b.append("OrderLocalID", pOrder->OrderLocalID);
	///交易所代码
	b.append("ExchangeID", pOrder->ExchangeID);
	///会员代码
	b.append("ParticipantID", pOrder->ParticipantID);
	///客户代码
	b.append("ClientID", pOrder->ClientID);
	///合约在交易所的代码
	b.append("ExchangeInstID", pOrder->ExchangeInstID);
	///交易所交易员代码
	b.append("TraderID", pOrder->TraderID);
	///安装编号
	b.append("InstallID", pOrder->InstallID);
	///报单提交状态
	b.append("OrderSubmitStatus", pOrder->OrderSubmitStatus);
	///报单提示序号
	b.append("NotifySequence", pOrder->NotifySequence);
	///交易日
	b.append("TradingDay", pOrder->TradingDay);
	///结算编号
	b.append("SettlementID", pOrder->SettlementID);
	///报单编号
	b.append("OrderSysID", pOrder->OrderSysID);
	///报单来源
	b.append("OrderSource", pOrder->OrderSource);
	///报单状态
	stringstream ss;
	string OrderStatus;
	ss << pOrder->OrderStatus;
	ss >> OrderStatus;
	b.append("OrderStatus", OrderStatus);
	///报单类型
	b.append("OrderType", pOrder->OrderType);
	///今成交数量
	b.append("VolumeTraded", pOrder->VolumeTraded);
	///剩余数量
	b.append("VolumeTotal", pOrder->VolumeTotal);
	///报单日期
	b.append("InsertDate", pOrder->InsertDate);
	///委托时间
	b.append("InsertTime", pOrder->InsertTime);
	///激活时间
	b.append("ActiveTime", pOrder->ActiveTime);
	///挂起时间
	b.append("SuspendTime", pOrder->SuspendTime);
	///最后修改时间
	b.append("UpdateTime", pOrder->UpdateTime);
	///撤销时间
	b.append("CancelTime", pOrder->CancelTime);
	///最后修改交易所交易员代码
	b.append("ActiveTraderID", pOrder->ActiveTraderID);
	///结算会员编号
	b.append("ClearingPartID", pOrder->ClearingPartID);
	///序号
	b.append("SequenceNo", pOrder->SequenceNo);
	///前置编号
	b.append("FrontID", pOrder->FrontID);
	///会话编号
	b.append("SessionID", pOrder->SessionID);
	///用户端产品信息
	b.append("UserProductInfo", pOrder->UserProductInfo);
	///状态信息
	b.append("StatusMsg", pOrder->StatusMsg);
	///用户强评标志
	b.append("UserForceClose", pOrder->UserForceClose);
	///操作用户代码
	b.append("ActiveUserID", pOrder->ActiveUserID);
	///经纪公司报单编号
	b.append("BrokerOrderSeq", pOrder->BrokerOrderSeq);
	///相关报单
	b.append("RelativeOrderSysID", pOrder->RelativeOrderSysID);
	///郑商所成交数量
	b.append("ZCETotalTradedVolume", pOrder->ZCETotalTradedVolume);
	///互换单标志
	b.append("IsSwapOrder", pOrder->IsSwapOrder);
	///营业部编号
	b.append("BranchID", pOrder->BranchID);
	///投资单元代码
	b.append("InvestUnitID", pOrder->InvestUnitID);
	///资金账号
	b.append("AccountID", pOrder->AccountID);
	///币种代码
	b.append("CurrencyID", pOrder->CurrencyID);
	///IP地址
	b.append("IPAddress", pOrder->IPAddress);
	///Mac地址
	b.append("MacAddress", pOrder->MacAddress);
	BSONObj p = b.obj();
	conn->insert(DB_ORDERCOMBINE_COLLECTION, p);
	USER_PRINT("DBManager::DB_OrderCombine ok");
}

void User::DB_OnRspOrderAction(mongo::DBClientConnection *conn, CThostFtdcInputOrderActionField *pInputOrderAction){
	USER_PRINT("User::DB_OnRspOrderAction DB Connection!");
	USER_PRINT(conn);
	BSONObjBuilder b;
	/// 交易员id
	b.append("OperatorID", this->getTraderID());
	///经纪公司代码
	b.append("BrokerID", pInputOrderAction->BrokerID);
	///投资者代码
	b.append("InvestorID", pInputOrderAction->InvestorID);
	///报单操作引用
	b.append("OrderActionRef", pInputOrderAction->OrderActionRef);
	///报单引用
	b.append("OrderRef", pInputOrderAction->OrderRef);
	///请求编号
	b.append("RequestID", pInputOrderAction->RequestID);
	///前置编号
	b.append("FrontID", pInputOrderAction->FrontID);
	///会话编号
	b.append("SessionID", pInputOrderAction->SessionID);
	///交易所代码
	b.append("ExchangeID", pInputOrderAction->ExchangeID);
	///报单编号
	b.append("OrderSysID", pInputOrderAction->OrderSysID);
	///操作标志
	b.append("ActionFlag", pInputOrderAction->ActionFlag);
	///价格
	b.append("LimitPrice", pInputOrderAction->LimitPrice);
	///数量变化
	b.append("VolumeChange", pInputOrderAction->VolumeChange);
	///用户代码
	b.append("UserID", pInputOrderAction->UserID);
	///合约代码
	b.append("InstrumentID", pInputOrderAction->InstrumentID);
	///投资单元代码
	b.append("InvestUnitID", pInputOrderAction->InvestUnitID);
	///IP地址
	b.append("IPAddress", pInputOrderAction->IPAddress);
	///Mac地址
	b.append("MacAddress", pInputOrderAction->MacAddress);
	BSONObj p = b.obj();
	conn->insert(DB_ONRSPORDERACTION_COLLECTION, p);
	USER_PRINT("DBManager::DB_OnRspOrderAction ok");
} // CTP认为撤单参数错误

void User::DB_OnErrRtnOrderAction(mongo::DBClientConnection *conn, CThostFtdcOrderActionField *pOrderAction){
	USER_PRINT("User::DB_OnErrRtnOrderAction DB Connection!");
	USER_PRINT(conn);
	BSONObjBuilder b;
	/// 交易员id
	b.append("OperatorID", this->getTraderID());
	///经纪公司代码
	b.append("BrokerID", pOrderAction->BrokerID);
	///投资者代码
	b.append("InvestorID", pOrderAction->InvestorID);
	///报单操作引用
	b.append("OrderActionRef", pOrderAction->OrderActionRef);
	///报单引用
	b.append("OrderRef", pOrderAction->OrderRef);
	///请求编号
	b.append("RequestID", pOrderAction->RequestID);
	///前置编号
	b.append("FrontID", pOrderAction->FrontID);
	///会话编号
	b.append("SessionID", pOrderAction->SessionID);
	///交易所代码
	b.append("ExchangeID", pOrderAction->ExchangeID);
	///报单编号
	b.append("OrderSysID", pOrderAction->OrderSysID);
	///操作标志
	b.append("ActionFlag", pOrderAction->ActionFlag);
	///价格
	b.append("LimitPrice", pOrderAction->LimitPrice);
	///数量变化
	b.append("VolumeChange", pOrderAction->VolumeChange);
	///操作日期
	b.append("ActionDate", pOrderAction->ActionDate);
	///操作时间
	b.append("ActionTime", pOrderAction->ActionTime);
	///交易所交易员代码
	b.append("TraderID", pOrderAction->TraderID);
	///安装编号
	b.append("InstallID", pOrderAction->InstallID);
	///本地报单编号
	b.append("OrderLocalID", pOrderAction->OrderLocalID);
	///操作本地编号
	b.append("ActionLocalID", pOrderAction->ActionLocalID);
	///会员代码
	b.append("ParticipantID", pOrderAction->ParticipantID);
	///客户代码
	b.append("ClientID", pOrderAction->ClientID);
	///业务单元
	b.append("BusinessUnit", pOrderAction->BusinessUnit);
	///报单操作状态
	b.append("OrderActionStatus", pOrderAction->OrderActionStatus);
	///用户代码
	b.append("UserID", pOrderAction->UserID);
	///状态信息
	b.append("StatusMsg", pOrderAction->StatusMsg);
	///合约代码
	b.append("InstrumentID", pOrderAction->InstrumentID);
	///营业部编号
	b.append("BranchID", pOrderAction->BranchID);
	///投资单元代码
	b.append("InvestUnitID", pOrderAction->InvestUnitID);
	///IP地址
	b.append("IPAddress", pOrderAction->IPAddress);
	///Mac地址
	b.append("MacAddress", pOrderAction->MacAddress);
	BSONObj p = b.obj();
	conn->insert(DB_ONERRRTNORDERACTION_COLLECTION, p);
	USER_PRINT("DBManager::DB_OnErrRtnOrderAction ok");
} // 交易所认为撤单错误

void User::DB_OnRspOrderInsert(mongo::DBClientConnection *conn, CThostFtdcInputOrderField *pInputOrder){
	USER_PRINT("User::DB_OnRspOrderInsert DB Connection!");
	USER_PRINT(conn);
	BSONObjBuilder b;
	/// 交易员id
	b.append("OperatorID", this->getTraderID());
	///经纪公司代码
	b.append("BrokerID", pInputOrder->BrokerID);
	///投资者代码
	b.append("InvestorID", pInputOrder->InvestorID);
	///合约代码
	b.append("InstrumentID", pInputOrder->InstrumentID);
	///报单引用
	b.append("OrderRef", pInputOrder->OrderRef);
	///用户代码
	b.append("UserID", pInputOrder->UserID);
	///报单价格条件
	b.append("OrderPriceType", pInputOrder->OrderPriceType);
	///买卖方向
	b.append("Direction", pInputOrder->Direction);
	///组合开平标志
	b.append("CombOffsetFlag", pInputOrder->CombOffsetFlag);
	///组合投机套保标志
	b.append("CombHedgeFlag", pInputOrder->CombHedgeFlag);
	///价格
	b.append("LimitPrice", pInputOrder->LimitPrice);
	///数量
	b.append("VolumeTotalOriginal", pInputOrder->VolumeTotalOriginal);
	///有效期类型
	b.append("TimeCondition", pInputOrder->TimeCondition);
	///GTD日期
	b.append("GTDDate", pInputOrder->GTDDate);
	///成交量类型
	b.append("VolumeCondition", pInputOrder->VolumeCondition);
	///最小成交量
	b.append("MinVolume", pInputOrder->MinVolume);
	///触发条件
	b.append("ContingentCondition", pInputOrder->ContingentCondition);
	///止损价
	b.append("StopPrice", pInputOrder->StopPrice);
	///强平原因
	b.append("ForceCloseReason", pInputOrder->ForceCloseReason);
	///自动挂起标志
	b.append("IsAutoSuspend", pInputOrder->IsAutoSuspend);
	///业务单元
	b.append("BusinessUnit", pInputOrder->BusinessUnit);
	///请求编号
	b.append("RequestID", pInputOrder->RequestID);
	///用户强评标志
	b.append("UserForceClose", pInputOrder->UserForceClose);
	///互换单标志
	b.append("IsSwapOrder", pInputOrder->IsSwapOrder);
	///交易所代码
	b.append("ExchangeID", pInputOrder->ExchangeID);
	///投资单元代码
	b.append("InvestUnitID", pInputOrder->InvestUnitID);
	///资金账号
	b.append("AccountID", pInputOrder->AccountID);
	///币种代码
	b.append("CurrencyID", pInputOrder->CurrencyID);
	///交易编码
	b.append("ClientID", pInputOrder->ClientID);
	///IP地址
	b.append("IPAddress", pInputOrder->IPAddress);
	///Mac地址
	b.append("MacAddress", pInputOrder->MacAddress);
	
	BSONObj p = b.obj();
	conn->insert(DB_ONRSPORDERINSERT_COLLECTION, p);
	USER_PRINT("DBManager::DB_OnRspOrderInsert ok");
} // CTP认为报单参数错误


void User::DB_OnErrRtnOrderInsert(mongo::DBClientConnection *conn, CThostFtdcInputOrderField *pInputOrder){
	USER_PRINT("User::DB_OnErrRtnOrderInsert DB Connection!");
	USER_PRINT(conn);
	BSONObjBuilder b;
	/// 交易员id
	b.append("OperatorID", this->getTraderID());
	///经纪公司代码
	b.append("BrokerID", pInputOrder->BrokerID);
	///投资者代码
	b.append("InvestorID", pInputOrder->InvestorID);
	///合约代码
	b.append("InstrumentID", pInputOrder->InstrumentID);
	///报单引用
	b.append("OrderRef", pInputOrder->OrderRef);
	///用户代码
	b.append("UserID", pInputOrder->UserID);
	///报单价格条件
	b.append("OrderPriceType", pInputOrder->OrderPriceType);
	///买卖方向
	b.append("Direction", pInputOrder->Direction);
	///组合开平标志
	b.append("CombOffsetFlag", pInputOrder->CombOffsetFlag);
	///组合投机套保标志
	b.append("CombHedgeFlag", pInputOrder->CombHedgeFlag);
	///价格
	b.append("LimitPrice", pInputOrder->LimitPrice);
	///数量
	b.append("VolumeTotalOriginal", pInputOrder->VolumeTotalOriginal);
	///有效期类型
	b.append("TimeCondition", pInputOrder->TimeCondition);
	///GTD日期
	b.append("GTDDate", pInputOrder->GTDDate);
	///成交量类型
	b.append("VolumeCondition", pInputOrder->VolumeCondition);
	///最小成交量
	b.append("MinVolume", pInputOrder->MinVolume);
	///触发条件
	b.append("ContingentCondition", pInputOrder->ContingentCondition);
	///止损价
	b.append("StopPrice", pInputOrder->StopPrice);
	///强平原因
	b.append("ForceCloseReason", pInputOrder->ForceCloseReason);
	///自动挂起标志
	b.append("IsAutoSuspend", pInputOrder->IsAutoSuspend);
	///业务单元
	b.append("BusinessUnit", pInputOrder->BusinessUnit);
	///请求编号
	b.append("RequestID", pInputOrder->RequestID);
	///用户强评标志
	b.append("UserForceClose", pInputOrder->UserForceClose);
	///互换单标志
	b.append("IsSwapOrder", pInputOrder->IsSwapOrder);
	///交易所代码
	b.append("ExchangeID", pInputOrder->ExchangeID);
	///投资单元代码
	b.append("InvestUnitID", pInputOrder->InvestUnitID);
	///资金账号
	b.append("AccountID", pInputOrder->AccountID);
	///币种代码
	b.append("CurrencyID", pInputOrder->CurrencyID);
	///交易编码
	b.append("ClientID", pInputOrder->ClientID);
	///IP地址
	b.append("IPAddress", pInputOrder->IPAddress);
	///Mac地址
	b.append("MacAddress", pInputOrder->MacAddress);
	BSONObj p = b.obj();
	conn->insert(DB_ONERRRTNORDERINSERT_COLLECTION, p);
	USER_PRINT("DBManager::DB_OnErrRtnOrderInsert ok");
} // 交易所认为报单错误

void User::DB_OnRspQryInvestorPosition(mongo::DBClientConnection *conn, CThostFtdcInvestorPositionField *pInvestorPosition){
	USER_PRINT("User::DB_OnRspQryInvestorPosition DB Connection!");
	USER_PRINT(conn);
	BSONObjBuilder b;
	/// 交易员id
	b.append("OperatorID", this->getTraderID());
	///合约代码
	b.append("InstrumentID", pInvestorPosition->InstrumentID);
	///经纪公司代码
	b.append("BrokerID", pInvestorPosition->BrokerID);
	///投资者代码
	b.append("InvestorID", pInvestorPosition->InvestorID);
	///持仓多空方向
	b.append("PosiDirection", pInvestorPosition->PosiDirection);
	///投机套保标志
	b.append("HedgeFlag", pInvestorPosition->HedgeFlag);
	///持仓日期
	b.append("PositionDate", pInvestorPosition->PositionDate);
	///上日持仓
	b.append("YdPosition", pInvestorPosition->YdPosition);
	///今日持仓
	b.append("Position", pInvestorPosition->Position);
	///多头冻结
	b.append("LongFrozen", pInvestorPosition->LongFrozen);
	///空头冻结
	b.append("ShortFrozen", pInvestorPosition->ShortFrozen);
	///开仓冻结金额
	b.append("LongFrozenAmount", pInvestorPosition->LongFrozenAmount);
	///开仓冻结金额
	b.append("ShortFrozenAmount", pInvestorPosition->ShortFrozenAmount);
	///开仓量
	b.append("OpenVolume", pInvestorPosition->OpenVolume);
	///平仓量
	b.append("CloseVolume", pInvestorPosition->CloseVolume);
	///开仓金额
	b.append("OpenAmount", pInvestorPosition->OpenAmount);
	///平仓金额
	b.append("CloseAmount", pInvestorPosition->CloseAmount);
	///持仓成本
	b.append("PositionCost", pInvestorPosition->PositionCost);
	///上次占用的保证金
	b.append("PreMargin", pInvestorPosition->PreMargin);
	///占用的保证金
	b.append("UseMargin", pInvestorPosition->UseMargin);
	///冻结的保证金
	b.append("FrozenMargin", pInvestorPosition->FrozenMargin);
	///冻结的资金
	b.append("FrozenCash", pInvestorPosition->FrozenCash);
	///冻结的手续费
	b.append("FrozenCommission", pInvestorPosition->FrozenCommission);
	///资金差额
	b.append("CashIn", pInvestorPosition->CashIn);
	///手续费
	b.append("Commission", pInvestorPosition->Commission);
	///平仓盈亏
	b.append("CloseProfit", pInvestorPosition->CloseProfit);
	///持仓盈亏
	b.append("PositionProfit", pInvestorPosition->PositionProfit);
	///上次结算价
	b.append("PreSettlementPrice", pInvestorPosition->PreSettlementPrice);
	///本次结算价
	b.append("SettlementPrice", pInvestorPosition->SettlementPrice);
	///交易日
	b.append("TradingDay", pInvestorPosition->TradingDay);
	///结算编号
	b.append("SettlementID", pInvestorPosition->SettlementID);
	///开仓成本
	b.append("OpenCost", pInvestorPosition->OpenCost);
	///交易所保证金
	b.append("ExchangeMargin", pInvestorPosition->ExchangeMargin);
	///组合成交形成的持仓
	b.append("CombPosition", pInvestorPosition->CombPosition);
	///组合多头冻结
	b.append("CombLongFrozen", pInvestorPosition->CombLongFrozen);
	///组合空头冻结
	b.append("CombShortFrozen", pInvestorPosition->CombShortFrozen);
	///逐日盯市平仓盈亏
	b.append("CloseProfitByDate", pInvestorPosition->CloseProfitByDate);
	///逐笔对冲平仓盈亏
	b.append("CloseProfitByTrade", pInvestorPosition->CloseProfitByTrade);
	///今日持仓
	b.append("TodayPosition", pInvestorPosition->TodayPosition);
	///保证金率
	b.append("MarginRateByMoney", pInvestorPosition->MarginRateByMoney);
	///保证金率(按手数)
	b.append("MarginRateByVolume", pInvestorPosition->MarginRateByVolume);
	///执行冻结
	b.append("StrikeFrozen", pInvestorPosition->StrikeFrozen);
	///执行冻结金额
	b.append("StrikeFrozenAmount", pInvestorPosition->StrikeFrozenAmount);
	///放弃执行冻结
	b.append("AbandonFrozen", pInvestorPosition->AbandonFrozen);
	BSONObj p = b.obj();
	conn->insert(DB_ONRSPQRYINVESTORPOSITION, p);
	USER_PRINT("DBManager::DB_OnRspQryInvestorPosition ok");
} // 持仓信息

/// 设置开关
int User::getOn_Off() {
	return this->on_off;
}
void User::setOn_Off(int on_off) {
	this->on_off = on_off;
}

/// 设置线程初始化状态，完成为true，否则false
bool User::getThread_Init_Status() {
	return this->thread_init_status;
}

void User::setThread_Init_Status(bool thread_init_status) {
	this->thread_init_status = thread_init_status;
}

/// 设置CTP_Manager
void User::setCTP_Manager(CTP_Manager *o_ctp) {
	this->o_ctp = o_ctp;
}
CTP_Manager * User::getCTP_Manager() {
	return this->o_ctp;
}

void User::setIsActive(string isActive) {
	this->isActive = isActive;
}
string User::getIsActive() {
	return this->isActive;
}

void User::setTradingDay(string stg_trading_day) {
	this->trading_day = stg_trading_day;
}

string User::getTradingDay() {
	return this->trading_day;
}

void User::QueryTrade() {
	this->UserTradeSPI->QryTrade();
}

void User::QueryOrder() {
	this->UserTradeSPI->QryOrder();
}

/// 得到数据库操作对象
DBManager * User::getDBManager() {
	return this->dbm;
}

void User::setDBManager(DBManager *dbm) {
	this->dbm = dbm;
}

int User::getSessionID() {
	return this->sid;
}

void User::setSessionID(int SessionID) {
	this->sid = SessionID;
}

list<Session *> * User::getL_Sessions() {
	return this->l_sessions;
}

void User::setL_Sessions(list<Session *> *l_sessions) {
	this->l_sessions = l_sessions;
}

// 对比本地维护持仓结果与CTP API返回持仓对比结果
bool User::ComparePositionTotal() {
	std::cout << "User::ComparePositionTotal()" << std::endl;
	bool is_same_flag = true;

	std::cout << "\tthis->l_position_detail_from_ctp->size() = " << this->l_position_detail_from_ctp->size() << std::endl;
	std::cout << "\tthis->l_position_detail_from_local_order->size() = " << this->l_position_detail_from_local_order->size() << std::endl;
	std::cout << "\tthis->l_position_detail_from_local_trade->size() = " << this->l_position_detail_from_local_trade->size() << std::endl;

	//判断比较的长度,有其中一组长度不相等,即存在错误仓位
	if ((this->l_position_detail_from_ctp->size() != this->l_position_detail_from_local_order->size()) ||
		(this->l_position_detail_from_local_order->size() != this->l_position_detail_from_local_trade->size())) {
		std::cout << "\t期货账户 = " << this->getUserID() << std::endl;
		std::cout << "\t持仓明细列表数目总数不正确!" << std::endl;
		is_same_flag = false;
		this->setIsPositionRight(is_same_flag);
		return is_same_flag;
	}

	// 如果全部size为0,直接返回,提高执行效率
	if ((this->l_position_detail_from_ctp->size() == 0) &&
		(this->l_position_detail_from_local_order->size() == 0) &&
		(this->l_position_detail_from_local_trade->size() == 0)
		)
	{
		std::cout << "\t期货账户 = " << this->getUserID() << std::endl;
		std::cout << "\t持仓明细均为空!" << std::endl;
		return is_same_flag;
	}

	


	list<USER_INSTRUMENT_POSITION *>::iterator order_itor;
	list<USER_INSTRUMENT_POSITION *>::iterator trade_itor;
	list<USER_INSTRUMENT_POSITION *>::iterator ctp_itor;




	//比较order 与 trade
	for (order_itor = this->l_position_detail_from_local_order->begin(); order_itor != this->l_position_detail_from_local_order->end(); order_itor++)
	{
		for (trade_itor = this->l_position_detail_from_local_trade->begin(); trade_itor != this->l_position_detail_from_local_trade->end(); trade_itor++)
		{
			if ((!strcmp((*order_itor)->InstrumentID, (*trade_itor)->InstrumentID)))
			{
				if (((*order_itor)->Buy_Today == (*trade_itor)->Buy_Today) &&
					((*order_itor)->Buy_Yesterday == (*trade_itor)->Buy_Yesterday) &&
					((*order_itor)->Sell_Today == (*trade_itor)->Sell_Today) &&
					((*order_itor)->Sell_Yesterday == (*trade_itor)->Sell_Yesterday)
					)
				{
					(*order_itor)->Is_Same = true;
					(*trade_itor)->Is_Same = true;
				}
			}
		}
	}

	// 对比order 与 trade后检查标志位
	for (order_itor = this->l_position_detail_from_local_order->begin(); order_itor != this->l_position_detail_from_local_order->end(); order_itor++)
	{
		if (!(*order_itor)->Is_Same)
		{
			is_same_flag = false;
		}
	}
	for (trade_itor = this->l_position_detail_from_local_trade->begin(); trade_itor != this->l_position_detail_from_local_trade->end(); trade_itor++)
	{
		if (!(*trade_itor)->Is_Same)
		{
			is_same_flag = false;
		}
	}


	if (!is_same_flag) {
		std::cout << "\t期货账户 = " << this->getUserID() << std::endl;
		std::cout << "\tOrder与Trade维护持仓明细不一致!请重新初始化或检查数据!" << std::endl;
		this->setIsPositionRight(is_same_flag);
		return is_same_flag;
	}

	// 对比ctp 与 order
	for (order_itor = this->l_position_detail_from_local_order->begin(); order_itor != this->l_position_detail_from_local_order->end(); order_itor++)
	{
		for (ctp_itor = this->l_position_detail_from_ctp->begin(); ctp_itor != this->l_position_detail_from_ctp->end(); ctp_itor++)
		{
			if ((!strcmp((*order_itor)->InstrumentID, (*ctp_itor)->InstrumentID)))
			{
				if (((*order_itor)->Buy_Today == (*ctp_itor)->Buy_Today) &&
					((*order_itor)->Buy_Yesterday == (*ctp_itor)->Buy_Yesterday) &&
					((*order_itor)->Sell_Today == (*ctp_itor)->Sell_Today) &&
					((*order_itor)->Sell_Yesterday == (*ctp_itor)->Sell_Yesterday)
					)
				{
					(*order_itor)->Is_Same = true;
					(*ctp_itor)->Is_Same = true;
				}
			}
		}
	}

	// 对比order 与 trade后检查标志位
	for (order_itor = this->l_position_detail_from_local_order->begin(); order_itor != this->l_position_detail_from_local_order->end(); order_itor++)
	{
		if (!(*order_itor)->Is_Same)
		{
			is_same_flag = false;
		}
	}
	
	for (ctp_itor = this->l_position_detail_from_ctp->begin(); ctp_itor != this->l_position_detail_from_ctp->end(); ctp_itor++)
	{
		if (!(*ctp_itor)->Is_Same)
		{
			is_same_flag = false;
		}
	}

	if (!is_same_flag) {
		std::cout << "\t期货账户 = " << this->getUserID() << std::endl;
		std::cout << "\tOrder与Trade维护持仓明细不一致!请重新初始化或检查数据!" << std::endl;
		this->setIsPositionRight(is_same_flag);
		return is_same_flag;
	}

	return is_same_flag;
}
#include <algorithm>
#include <sstream>
#include "Strategy.h"

Strategy::Strategy() {
	this->on_off = 0;

	this->l_instruments = new list<string>();

	this->stg_list_order_pending = new list<CThostFtdcOrderField *>();
	this->stg_list_position_detail = new list<CThostFtdcTradeField *>();

	this->stg_a_order_insert_args = new CThostFtdcInputOrderField();
	this->stg_b_order_insert_args = new CThostFtdcInputOrderField();

	stg_instrument_A_tick = new CThostFtdcDepthMarketDataField();	// A合约tick（第一腿）
	stg_instrument_B_tick = new CThostFtdcDepthMarketDataField();	// B合约tick（第二腿）
	stg_instrument_A_tick_last = new CThostFtdcDepthMarketDataField();	// A合约tick（第一腿）交易前最后一次
	stg_instrument_B_tick_last = new CThostFtdcDepthMarketDataField();	// B合约tick（第二腿）交易前最后一次

	this->stg_trade_tasking = false;
}

/// 初始化
void Strategy::InitStrategy() {
	
}

/// 拷贝结构体CThostFtdcDepthMarketDataField
void Strategy::CopyTickData(CThostFtdcDepthMarketDataField *dst, CThostFtdcDepthMarketDataField *src) {
	///交易日
	strcpy(dst->TradingDay, src->TradingDay);
	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);
	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);
	///合约在交易所的代码
	strcpy(dst->ExchangeInstID, src->ExchangeInstID);
	///最新价
	dst->LastPrice = src->LastPrice;
	///上次结算价
	dst->PreSettlementPrice = src->PreSettlementPrice;
	///昨收盘
	dst->PreClosePrice = src->PreClosePrice;
	///昨持仓量
	dst->PreOpenInterest = src->PreOpenInterest;
	///今开盘
	dst->OpenPrice = src->OpenPrice;
	///最高价
	dst->HighestPrice = src->HighestPrice;
	///最低价
	dst->LowestPrice = src->LowestPrice;
	///数量
	dst->Volume = src->Volume;
	///成交金额
	dst->Turnover = src->Turnover;
	///持仓量
	dst->OpenInterest = src->OpenInterest;
	///今收盘
	dst->ClosePrice = src->ClosePrice;
	///本次结算价
	dst->SettlementPrice = src->SettlementPrice;
	///涨停板价
	dst->UpperLimitPrice = src->UpperLimitPrice;
	///跌停板价
	dst->LowerLimitPrice = src->LowerLimitPrice;
	///昨虚实度
	dst->PreDelta = src->PreDelta;
	///今虚实度
	dst->CurrDelta = src->CurrDelta;
	///最后修改时间
	strcpy(dst->UpdateTime, src->UpdateTime);
	///最后修改毫秒
	dst->UpdateMillisec = src->UpdateMillisec;
	///申买价一
	dst->BidPrice1 = src->BidPrice1;
	///申买量一
	dst->BidVolume1 = src->BidVolume1;
	///申卖价一
	dst->AskPrice1 = src->AskPrice1;
	///申卖量一
	dst->AskVolume1 = src->AskVolume1;
	///申买价二
	dst->BidPrice2 = src->BidPrice2;
	///申买量二
	dst->BidVolume2 = src->BidVolume2;
	///申卖价二
	dst->AskPrice2 = src->AskPrice2;
	///申卖量二
	dst->AskVolume2 = src->AskVolume2;
	///申买价三
	dst->BidPrice3 = src->BidPrice3;
	///申买量三
	dst->BidVolume3 = src->BidVolume3;
	///申卖价三
	dst->AskPrice3 = src->AskPrice3;
	///申卖量三
	dst->AskVolume3 = src->AskVolume3;
	///申买价四
	dst->BidPrice4 = src->BidPrice4;
	///申买量四
	dst->BidVolume4 = src->BidVolume4;
	///申卖价四
	dst->AskPrice4 = src->AskPrice4;
	///申卖量四
	dst->AskVolume4 = src->AskVolume4;
	///申买价五
	dst->BidPrice5 = src->BidPrice5;
	///申买量五
	dst->BidVolume5 = src->BidVolume5;
	///申卖价五
	dst->AskPrice5 = src->AskPrice5;
	///申卖量五
	dst->AskVolume5 = src->AskVolume5;
	///当日均价
	dst->AveragePrice = src->AveragePrice;
	///业务日期
	strcpy(dst->ActionDay, src->ActionDay);
}

/// 拷贝结构体CThostFtdcOrderField
void Strategy::CopyOrderData(CThostFtdcOrderField *dst, CThostFtdcOrderField *src) {
	
	///经纪公司代码
	strcpy(dst->BrokerID, src->BrokerID);

	///投资者代码
	strcpy(dst->InvestorID, src->InvestorID);

	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);

	///报单引用
	strcpy(dst->OrderRef, src->OrderRef);

	///用户代码
	strcpy(dst->UserID, src->UserID);

	///报单价格条件
	dst->OrderPriceType = src->OrderPriceType;

	///买卖方向
	dst->Direction = src->Direction;

	///组合开平标志
	strcpy(dst->CombOffsetFlag, src->CombOffsetFlag);

	///组合投机套保标志
	strcpy(dst->CombHedgeFlag, src->CombHedgeFlag);

	///价格
	dst->LimitPrice = src->LimitPrice;

	///数量
	dst->VolumeTotalOriginal = src->VolumeTotalOriginal;

	///有效期类型
	dst->TimeCondition = src->TimeCondition;

	///GTD日期
	strcpy(dst->GTDDate, src->GTDDate);

	///成交量类型
	dst->VolumeCondition = src->VolumeCondition;

	///最小成交量
	dst->MinVolume = src->MinVolume;

	///触发条件
	dst->ContingentCondition = src->ContingentCondition;

	///止损价
	dst->StopPrice = src->StopPrice;

	///强平原因
	dst->ForceCloseReason = src->ForceCloseReason;

	///自动挂起标志
	dst->IsAutoSuspend = src->IsAutoSuspend;

	///业务单元
	strcpy(dst->BusinessUnit, src->BusinessUnit);

	///请求编号
	dst->RequestID = src->RequestID;

	///本地报单编号
	strcpy(dst->OrderLocalID, src->OrderLocalID);

	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);

	///会员代码
	strcpy(dst->ParticipantID, src->ParticipantID);

	///客户代码
	strcpy(dst->ClientID, src->ClientID);

	///合约在交易所的代码
	strcpy(dst->ExchangeInstID, src->ExchangeInstID);

	///交易所交易员代码
	strcpy(dst->TraderID, src->TraderID);

	///安装编号
	dst->InstallID = src->InstallID;

	///报单提交状态
	dst->OrderSubmitStatus = src->OrderSubmitStatus;

	///报单提示序号
	dst->NotifySequence = src->NotifySequence;

	///交易日
	strcpy(dst->TradingDay, src->TradingDay);

	///结算编号
	dst->SettlementID = src->SettlementID;

	///报单编号
	strcpy(dst->OrderSysID, src->OrderSysID);

	///报单来源
	dst->OrderSource = src->OrderSource;

	///报单状态
	dst->OrderStatus = src->OrderStatus;

	///报单类型
	dst->OrderType = src->OrderType;

	///今成交数量
	dst->VolumeTraded = src->VolumeTraded;

	///剩余数量
	dst->VolumeTotal = src->VolumeTotal;

	///报单日期
	strcpy(dst->InsertDate, src->InsertDate);

	///委托时间
	strcpy(dst->InsertTime, src->InsertTime);

	///激活时间
	strcpy(dst->ActiveTime, src->ActiveTime);

	///挂起时间
	strcpy(dst->SuspendTime, src->SuspendTime);

	///最后修改时间
	strcpy(dst->UpdateTime, src->UpdateTime);

	///撤销时间
	strcpy(dst->CancelTime, src->CancelTime);

	///最后修改交易所交易员代码
	strcpy(dst->ActiveTraderID, src->ActiveTraderID);

	///结算会员编号
	strcpy(dst->ClearingPartID, src->ClearingPartID);

	///序号
	dst->SequenceNo = src->SequenceNo;

	///前置编号
	dst->FrontID = src->FrontID;

	///会话编号
	dst->SessionID = src->SessionID;

	///用户端产品信息
	strcpy(dst->UserProductInfo, src->UserProductInfo);

	///状态信息
	strcpy(dst->StatusMsg, src->StatusMsg);

	///用户强评标志
	dst->UserForceClose = src->UserForceClose;

	///操作用户代码
	strcpy(dst->ActiveUserID, src->ActiveUserID);

	///经纪公司报单编号
	dst->BrokerOrderSeq = src->BrokerOrderSeq;

	///相关报单
	strcpy(dst->RelativeOrderSysID, src->RelativeOrderSysID);

	///郑商所成交数量
	dst->ZCETotalTradedVolume = src->ZCETotalTradedVolume;

	///互换单标志
	dst->IsSwapOrder = src->IsSwapOrder;

	///营业部编号
	strcpy(dst->BranchID, src->BranchID);

	///投资单元代码
	strcpy(dst->InvestUnitID, src->InvestUnitID);

	///资金账号
	strcpy(dst->AccountID, src->AccountID);

	///币种代码
	strcpy(dst->CurrencyID, src->CurrencyID);

	///IP地址
	strcpy(dst->IPAddress, src->IPAddress);

	///Mac地址
	strcpy(dst->MacAddress, src->MacAddress);
}

/// 拷贝结构体CThostFtdcTradeField
void Strategy::CopyTradeData(CThostFtdcTradeField *dst, CThostFtdcTradeField *src) {
	///经纪公司代码
	strcpy(dst->BrokerID, src->BrokerID);

	///投资者代码
	strcpy(dst->InvestorID, src->InvestorID);

	///合约代码
	strcpy(dst->InstrumentID, src->InstrumentID);

	///报单引用
	strcpy(dst->OrderRef, src->OrderRef);

	///用户代码
	strcpy(dst->UserID, src->UserID);

	///交易所代码
	strcpy(dst->ExchangeID, src->ExchangeID);

	///成交编号
	strcpy(dst->TradeID, src->TradeID);

	///买卖方向
	dst->Direction = src->Direction;

	///报单编号
	strcpy(dst->OrderSysID, src->OrderSysID);

	///会员代码
	strcpy(dst->ParticipantID, src->ParticipantID);

	///客户代码
	strcpy(dst->ClientID, src->ClientID);

	///交易角色
	dst->TradingRole = src->TradingRole;

	///合约在交易所的代码
	strcpy(dst->ExchangeInstID, src->ExchangeInstID);

	///开平标志
	dst->OffsetFlag = src->OffsetFlag;

	///投机套保标志
	dst->HedgeFlag = src->HedgeFlag;

	///价格
	dst->Price = src->Price;

	///数量
	dst->Volume = src->Volume;

	///成交时期
	strcpy(dst->TradeDate, src->TradeDate);

	///成交时间
	strcpy(dst->TradeTime, src->TradeTime);

	///成交类型
	dst->TradeType = src->TradeType;

	///成交价来源
	dst->PriceSource = src->PriceSource;

	///交易所交易员代码
	strcpy(dst->TraderID, src->TraderID);

	///本地报单编号
	strcpy(dst->OrderLocalID, src->OrderLocalID);

	///结算会员编号
	strcpy(dst->ClearingPartID, src->ClearingPartID);

	///业务单元
	strcpy(dst->BusinessUnit, src->BusinessUnit);

	///序号
	dst->SequenceNo = src->SequenceNo;

	///交易日
	strcpy(dst->TradingDay, src->TradingDay);

	///结算编号
	dst->SettlementID = src->SettlementID;

	///经纪公司报单编号
	dst->BrokerOrderSeq = src->BrokerOrderSeq;

	///成交来源
	dst->TradeSource = src->TradeSource;
}

/// 设置开关
int Strategy::getOn_Off() {
	return this->on_off;
}

void Strategy::setOn_Off(int on_off) {
	this->on_off = on_off;
}

/// 更新交易状态
void Strategy::update_task_status() {
	std::cout << "Before update this.trade_tasking = " << this->stg_trade_tasking << endl;
	if ((this->stg_position_a_buy_today == this->stg_position_b_sell_today)
		&& (this->stg_position_a_buy_yesterday == this->stg_position_b_sell_yesterday)
		&& (this->stg_position_a_sell_today == this->stg_position_b_buy_today)
		&& (this->stg_position_a_sell_yesterday == this->stg_position_b_buy_yesterday)) {
			this->stg_trade_tasking = false;
	}
	else
	{
		this->stg_trade_tasking = true;
	}
	std::cout << "After update this.trade_tasking = " << this->stg_trade_tasking << endl;
}

void Strategy::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
	USER_PRINT(this->getStgInstrumentIdA());
	USER_PRINT(this->getStgInstrumentIdB());
	// Get Own Instrument
	//USER_PRINT("Strategy::OnRtnDepthMarketData");
	if (!strcmp(pDepthMarketData->InstrumentID, this->getStgInstrumentIdA().c_str())) {
		//USER_PRINT("stg_instrument_A_tick ask_volume bid_volume");
		this->CopyTickData(stg_instrument_A_tick, pDepthMarketData);
		std::cout << "stg_instrument_A_tick = " << stg_instrument_A_tick->InstrumentID << endl;
		std::cout << "stg_instrument_A_tick->AskVolume1 = " << stg_instrument_A_tick->AskVolume1 << endl;
		std::cout << "stg_instrument_A_tick->BidVolume1 = " << stg_instrument_A_tick->BidVolume1 << endl;
		std::cout << "stg_instrument_A_tick->AskPrice1 = " << stg_instrument_A_tick->AskPrice1 << endl;
		std::cout << "stg_instrument_A_tick->BidPrice1 = " << stg_instrument_A_tick->BidPrice1 << endl;
	}
	else if (!strcmp(pDepthMarketData->InstrumentID, this->getStgInstrumentIdB().c_str()))
	{
		//USER_PRINT("stg_instrument_B_tick ask_volume bid_volume");
		this->CopyTickData(stg_instrument_B_tick, pDepthMarketData);
		std::cout << "stg_instrument_B_tick = " << stg_instrument_B_tick->InstrumentID << endl;
		std::cout << "stg_instrument_B_tick->AskVolume1 = " << stg_instrument_B_tick->AskVolume1 << endl;
		std::cout << "stg_instrument_B_tick->BidVolume1 = " << stg_instrument_B_tick->BidVolume1 << endl;
		std::cout << "stg_instrument_B_tick->AskPrice1 = " << stg_instrument_B_tick->AskPrice1 << endl;
		std::cout << "stg_instrument_B_tick->BidPrice1 = " << stg_instrument_B_tick->BidPrice1 << endl;
	}	
	/// 如果正在交易,继续更新tick进行交易
	if (this->stg_trade_tasking) {
		this->Exec_OnTickComing(pDepthMarketData);
	}
	else { /// 如果未在交易，那么选择下单算法
		this->Select_Order_Algorithm(this->getStgOrderAlgorithm());
	}
}

//选择下单算法
void Strategy::Select_Order_Algorithm(string stg_order_algorithm) {
	//USER_PRINT("Strategy::Select_Order_Algorithm");
	int select_num;
	//如果正在交易,直接返回0
	if (this->stg_trade_tasking) {
		return;
	}
	//如果有挂单,返回0
	if (this->stg_list_order_pending->size() > 0) {
		return;
	}

	if (!((this->stg_position_a_sell == this->stg_position_b_buy) && 
		(this->stg_position_a_buy == this->stg_position_b_sell))) {
		// 有撇腿
		return;
	}

	if (stg_order_algorithm == ALGORITHM_ONE) { //下单算法1
		this->Order_Algorithm_One();
	}
	else if (stg_order_algorithm == ALGORITHM_TWO) { // 下单算法2
		this->Order_Algorithm_Two();
	}
	else if (stg_order_algorithm == ALGORITHM_THREE) { // 下单算法3
		
		this->Order_Algorithm_Three();
	}
	else {
		std::cout << "Select_Order_Algorithm has no algorithm for you!" << endl;
	}
}

//下单算法1
void Strategy::Order_Algorithm_One() {
	USER_PRINT("Order_Algorithm_One");
	// 计算盘口价差，量
	if ((this->stg_instrument_A_tick->AskPrice1 != 0) &&
		(this->stg_instrument_A_tick->BidPrice1 != 0) &&
		(this->stg_instrument_A_tick->AskVolume1 != 0) &&
		(this->stg_instrument_A_tick->BidVolume1 != 0) &&
		(this->stg_instrument_B_tick->AskPrice1 != 0) &&
		(this->stg_instrument_B_tick->BidPrice1 != 0) &&
		(this->stg_instrument_B_tick->AskVolume1 != 0) &&
		(this->stg_instrument_B_tick->BidVolume1 != 0))
	{
		std::cout << "计算多头" << endl;
		std::cout << "A_tick->BidPrice1 = " << this->stg_instrument_A_tick->BidPrice1 << endl;
		std::cout << "B_tick->AskPrice1 = " << this->stg_instrument_B_tick->AskPrice1 << endl;
		//市场多头价差
		this->stg_spread_long = this->stg_instrument_A_tick->BidPrice1 - 
								this->stg_instrument_B_tick->AskPrice1;

		std::cout << "A_tick->BidVolume1 = " << this->stg_instrument_A_tick->BidVolume1 << endl;
		std::cout << "B_tick->AskVolume1 = " << this->stg_instrument_B_tick->AskVolume1 << endl;

		//市场多头价差挂单量
		this->stg_spread_long_volume = std::min(this->stg_instrument_A_tick->BidVolume1,
			this->stg_instrument_B_tick->AskVolume1);

		std::cout << "stg_spread_long_volume = " << this->stg_spread_long_volume << endl;

		std::cout << "计算空头" << endl;

		std::cout << "A_tick->AskPrice1 = " << this->stg_instrument_A_tick->AskPrice1 << endl;
		std::cout << "B_tick->BidPrice1 = " << this->stg_instrument_B_tick->BidPrice1 << endl;
		// 市场空头价差
		this->stg_spread_short = this->stg_instrument_A_tick->AskPrice1 -
			this->stg_instrument_B_tick->BidPrice1;


		std::cout << "A_tick->AskVolume1 = " << this->stg_instrument_A_tick->AskVolume1 << endl;
		std::cout << "B_tick->BidVolume1 = " << this->stg_instrument_B_tick->BidVolume1 << endl;

		// 市场空头价差挂单量
		this->stg_spread_short_volume = std::min(this->stg_instrument_A_tick->AskVolume1,
			this->stg_instrument_B_tick->BidVolume1);

		std::cout << "stg_spread_short_volume = " << this->stg_spread_short_volume << endl;
	} 
	else
	{
		return;
	}

	std::cout << "测试是否是stg_user问题" << std::endl;
	std::cout << "this->stg_user = " << this->stg_user << std::endl;
	std::cout << "this->stg_user->getCTP_Manager() = " << this->stg_user->getCTP_Manager() << std::endl;
	std::cout << "策略开关,期货账户开关,总开关" << std::endl;

	/// 策略开关，期货账户开关，总开关
	if (!((this->getOn_Off()) & (this->stg_user->getOn_Off()) & (this->stg_user->getCTP_Manager()->getOn_Off()))) {
		std::cout << "请检查开关状态!" << std::endl;
		std::cout << "总开关 = " << this->stg_user->getCTP_Manager()->getOn_Off() << std::endl;
		std::cout << "账户开关 = " << this->stg_user->getOn_Off() << std::endl;
		std::cout << "策略开关 = " << this->getOn_Off() << std::endl;
		return;
	}

	std::cout << "策略开关,期货账户开关,总开关22222" << std::endl;

	/// 价差多头卖平(b)
	if ((this->stg_spread_long >= this->stg_sell_close) 
		&& (this->stg_position_a_sell == this->stg_position_b_buy)
		&& (this->stg_position_a_sell > 0)) {
		/// 市场多头价差大于触发参数， AB持仓量相等且大于0
		std::cout << "策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差多头卖平" << endl;

		std::cout << "user_id = " << this->stg_user_id << ", "
			<< "strategy_id = " << this->stg_strategy_id << ", "
			<< "instrument_A = " << this->stg_instrument_id_A << ", "
			<< "instrument_B = " << this->stg_instrument_id_B << ", "
			<< "spread long = " << this->stg_spread_long << ", "
			<< "spread long volume = " << this->stg_spread_long_volume << ", "
			<< "spread short = " << this->stg_spread_short << ", "
			<< "spread short volume = " << this->stg_spread_short_volume << endl;

		/// 满足交易任务之前的一个tick
		this->CopyTickData(stg_instrument_A_tick_last, stg_instrument_A_tick);
		this->CopyTickData(stg_instrument_B_tick_last, stg_instrument_B_tick);

		int order_volume = 0;

		/// 优先平昨仓
		/// 报单手数：盘口挂单量、每份发单手数、持仓量
		if (this->stg_position_a_sell_yesterday > 0) {
			order_volume = this->getMinNum(this->stg_spread_short_volume, this->stg_lots_batch, this->stg_position_a_sell_yesterday);
			this->stg_a_order_insert_args->CombOffsetFlag[0] = '4'; /// 平昨
			this->stg_b_order_insert_args->CombOffsetFlag[0] = '4'; /// 平昨
		}
		else if ((this->stg_position_a_sell_yesterday == 0) && (this->stg_position_a_sell_today > 0)) {
			order_volume = this->getMinNum(this->stg_spread_short_volume, this->stg_lots_batch, this->stg_position_a_sell_today);
			this->stg_a_order_insert_args->CombOffsetFlag[0] = '3'; /// 平今
			this->stg_b_order_insert_args->CombOffsetFlag[0] = '3'; /// 平今
		}
		if ((order_volume <= 0)) {
			std::cout << "发单手数错误值 = " << order_volume << endl;
		} else {
			std::cout << "发单手数 = " << order_volume << endl;
		}

		this->stg_order_ref_a = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_a;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_a);

		/// A合约报单参数，全部确定
		// 报单引用
		strcpy(this->stg_a_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_a_order_insert_args->InstrumentID, this->stg_instrument_id_A.c_str());
		// 限价
		this->stg_a_order_insert_args->LimitPrice = this->stg_instrument_A_tick->AskPrice1;
		// 数量
		this->stg_a_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_a_order_insert_args->Direction = '0'; // 0买 1卖
		// 组合开平标志
		/// this->stg_a_order_insert_args->CombOffsetFlag[0] = '0';
		// 组合投机套保标志
		this->stg_a_order_insert_args->CombHedgeFlag[0] = '1'; // 1投机 2套利 3保值


		// B合约报单参数,部分确定
		// 报单引用
		//strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_b_order_insert_args->InstrumentID, this->stg_instrument_id_B.c_str());
		// 限价
		this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->BidPrice1;
		// 数量
		//this->stg_b_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_b_order_insert_args->Direction = '1'; // 0买 1卖
		// 组合开平标志
		/// this->stg_b_order_insert_args->CombOffsetFlag[0] = '0'; //组合开平标志0开仓 上期所3平今、4平昨，其他交易所1平仓
		// 组合投机套保标志
		this->stg_b_order_insert_args->CombHedgeFlag[0] = '1'; // 1投机 2套利 3保值

		/// 执行下单任务
		this->Exec_OrderInsert(this->stg_a_order_insert_args);
		this->stg_trade_tasking = true;

	}
	/// 价差空头买平(f)
	/*else if ((this->stg_spread_short <= this->stg_buy_close) && (this->stg_position_a_sell == this->stg_position_b_buy) &&
		(this->stg_position_a_sell > 0)) {*/
	else if ((this->stg_position_a_sell == this->stg_position_b_buy) &&
		(this->stg_position_a_sell > 0)) {
		/// 市场空头价差小于等于触发参数， AB持仓量相等且大于0
		std::cout << "策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差空头买平" << endl;

		std::cout << "user_id = " << this->stg_user_id << ", "
			<< "strategy_id = " << this->stg_strategy_id << ", "
			<< "instrument_A = " << this->stg_instrument_id_A << ", "
			<< "instrument_B = " << this->stg_instrument_id_B << ", "
			<< "spread long = " << this->stg_spread_long << ", "
			<< "spread long volume = " << this->stg_spread_long_volume << ", "
			<< "spread short = " << this->stg_spread_short << ", "
			<< "spread short volume = " << this->stg_spread_short_volume << ", "
			<< "stg_lots_batch = " << this->stg_lots_batch << ", "
			<< "stg_position_a_sell_yesterday = " << this->stg_position_a_sell_yesterday << endl;

		/// 满足交易任务之前的一个tick
		this->CopyTickData(stg_instrument_A_tick_last, stg_instrument_A_tick);
		this->CopyTickData(stg_instrument_B_tick_last, stg_instrument_B_tick);

		//USER_PRINT("stg_instrument_A_tick_last");
		//USER_PRINT(stg_instrument_A_tick_last);
		//USER_PRINT("stg_instrument_B_tick_last");
		//USER_PRINT(stg_instrument_B_tick_last);

		int order_volume = 0;

		/// 优先平昨仓
		/// 报单手数：盘口挂单量、每份发单手数、持仓量
		if (this->stg_position_a_sell_yesterday > 0) {
			
			order_volume = this->getMinNum(this->stg_spread_short_volume, this->stg_lots_batch, this->stg_position_a_sell_yesterday);
			this->stg_a_order_insert_args->CombOffsetFlag[0] = '4'; /// 平昨
			this->stg_b_order_insert_args->CombOffsetFlag[0] = '4'; /// 平昨
		}
		else if ((this->stg_position_a_sell_yesterday == 0) && (this->stg_position_a_sell_today > 0)) {
			
			order_volume = this->getMinNum(this->stg_spread_short_volume, this->stg_lots_batch, this->stg_position_a_sell_today);
			this->stg_a_order_insert_args->CombOffsetFlag[0] = '3'; /// 平今
			this->stg_b_order_insert_args->CombOffsetFlag[0] = '3'; /// 平今
		}
		if ((order_volume <= 0)) {
			std::cout << "发单手数错误值 = " << order_volume << endl;
			return;
		} else {
			std::cout << "发单手数 = " << order_volume << endl;
		}

		this->stg_order_ref_a = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_a;

		//USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_a);

		/// A合约报单参数，全部确定
		// 报单引用
		strcpy(this->stg_a_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_a_order_insert_args->InstrumentID, this->stg_instrument_id_A.c_str());
		// 限价
		this->stg_a_order_insert_args->LimitPrice = this->stg_instrument_A_tick->AskPrice1;
		// 数量
		this->stg_a_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_a_order_insert_args->Direction = '0';
		// 组合开平标志
		/// this->stg_a_order_insert_args->CombOffsetFlag[0] = '0';
		// 组合投机套保标志
		this->stg_a_order_insert_args->CombHedgeFlag[0] = '1'; /// 1投机 2套利 3保值


		// B合约报单参数,部分确定
		// 报单引用
		//strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_b_order_insert_args->InstrumentID, this->stg_instrument_id_B.c_str());
		// 限价
		this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->BidPrice1;
		// 数量
		//this->stg_b_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_b_order_insert_args->Direction = '1';
		// 组合开平标志
		/// this->stg_b_order_insert_args->CombOffsetFlag[0] = '0'; //组合开平标志0开仓 上期所3平今、4平昨，其他交易所1平仓
		// 组合投机套保标志
		this->stg_b_order_insert_args->CombHedgeFlag[0] = '1';

		/// 执行下单任务
		this->Exec_OrderInsert(this->stg_a_order_insert_args);
		this->stg_trade_tasking = true;
	}

	/// 价差多头卖开(f)
	//else if ((this->stg_spread_long >= this->stg_sell_open) && (this->stg_position_a_buy + this->stg_position_a_sell < this->stg_lots)) {
	else if ((this->stg_position_a_buy + this->stg_position_a_sell < this->stg_lots)) {
		/** 市场多头价差大于触发参数
		A合约买持仓加B合约买小于总仓位**/

		std::cout << "策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差多头卖开" << endl;
		
		std::cout << "user_id = " << this->stg_user_id << ", "
			<< "strategy_id = " << this->stg_strategy_id << ", "
			<< "instrument_A = " << this->stg_instrument_id_A << ", "
			<< "instrument_B = " << this->stg_instrument_id_B << ", "
			<< "spread long = " << this->stg_spread_long << ", "
			<< "spread long volume = " << this->stg_spread_long_volume << ", "
			<< "spread short = " << this->stg_spread_short << ", "
			<< "spread short volume = " << this->stg_spread_short_volume << ", "
			<< "stg_lots_batch = " << this->stg_lots_batch << ", "
			<< "stg_lots = " << this->stg_lots << endl;

		/// 满足交易任务之前的tick
		this->CopyTickData(stg_instrument_A_tick_last, stg_instrument_A_tick);
		this->CopyTickData(stg_instrument_B_tick_last, stg_instrument_B_tick);

		//USER_PRINT("stg_instrument_A_tick_last");
		//USER_PRINT(stg_instrument_A_tick_last);
		//USER_PRINT("stg_instrument_B_tick_last");
		//USER_PRINT(stg_instrument_B_tick_last);

		/// 报单手数：盘口挂单量,每份发单手数,剩余可开仓手数中取最小值
		int order_volume = this->getMinNum(this->stg_spread_long_volume, this->stg_lots_batch, this->stg_lots - (this->stg_position_a_buy + this->stg_position_b_buy));


		if (order_volume <= 0) {
			std::cout << "发单手数错误值 = " << order_volume << endl;
			return;
		} else {
			std::cout << "发单手数 = " << order_volume << endl;
		}

		this->stg_order_ref_a = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_a;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_a);

		// A合约报单参数,全部确定
		// 报单引用
		std::strcpy(this->stg_a_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_a_order_insert_args->InstrumentID, this->stg_instrument_id_A.c_str());
		// 限价
		this->stg_a_order_insert_args->LimitPrice = this->stg_instrument_A_tick->BidPrice1;
		// 数量
		this->stg_a_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_a_order_insert_args->Direction = '1'; // 0买 1卖
		// 组合开平标志
		this->stg_a_order_insert_args->CombOffsetFlag[0] = '0';  /// 0开仓 1平仓 3平今 4平昨
		// 组合投机套保标志
		this->stg_a_order_insert_args->CombHedgeFlag[0] = '1'; /// 1投机 2套利 3保值


		// B合约报单参数,部分确定
		// 报单引用
		//strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_b_order_insert_args->InstrumentID, this->stg_instrument_id_B.c_str());
		// 限价
		this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->AskPrice1;
		// 数量
		//this->stg_b_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_b_order_insert_args->Direction = '0';
		// 组合开平标志
		this->stg_b_order_insert_args->CombOffsetFlag[0] = '0'; //组合开平标志0开仓 上期所3平今、4平昨，其他交易所1平仓
		// 组合投机套保标志
		this->stg_b_order_insert_args->CombHedgeFlag[0] = '1';

		/// 执行下单任务
		this->Exec_OrderInsert(this->stg_a_order_insert_args);
		this->stg_trade_tasking = true;
	}

	/// 价差空头买开(b)
	/*else if ((this->stg_spread_short <= this->stg_buy_open) && (this->stg_position_a_buy + this->stg_position_a_sell < this->stg_lots)) {*/
	else if ((false) && (this->stg_position_a_buy + this->stg_position_a_sell < this->stg_lots)) {
		std::cout << "策略编号：" << this->stg_strategy_id << ", 交易信号触发，价差空头买开" << endl;


		std::cout << "user_id = " << this->stg_user_id << ", "
			<< "strategy_id = " << this->stg_strategy_id << ", "
			<< "instrument_A = " << this->stg_instrument_id_A << ", "
			<< "instrument_B = " << this->stg_instrument_id_B << ", "
			<< "spread long = " << this->stg_spread_long << ", "
			<< "spread long volume = " << this->stg_spread_long_volume << ", "
			<< "spread short = " << this->stg_spread_short << ", "
			<< "spread short volume = " << this->stg_spread_short_volume << endl;

		/// 满足交易任务之前的tick
		this->CopyTickData(stg_instrument_A_tick_last, stg_instrument_A_tick);
		this->CopyTickData(stg_instrument_B_tick_last, stg_instrument_B_tick);

		/// 报单手数：盘口挂单量,每份发单手数,剩余可开仓手数中取最小值
		int order_volume = this->getMinNum(this->stg_spread_long_volume, this->stg_lots_batch, this->stg_lots - (this->stg_position_a_buy + this->stg_position_b_buy));

		if (order_volume <= 0) {
			std::cout << "发单手数错误值 = " << order_volume << endl;
			return;
		} else {
			std::cout << "发单手数 = " << order_volume << endl;
		}

		this->stg_order_ref_a = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_a;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_a);

		// A合约报单参数,全部确定
		// 报单引用
		std::strcpy(this->stg_a_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_a_order_insert_args->InstrumentID, this->stg_instrument_id_A.c_str());
		// 限价
		this->stg_a_order_insert_args->LimitPrice = this->stg_instrument_A_tick->BidPrice1;
		// 数量
		this->stg_a_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_a_order_insert_args->Direction = '0'; // 0买 1卖
		// 组合开平标志
		this->stg_a_order_insert_args->CombOffsetFlag[0] = '0';  /// 0开仓 1平仓 3平今 4平昨
		// 组合投机套保标志
		this->stg_a_order_insert_args->CombHedgeFlag[0] = '1'; /// 1投机 2套利 3保值


		// B合约报单参数,部分确定
		// 报单引用
		//strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_a.c_str());
		// 合约代码
		std::strcpy(this->stg_b_order_insert_args->InstrumentID, this->stg_instrument_id_B.c_str());
		// 限价
		this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->AskPrice1;
		// 数量
		//this->stg_b_order_insert_args->VolumeTotalOriginal = order_volume;
		// 买卖方向
		this->stg_b_order_insert_args->Direction = '1'; // 0买 1卖
		// 组合开平标志
		this->stg_b_order_insert_args->CombOffsetFlag[0] = '0'; //组合开平标志0开仓 上期所3平今、4平昨，其他交易所1平仓
		// 组合投机套保标志
		this->stg_b_order_insert_args->CombHedgeFlag[0] = '1'; /// 1投机 2套利 3保值

		/// 执行下单任务
		this->Exec_OrderInsert(this->stg_a_order_insert_args);
		this->stg_trade_tasking = true;
	}



}
//下单算法2
void Strategy::Order_Algorithm_Two() {

}
//下单算法3
void Strategy::Order_Algorithm_Three() {

}

/// 生成报单引用
string Strategy::Generate_Order_Ref() {

	/*std::stringstream strstream;
	std::string number;
	strstream << (this->stg_order_ref_base + 1L);
	strstream >> number;
	string order_ref_base = number + this->stg_strategy_id;*/
	
	//USER_PRINT("Strategy::Generate_Order_Ref()");
	//USER_PRINT("this->stg_user");
	//USER_PRINT(this->stg_user);
	this->stg_user->setStgOrderRefBase(this->stg_user->getStgOrderRefBase() + 1);
	this->stg_order_ref_base = this->stg_user->getStgOrderRefBase(); // 更新基准数
	string order_ref_base = std::to_string(this->stg_order_ref_base) + this->stg_strategy_id;

	//USER_PRINT("Generate_Order_Ref");
	USER_PRINT(order_ref_base);

	//stringstream strValue;
	//strValue << order_ref_base;
	//strValue << this->stg_order_ref_base; // 更新
	return order_ref_base;
}


// 报单
void Strategy::Exec_OrderInsert(CThostFtdcInputOrderField *insert_order) {
	std::cout << "====Strategy::Exec_OrderInsert()====" << std::endl;
	std::cout << "OrderRef = " << insert_order->OrderRef << std::endl;
	std::cout << "InstrumentID = " << insert_order->InstrumentID << std::endl;
	std::cout << "LimitPrice = " << insert_order->LimitPrice << std::endl;
	std::cout << "VolumeTotalOriginal = " << insert_order->VolumeTotalOriginal << std::endl;
	std::cout << "Direction = " << insert_order->Direction << std::endl;
	std::cout << "CombOffsetFlag = " << insert_order->CombOffsetFlag[0] << std::endl;
	std::cout << "CombHedgeFlag = " << insert_order->CombHedgeFlag[0] << std::endl;

	//下单操作
	this->stg_user->getUserTradeSPI()->OrderInsert(this->stg_user, insert_order);
}

// 报单录入请求
void Strategy::Exec_OnRspOrderInsert() {
	USER_PRINT("Exec_OnRspOrderInsert()");
}

// 报单操作请求响应
void Strategy::Exec_OnRspOrderAction() {
	USER_PRINT("Exec_OnRspOrderAction()");
}

// 报单回报
void Strategy::Exec_OnRtnOrder(CThostFtdcOrderField *pOrder) {
	USER_PRINT("Exec_OnRtnOrder");
	std::cout << "pOrder->InstrumentID = " << pOrder->InstrumentID << std::endl;
	std::cout << "pOrder->VolumeTraded = " << pOrder->VolumeTraded << std::endl;
	//更新挂单列表，持仓信息
	this->update_pending_order_list(pOrder);
	this->update_task_status();

	/// A成交回报,B发送等量的报单
	if ((!strcmp(pOrder->InstrumentID, this->stg_instrument_id_A.c_str())) && ((pOrder->OrderStatus == '0') || (pOrder->OrderStatus == '1')) && (strlen(pOrder->OrderSysID) != 0)) { //只有全部成交或者部分成交还在队列中

		if (this->stg_list_order_pending->size() == 0) { // 无挂单
			std::cout << "A无挂单" << std::endl;
			this->stg_b_order_insert_args->VolumeTotalOriginal = pOrder->VolumeTraded;
		}
		else { // 有挂单
			std::cout << "A有挂单" << std::endl;
			bool b_fined = false;
			list<CThostFtdcOrderField *>::iterator Itor;
			for (Itor = this->stg_list_order_pending->begin(); Itor != this->stg_list_order_pending->end(); Itor++) {
				if (!strcmp((*Itor)->OrderRef, pOrder->OrderRef)) { //报单引用相等
					std::cout << "挂单列表找到A合约" << std::endl;
					std::cout << "pOrder->VolumeTraded = " << pOrder->VolumeTraded << endl;
					std::cout << "(*Itor)->VolumeTraded = " << (*Itor)->VolumeTraded << endl;

					this->stg_b_order_insert_args->VolumeTotalOriginal = pOrder->VolumeTraded - (*Itor)->VolumeTraded; // B发单量等于本次回报A的成交量
					b_fined = true;
					break;
				}
			}
			if (!b_fined) { // 无挂单，但是属于分批成交, 第一批
				std::cout << "无挂单，但是属于分批成交, 第一批" << std::endl;
				std::cout << "pOrder->VolumeTraded = " << pOrder->VolumeTraded << endl;
				this->stg_b_order_insert_args->VolumeTotalOriginal = pOrder->VolumeTraded;
			}
		}

		this->stg_order_ref_b = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_b;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_b);
		
		strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_b.c_str());

		//this->stg_user->getUserTradeSPI()->OrderInsert(this->stg_user, this->stg_b_order_insert_args);
		this->Exec_OrderInsert(this->stg_b_order_insert_args);
	}
	/// B成交回报
	else if ((!strcmp(pOrder->InstrumentID, this->stg_instrument_id_B.c_str())) && (pOrder->OrderStatus == '0' || pOrder->OrderStatus == '1') && (strlen(pOrder->OrderSysID) != 0)) { // 全部成交或者部分成交

	}
	/// B撤单回报，启动B重新发单一定成交策略
	else if ((!strcmp(pOrder->InstrumentID, this->stg_instrument_id_B.c_str())) && (pOrder->OrderStatus == '5') && (strlen(pOrder->OrderSysID) != 0)) {
		this->stg_order_ref_b = this->Generate_Order_Ref();
		this->stg_order_ref_last = this->stg_order_ref_b;

		USER_PRINT("OrderRef");
		USER_PRINT(this->stg_order_ref_b);

		if (pOrder->Direction == '0') {
			this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->AskPrice1;
		}
		else if (pOrder->Direction == '1') {
			this->stg_b_order_insert_args->LimitPrice = this->stg_instrument_B_tick->BidPrice1;
		}

		strcpy(this->stg_b_order_insert_args->OrderRef, this->stg_order_ref_b.c_str());
		this->stg_b_order_insert_args->VolumeTotalOriginal = pOrder->VolumeTotal;
		this->stg_b_order_insert_args->Direction = pOrder->Direction;
		this->stg_b_order_insert_args->CombOffsetFlag[0] = pOrder->CombOffsetFlag[0];
		this->stg_b_order_insert_args->CombHedgeFlag[0] = pOrder->CombHedgeFlag[0];

		//this->stg_user->getUserTradeSPI()->OrderInsert(this->stg_user, this->stg_b_order_insert_args);
		this->Exec_OrderInsert(this->stg_b_order_insert_args);
	}

}

// 成交回报
void Strategy::ExEc_OnRtnTrade(CThostFtdcTradeField *pTrade) {
	/*1:更新持仓明细*/
	this->update_position_detail(pTrade); 
	this->update_position(pTrade);
	this->update_task_status();
}

// 报单录入错误回报
void Strategy::Exec_OnErrRtnOrderInsert() {
	USER_PRINT("Exec_OnErrRtnOrderInsert()");
}

// 报单操作错误回报
void Strategy::Exec_OnErrRtnOrderAction() {
	USER_PRINT("Exec_OnErrRtnOrderAction()");
}

// 行情回调,执行交易任务
void Strategy::Exec_OnTickComing(CThostFtdcDepthMarketDataField *pDepthMarketData) {
	//USER_PRINT("Exec_OnTickComing()");
	list<CThostFtdcOrderField *>::iterator Itor;
	for (Itor = this->stg_list_order_pending->begin(); Itor != this->stg_list_order_pending->end(); Itor++) {
		/// A有挂单，判断是否需要撤单
		if (!strcmp((*Itor)->InstrumentID, this->stg_instrument_id_A.c_str())) {
			/// 通过A最新tick判断A合约是否需要撤单
			if (!strcmp(pDepthMarketData->InstrumentID, this->stg_instrument_id_A.c_str())) {
				/// A挂单方向为买
				if ((*Itor)->Direction == '0') {
					/// 挂单价格与盘口买一价比较，如果与盘口价格差距n个最小跳以上，撤单
					if (pDepthMarketData->BidPrice1 > (*Itor)->LimitPrice + (this->stg_a_wait_price_tick * this->stg_a_price_tick)) {
						std::cout << "A合约通过最新tick判断A合约买挂单符合撤单条件" << endl;
						/// A合约撤单
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
				else if ((*Itor)->Direction == '1') {
					if (pDepthMarketData->AskPrice1 < ((*Itor)->LimitPrice - (this->stg_a_wait_price_tick * this->stg_a_price_tick))) {
						/// A合约撤单
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
			}

			//根据B的行情判断A是否需要撤单
			else if (!strcmp(pDepthMarketData->InstrumentID, this->stg_instrument_id_B.c_str())) {
				/// A挂单的买卖方向为买
				if ((*Itor)->Direction == '0') {
					/// B最新tick的对手价如果与开仓信号触发时B的tick对手价发生不利变化则A撤单
					if (pDepthMarketData->BidPrice1 < this->stg_instrument_B_tick_last->BidPrice1) {
						USER_PRINT("Strategy.trade_task() 通过B最新tick判断A合约买挂单符合撤单条件");
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
				/// A挂单的买卖方向为卖
				else if ((*Itor)->Direction == '1') {
					/// B最新tick的对手价如果与开仓信号触发时B的tick对手价发生不利变化则A撤单
					if (pDepthMarketData->AskPrice1 > this->stg_instrument_B_tick_last->AskPrice1) {
						USER_PRINT("Strategy.trade_task()通过B最新tick判断A合约卖挂单符合撤单条件");
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
			}
		}

		/// B有挂单，判断是否需要撤单，并启动B合约一定成交策略
		if (!strcmp((*Itor)->InstrumentID, this->stg_instrument_id_B.c_str())) {
			/// 通过B最新tick判断B合约是否需要撤单
			if (!strcmp(pDepthMarketData->InstrumentID, this->stg_instrument_id_B.c_str())) {
				/// B挂单的买卖方向为买
				if ((*Itor)->Direction == '0') {
					/// 挂单价格与盘口买一价比较，如果与盘口价格差距n个最小跳以上，撤单
					if (pDepthMarketData->BidPrice1 >= (*Itor)->LimitPrice + this->stg_b_wait_price_tick * this->stg_b_price_tick) {
						USER_PRINT("通过B最新tick判断B合约买挂单符合撤单条件");
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
				/// B挂单的买卖方向为卖
				else if ((*Itor)->Direction == '1') {
					/// 挂单价格与盘口卖一价比较，如果与盘口价格差距n个最小跳以上，撤单
					if (pDepthMarketData->AskPrice1 <= (*Itor)->LimitPrice - this->stg_b_wait_price_tick * this->stg_b_price_tick) {
						USER_PRINT("通过B最新tick判断B合约卖挂单符合撤单条件");
						this->stg_user->getUserTradeSPI()->OrderAction((*Itor)->ExchangeID, (*Itor)->OrderRef, (*Itor)->OrderSysID);
					}
				}
			}
		}
	}
}

/// 更新挂单list
void Strategy::update_pending_order_list(CThostFtdcOrderField *pOrder) {
	/************************************************************************/
	/* 此处pOrder应该谨慎操作                                                  */
	/************************************************************************/
	std::cout << "pending_order_list size = "<< this->stg_list_order_pending->size() << std::endl;

	if (strlen(pOrder->OrderSysID) != 0) {
		//如果list为空,直接添加到挂单列表里
		if (this->stg_list_order_pending->size() == 0) {
			// 深复制对象
			CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
			memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
			this->CopyOrderData(pOrder_tmp, pOrder);

			this->stg_list_order_pending->push_back(pOrder_tmp);
			return;
		} else {
			/// 挂单列表不为空时
			list<CThostFtdcOrderField *>::iterator Itor;
			for (Itor = this->stg_list_order_pending->begin(); Itor != this->stg_list_order_pending->end();) {
				if (!strcmp((*Itor)->OrderRef, pOrder->OrderRef)) { // 先前已经有orderref存在挂单编号里
					if (pOrder->OrderStatus == '0') { // 全部成交

						USER_PRINT("全部成交");
						delete (*Itor);
						Itor = this->stg_list_order_pending->erase(Itor); //移除

					} else if (pOrder->OrderStatus == '1') { // 部分成交还在队列中

						USER_PRINT("部分成交还在队列中");

						// 深复制对象
						CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
						memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
						this->CopyOrderData(pOrder_tmp, pOrder);

						*Itor = pOrder_tmp;

					} else if (pOrder->OrderStatus == '2') { // 部分成交不在队列中

						USER_PRINT("部分成交不在队列中");

					} else if (pOrder->OrderStatus == '3') { // 未成交还在队列中

						USER_PRINT("未成交还在队列中");
						// 深复制对象
						CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
						memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
						this->CopyOrderData(pOrder_tmp, pOrder);

						*Itor = pOrder_tmp;

					} else if (pOrder->OrderStatus == '4') { // 未成交不在队列中

						USER_PRINT("未成交不在队列中");


					} else if (pOrder->OrderStatus == '5') { // 撤单

						USER_PRINT("撤单");
						delete (*Itor);
						Itor = this->stg_list_order_pending->erase(Itor); //移除

					} else if (pOrder->OrderStatus == 'a') { // 未知

						USER_PRINT("未知");

					} else if (pOrder->OrderStatus == 'b') { // 尚未触发

						USER_PRINT("尚未触发");

					} else if (pOrder->OrderStatus == 'c') { // 已触发

						USER_PRINT("已触发");

					}

				} else {	// 没有orderref存在挂单编号里,第一次
					if (pOrder->OrderStatus == '0') { // 全部成交

					}
					else if (pOrder->OrderStatus == '1') { // 部分成交还在队列中
						USER_PRINT("没有orderref存在挂单列表,部分成交还在队列中");
						// 深复制对象
						CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
						memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
						this->CopyOrderData(pOrder_tmp, pOrder);
						this->stg_list_order_pending->push_back(pOrder_tmp);
					}
					else if (pOrder->OrderStatus == '2') { // 部分成交不在队列中

					}
					else if (pOrder->OrderStatus == '3') { // 未成交还在队列中
						USER_PRINT("没有orderref存在挂单列表,未成交还在队列中");
						// 深复制对象
						CThostFtdcOrderField *pOrder_tmp = new CThostFtdcOrderField();
						memset(pOrder_tmp, 0x00, sizeof(CThostFtdcOrderField));
						this->CopyOrderData(pOrder_tmp, pOrder);
						this->stg_list_order_pending->push_back(pOrder_tmp);
					}
					else if (pOrder->OrderStatus == '4') { // 未成交不在队列中

					}
					else if (pOrder->OrderStatus == '5') { // 撤单

					}
					else if (pOrder->OrderStatus == 'a') { // 未知

					}
					else if (pOrder->OrderStatus == 'b') { // 尚未触发

					}
					else if (pOrder->OrderStatus == 'c') { // 已触发

					}
				}
				Itor++;
			}
		}
	}
}

/// 更新持仓量
void Strategy::update_position(CThostFtdcOrderField *pOrder) {
	USER_PRINT("update_position");

	if (!strcmp(pOrder->InstrumentID, this->stg_instrument_id_A.c_str())) { // A合约
		if (pOrder->CombOffsetFlag[0] == '0') { // A开仓成交回报
			if (pOrder->Direction == '0') { // A买开仓成交回报
				this->stg_position_a_buy_today += pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // A卖开仓成交回报
				this->stg_position_a_sell_today += pOrder->VolumeTraded;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '3') { // A平今成交回报
			if (pOrder->Direction == '0') { // A买平今成交回报
				this->stg_position_a_sell_today -= pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // A卖平今成交回报
				this->stg_position_a_buy_today -= pOrder->VolumeTraded;
			}
		} 
		else if (pOrder->CombOffsetFlag[0] == '4') { // A平昨成交回报
			if (pOrder->Direction == '0') { // A买平昨成交回报
				this->stg_position_a_sell_yesterday -= pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // A卖平昨成交回报
				this->stg_position_a_buy_yesterday -= pOrder->VolumeTraded;
			}
		}

		this->stg_position_a_buy = this->stg_position_a_buy_today + this->stg_position_a_buy_yesterday;
		this->stg_position_a_sell = this->stg_position_a_sell_today + this->stg_position_a_sell_yesterday;
	}
	else if (!strcmp(pOrder->InstrumentID, this->stg_instrument_id_B.c_str())) { // B合约
		if (pOrder->CombOffsetFlag[0] == '0') { // B开仓成交回报
			if (pOrder->Direction == '0') { // B买开仓成交回报
				this->stg_position_b_buy_today += pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // B卖开仓成交回报
				this->stg_position_b_sell_today += pOrder->VolumeTraded;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '3') { // B平今成交回报
			if (pOrder->Direction == '0') { // B买平今成交回报
				this->stg_position_b_sell_today -= pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // B卖平今成交回报
				this->stg_position_b_buy_today -= pOrder->VolumeTraded;
			}
		}
		else if (pOrder->CombOffsetFlag[0] == '4') { // B平昨成交回报
			if (pOrder->Direction == '0') { // B买平昨成交回报
				this->stg_position_b_sell_yesterday -= pOrder->VolumeTraded;
			}
			else if (pOrder->Direction == '1') { // B卖平昨成交回报
				this->stg_position_b_buy_yesterday -= pOrder->VolumeTraded;
			}
		}

		this->stg_position_b_buy = this->stg_position_b_buy_today + this->stg_position_b_buy_yesterday;
		this->stg_position_b_sell = this->stg_position_b_sell_today + this->stg_position_b_sell_yesterday;
	}
}

/// 更新持仓量
void Strategy::update_position(CThostFtdcTradeField *pTrade) {
	std::cout << "update_position(CThostFtdcTradeField *pTrade)" << std::endl;
	/// A成交
	if (!strcmp(pTrade->InstrumentID, this->stg_instrument_id_A.c_str())) {
		if (pTrade->OffsetFlag == '0') { // A开仓成交回报
			if (pTrade->Direction == '0') { // A买开仓
				this->stg_position_a_buy_today += pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖开仓
				this->stg_position_a_sell_today += pTrade->Volume;
			}
		}
		else if (pTrade->OffsetFlag == '3') { // A平今
			if (pTrade->Direction == '0') { // A买平今
				this->stg_position_a_sell_today -= pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖平今
				this->stg_position_a_buy_today -= pTrade->Volume;
			}
		}
		else if (pTrade->OffsetFlag == '4') { // A平昨
			if (pTrade->Direction == '0') { // A买平昨
				this->stg_position_a_sell_yesterday -= pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖平昨
				this->stg_position_a_buy_yesterday -= pTrade->Volume;
			}
		}
		this->stg_position_a_buy = this->stg_position_a_buy_today + this->stg_position_a_buy_yesterday;
		this->stg_position_a_sell = this->stg_position_a_sell_today + this->stg_position_a_sell_yesterday;
	}
	/// B成交
	else if (!strcmp(pTrade->InstrumentID, this->stg_instrument_id_B.c_str())) {
		if (pTrade->OffsetFlag == '0') { // A开仓成交回报
			if (pTrade->Direction == '0') { // A买开仓
				this->stg_position_b_buy_today += pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖开仓
				this->stg_position_b_sell_today += pTrade->Volume;
			}
		}
		else if (pTrade->OffsetFlag == '3') { // A平今
			if (pTrade->Direction == '0') { // A买平今
				this->stg_position_b_sell_today -= pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖平今
				this->stg_position_b_buy_today -= pTrade->Volume;
			}
		}
		else if (pTrade->OffsetFlag == '4') { // A平昨
			if (pTrade->Direction == '0') { // A买平昨
				this->stg_position_b_sell_yesterday -= pTrade->Volume;
			}
			else if (pTrade->Direction == '1') { // A卖平昨
				this->stg_position_b_buy_yesterday -= pTrade->Volume;
			}
		}
		this->stg_position_b_buy = this->stg_position_b_buy_today + this->stg_position_b_buy_yesterday;
		this->stg_position_b_sell = this->stg_position_b_sell_today + this->stg_position_b_sell_yesterday;
	}

	std::cout << "A合约今买 = " << this->stg_position_a_buy_today << std::endl;
	std::cout << "A合约昨买 = " << this->stg_position_a_buy_yesterday << std::endl;
	std::cout << "A合约总买 = " << this->stg_position_a_buy << std::endl;
	std::cout << "A合约今卖 = " << this->stg_position_a_sell_today << std::endl;
	std::cout << "A合约昨卖 = " << this->stg_position_a_buy_yesterday << std::endl;
	std::cout << "A合约总卖 = " << this->stg_position_a_sell << std::endl;
	
	std::cout << "B合约今买 = " << this->stg_position_b_buy_today << std::endl;
	std::cout << "B合约昨买 = " << this->stg_position_b_buy_yesterday << std::endl;
	std::cout << "B合约总买 = " << this->stg_position_b_buy << std::endl;
	std::cout << "B合约今卖 = " << this->stg_position_b_sell_today << std::endl;
	std::cout << "B合约昨卖 = " << this->stg_position_b_buy_yesterday << std::endl;
	std::cout << "B合约总卖 = " << this->stg_position_b_sell << std::endl;

}

/// 更新持仓明细
void Strategy::update_position_detail(CThostFtdcTradeField *pTrade_cal) {
	CThostFtdcTradeField *pTrade = new CThostFtdcTradeField();
	memset(pTrade, 0x00, sizeof(CThostFtdcTradeField));
	this->CopyTradeData(pTrade, pTrade_cal);


	// 开仓单,添加到list，添加到list尾部
	if (pTrade->OffsetFlag == '0') {
		CThostFtdcTradeField *pTrade_tmp = new CThostFtdcTradeField();
		memset(pTrade_tmp, 0x00, sizeof(CThostFtdcTradeField));
		this->CopyTradeData(pTrade_tmp, pTrade_cal);

		this->stg_list_position_detail->push_back(pTrade_tmp);
	}
	else if (pTrade->OffsetFlag == '1') { // 平仓单,先开先平的原则从list里删除
		list<CThostFtdcTradeField *>::iterator Itor;
		for (Itor = this->stg_list_position_detail->begin(); Itor != this->stg_list_position_detail->end();) {
			if (pTrade->OffsetFlag == '3') { // 平今
				if ((!strcmp((*Itor)->TradingDay, pTrade->TradingDay)) 
					&& (!strcmp((*Itor)->InstrumentID, pTrade->InstrumentID))
					&& ((*Itor)->HedgeFlag == pTrade->HedgeFlag) 
					&& ((*Itor)->Direction == pTrade->Direction)) {
						// pTrade的volume等于持仓明细列表里的volume
						if (((*Itor)->Volume) == (pTrade->Volume)) {
							delete (*Itor);
							Itor = this->stg_list_position_detail->erase(Itor);
						}
						else if (((*Itor)->Volume) > (pTrade->Volume)) {
							(*Itor)->Volume -= pTrade->Volume;
						}
						else if (((*Itor)->Volume) < (pTrade->Volume)) {
							pTrade->Volume -= (*Itor)->Volume;
							Itor = this->stg_list_position_detail->erase(Itor);
						}
				}
			}
			else if (pTrade->OffsetFlag == '4') {
				if ((!strcmp((*Itor)->TradingDay, pTrade->TradingDay))
					&& (!strcmp((*Itor)->InstrumentID, pTrade->InstrumentID))
					&& ((*Itor)->HedgeFlag == pTrade->HedgeFlag)
					&& ((*Itor)->Direction == pTrade->Direction)) {
						// pTrade的volume等于持仓明细列表里的volume
						if (((*Itor)->Volume) == (pTrade->Volume)) {
							delete (*Itor);
							Itor = this->stg_list_position_detail->erase(Itor);
						}
						else if (((*Itor)->Volume) > (pTrade->Volume)) {
							(*Itor)->Volume -= pTrade->Volume;
						}
						else if (((*Itor)->Volume) < (pTrade->Volume)) {
							pTrade->Volume -= (*Itor)->Volume;
							Itor = this->stg_list_position_detail->erase(Itor);
						}
				}
			}
		}
	}
}

/// 得到三个数最小值
int Strategy::getMinNum(int num1, int num2, int num3) {
	int minNum = 0;
	if (num1 < num2) {
		minNum = num1;
	}
	else {
		minNum = num2;
	}
	if (num3 < minNum) {
		minNum = num3;
	}
	return minNum;
}

void Strategy::setTrader(Trader *trader) {
	this->trader = trader;
}

Trader * Strategy::getTrader() {
	return this->trader;
}

void Strategy::setUser(User *user) {
	this->user = user;
}

User * Strategy::getUser() {
	return this->user;
}

void Strategy::setStrategyId(string strategyid) {
	this->strategyid = strategyid;
}

string Strategy::getStrategyId() {
	return this->strategyid;
}

void Strategy::setAlgorithm(Algorithm *alg) {
	this->alg = alg;
}

Algorithm * Strategy::getAlgorithm() {
	return this->alg;
}

void Strategy::addInstrumentToList(string instrument) {
	this->l_instruments->push_back(instrument);
}

list<string> * Strategy::getListInstruments() {
	return this->l_instruments;
}

void Strategy::setTraderID(string traderid) {
	this->traderid = traderid;
}
string Strategy::getTraderID() {
	return this->traderid;
}

void Strategy::setUserID(string userid) {
	this->userid = userid;
}
string Strategy::getUserID() {
	return this->userid;
}



CThostFtdcInputOrderField* Strategy::getStgAOrderInsertArgs() {
	return stg_a_order_insert_args;
}

void Strategy::setStgAOrderInsertArgs(
	CThostFtdcInputOrderField* stgAOrderInsertArgs) {
	stg_a_order_insert_args = stgAOrderInsertArgs;
}

double Strategy::getStgAPriceTick() {
	return stg_a_price_tick;
}

void Strategy::setStgAPriceTick(double stgAPriceTick) {
	stg_a_price_tick = stgAPriceTick;
}

double Strategy::getStgAWaitPriceTick() {
	return stg_a_wait_price_tick;
}

void Strategy::setStgAWaitPriceTick(double stgAWaitPriceTick) {
	stg_a_wait_price_tick = stgAWaitPriceTick;
}

CThostFtdcInputOrderField* Strategy::getStgBOrderInsertArgs() {
	return stg_b_order_insert_args;
}

void Strategy::setStgBOrderInsertArgs(
	CThostFtdcInputOrderField* stgBOrderInsertArgs) {
	stg_b_order_insert_args = stgBOrderInsertArgs;
}

double Strategy::getStgBPriceTick() {
	return stg_b_price_tick;
}

void Strategy::setStgBPriceTick(double stgBPriceTick) {
	stg_b_price_tick = stgBPriceTick;
}

double Strategy::getStgBWaitPriceTick() {
	return stg_b_wait_price_tick;
}

void Strategy::setStgBWaitPriceTick(double stgBWaitPriceTick) {
	stg_b_wait_price_tick = stgBWaitPriceTick;
}

double Strategy::getStgBuyClose() {
	return stg_buy_close;
}

void Strategy::setStgBuyClose(double stgBuyClose) {
	stg_buy_close = stgBuyClose;
}

double Strategy::getStgBuyOpen() {
	return stg_buy_open;
}

void Strategy::setStgBuyOpen(double stgBuyOpen) {
	stg_buy_open = stgBuyOpen;
}

DBManager* Strategy::getStgDbm() {
	return stg_DBM;
}

void Strategy::setStgDbm(DBManager* stgDbm) {
	stg_DBM = stgDbm;
}

CThostFtdcDepthMarketDataField* Strategy::getStgInstrumentATick() {
	return stg_instrument_A_tick;
}

void Strategy::setStgInstrumentATick(
	CThostFtdcDepthMarketDataField* stgInstrumentATick) {
	stg_instrument_A_tick = stgInstrumentATick;
}

CThostFtdcDepthMarketDataField* Strategy::getStgInstrumentBTick() {
	return stg_instrument_B_tick;
}

void Strategy::setStgInstrumentBTick(
	CThostFtdcDepthMarketDataField* stgInstrumentBTick) {
	stg_instrument_B_tick = stgInstrumentBTick;
}

CThostFtdcDepthMarketDataField* Strategy::getStgInstrumentATickLast() {
	return stg_instrument_A_tick_last;
}
void Strategy::setStgInstrumentATickLast(
	CThostFtdcDepthMarketDataField* stgInstrumentATick) {
	stg_instrument_A_tick_last = stgInstrumentATick;
}

CThostFtdcDepthMarketDataField* Strategy::getStgInstrumentBTickLast() {
	return stg_instrument_B_tick_last;
}
void Strategy::setStgInstrumentBTickLast(
	CThostFtdcDepthMarketDataField* stgInstrumentBTick) {
	stg_instrument_B_tick_last = stgInstrumentBTick;
}

string Strategy::getStgInstrumentIdA() {
	return stg_instrument_id_A;
}

void Strategy::setStgInstrumentIdA(string stgInstrumentIdA) {
	stg_instrument_id_A = stgInstrumentIdA;
}

string Strategy::getStgInstrumentIdB() {
	return stg_instrument_id_B;
}

void Strategy::setStgInstrumentIdB(string stgInstrumentIdB) {
	stg_instrument_id_B = stgInstrumentIdB;
}

bool Strategy::isStgIsActive() {
	return stg_is_active;
}

void Strategy::setStgIsActive(bool stgIsActive) {
	stg_is_active = stgIsActive;
}

list< CThostFtdcOrderField *> * Strategy::getStgListOrderPending() {
	return stg_list_order_pending;
}

void Strategy::setStgListOrderPending(list<CThostFtdcOrderField *> * stgListOrderPending) {
	stg_list_order_pending = stgListOrderPending;
}

int Strategy::getStgLots() {
	return stg_lots;
}

void Strategy::setStgLots(int stgLots) {
	stg_lots = stgLots;
}

int Strategy::getStgLotsBatch() {
	return stg_lots_batch;
}

void Strategy::setStgLotsBatch(int stgLotsBatch) {
	stg_lots_batch = stgLotsBatch;
}

bool Strategy::isStgOnlyClose() {
	return stg_only_close;
}

void Strategy::setStgOnlyClose(bool stgOnlyClose) {
	stg_only_close = stgOnlyClose;
}

int Strategy::getStgOrderActionTiresLimit() {
	return stg_order_action_tires_limit;
}

void Strategy::setStgOrderActionTiresLimit(int stgOrderActionTiresLimit) {
	stg_order_action_tires_limit = stgOrderActionTiresLimit;
}

string Strategy::getStgOrderAlgorithm() {
	return stg_order_algorithm;
}

void Strategy::setStgOrderAlgorithm(string stgOrderAlgorithm) {
	stg_order_algorithm = stgOrderAlgorithm;
}

string Strategy::getStgOrderRefA() {
	return stg_order_ref_a;
}

void Strategy::setStgOrderRefA(string stgOrderRefA) {
	stg_order_ref_a = stgOrderRefA;
}

string Strategy::getStgOrderRefB() {
	return stg_order_ref_b;
}

void Strategy::setStgOrderRefB(string stgOrderRefB) {
	stg_order_ref_b = stgOrderRefB;
}

string Strategy::getStgOrderRefLast() {
	return stg_order_ref_last;
}

void Strategy::setStgOrderRefLast(string stgOrderRefLast) {
	stg_order_ref_last = stgOrderRefLast;
}

int Strategy::getStgPositionABuy() {
	return stg_position_a_buy;
}

void Strategy::setStgPositionABuy(int stgPositionABuy) {
	stg_position_a_buy = stgPositionABuy;
}

int Strategy::getStgPositionABuyToday() {
	return stg_position_a_buy_today;
}

void Strategy::setStgPositionABuyToday(int stgPositionABuyToday) {
	stg_position_a_buy_today = stgPositionABuyToday;
}

int Strategy::getStgPositionABuyYesterday() {
	return stg_position_a_buy_yesterday;
}

void Strategy::setStgPositionABuyYesterday(int stgPositionABuyYesterday) {
	stg_position_a_buy_yesterday = stgPositionABuyYesterday;
}

int Strategy::getStgPositionASell() {
	return stg_position_a_sell;
}

void Strategy::setStgPositionASell(int stgPositionASell) {
	stg_position_a_sell = stgPositionASell;
}

int Strategy::getStgPositionASellToday() {
	return stg_position_a_sell_today;
}

void Strategy::setStgPositionASellToday(int stgPositionASellToday) {
	stg_position_a_sell_today = stgPositionASellToday;
}

int Strategy::getStgPositionASellYesterday() {
	return stg_position_a_sell_yesterday;
}

void Strategy::setStgPositionASellYesterday(int stgPositionASellYesterday) {
	stg_position_a_sell_yesterday = stgPositionASellYesterday;
}

int Strategy::getStgPositionBBuy() {
	return stg_position_b_buy;
}

void Strategy::setStgPositionBBuy(int stgPositionBBuy) {
	stg_position_b_buy = stgPositionBBuy;
}

int Strategy::getStgPositionBBuyToday() {
	return stg_position_b_buy_today;
}

void Strategy::setStgPositionBBuyToday(int stgPositionBBuyToday) {
	stg_position_b_buy_today = stgPositionBBuyToday;
}

int Strategy::getStgPositionBBuyYesterday() {
	return stg_position_b_buy_yesterday;
}

void Strategy::setStgPositionBBuyYesterday(int stgPositionBBuyYesterday) {
	stg_position_b_buy_yesterday = stgPositionBBuyYesterday;
}

int Strategy::getStgPositionBSell() {
	return stg_position_b_sell;
}

void Strategy::setStgPositionBSell(int stgPositionBSell) {
	stg_position_b_sell = stgPositionBSell;
}

int Strategy::getStgPositionBSellToday() {
	return stg_position_b_sell_today;
}

void Strategy::setStgPositionBSellToday(int stgPositionBSellToday) {
	stg_position_b_sell_today = stgPositionBSellToday;
}

int Strategy::getStgPositionBSellYesterday() {
	return stg_position_b_sell_yesterday;
}

void Strategy::setStgPositionBSellYesterday(int stgPositionBSellYesterday) {
	stg_position_b_sell_yesterday = stgPositionBSellYesterday;
}

double Strategy::getStgSellClose() {
	return stg_sell_close;
}

void Strategy::setStgSellClose(double stgSellClose) {
	stg_sell_close = stgSellClose;
}

double Strategy::getStgSellOpen() {
	return stg_sell_open;
}

void Strategy::setStgSellOpen(double stgSellOpen) {
	stg_sell_open = stgSellOpen;
}

double Strategy::getStgSpread() {
	return stg_spread;
}

void Strategy::setStgSpread(double stgSpread) {
	stg_spread = stgSpread;
}

double Strategy::getStgSpreadLong() {
	return stg_spread_long;
}

void Strategy::setStgSpreadLong(double stgSpreadLong) {
	stg_spread_long = stgSpreadLong;
}

int Strategy::getStgSpreadLongVolume() {
	return stg_spread_long_volume;
}

void Strategy::setStgSpreadLongVolume(int stgSpreadLongVolume) {
	stg_spread_long_volume = stgSpreadLongVolume;
}

double Strategy::getStgSpreadShift() {
	return stg_spread_shift;
}

void Strategy::setStgSpreadShift(double stgSpreadShift) {
	stg_spread_shift = stgSpreadShift;
}

double Strategy::getStgSpreadShort() {
	return stg_spread_short;
}

void Strategy::setStgSpreadShort(double stgSpreadShort) {
	stg_spread_short = stgSpreadShort;
}

int Strategy::getStgSpreadShortVolume() {
	return stg_spread_short_volume;
}

void Strategy::setStgSpreadShortVolume(int stgSpreadShortVolume) {
	stg_spread_short_volume = stgSpreadShortVolume;
}

double Strategy::getStgStopLoss() {
	return stg_stop_loss;
}

void Strategy::setStgStopLoss(double stgStopLoss) {
	stg_stop_loss = stgStopLoss;
}

string Strategy::getStgStrategyId() {
	return stg_strategy_id;
}

void Strategy::setStgStrategyId(string stgStrategyId) {
	stg_strategy_id = stgStrategyId;
}

bool Strategy::isStgTradeTasking() {
	return stg_trade_tasking;
}

void Strategy::setStgTradeTasking(bool stgTradeTasking) {
	stg_trade_tasking = stgTradeTasking;
}

string Strategy::getStgTraderId() {
	return stg_trader_id;
}

void Strategy::setStgTraderId(string stgTraderId) {
	stg_trader_id = stgTraderId;
}

User* Strategy::getStgUser() {
	return stg_user;
}

void Strategy::setStgUser(User* stgUser) {
	stg_user = stgUser;
}

string Strategy::getStgUserId() {
	return stg_user_id;
}

void Strategy::setStgUserId(string stgUserId) {
	stg_user_id = stgUserId;
}

void Strategy::setStgOrderRefBase(long long stg_order_ref_base) {
	this->stg_order_ref_base = stg_order_ref_base;
}

long long Strategy::getStgOrderRefBase() {
	return this->stg_order_ref_base;
}

/************************************************************************/
/* 交易相关的回报函数                                                      */
/************************************************************************/
//下单
void Strategy::OrderInsert(User *user, char *InstrumentID, char CombOffsetFlag, char Direction, int Volume, double Price, string OrderRef) {
	USER_PRINT("Strategy::OrderInsert");
}

//下单响应
void Strategy::OnRtnOrder(CThostFtdcOrderField *pOrder) {
	USER_PRINT("Strategy::OnRtnOrder");

	

	/// 如果合约在撤单维护列表里，那么撤单次数增加1
	this->stg_user->add_instrument_id_action_counter(pOrder->InstrumentID);
	this->Exec_OnRtnOrder(pOrder);
}

//成交通知
void Strategy::OnRtnTrade(CThostFtdcTradeField *pTrade) {
	USER_PRINT("Strategy::OnRtnTrade");
	this->ExEc_OnRtnTrade(pTrade);
}

//下单错误响应
void Strategy::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder) {
	USER_PRINT("Strategy::OnErrRtnOrderInsert");
}

///报单录入请求响应
void Strategy::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder) {
	USER_PRINT("Strategy::OnRspOrderInsert");
	this->Exec_OnRspOrderInsert();
}

//撤单
void Strategy::OrderAction(string ExchangeID, string OrderRef, string OrderSysID) {
	USER_PRINT("Strategy::OrderAction");
}

//撤单错误响应
void Strategy::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction) {
	USER_PRINT("Strategy::OnRspOrderAction");
}

//撤单错误
void Strategy::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction) {
	USER_PRINT("Strategy::OnErrRtnOrderAction");
}


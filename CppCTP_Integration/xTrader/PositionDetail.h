#ifndef QUANT_POSITION_DETAIL_H
#define QUANT_POSITION_DETAIL_H

#include <iostream>

using namespace std;
using std::string;

class PositionDetail
{
public:
	
	void setInstrumentID(string	InstrumentID);
	string getInstrumentID();

	void setOrderRef(string OrderRef);
	string getOrderRef();

	void setUserID(string UserID);
	string getUserID();

	void setDirection(int Direction);
	int getDirection();

	///组合开平标志
	void setCombOffsetFlag(string CombOffsetFlag);
	string	getCombOffsetFlag();

	///组合投机套保标志
	void setCombHedgeFlag(string CombHedgeFlag);
	string	getCombHedgeFlag();

	///价格
	void setLimitPrice(double LimitPrice);
	double getLimitPrice();

	///数量
	void setVolumeTotalOriginal(int	VolumeTotalOriginal);
	int	getVolumeTotalOriginal();

	///交易日
	void setTradingDay(string TradingDay);
	string	getTradingDay();

	///报单状态
	void	setOrderStatus(int OrderStatus);
	int	getOrderStatus();

	///今成交数量
	void	setVolumeTraded(int VolumeTraded);
	int	getVolumeTraded();

	///剩余数量
	void	setVolumeTotal(int VolumeTotal);
	int	getVolumeTotal();

	///报单日期
	void setInsertDate(string InsertDate);
	string	getInsertDate();

	///委托时间
	void setInsertTime(string InsertTime);
	string	getInsertTime();

	///策略编号(额外添加)
	void setStrategyID(string StrategyID);
	string getStrategyID();

	///一批成交量
	void setVolumeTradedBatch(int VolumeTradedBatch);
	int getVolumeTradedBatch();

	void setIsActive(string isActive);
	string getIsActive();

private:
	///合约代码
	string	InstrumentID;
	///报单引用
	string	OrderRef;
	///用户代码
	string	UserID;
	///买卖方向
	int	Direction;
	///组合开平标志
	string	CombOffsetFlag;
	///组合投机套保标志
	string	CombHedgeFlag;
	///价格
	double	LimitPrice;
	///数量
	int	VolumeTotalOriginal;
	///交易日
	string	TradingDay;
	///报单状态
	int	OrderStatus;
	///今成交数量
	int	VolumeTraded;
	///剩余数量
	int	VolumeTotal;
	///报单日期
	string	InsertDate;
	///委托时间
	string	InsertTime;
	///策略编号(额外添加)
	string StrategyID;
	///一批成交量
	int VolumeTradedBatch;
	///是否激活
	string isActive;
};

#endif
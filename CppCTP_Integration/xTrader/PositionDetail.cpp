#include "PositionDetail.h"

void PositionDetail::setInstrumentID(string	InstrumentID){
	this->InstrumentID = InstrumentID;
}
string PositionDetail::getInstrumentID(){
	return this->InstrumentID;
}

void PositionDetail::setOrderRef(string OrderRef){
	this->OrderRef = OrderRef;
}
string PositionDetail::getOrderRef(){
	return this->OrderRef;
}

void PositionDetail::setUserID(string UserID){
	this->UserID = UserID;
}
string PositionDetail::getUserID(){
	return this->UserID;
}

void PositionDetail::setDirection(int Direction){
	this->Direction = Direction;
}
int PositionDetail::getDirection(){
	return this->Direction;
}

///组合开平标志
void PositionDetail::setCombOffsetFlag(string CombOffsetFlag){
	this->CombOffsetFlag = CombOffsetFlag;
}
string	PositionDetail::getCombOffsetFlag(){
	return this->CombOffsetFlag;
}

///组合投机套保标志
void PositionDetail::setCombHedgeFlag(string CombHedgeFlag){
	this->CombHedgeFlag = CombHedgeFlag;
}
string	PositionDetail::getCombHedgeFlag() {
	return this->CombHedgeFlag;
}

///价格
void PositionDetail::setLimitPrice(double LimitPrice) {
	this->LimitPrice = LimitPrice;
}

double PositionDetail::getLimitPrice() {
	return this->getLimitPrice();
}

///数量
void PositionDetail::setVolumeTotalOriginal(int	VolumeTotalOriginal){
	this->VolumeTotalOriginal = VolumeTotalOriginal;
}
int	PositionDetail::getVolumeTotalOriginal(){
	return this->VolumeTotalOriginal;
}

///交易日
void PositionDetail::setTradingDay(string TradingDay){
	this->TradingDay = TradingDay;
}
string	PositionDetail::getTradingDay(){
	return this->TradingDay;
}

///报单状态
void PositionDetail::setOrderStatus(int OrderStatus){
	this->OrderStatus = OrderStatus;
}

int PositionDetail::getOrderStatus(){
	return this->OrderStatus;
}

///今成交数量
void PositionDetail::setVolumeTraded(int VolumeTraded){
	this->VolumeTraded = VolumeTraded;
}
int	PositionDetail::getVolumeTraded(){
	return this->VolumeTraded;
}

///剩余数量
void PositionDetail::setVolumeTotal(int VolumeTotal){
	this->VolumeTotal = VolumeTotal;
}

int	PositionDetail::getVolumeTotal() {
	return this->VolumeTotal;
}

///报单日期
void PositionDetail::setInsertDate(string InsertDate){
	this->InsertDate = InsertDate;
}

string PositionDetail::getInsertDate() {
	return this->InsertDate;
}

///委托时间
void PositionDetail::setInsertTime(string InsertTime){
	this->InsertTime = InsertTime;
}

string PositionDetail::getInsertTime(){
	return this->InsertTime;
}

///策略编号(额外添加)
void PositionDetail::setStrategyID(string StrategyID){
	this->StrategyID = StrategyID;
}

string PositionDetail::getStrategyID(){
	return this->StrategyID;
}

///一批成交量
void PositionDetail::setVolumeTradedBatch(int VolumeTradedBatch){
	this->VolumeTradedBatch = VolumeTradedBatch;
}

int PositionDetail::getVolumeTradedBatch(){
	return this->VolumeTradedBatch;
}

void PositionDetail::setIsActive(string isActive) {
	this->isActive = isActive;
}
string PositionDetail::getIsActive() {
	return this->isActive;
}
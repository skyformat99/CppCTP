# distutils: language=c++
from libc cimport stdlib
from libcpp.string cimport string
from libc.string cimport strcpy, memset
from SecurityFtdcMdApi cimport *

cdef void MdApi_Release(MdApi self):
    ReleaseMdApi(self.api, self.spi)
    self.api = self.spi = NULL

cdef class MdApi:
    cdef CMdApi *api
    cdef CMdSpi *spi

    def __dealloc__(self):
        MdApi_Release(self)

    def Alive(self):
        if self.spi is not NULL: return self.spi.tid
        if self.api is not NULL: return False

    def Create(self, const_char *pszFlowPath=""):
        if self.api is not NULL: return
        self.api = CreateFtdcMdApi(pszFlowPath)
        CheckMemory(self.api)

    def Release(self):
        MdApi_Release(self)

    def Init(self):
        # print(">>> SecurityFtdcMdApi.pyx Init() begin")
        if self.api is NULL or self.spi is not NULL: return
        self.spi = new CMdSpi(<PyObject *>self)
        self.spi.InitPyThread()
        CheckMemory(self.spi)
        self.api.RegisterSpi(self.spi)
        self.api.Init()

    def Join(self):
        cdef int ret
        if self.spi is NULL: return
        with nogil: ret = self.api.Join()
        return ret

    def GetTradingDay(self):
        cdef const_char *ret
        if self.spi is NULL: return
        with nogil: ret = self.api.GetTradingDay()
        return ret

    def RegisterFront(self, const_char *pszFrontAddress):
        if self.api is NULL: return
        cdef char *c_pszFrontAddress
        strcpy(c_pszFrontAddress, pszFrontAddress)
        self.api.RegisterFront(c_pszFrontAddress)

    def SubscribeMarketData(self, InstrumentIDs, ExchageID):
        # pExchageID = bytes(ExchageID, 'gbk')
        # pExchageID = ExchageID.decode('utf-8').encode('gbk')
        pExchageID = ExchageID
        cdef int nCount
        cdef char **ppInstrumentID
        cdef char *cExchangeID
        strcpy(cExchangeID, pExchageID)
        # print(">>> SubscribeMarketData() cExchangeID = ", cExchangeID)
        if self.spi is NULL: return
        nCount = len(InstrumentIDs)
        ppInstrumentID = <char **>stdlib.malloc(sizeof(char *) * nCount)
        CheckMemory(ppInstrumentID)
        try:
            for i from 0 <= i < nCount:
                ppInstrumentID[i] = InstrumentIDs[i]
                # print("ppInstrumentID[i] = ", ppInstrumentID[i])
            with nogil: nCount = self.api.SubscribeMarketData(ppInstrumentID, nCount, cExchangeID)
        finally:
            stdlib.free(ppInstrumentID)
        return nCount

    def UnSubscribeMarketData(self, pInstrumentIDs, char *pExchageID):
        cdef int nCount
        cdef char **ppInstrumentID
        if self.spi is NULL: return
        nCount = len(pInstrumentIDs)
        ppInstrumentID = <char **>stdlib.malloc(sizeof(char *) * nCount)
        CheckMemory(ppInstrumentID)
        try:
            for i from 0 <= i < nCount:
                ppInstrumentID[i] = pInstrumentIDs[i]
            with nogil: nCount = self.api.UnSubscribeMarketData(ppInstrumentID, nCount, pExchageID)
        finally:
            stdlib.free(ppInstrumentID)
        return nCount

    def ReqUserLogin(self, pReqUserLogin, int nRequestID):
        if self.spi is NULL: return
        # print(">>> SecurityFtdcMdApi.pyx ReqUserLogin", pReqUserLogin, pReqUserLogin['BrokerID'])
        cdef CSecurityFtdcReqUserLoginField loginField
        memset(loginField.UserID, 0, sizeof(loginField.UserID))
        memset(loginField.BrokerID, 0, sizeof(loginField.BrokerID))
        memset(loginField.Password, 0, sizeof(loginField.Password))
        strcpy(loginField.BrokerID, pReqUserLogin['BrokerID'])
        strcpy(loginField.UserID, pReqUserLogin['UserID'])
        strcpy(loginField.Password, pReqUserLogin['Password'])

        with nogil: nRequestID = self.api.ReqUserLogin(&loginField, nRequestID)
        return nRequestID

    def ReqUserLogout(self, pUserLogout, int nRequestID):
        if self.spi is NULL: return
        cdef CSecurityFtdcUserLogoutField logoutField
        logoutField.UserID = pUserLogout['UserID']
        logoutField.BrokerID = pUserLogout['BrokerID']
        with nogil: nRequestID = self.api.ReqUserLogout(&logoutField, nRequestID)
        return nRequestID


cdef extern int MdSpi_OnFrontConnected(self) except -1:
    self.OnFrontConnected()
    return 0

cdef extern int MdSpi_OnFrontDisconnected(self, int nReason) except -1:
    self.OnFrontDisconnected(nReason)
    return 0

cdef extern int MdSpi_OnHeartBeatWarning(self, int nTimeLapse) except -1:
    self.OnHeartBeatWarning(nTimeLapse)
    return 0

cdef extern int MdSpi_OnRspError(self, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, cbool bIsLast) except -1:
    dict_pRspInfo = {}
    dict_pRspInfo['ErrorMsg'] = pRspInfo.ErrorMsg
    dict_pRspInfo['ErrorID'] = pRspInfo.ErrorID
    self.OnRspError(dict_pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRspUserLogin(self, CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, cbool bIsLast) except -1:
    # py_obj = <object>pRspUserLogin
    # print(">>> MdSpi_OnRspUserLogin() py_obj ", py_obj)
    dict_pRspUserLogin = {}
    dict_pRspUserLogin['BrokerID'] = pRspUserLogin.BrokerID
    dict_pRspUserLogin['UserID'] = pRspUserLogin.UserID
    dict_pRspUserLogin['FrontID'] = pRspUserLogin.FrontID
    dict_pRspUserLogin['LoginTime'] = pRspUserLogin.LoginTime
    dict_pRspUserLogin['MaxOrderRef'] = pRspUserLogin.MaxOrderRef
    dict_pRspUserLogin['SessionID'] = pRspUserLogin.SessionID

    dict_pRspInfo = {}
    dict_pRspInfo['ErrorID'] = pRspInfo.ErrorID
    dict_pRspInfo['ErrorMsg'] = pRspInfo.ErrorMsg

    self.OnRspUserLogin(dict_pRspUserLogin, dict_pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRspUserLogout(self, CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, cbool bIsLast) except -1:

    # self.OnRspUserLogout(pUserLogout, pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRspSubMarketData(self, CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, cbool bIsLast) except -1:
    # self.OnRspSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast)
    print("MdSpi_OnRspSubMarketData() >>>", pSpecificInstrument.InstrumentID, pSpecificInstrument.ExchangeID)
    dict_pSpecificInstrument = {}
    ###合约代码
    dict_pSpecificInstrument['InstrumentID'] = pSpecificInstrument.InstrumentID
    ###交易所代码
    dict_pSpecificInstrument['ExchangeID'] = pSpecificInstrument.ExchangeID

    dict_pRspInfo = {}
    dict_pRspInfo['ErrorID'] = pRspInfo.ErrorID
    dict_pRspInfo['ErrorMsg'] = pRspInfo.ErrorMsg

    self.OnRspSubMarketData(dict_pSpecificInstrument, dict_pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRspUnSubMarketData(self, CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, cbool bIsLast) except -1:
    # self.OnRspUnSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRtnDepthMarketData(self, CSecurityFtdcDepthMarketDataField *pDepthMarketData) except -1:
    # self.OnRtnDepthMarketData(pDepthMarketData)
    # print(">>> MdSpi_OnRtnDepthMarketData()...")
    dict_pDepthMarketData = {}
    ###交易日
    dict_pDepthMarketData['TradingDay'] = pDepthMarketData.TradingDay
    ###合约代码
    dict_pDepthMarketData['InstrumentID'] = pDepthMarketData.InstrumentID
    ###交易所代码
    dict_pDepthMarketData['ExchangeID'] = pDepthMarketData.ExchangeID
    ###合约在交易所的代码
    dict_pDepthMarketData['ExchangeInstID'] = pDepthMarketData.ExchangeInstID
    ###最新价
    dict_pDepthMarketData['LastPrice'] = pDepthMarketData.LastPrice
    ###上次结算价
    dict_pDepthMarketData['PreSettlementPrice'] = pDepthMarketData.PreSettlementPrice
    ###昨收盘
    dict_pDepthMarketData['PreClosePrice'] = pDepthMarketData.PreClosePrice
    ###昨持仓量
    dict_pDepthMarketData['PreOpenInterest'] = pDepthMarketData.PreOpenInterest
    ###今开盘
    dict_pDepthMarketData['OpenPrice'] = pDepthMarketData.OpenPrice
    ###最高价
    dict_pDepthMarketData['HighestPrice'] = pDepthMarketData.HighestPrice
    ###最低价
    dict_pDepthMarketData['LowestPrice'] = pDepthMarketData.LowestPrice
    ###数量
    dict_pDepthMarketData['Volume'] = pDepthMarketData.Volume
    ###成交金额
    dict_pDepthMarketData['Turnover'] = pDepthMarketData.Turnover
    ###持仓量
    dict_pDepthMarketData['OpenInterest'] = pDepthMarketData.OpenInterest
    ###今收盘
    dict_pDepthMarketData['ClosePrice'] = pDepthMarketData.ClosePrice
    ###本次结算价
    dict_pDepthMarketData['SettlementPrice'] = pDepthMarketData.SettlementPrice
    ###涨停板价
    dict_pDepthMarketData['UpperLimitPrice'] = pDepthMarketData.UpperLimitPrice
    ###跌停板价
    dict_pDepthMarketData['LowerLimitPrice'] = pDepthMarketData.LowerLimitPrice
    ###昨虚实度
    dict_pDepthMarketData['PreDelta'] = pDepthMarketData.PreDelta
    ###今虚实度
    dict_pDepthMarketData['CurrDelta'] = pDepthMarketData.CurrDelta
    ###昨日基金净值
    dict_pDepthMarketData['PreIOPV'] = pDepthMarketData.PreIOPV
    ###基金净值
    dict_pDepthMarketData['IOPV'] = pDepthMarketData.IOPV
    ###动态参考价格
    dict_pDepthMarketData['AuctionPrice'] = pDepthMarketData.AuctionPrice
    ###最后修改时间
    dict_pDepthMarketData['UpdateTime'] = pDepthMarketData.UpdateTime
    ###最后修改毫秒
    dict_pDepthMarketData['UpdateMillisec'] = pDepthMarketData.UpdateMillisec
    ###申买价一
    dict_pDepthMarketData['BidPrice1'] = pDepthMarketData.BidPrice1
    ###申买量一
    dict_pDepthMarketData['BidVolume1'] = pDepthMarketData.BidVolume1
    ###申卖价一
    dict_pDepthMarketData['AskPrice1'] = pDepthMarketData.AskPrice1
    ###申卖量一
    dict_pDepthMarketData['AskVolume1'] = pDepthMarketData.AskVolume1
    ###申买价二
    dict_pDepthMarketData['BidPrice2'] = pDepthMarketData.BidPrice2
    ###申买量二
    dict_pDepthMarketData['BidVolume2'] = pDepthMarketData.BidVolume2
    ###申卖价二
    dict_pDepthMarketData['AskPrice2'] = pDepthMarketData.AskPrice2
    ###申卖量二
    dict_pDepthMarketData['AskVolume2'] = pDepthMarketData.AskVolume2
    ###申买价三
    dict_pDepthMarketData['BidPrice3'] = pDepthMarketData.BidPrice3
    ###申买量三
    dict_pDepthMarketData['BidVolume3'] = pDepthMarketData.BidVolume3
    ###申卖价三
    dict_pDepthMarketData['AskPrice3'] = pDepthMarketData.AskPrice3
    ###申卖量三
    dict_pDepthMarketData['AskVolume3'] = pDepthMarketData.AskVolume3
    ###申买价四
    dict_pDepthMarketData['BidPrice4'] = pDepthMarketData.BidPrice4
    ###申买量四
    dict_pDepthMarketData['BidVolume4'] = pDepthMarketData.BidVolume4
    ###申卖价四
    dict_pDepthMarketData['AskPrice4'] = pDepthMarketData.AskPrice4
    ###申卖量四
    dict_pDepthMarketData['AskVolume4'] = pDepthMarketData.AskVolume4
    ###申买价五
    dict_pDepthMarketData['BidPrice5'] = pDepthMarketData.BidPrice5
    ###申买量五
    dict_pDepthMarketData['BidVolume5'] = pDepthMarketData.BidVolume5
    ###申卖价五
    dict_pDepthMarketData['AskPrice5'] = pDepthMarketData.AskPrice5
    ###申卖量五
    dict_pDepthMarketData['AskVolume5'] = pDepthMarketData.AskVolume5
    ###当日均价
    dict_pDepthMarketData['AveragePrice'] = pDepthMarketData.AveragePrice
    ###业务日期
    dict_pDepthMarketData['ActionDay'] = pDepthMarketData.ActionDay
    ###交易阶段
    dict_pDepthMarketData['TradingPhase'] = pDepthMarketData.TradingPhase
    ###开仓限制
    dict_pDepthMarketData['OpenRestriction'] = pDepthMarketData.OpenRestriction

    self.OnRtnDepthMarketData(dict_pDepthMarketData)

    return 0
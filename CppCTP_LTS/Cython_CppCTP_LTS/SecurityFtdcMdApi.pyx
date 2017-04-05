# distutils: language=c++
from libc cimport stdlib
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
        if self.api is NULL or self.spi is not NULL: return
        self.spi = new CMdSpi(<PyObject *>self)
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

    def RegisterFront(self, char *pszFrontAddress):
        if self.api is NULL: return
        self.api.RegisterFront(pszFrontAddress)

    def SubscribeMarketData(self, pInstrumentIDs, char *pExchageID):
        cdef int nCount
        cdef char **ppInstrumentID
        if self.spi is NULL: return
        nCount = len(pInstrumentIDs)
        ppInstrumentID = <char **>stdlib.malloc(sizeof(char *) * nCount)
        CheckMemory(ppInstrumentID)
        try:
            for i from 0 <= i < nCount:
                ppInstrumentID[i] = pInstrumentIDs[i]
            with nogil: nCount = self.api.SubscribeMarketData(ppInstrumentID, nCount, pExchageID)
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
        cdef CSecurityFtdcReqUserLoginField loginField
        loginField.BrokerID = pReqUserLogin['BrokerID']
        loginField.UserID = pReqUserLogin['UserID']
        loginField.Password = pReqUserLogin['Password']
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
    self.OnRspError(pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRspUserLogin(self, CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, cbool bIsLast) except -1:
    self.OnRspUserLogin(pRspUserLogin, pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRspUserLogout(self, CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, cbool bIsLast) except -1:
    self.OnRspUserLogout(pUserLogout, pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRspSubMarketData(self, CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, cbool bIsLast) except -1:
    self.OnRspSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRspUnSubMarketData(self, CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, cbool bIsLast) except -1:
    self.OnRspUnSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast)
    return 0

cdef extern int MdSpi_OnRtnDepthMarketData(self, CSecurityFtdcDepthMarketDataField *pDepthMarketData) except -1:
    self.OnRtnDepthMarketData(pDepthMarketData)
    return 0
# distutils: language=c++
from libc cimport stdlib
from SecurityFtdcMdApi cimport *

cdef class pyMdApi:
    cdef CMDApi *basePtr
    cdef CMDSpi *spiPtr
    def __cinit__(self, char *pszFlowPath = ""):
        self.basePtr = CreateFtdcMdApi(pszFlowPath)
        self.spiPtr = new CMDSpi()
        pass
    ###创建MdApi
    ###@param pszFlowPath 存贮订阅信息文件的目录，默认为当前目录
    ###@return 创建出的UserApi
    ###modify for udp marketdata
    # def CreateFtdcMdApi(self):
    #     return self.basePtr

    ###删除接口对象本身
    ###@remark 不再使用本接口对象时,调用该函数删除接口对象
    def Release(self):
        self.basePtr.Release()

    ###初始化
    ###@remark 初始化运行环境,只有调用后,接口才开始工作
    def Init(self):
        self.basePtr.Init()
        pass

    ###等待接口线程结束运行
    ###@return 线程退出代码
    def Join(self):
        self.basePtr.Join()

    ###获取当前交易日
    ###@retrun 获取到的交易日
    ###@remark 只有登录成功后,才能得到正确的交易日
    def GetTradingDay(self):
        return self.basePtr.GetTradingDay()
        pass

    ###注册前置机网络地址
    ###@param pszFrontAddress：前置机网络地址。
    ###@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:17001”。
    ###@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”17001”代表服务器端口号。
    def RegisterFront(self, char *pszFrontAddress):
        return self.basePtr.RegisterFront(pszFrontAddress)
        pass

    ###注册回调接口
    ###@param pSpi 派生自回调接口类的实例
    def RegisterSpi(self):
        self.basePtr.RegisterSpi(self.spiPtr)
        pass

    ###订阅行情。
    ###@param ppInstrumentID 合约ID
    ###@param nCount 要订阅/退订行情的合约个数
    ###@remark
    def SubscribeMarketData(self, pInstrumentIDs, char* pExchageID):
        cdef int nCount
        cdef char **ppInstrumentID
        nCount = len(pInstrumentIDs)
        ppInstrumentID = <char **>stdlib.malloc(sizeof(char *) * nCount)
        try:
            for i from 0 <= i < nCount:
                ppInstrumentID[i] = pInstrumentIDs[i]
            nCount = self.basePtr.SubscribeMarketData(ppInstrumentID, nCount, pExchageID)
        finally:
            stdlib.free(ppInstrumentID)
        return nCount

    ###退订行情。
    ###@param ppInstrumentID 合约ID
    ###@param nCount 要订阅/退订行情的合约个数
    ###@remark
    def UnSubscribeMarketData(self, pInstrumentIDs, char* pExchageID):
        cdef int nCount
        cdef char **ppInstrumentID
        nCount = len(pInstrumentIDs)
        ppInstrumentID = <char **>stdlib.malloc(sizeof(char *) * nCount)
        try:
            for i from 0 <= i < nCount:
                ppInstrumentID[i] = pInstrumentIDs[i]
            nCount = self.basePtr.UnSubscribeMarketData(ppInstrumentID, nCount, pExchageID)
        finally:
            stdlib.free(ppInstrumentID)
        return nCount

    ###用户登录请求
    def ReqUserLogin(self, loginField, int nRequestID):
        cdef CSecurityFtdcReqUserLoginField *loginFieldPtr
        loginFieldPtr = <CSecurityFtdcReqUserLoginField *>stdlib.malloc(sizeof(CSecurityFtdcReqUserLoginField))
        loginFieldPtr.BrokerID = loginField["BrokerID"]
        loginFieldPtr.UserID = loginField["UserID"]
        loginFieldPtr.Password = loginField["Password"]
        self.basePtr.ReqUserLogin(loginFieldPtr, nRequestID)
        stdlib.free(loginFieldPtr)
        pass

    ###登出请求
    def ReqUserLogout(self, logoutField, int nRequestID):
        cdef CSecurityFtdcUserLogoutField *logoutFieldPtr
        logoutFieldPtr = <CSecurityFtdcUserLogoutField *>stdlib.malloc(sizeof(CSecurityFtdcUserLogoutField))
        logoutFieldPtr.UserID = logoutField["UserID"]
        logoutFieldPtr.BrokerID = logoutField["BrokerID"]
        self.basePtr.ReqUserLogout(logoutFieldPtr, nRequestID)
        stdlib.free(logoutFieldPtr)
        pass
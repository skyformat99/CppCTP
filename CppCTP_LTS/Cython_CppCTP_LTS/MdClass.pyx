# distutils: language = c++
# distutils: sources = MdClass.cpp
cimport MdClass
from MdClass cimport *

cdef class pyMdApi:
    cdef CSecurityFtdcMdApi *basePtr
    def __cinit__(self):
        pass
    ###创建MdApi
    ###@param pszFlowPath 存贮订阅信息文件的目录，默认为当前目录
    ###@return 创建出的UserApi
    ###modify for udp marketdata
    def CreateFtdcMdApi(self, const char *pszFlowPath = ""):
        self.basePtr = CSecurityFtdcMdApi.CreateFtdcMdApi(pszFlowPath)
        return self.basePtr

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
    def RegisterSpi(self, CSecurityFtdcMdSpi *pSpi):
        self.basePtr.RegisterSpi(pSpi)
        pass

    ###订阅行情。
    ###@param ppInstrumentID 合约ID
    ###@param nCount 要订阅/退订行情的合约个数
    ###@remark
    def SubscribeMarketData(self, char *ppInstrumentID[], int nCount, char* pExchageID):
        self.basePtr.SubscribeMarketData(ppInstrumentID, nCount, pExchageID)
        pass

    ###退订行情。
    ###@param ppInstrumentID 合约ID
    ###@param nCount 要订阅/退订行情的合约个数
    ###@remark
    def UnSubscribeMarketData(self, char *ppInstrumentID[], int nCount, char* pExchageID):
        self.basePtr.UnSubscribeMarketData(ppInstrumentID, nCount, pExchageID)
        pass

    ###用户登录请求
    def ReqUserLogin(self, CSecurityFtdcReqUserLoginField *pReqUserLoginField, int nRequestID):
        self.basePtr.ReqUserLogin(pReqUserLoginField, nRequestID)
        pass

    ###登出请求
    def ReqUserLogout(self, CSecurityFtdcUserLogoutField *pUserLogout, int nRequestID):
        self.basePtr.ReqUserLogout(pUserLogout, nRequestID)
        pass

cdef class pyCSecurityFtdcMdSpi:
    cdef CSecurityFtdcMdSpi *basePtr

    def __cinit__(self):
        if type(self) is pyCSecurityFtdcMdSpi:
            self.basePtr = new CSecurityFtdcMdSpi()

    ###当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    def OnFrontConnected(self):
        pass

    ###当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ###@param nReason 错误原因
    ###        0x1001 网络读失败
    ###        0x1002 网络写失败
    ###        0x2001 接收心跳超时
    ###        0x2002 发送心跳失败
    ###        0x2003 收到错误报文
    def OnFrontDisconnected(self, int nReason):
        pass

    ###心跳超时警告。当长时间未收到报文时，该方法被调用。
    ###@param nTimeLapse 距离上次接收报文的时间
    def OnHeartBeatWarning(self, int nTimeLapse):
        pass


    ###错误应答
    def OnRspError(self, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID,  bint bIsLast):
        pass

    ###登录请求响应
    def OnRspUserLogin(self, CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bint bIsLast):
        pass

    ###登出请求响应
    def OnRspUserLogout(self, CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bint bIsLast):
        pass

    ###订阅行情应答
    def OnRspSubMarketData(self, CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bint bIsLast):
        pass

    ###取消订阅行情应答
    def OnRspUnSubMarketData(self, CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bint bIsLast):
        pass

    ###深度行情通知
    def OnRtnDepthMarketData(self, CSecurityFtdcDepthMarketDataField *pDepthMarketData):
        pass

cdef class MdClass(CSecurityFtdcMdSpi):
    ## 构造函数带front_id, broker_id, user_id, password
        MdClass(char *front_address, char *broker_id, char *user_id, char *password, const char *pszFlowPath = "") except +

        ##登陆
        def Login(self):
            pass

        ##/当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
        def OnFrontConnected(self):
            pass

        ##/当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
        ##/@param nReason 错误原因
        ##/        0x1001 网络读失败
        ##/        0x1002 网络写失败
        ##/        0x2001 接收心跳超时
        ##/        0x2002 发送心跳失败
        ##/        0x2003 收到错误报文
        def OnFrontDisconnected(int nReason)

        ##/心跳超时警告。当长时间未收到报文时，该方法被调用。
        ##/@param nTimeLapse 距离上次接收报文的时间
        def OnHeartBeatWarning(int nTimeLapse)

         ##订阅行情
        def SubMarketData(list[string] list_instrumentid, char* pExchageID)

         ##取消订阅行情
        def UnSubMarketData(list[string] list_instrumentid, char* pExchageID)

        ##/错误应答
        def OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bint bIsLast)

        ##/登录请求响应
        def OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bint bIsLast)

        ##/登出请求响应
        def OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bint bIsLast)

        ##/订阅行情应答
        def OnRspSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bint bIsLast)

        ##/取消订阅行情应答
        def OnRspUnSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bint bIsLast)

        ##/深度行情通知
        def OnRtnDepthMarketData(CSecurityFtdcDepthMarketDataField *pDepthMarketData)

        ##返回数据是否报错
        int IsErrorRspInfo(CSecurityFtdcRspInfoField *pRspInfo)
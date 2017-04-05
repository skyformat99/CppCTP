# distutils: language=c++
from SecurityFtdcMdApi cimport *
from SecurityFtdcUserApiStruct cimport *
from libc.string cimport const_char
from libcpp cimport bool as cbool

cdef extern from "Python.h":
    ctypedef struct PyObject

cdef extern from "SecurityFtdcMdApi.h":
    cdef cppclass CMdApi "CSecurityFtdcMdApi":
        ###删除接口对象本身
        ###@remark 不再使用本接口对象时,调用该函数删除接口对象
        void Release() nogil

        ###初始化
        ###@remark 初始化运行环境,只有调用后,接口才开始工作
        void Init() nogil

        ###等待接口线程结束运行
        ###@return 线程退出代码
        int Join() nogil

        ###获取当前交易日
        ###@retrun 获取到的交易日
        ###@remark 只有登录成功后,才能得到正确的交易日
        const char *GetTradingDay() nogil

        ###注册前置机网络地址
        ###@param pszFrontAddress：前置机网络地址。
        ###@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:17001”。
        ###@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”17001”代表服务器端口号。
        void RegisterFront(char *pszFrontAddress) nogil

        ###注册回调接口
        ###@param pSpi 派生自回调接口类的实例
        void RegisterSpi(CMdSpi *pSpi) nogil

        ###订阅行情。
        ###@param ppInstrumentID 合约ID
        ###@param nCount 要订阅/退订行情的合约个数
        ###@remark
        int SubscribeMarketData(char *ppInstrumentID[], int nCount, char* pExchageID) nogil

        ###退订行情。
        ###@param ppInstrumentID 合约ID
        ###@param nCount 要订阅/退订行情的合约个数
        ###@remark
        int UnSubscribeMarketData(char *ppInstrumentID[], int nCount, char* pExchageID) nogil

        ###用户登录请求
        int ReqUserLogin(CSecurityFtdcReqUserLoginField *pReqUserLoginField, int nRequestID) nogil

        ###登出请求
        int ReqUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, int nRequestID) nogil

cdef extern from "SecurityFtdcMdApi.h" namespace "CSecurityFtdcMdApi":
    ###创建MdApi
    ###@param pszFlowPath 存贮订阅信息文件的目录，默认为当前目录
    ###@return 创建出的UserApi
    ###modify for udp marketdata
    CMdApi *CreateFtdcMdApi(const char *pszFlowPath)  nogil except +

cdef extern from "SecurityFtdcMdApiImpl.h":
    cdef cppclass CMdSpi "CSecurityFtdcMdSpi":
        CMdSpi(PyObject *obj)
        long tid
    void ReleaseMdApi(CMdApi *api, CMdSpi *spi)
    int CheckMemory(void *) except 0


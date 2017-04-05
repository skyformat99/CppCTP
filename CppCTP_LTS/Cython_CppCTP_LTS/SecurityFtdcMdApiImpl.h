#ifndef CMDAPI_H
#define CMDAPI_H

#include "Python.h"
#include "pythread.h"
#include "SecurityFtdcMdApi.h"

static inline int MdSpi_OnFrontConnected(PyObject *);
static inline int MdSpi_OnFrontDisconnected(PyObject *, int);
static inline int MdSpi_OnHeartBeatWarning(PyObject *, int);
static inline int MdSpi_OnRspError(PyObject *, CSecurityFtdcRspInfoField *, int, bool);
static inline int MdSpi_OnRspUserLogin(PyObject *, CSecurityFtdcRspUserLoginField *, CSecurityFtdcRspInfoField *, int, bool);
static inline int MdSpi_OnRspUserLogout(PyObject *, CSecurityFtdcUserLogoutField *, CSecurityFtdcRspInfoField *, int, bool);
static inline int MdSpi_OnRspSubMarketData(PyObject *, CSecurityFtdcSpecificInstrumentField *, CSecurityFtdcRspInfoField *, int, bool);
static inline int MdSpi_OnRspUnSubMarketData(PyObject *, CSecurityFtdcSpecificInstrumentField *, CSecurityFtdcRspInfoField *, int, bool);
static inline int MdSpi_OnRtnDepthMarketData(PyObject *, CSecurityFtdcDepthMarketDataField *);

#define X_CALL(op) \
	do { \
		PyGILState_STATE gilstate = PyGILState_Ensure(); \
		tid = PyThread_get_thread_ident(); \
		if ((op) == -1) PyErr_Print(); \
		PyGILState_Release(gilstate); \
	} while (0)

class CMdSpi : public CSecurityFtdcMdSpi {
public:
	long tid;

	CMdSpi(PyObject *obj):tid(0),self(obj) {}
	virtual ~CMdSpi() {}

	virtual void OnFrontConnected() {
		X_CALL(MdSpi_OnFrontConnected(self));
	}

	virtual void OnFrontDisconnected(int nReason) {
		X_CALL(MdSpi_OnFrontDisconnected(self, nReason));
	}

	virtual void OnHeartBeatWarning(int nTimeLapse) {
		X_CALL(MdSpi_OnHeartBeatWarning(self, nTimeLapse));
	}

	virtual void OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		X_CALL(MdSpi_OnRspError(self, pRspInfo, nRequestID, bIsLast));
	}

	virtual void OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		X_CALL(MdSpi_OnRspUserLogin(self, pRspUserLogin, pRspInfo, nRequestID, bIsLast));
	}

	virtual void OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		X_CALL(MdSpi_OnRspUserLogout(self, pUserLogout, pRspInfo, nRequestID, bIsLast));
	}

	virtual void OnRspSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		X_CALL(MdSpi_OnRspSubMarketData(self, pSpecificInstrument, pRspInfo, nRequestID, bIsLast));
	}

	virtual void OnRspUnSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		X_CALL(MdSpi_OnRspUnSubMarketData(self, pSpecificInstrument, pRspInfo, nRequestID, bIsLast));
	}

	virtual void OnRtnDepthMarketData(CSecurityFtdcDepthMarketDataField *pDepthMarketData) {
		X_CALL(MdSpi_OnRtnDepthMarketData(self, pDepthMarketData));
	}

private:
	PyObject *self;
};

static CSecurityFtdcMdApi *g_GcApi = NULL;
static long g_GcTid = 0;

static inline void ReleaseMdApi(CSecurityFtdcMdApi *api, CMdSpi *spi) {
	long tid = PyThread_get_thread_ident();
	if (g_GcApi && g_GcTid != tid) {
		g_GcApi->Release();
		g_GcApi = NULL;
	}
	if (api) {
		api->RegisterSpi(NULL);
		if (!spi) {
#ifndef _WIN32
			api->Init(); usleep(100000);
#endif
			api->Release();
		} else {
			if (spi->tid != tid) {
				api->Release();
			} else {
				g_GcApi = api;
				g_GcTid = tid;
			}
			delete spi;
		}
	}
}

#define CheckMemory(p) (p || PyErr_NoMemory())

#endif /* CMDAPI_H */
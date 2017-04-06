# -*- coding: utf-8 -*-

import hashlib, os, sys, tempfile, time
from SecurityFtdcMdApi import MdApi

class MyMdApi(MdApi):

    def OnFrontConnected(self):
        print('OnFrontConnected: Login...')
        pReqUserLogin = {}
        pReqUserLogin['BrokerID'] = b'2011'
        pReqUserLogin['UserID'] = b'010000040178'
        pReqUserLogin['Password'] = b'7174516'

        # pReqUserLogin = {}
        # pReqUserLogin['BrokerID'] = '2011'
        # pReqUserLogin['UserID'] = '010000040178'
        # pReqUserLogin['Password'] = '7174516'

        self.ReqUserLogin(pReqUserLogin, 1)

    def OnFrontDisconnected(self, nReason):
        print('OnFrontDisconnected:', nReason)

    def OnHeartBeatWarning(self, nTimeLapse):
        print('OnHeartBeatWarning:', nTimeLapse)

    def OnRspUserLogin(self, pRspUserLogin, pRspInfo, nRequestID, bIsLast):
        print('OnRspUserLogin:', pRspInfo)
        if pRspInfo['ErrorID'] == 0: # Success
            print('GetTradingDay:', self.GetTradingDay())
            # self.SubscribeMarketData(self.instrumentIDs)
        else:
            print('>>> MyMdApi OnRspUserLogin() ErrorID = ', pRspInfo['ErrorID'])

    def OnRspSubMarketData(self, pSpecificInstrument, pRspInfo, nRequestID, bIsLast):
        print('OnRspSubMarketData:', pRspInfo)

    def OnRspUnSubMarketData(self, pSpecificInstrument, pRspInfo, nRequestID, bIsLast):
        print('OnRspUnSubMarketData:', pRspInfo)

    def OnRspError(self, pRspInfo, nRequestID, bIsLast):
        print('OnRspError:', pRspInfo)

    def OnRspUserLogout(self, pUserLogout, pRspInfo, nRequestID, bIsLast):
        print('OnRspUserLogout:', pRspInfo)

    def OnRtnDepthMarketData(self, pDepthMarketData):
        print('OnRtnDepthMarketData:', pDepthMarketData)

if __name__ == '__main__':
    mdapi = MyMdApi()
    mdapi.Create(b'conn/md')
    mdapi.RegisterFront(b'tcp://180.166.216.228:24513')
    mdapi.Init()

    try:
        while 1:
            print("in while...")
            time.sleep(2)
    except KeyboardInterrupt:
        pass

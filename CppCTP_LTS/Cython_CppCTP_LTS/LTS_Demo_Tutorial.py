# -*- coding: utf-8 -*-

import hashlib, os, sys, tempfile, time
from SecurityFtdcMdApi import MdApi

class MyMdApi(MdApi):

    def OnFrontConnected(self):
        print('OnFrontConnected: Login...')

    def OnFrontDisconnected(self, nReason):
        print('OnFrontDisconnected:', nReason)

    def OnHeartBeatWarning(self, nTimeLapse):
        print('OnHeartBeatWarning:', nTimeLapse)

    def OnRspUserLogin(self, pRspUserLogin, pRspInfo, nRequestID, bIsLast):
        print('OnRspUserLogin:', pRspInfo)
        if pRspInfo['ErrorID'] == 0: # Success
            print('GetTradingDay:', self.GetTradingDay())
        else:
            print('>>> OnRspUserLogin() ErrorID = ', pRspInfo['ErrorID'])

    def OnRspSubMarketData(self, pSpecificInstrument, pRspInfo, nRequestID, bIsLast):
        print('OnRspSubMarketData:', pSpecificInstrument)
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
    #测试从此处开始
    mdapi = MyMdApi()
    # 创建流文件
    mdapi.Create(b'conn/md')
    # 注册前置地址
    mdapi.RegisterFront(b'tcp://180.166.216.228:24513')
    # 初始化
    mdapi.Init()
    time.sleep(1)

    # 登陆
    pReqUserLogin = {}
    pReqUserLogin['BrokerID'] = b'2011'
    pReqUserLogin['UserID'] = b'010000040178'
    pReqUserLogin['Password'] = b'7174516'
    mdapi.ReqUserLogin(pReqUserLogin, 1)
    time.sleep(1)

    # 订阅行情
    instrumentIDs = [b'000001']
    pExchangeID = b'SSE'
    mdapi.SubscribeMarketData(instrumentIDs, pExchangeID)

    try:
        while 1:
            # print("in while...")
            time.sleep(1)
    except KeyboardInterrupt:
        pass

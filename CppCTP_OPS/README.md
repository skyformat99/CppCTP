CppCTP
=========
上期信息技术CTP开发(C++实现)

开发环境
--------

    CentOS 6.5, gcc:v4.8.2, cmake:v2.8.12.2, lang:c++, MongoDB v3.2

开发顺序
--------

    CppCTP_API -> CppCTP_Integration

## 简单演示
* CppCTP与客户端交互

![show_demo](https://user-images.githubusercontent.com/3973297/27087027-80820c0e-5086-11e7-9cf6-0a13a6b0f037.gif)

* 客户端策略的增删改查

![strategy_crud](https://user-images.githubusercontent.com/3973297/27087656-2b05649a-5088-11e7-8a14-58dfc80cdd01.gif)
## 各个目录说明
* CppCTP_API
  * 本目录是针对CTP的API库进行的API测试，分为md(市场行情),td(交易),初学者最适宜从API版本学起，学习最基本的API调用
* CppCTP_Integration
  * xTrader
    * 该目录是CppCTP系统源码，支持多账户，多策略交易使用
  * xTrader_doc
    * 该目录是CppCTP系统开发中的一些协议文档相关的资料说明
* CppCTP_SOCKET
  * 本目录是CTP实现socket通信版本,是C++与Python的socket通信底层代码

## 致谢
CppCTP中使用了一些第三方开源库，非常感谢开源作者提供优秀的库，下面列举出该系统使用的开源库：
* log日志库:[spdlog](https://github.com/gabime/spdlog)
* 多线程安全队列:[concurrentqueue](https://github.com/cameron314/concurrentqueue)
* C++高性能json解析库:[rapidjson](https://github.com/miloyip/rapidjson)
* C++ ini文件解析库:[inih](https://github.com/jtilly/inih)

## 相关开发教程
整个CppCTP的开发周期还是比较漫长的，从接触API文档到后期策略级别上的开发，中间踩过不少坑。

后续我会把CppCTP交易系统的开发环境搭建以及CTP开发教程整理出来。有兴趣可以从main.cpp源码泛读代码。

个人博客地址：http://www.jyguagua.com


### 更新时间:2017.06.13

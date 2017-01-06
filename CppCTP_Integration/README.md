# CppCTP_Integration
实现CTP多账户交易,集成行情接口，交易接口

# 开发环境
    CentOS 6.5, gcc:v4.8.0, cmake:v2.8.12.2, lang:c++

## 编译方法
```c++
sh get.sh
```

##  执行
```c++
./bin/quant_ctp_XTrader_no_debug [port] [0|1]
```
其中port是端口
1对应的是simnow盘中模式
0对应的是simnow离线模式

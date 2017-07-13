    cmake_minimum_required(VERSION 2.8)
    #project name
    project(quant_ctp_xTrader)

	set(CMAKE_CXX_COMPILER "g++")

    #set compiler for c++ language
	#-fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections
	#(to solve double free)options  -fvisibility=hidden -g
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++0x -pthread -lrt -I. -D_GLIBCXX_USE_NANOSLEEP")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++0x -pthread -lrt -I. -D_GLIBCXX_USE_NANOSLEEP -Wall -Wshadow -Wextra -pedantic -O2")

    #source files(can add source files manually)
    #set(SOURCE_FILES main.cpp ThostFtdcMdApi.h ThostFtdcTraderApi.h ThostFtdcUserApiDataType.h ThostFtdcUserApiStruct.h MdSpi.cpp MdSpi.h TdSpi.cpp TdSpi.h CTP_Manager.cpp CTP_Manager.h Login.h Order.cpp Order.h User.cpp User.h Utils.cpp Utils.h DBManager.h DBManager.cpp Trader.h Trader.cpp FutureAccount.h FutureAccount.cpp Strategy.h Strategy.cpp Algorithm.h Algorithm.cpp)


	#source directory(add source files automatically)
	aux_source_directory(. SOURCE_FILES)
	#set exe folder
	set(EXECUTABLE_OUTPUT_PATH ./bin/)

    #set extern libraries
    set(LIBRARIES libsgitquotapi.so libsgittradeapi.so libmongoclient.so libboost_thread.so libboost_system.so libboost_regex.so)

    #add execute file
    add_executable(quant_ctp_XTrader ${SOURCE_FILES})

    #add link library
    target_link_libraries(quant_ctp_XTrader ${LIBRARIES})
    cmake_minimum_required(VERSION 2.8)
    #project name
    project(multi_thread_socket_demo)

	set(CMAKE_CXX_COMPILER "g++")

    #set compiler for c++ language
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -lrt -lpthread -D_GLIBCXX_USE_NANOSLEEP")

    #source files(can add source files manually)
    #set(SOURCE_FILES main.cpp ThostFtdcMdApi.h ThostFtdcTraderApi.h ThostFtdcUserApiDataType.h ThostFtdcUserApiStruct.h MdSpi.cpp MdSpi.h TdSpi.cpp TdSpi.h CTP_Manager.cpp CTP_Manager.h Login.h Order.cpp Order.h User.cpp User.h Utils.cpp Utils.h DBManager.h DBManager.cpp Trader.h Trader.cpp FutureAccount.h FutureAccount.cpp Strategy.h Strategy.cpp Algorithm.h Algorithm.cpp)

	#source directory(add source files automatically)
	aux_source_directory(. SOURCE_FILES)

	find_package(Threads)

	

    #set extern libraries
    #set(LIBRARIES libthostmduserapi.so libthosttraderapi.so libmongoclient.so libboost_thread.so libboost_system.so libboost_regex.so)

    #add execute file
    add_executable(multi-thread-socket-server ${SOURCE_FILES})

    #add link library
	target_link_libraries(multi-thread-socket-server ${CMAKE_THREAD_LIBS_INIT})

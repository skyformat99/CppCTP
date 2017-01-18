#ifndef QUANT_CTP_DEBUG_H
#define QUANT_CTP_DEBUG_H
#include <iostream>
#include <ratio>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <string>  // string
#include <cstring>

using namespace std;
using namespace std::chrono;
using std::string;

//#define DEBUG
#ifdef DEBUG
#define USER_PRINT(x) {time_t tt = system_clock::to_time_t(system_clock::now()); std::string nowt(std::ctime(&tt)); std::cerr << "[DEBUG] - " << nowt.substr(0, nowt.length() - 1) << ", " << __FILE__ << ", Line - " << __LINE__ << " - " << #x << " = " << x << std::endl;}
#else
#define USER_PRINT(x)
#endif

#endif
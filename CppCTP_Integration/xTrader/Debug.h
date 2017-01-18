#ifndef QUANT_CTP_DEBUG_H
#define QUANT_CTP_DEBUG_H
#include <iostream>
#include <ratio>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <string>  // string

using namespace std;
using namespace std::chrono;
using std::string;

//#define DEBUG
#ifdef DEBUG
#define USER_PRINT(x) {time_t tt = system_clock::to_time_t(system_clock::now()); std::cerr << "[DEBUG] - " << std::ctime(&tt) << "" << __FILE__ << ", Line - " << __LINE__ << " - " << " = " << x << std::endl;}
#else
#define USER_PRINT(x)
#endif

#endif
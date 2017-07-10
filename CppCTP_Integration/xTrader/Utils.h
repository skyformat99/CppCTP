#ifndef QUANT_CTP_UTILS_H
#define QUANT_CTP_UTILS_H
#include <string>
#include <sstream>

using std::string;

class Utils {

public:
	static int IsFolderExist(const char* path);
	static int IsFileExist(const char* path);
	static int CreateFolder(const char* path);
	static int Utf8ToGb2312(char *sOut, int iMaxOutLen, const char *sIn, int iInLen);
	static int Gb2312ToUtf8(char *sOut, int iMaxOutLen, const char *sIn, int iInLen);
	static int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y);
	static string getNowTimeNs();
	static string getNowTimeMs();
	static string getNowTime();
	static bool compareTradingDay(const char *compare_day, const char *today);
	static bool compareTradingDaySeconds(const char *compare_day, const char *today);
	static long long strtolonglong(string str);
	static string longlongtostr(long long num);
	static string getDate();
	static string getYMDDate();
	static string getYMDYesterdayDate();
	static void printGreenColor(string text);
	static void printRedColor(string text);
	static void printGreenColorWithKV(string key, string value);
	static void printRedColorWithKV(string key, string value);
	static void printGreenColorWithKV(string key, int value);
	static void printRedColorWithKV(string key, int value);
	static int match_instrumentid(string instrument_id);
};

#endif
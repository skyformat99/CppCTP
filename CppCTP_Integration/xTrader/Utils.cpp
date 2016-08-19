#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <iostream>  
#include <string>  
#include <errno.h>  
#include <iconv.h>
#include <cstring>
#include <errno.h>
#include "Utils.h"

//check folder exist
int Utils::IsFolderExist(const char* path) {
	DIR *dp;
	if ((dp = opendir(path)) == NULL) {
		return 0;
	}
	closedir(dp);
	return -1;
}

//check folder exist
int Utils::IsFileExist(const char* path) {
	return !access(path, F_OK);
}

/************************************************************************/
/* create folder                                                        */
/************************************************************************/
int Utils::CreateFolder(const char* path) {
	int i, len;

	len = strlen(path);
	char dir_path[len + 1];
	dir_path[len] = '\0';

	strncpy(dir_path, path, len);

	for (i = 0; i<len; i++) {
		if (dir_path[i] == '/' && i > 0) {
			dir_path[i] = '\0';
			if (access(dir_path, F_OK) < 0) {
				if (mkdir(dir_path, 0755) < 0) {
					printf("mkdir=%s:msg=%s\n", dir_path, strerror(errno));
					return -1;
				}
			}
			dir_path[i] = '/';
		}
	}

	return 0;
}

//iInLen的长度不包括\0，应该用strlen。返回值是处理后的sOut长度
int Utils::Utf8ToGb2312(char *sOut, int iMaxOutLen, const char *sIn, int iInLen) {
	char *pIn = (char *)sIn;
	char *pOut = sOut;
	size_t ret;
	size_t iLeftLen = iMaxOutLen;
	iconv_t cd;

	cd = iconv_open("gb2312", "utf-8");
	if (cd == (iconv_t)-1)
	{
		return -1;
	}
	size_t iSrcLen = iInLen;
	ret = iconv(cd, &pIn, &iSrcLen, &pOut, &iLeftLen);
	if (ret == (size_t)-1)
	{
		iconv_close(cd);
		return -1;
	}

	iconv_close(cd);

	return (iMaxOutLen - iLeftLen);
}

//iInLen的长度不包括\0，应该用strlen。返回值是处理后的sOut长度
int Utils::Gb2312ToUtf8(char *sOut, int iMaxOutLen, const char *sIn, int iInLen) {
	char *pIn = (char *)sIn;
	char *pOut = sOut;
	size_t ret;
	size_t iLeftLen = iMaxOutLen;
	iconv_t cd;

	cd = iconv_open("utf-8", "gb2312");
	if (cd == (iconv_t)-1) {
		return -1;
	}
	size_t iSrcLen = iInLen;
	ret = iconv(cd, &pIn, &iSrcLen, &pOut, &iLeftLen);
	if (ret == (size_t)-1) {
		iconv_close(cd);
		return -1;
	}

	iconv_close(cd);

	return (iMaxOutLen - iLeftLen);
}

int Utils::timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y) {
	int nsec;

	if ((x->tv_sec) > y->tv_sec)
		return -1;

	if ((x->tv_sec == y->tv_sec) && (x->tv_usec > y->tv_usec))
		return -1;

	result->tv_sec = (y->tv_sec - x->tv_sec);
	result->tv_usec = (y->tv_usec - x->tv_usec);

	if (result->tv_usec < 0) {
		result->tv_sec--;
		result->tv_usec += 1000000;
	}

	return 0;
}

string Utils::getNowTimeNs() {
	printf("---------------------------struct timespec---------------------------------------\n");
	printf("[time(NULL)]     :     %ld\n", time(NULL));
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	printf("clock_gettime : tv_sec=%ld, tv_nsec=%ld\n", ts.tv_sec, ts.tv_nsec);

	struct tm t;
	char date_time[64];
	strftime(date_time, sizeof(date_time), "%Y-%m-%d %H:%M:%S", localtime_r(&ts.tv_sec, &t));
	printf("clock_gettime : date_time=%s, tv_nsec=%ld, tv_usec=%ld, compare_str=%s\n", date_time, ts.tv_nsec, ts.tv_nsec / 1000, (":"
		+ std::to_string(ts.tv_nsec)).c_str());
	string s = ":";
	return (date_time + s + std::to_string(ts.tv_nsec));
}

string Utils::getNowTimeMs() {
	printf("---------------------------struct timeval----------------------------------------\n");
	printf("[time(NULL)]    :    %ld\n", time(NULL));
	struct timeval us;
	gettimeofday(&us, NULL);
	printf("gettimeofday: tv_sec=%ld, tv_usec=%ld\n", us.tv_sec, us.tv_usec);

	struct tm t;
	char date_time[64];
	strftime(date_time, sizeof(date_time), "%Y-%m-%d %H:%M:%S", localtime_r(&us.tv_sec, &t));
	printf("gettimeofday: date_time=%s, tv_usec=%ld, tv_msec=%ld\n", date_time, us.tv_usec, us.tv_usec / 1000);
	string s = "_";
	return (s + date_time + s + std::to_string(us.tv_usec));
}
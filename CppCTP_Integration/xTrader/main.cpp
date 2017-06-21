#include <iostream>
#include <string>
#include <list>
#include <mongo/client/dbclient.h>
#include <stdio.h>
/*socket头文件*/
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include "ThostFtdcTraderApi.h"
#include "TdSpi.h"
#include "CTP_Manager.h"
#include "Utils.h"
#include "Debug.h"
#include "DBManager.h"
#include "Trader.h"
#include "FutureAccount.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "msg.h"
#include "Timer.h"

using std::cout;
using std::cin;
using namespace rapidjson;
using namespace spdlog;
using spdlog::logger;

/*宏定义*/
#define MAXCONNECTIONS 100
#define MAX_BUFFER_SIZE	2*1024

int sockfd;
CTP_Manager *ctp_m = NULL;
std::shared_ptr<spdlog::logger> xts_logger = NULL;

string one_min_time = "14:50:00";
string one_sec_time = "14:58:00";
string stop_trading_time = "14:59:55";
string close_time = "15:00:00";
string stopsave_time = "15:00:02";
string afternoon_exit_time = "15:40:00";
string stop_start_from_today_position_time = "20:00:00";


string morning_open_time = "08:59:55";						// 早上开盘时间
string moring_break_time = "10:14:55";						// 中午休盘
string morning_continue_time = "10:29:55";					// 中午休盘结束时间
string morning_close_time = "11:29:55";						// 中午收盘
string afternoon_open_time = "13:29:55";					// 下午开盘
string afternoon_close_time = "14:59:50";					// 下午收盘(停止新的发单任务)
string afternoon_end_task_time_one_time = "14:59:55";		// 下午收盘(把挂单列表全部成交)
string afternoon_end_task_time_second_time = "14:59:56";	// 下午收盘(把挂单列表全部成交)
string afternoon_end_task_time_third_time = "14:59:58";		// 下午收盘(把挂单列表全部成交)
string night_open_time = "20:59:55";						// 夜盘开始
string night_day_time = "00:00:00";							// 凌晨12点
string night_start_close_time = "02:44:55";					// 夜盘准备收盘,停止新的交易任务
string night_close_time = "02:45:00";						// 夜盘收盘时间
string night_stop_save_time = "02:45:02";					// 夜盘存储数据时间

/* ctrl + c 信号处理 */
void sig_handler(int signo) {

	if (signo == SIGINT) {

		if (ctp_m) {

			bool isTrading = false;

			// 遍历策略列表，看是否还有正在交易中的策略，如果是不能关闭
			list<Strategy *>::iterator stg_itor;
			for (stg_itor = ctp_m->getListStrategy()->begin();
				stg_itor != ctp_m->getListStrategy()->end(); stg_itor++) { // 遍历ctp_m维护的Strategy List
				if ((*stg_itor)->isStgTradeTasking()) {
					std::cout << "\t\033[31m策略(" << (*stg_itor)->getStgUserId() << ", " << (*stg_itor)->getStgStrategyId() << ") 处于交易状态\033[0m" << std::endl;
					/// 一旦有策略仍然在交易中,跳出循环
					isTrading = true;
					break;
				}
			}

			if (!isTrading) { // 如果策略全部处于非交易状态,则可以进行关闭
				if (!ctp_m->getCTPFinishedPositionInit()) { // ctp_m未初始化成功，则不做保存工作
					Utils::printRedColor("服务端 未 完成初始化!!!");
				}
				else {
					Utils::printGreenColor("服务端正常关闭，开始保存策略持仓明细.");
					ctp_m->saveAllStrategyPositionDetail();

				}
				/// 正常关闭,更新标志位
				ctp_m->updateSystemFlag();
				/// 关闭所有的log
				spdlog::drop_all();
				close(sockfd);
				exit(1);
			}
			else {
				Utils::printRedColor("服务端 有 策略处于交易状态,稍后再试!!!");
			}
		}
		else {
			Utils::printRedColor("服务端 未 正常关闭!!!");
			close(sockfd);
			exit(1);
		}
	}
}

/*输出连接上来的客户端的相关信息*/
void out_addr(struct sockaddr_in *clientaddr) {
	//将端口从网络字节序转换成主机字节序
	int port = ntohs(clientaddr->sin_port);
	char ip[16];
	memset(ip, 0, sizeof(ip));
	//将ip地址从网络字节序转换成点分十进制
	inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr, ip, sizeof(ip));
	printf("客户端IP:%s 端口:%d 已连接\n", ip, port);
}

/*输出服务器端时间*/
void do_service(int fd) {
	printf("main.cpp do_service()");
	/*和客户端进行读写操作(双向通信)*/
	char buff[MAX_BUFFER_SIZE];
	while (1)
	{
		memset(buff, 0, sizeof(buff));

		size_t size;
		//printf("sizeof buff is %d = \n", sizeof(buff));
		if ((size = read_msg(fd, buff, sizeof(buff))) < 0) {
			printf("protocal error");
			break;
		}
		else if (size == 0) {
			break;
		}
		else {
			//printf("服务端收到 = %s\n", buff);
			CTP_Manager::HandleMessage(fd, buff, ctp_m);
			//printf("socket_server send size = %d \n", strlen(buff));
			//printf("socket_server fd = %d \n", fd);
			//printf("socket_server send size = %d \n", sizeof(buff));
			//printf("socket_server send size = %d \n", strlen(buff));
			//if (write_msg(fd, buff, sizeof(buff)) < 0) {
			//	printf("先前客户端已断开!!!\n");
			//	//printf("errorno = %d, 先前客户端已断开!!!\n", errno);
			//	if (errno == EPIPE) {
			//		break;
			//	}
			//	perror("protocal error");
			//}
		}
	}
}

/*输出文件描述符*/
void out_fd(int fd) {
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	//从fd中获取连接的客户端相关信息
	if (getpeername(fd, (struct sockaddr *)&addr, &len) < 0) {
		perror("getpeername error");
		return;
	}
	char ip[16];
	memset(ip, 0, sizeof(ip));
	int port = ntohs(addr.sin_port);
	inet_ntop(AF_INET, &addr.sin_addr.s_addr, ip, sizeof(ip));
	printf("客户端IP:%16s 端口:%5d 已连接\n", ip, port);
}

/*线程调用*/
void *th_fn(void *arg) {
	int fd = *(reinterpret_cast<int*>(arg));
	out_fd(fd);
	do_service(fd);
	close(fd);
	return (void *)0;
}

void timer_handler() {

	/*time_t tt = system_clock::to_time_t(system_clock::now()); 
	std::string nowt(std::ctime(&tt));*/
	//std::cout << "main.cpp timer_handler()" << std::endl;
	//std::cout << "\t定时器输出" << std::endl;
	//std::cout << "\t现在时间:" << nowt.substr(0, nowt.length() - 1) << std::endl;


	/************************************************************************/
	/*	string one_min_time = "14：50：00";
		string one_sec_time = "14：58：00";
		string stop_trading_time = "14：59：55";
		string close_time = "15：00：00";
		时间在14:49:00之前，按照10分钟一次计时
		一旦时间超过14：50：00，我就按照一分钟计时一次；
		一旦时间超过14：59：00，我就按照1秒计时一次；
		一旦时间超过14：59：55，不开始新的交易任务。
		*/
	/************************************************************************/
	/************************************************************************/
	/*	string morning_open_time = "08:59:55"; // 早上开盘时间
		string moring_break_time = "10:14:55"; // 中午休盘
		string morning_continue_time = "10:29:55"; // 中午休盘结束时间
		string morning_close_time =	"11:29:55"; // 中午收盘
		string afternoon_open_time = "13:29:55"; // 下午开盘
		string afternoon_close_time = "14:59:55"; // 下午收盘
		string night_open_time = "20:59:55"; // 夜盘开始
		string night_close_time = "22:59:55"; //夜盘收盘                                                                    */
	/************************************************************************/

	list<Strategy *>::iterator itor;

	// 15:00:00是否需要执行收盘工作
	bool is_need_save_data_afternoon = false;
	bool is_need_to_exit = false;
	// 是否需要停止计时器
	bool is_need_to_stop_timer = false;
	string nowtime = Utils::getNowTime();
	string ymd_date = Utils::getYMDDate();
	//std::cout << "现在时间 = " << nowtime << std::endl;
	// 当有其他地方调用策略列表,阻塞,信号量P操作
	sem_wait((ctp_m->getSem_strategy_handler()));
	if (ctp_m->getCalTimer()->running()) //定时器在运行中
	{
		for (itor = ctp_m->getListStrategy()->begin(); itor != ctp_m->getListStrategy()->end(); itor++) {
			
			if ((*itor) != NULL) // 策略不为空
			{
				// 时间大于早上开盘时间
				if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getMorning_opentime()).c_str())) {
					
					//Utils::printGreenColorWithKV("getMorning_opentime", (ymd_date + (*itor)->getMorning_opentime()));

					(*itor)->setIsMarketCloseFlag(false);

					//时间大于中午休盘
					if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getMorning_begin_breaktime()).c_str()))
					{
						//Utils::printGreenColorWithKV("getMorning_begin_breaktime", (ymd_date + (*itor)->getMorning_begin_breaktime()));
						if ((*itor)->getHasMorningBreakTime())
						{
							(*itor)->setIsMarketCloseFlag(true);
						}
						

						//时间大于中午休盘结束时间
						if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getMorning_recoverytime()).c_str()))
						{

							//Utils::printGreenColorWithKV("getMorning_recoverytime", (ymd_date + (*itor)->getMorning_recoverytime()));

							if ((*itor)->getHasMorningBreakTime())
							{
								(*itor)->setIsMarketCloseFlag(true);
							}

							//时间大于中午收盘
							if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getMorning_begin_closetime()).c_str()))
							{

								//Utils::printGreenColorWithKV("getMorning_begin_closetime", (ymd_date + (*itor)->getMorning_begin_closetime()));

								(*itor)->setIsMarketCloseFlag(true);

								// 时间大于13:29:55
								if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getAfternoon_opentime()).c_str()))
								{

									//Utils::printGreenColorWithKV("getAfternoon_opentime", (ymd_date + (*itor)->getAfternoon_opentime()));

									(*itor)->setIsMarketCloseFlag(false);

									// 时间大于14:59:50
									if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getAfternoon_begin_closetime()).c_str()))
									{
										//Utils::printGreenColorWithKV("getAfternoon_begin_closetime", (ymd_date + (*itor)->getAfternoon_begin_closetime()));

										(*itor)->setIsMarketCloseFlag(true);

										// 时间大于14:59:55
										if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + afternoon_end_task_time_one_time).c_str())) {


											//Utils::printGreenColorWithKV("afternoon_end_task_time_one_time", (ymd_date + afternoon_end_task_time_one_time));

											(*itor)->setIsMarketCloseFlag(true);
											if ((*itor)->getEnd_task_afternoon_first())
											{
												(*itor)->setEnd_task_afternoon_first(false);
												(*itor)->setStgOnOffEndTask(true);
											}
											

											// 时间大于14:59:56
											if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + afternoon_end_task_time_second_time).c_str()))
											{

												//Utils::printGreenColorWithKV("afternoon_end_task_time_second_time", (ymd_date + afternoon_end_task_time_second_time));

												(*itor)->setIsMarketCloseFlag(true);

												// 时间大于14:59:58
												if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + afternoon_end_task_time_third_time).c_str()))
												{

													//Utils::printGreenColorWithKV("afternoon_end_task_time_third_time", (ymd_date + afternoon_end_task_time_third_time));

													(*itor)->setIsMarketCloseFlag(true);

													if ((*itor)->getEnd_task_afternoon_second())
													{
														(*itor)->setEnd_task_afternoon_second(false);
														(*itor)->setStgOnOffEndTask(true);
													}

													// 时间大于15:00:00
													if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getAfternoon_closetime()).c_str())) { // 时间大于15:00:00
														

														//Utils::printGreenColorWithKV("getAfternoon_closetime", (ymd_date + (*itor)->getAfternoon_closetime()));

														(*itor)->setIsMarketCloseFlag(true);
														// 结束任务执行false
														(*itor)->setStgOnOffEndTask(false);

														// 时间大于15:00:02
														if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + stopsave_time).c_str())) { // 时间大于15:00:00小于15:00:02
															

															//Utils::printGreenColorWithKV("stopsave_time", (ymd_date + stopsave_time));

															(*itor)->setIsMarketCloseFlag(true);
															// 结束任务执行false
															(*itor)->setStgOnOffEndTask(false);
															is_need_save_data_afternoon = true;

															// 时间大于15:00:02
															if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + afternoon_exit_time).c_str())) {
															

																//Utils::printGreenColorWithKV("afternoon_exit_time", (ymd_date + afternoon_exit_time));

																(*itor)->setIsMarketCloseFlag(true);
																// 结束任务执行false
																(*itor)->setStgOnOffEndTask(false);
																is_need_save_data_afternoon = false;
																is_need_to_exit = true;

																// 时间大于20:00:00
																if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + stop_start_from_today_position_time).c_str())) // 时间大于15:00:02小于18:00:00
																{
																	//Utils::printGreenColorWithKV("stop_start_from_today_position_time", (ymd_date + stop_start_from_today_position_time));

																	is_need_to_exit = false;
																	is_need_save_data_afternoon = false;

																	// 有夜盘的继续维护标志位
																	if ((*itor)->getHasNightMarket() && (!(*itor)->getHasMidnightMarket()))
																	{
																		(*itor)->setIsMarketCloseFlag(true);

																		//时间大于20:59:55
																		if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getEvening_opentime()).c_str())) {

																			//Utils::printGreenColorWithKV("getEvening_opentime", (ymd_date + (*itor)->getEvening_opentime()));

																			(*itor)->setIsMarketCloseFlag(false);

																			// 时间大于夜间停止新任务时间(提前10s)
																			if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getEvening_stop_opentime()).c_str())) {

																				//Utils::printGreenColorWithKV("getEvening_stop_opentime", (ymd_date + (*itor)->getEvening_stop_opentime()));

																				// 停止新任务
																				(*itor)->setIsMarketCloseFlag(true);

																				// 时间大于夜间第一次结束任务(提前5s)
																				if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getEvening_first_end_tasktime()).c_str())) {

																					//Utils::printGreenColorWithKV("getEvening_first_end_tasktime", (ymd_date + (*itor)->getEvening_first_end_tasktime()));

																					// 停止新任务
																					(*itor)->setIsMarketCloseFlag(true);


																					if ((*itor)->getEnd_task_evening_first())
																					{
																						(*itor)->setEnd_task_evening_first(false);
																						// 结束任务执行1
																						(*itor)->setStgOnOffEndTask(true);
																					}

																					

																					// 时间大于夜间第二次结束任务(提前3s)
																					if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getEvening_second_end_tasktime()).c_str())) {

																						//Utils::printGreenColorWithKV("getEvening_second_end_tasktime", (ymd_date + (*itor)->getEvening_second_end_tasktime()));

																						// 停止新任务
																						(*itor)->setIsMarketCloseFlag(true);

																						if ((*itor)->getEnd_task_evening_second())
																						{
																							(*itor)->setEnd_task_evening_second(false);
																							// 结束任务执行2
																							(*itor)->setStgOnOffEndTask(true);
																						}

																						// 时间大于夜间收盘时间
																						if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getEvening_closetime()).c_str())) { // 时间大于20:59:55

																							//Utils::printGreenColorWithKV("getEvening_closetime", (ymd_date + (*itor)->getEvening_closetime()));

																							(*itor)->setIsMarketCloseFlag(true);
																							// 结束任务执行false
																							(*itor)->setStgOnOffEndTask(false);

																							is_need_save_data_afternoon = true;
																						}
																					}
																				}
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
				else {
					// 如果有夜盘
					if ((*itor)->getHasNightMarket() && (*itor)->getHasMidnightMarket())
					{
						// 时间大于00:00:00
						if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + night_day_time).c_str()))
						{
							//Utils::printGreenColorWithKV("night_day_time", (ymd_date + night_day_time));

							is_need_save_data_afternoon = false;

							// 停止新任务
							(*itor)->setIsMarketCloseFlag(false);

							//时间大于夜间准备收盘时间
							if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getEvening_stop_opentime()).c_str())) {

								//Utils::printGreenColorWithKV("getEvening_stop_opentime", (ymd_date + (*itor)->getEvening_stop_opentime()));

								// 停止新任务
								(*itor)->setIsMarketCloseFlag(true);

								//时间大于夜间第一次结束任务
								if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getEvening_first_end_tasktime()).c_str())) {
									
									//Utils::printGreenColorWithKV("getEvening_first_end_tasktime", (ymd_date + (*itor)->getEvening_first_end_tasktime()));

									// 停止新任务
									(*itor)->setIsMarketCloseFlag(true);


									if ((*itor)->getEnd_task_morning_first())
									{
										(*itor)->setEnd_task_morning_first(false);
										// 结束任务执行1
										(*itor)->setStgOnOffEndTask(true);
									}

									if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getEvening_second_end_tasktime()).c_str())) {

										//Utils::printGreenColorWithKV("getEvening_second_end_tasktime", (ymd_date + (*itor)->getEvening_second_end_tasktime()));

										// 停止新任务
										(*itor)->setIsMarketCloseFlag(true);

										if ((*itor)->getEnd_task_morning_second())
										{
											(*itor)->setEnd_task_morning_second(false);
											// 结束任务执行2
											(*itor)->setStgOnOffEndTask(true);
										}

										//时间大于夜间收盘时间
										if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + (*itor)->getEvening_closetime()).c_str())) {

											//Utils::printGreenColorWithKV("getEvening_closetime", (ymd_date + (*itor)->getEvening_closetime()));

											(*itor)->setIsMarketCloseFlag(true);
											// 结束任务执行false
											(*itor)->setStgOnOffEndTask(false);

											// 时间大于02:44:55
											if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + night_start_close_time).c_str()))
											{

												//Utils::printGreenColorWithKV("night_start_close_time", (ymd_date + night_start_close_time));

												is_need_save_data_afternoon = false;
												// 停止新任务
												(*itor)->setIsMarketCloseFlag(true);

												// 时间大于02:45:00
												if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + night_close_time).c_str()))
												{
													//Utils::printGreenColorWithKV("night_close_time", (ymd_date + night_close_time));

													is_need_save_data_afternoon = true;
													// 停止新任务
													(*itor)->setIsMarketCloseFlag(true);

													// 时间大于02:45:02
													if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + night_stop_save_time).c_str()))
													{
														//Utils::printGreenColorWithKV("night_stop_save_time", (ymd_date + night_stop_save_time));

														is_need_save_data_afternoon = false;
														// 停止新任务
														(*itor)->setIsMarketCloseFlag(true);
														is_need_to_exit = true;
													}
												}
											}
										}
									}	
								}
							}
						}
					}	
				}
			} 
			else
			{
				Utils::printRedColor("timer_handler() (*itor) is NULL!");
			}
		}

		
	}
	// 释放信号量,信号量V操作
	sem_post((ctp_m->getSem_strategy_handler()));

	//if (ctp_m->getIsMarketClose() != market_close_flag)
	//{
	//	//Utils::printGreenColorWithKV("timer_handler() 现在时间:", nowtime);
	//	ctp_m->setIsMarketClose(market_close_flag);
	//}

	//// 是否需要停止定时器
	//if (is_need_to_stop_timer)
	//{
	//	//Utils::printGreenColorWithKV("现在时间", nowtime);
	//	ctp_m->getCalTimer()->stop();
	//	Utils::printGreenColor("已关闭定时器!");
	//}

	//Utils::printGreenColorWithKV("现在时间", nowtime);

	// 是否需要保存数据
	if (is_need_save_data_afternoon)
	{
		if (!ctp_m->getIsClosingSaved())
		{
			Utils::printGreenColorWithKV("现在时间", nowtime);
			Utils::printGreenColor("收盘工作:保存策略参数,更新运行状态.");

			// 保存最后策略参数,更新运行状态正常收盘
			ctp_m->saveAllStrategyPositionDetail();
			ctp_m->updateSystemFlag();

			// 保存策略参数,关闭定时器
			// ctp_m->getCalTimer()->stop();
			// 关闭所有的log
			spdlog::drop_all();
			Utils::printGreenColor("系统收盘工作正常结束.");

			ctp_m->setIsClosingSaved(true);
		}
	}

	if (is_need_to_exit)
	{
		Utils::printGreenColorWithKV("现在时间", nowtime);
		Utils::printGreenColor("正常退出系统.");
		exit(1);
	}


}



int main(int argc, char *argv[]) {

	if (argc < 3) {
		printf("usage: %s port mode\n", argv[0]);
		printf("port: 0~65536\n");
		printf("mode: 1:online 0:offline\n");
		exit(1);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR) { // 按下crtl+c键后的处理
		perror("signal sigint error");
		exit(1);
	}

	signal(SIGPIPE, SIG_IGN);

	// 初始化mongoDB
	mongo::client::initialize();

	// 初始化CTP_Manager
	ctp_m = new CTP_Manager();
	
	bool init_flag = true;

	if (!strcmp("1", argv[2])) {
		init_flag = true;
		//printf("盘中模式... \n");
	}
	else if (!strcmp("0", argv[2])) {
		init_flag = false;
		//printf("离线模式... \n");
	}
	else {
		printf("usage: %s #port #mode\n", argv[0]);
		printf("port: 0~65536\n");
		printf("mode: 1:online 0:offline\n");
		spdlog::drop_all();
		exit(1);
	}

	// 多线程定时器
	/*Timer tHello([]()
	{
		std::cout << "main()" << std::endl;
		std::cout << "\t系统定时器tick" << std::endl;
	});*/

	Timer tHello(timer_handler);

	//CTPManager设置定时器
	ctp_m->setCalTimer(&tHello);


	//开启定时器
	tHello.setSingleShot(false);
	//tHello.setInterval(Timer::Interval(1000 * 60 * 10));
	tHello.setInterval(Timer::Interval(1000));
	

	// 程序入口，初始化资源
	if (!ctp_m->init(init_flag)) {
		std::cout << "系统初始化失败!请检查日志!" << std::endl;
		spdlog::drop_all();
		exit(1);
	}

	tHello.start(true);

	/*while (true)
	{
	std::cout << "In Thread = " << std::this_thread::get_id() << std::endl;
	sleep(5);
	}*/

	/*算法单元测试
	Algorithm *alg = new Algorithm();
	alg->setAlgName("01");
	ctp_m->getDBManager()->CreateAlgorithm(alg);
	alg->setIsActive("1");
	ctp_m->getDBManager()->UpdateAlgorithm(alg);
	list<Algorithm *> l_alg;
	ctp_m->getDBManager()->getAllAlgorithm(&l_alg);
	list<Algorithm *>::iterator alg_itor;
	for (alg_itor = l_alg.begin(); alg_itor != l_alg.end(); alg_itor++) {

	cout << (*alg_itor)->getAlgName() << endl;
	}*/


	/*步骤1:创建socket(套接字)*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/*socket选项设置*/
	// a：设置套接字的属性使它能够在计算机重启的时候可以再次使用套接字的端口和IP
	int err, sock_reuse = 1;
	err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_reuse, sizeof(sock_reuse));
	if (err != 0) {
		printf("SO_REUSEADDR Setting Failed!\n");
		spdlog::drop_all();
		exit(1);
	}
	// b：设置接收缓冲区大小
	int nRecvBuf = MAX_BUFFER_SIZE; //数据最大长度
	err = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	if (err != 0) {
		printf("SO_RCVBUF Setting Failed!\n");
		spdlog::drop_all();
		exit(1);
	}
	// c：设置发送缓冲区大小
	int nSendBuf = MAX_BUFFER_SIZE; //数据最大长度
	err = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	if (err != 0) {
		printf("SO_SNDBUF Setting Failed!\n");
		spdlog::drop_all();
		exit(1);
	}

	/*步骤2:将socket和地址(包括ip,port)进行绑定*/
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	/*向地址中填入ip,port,internet地址簇类型*/
	serveraddr.sin_family = AF_INET; //ipv4
	serveraddr.sin_port = htons(atoi(argv[1])); //port
	serveraddr.sin_addr.s_addr = INADDR_ANY; //接收所有网卡地址
	if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		perror("bind error");
		spdlog::drop_all();
		exit(1);
	}

	/*步骤3:调用listen函数启动监听(指定port监听)
	通知系统去接受来自客户端的连接请求
	第二个参数:指定队列的长度*/
	if (listen(sockfd, MAXCONNECTIONS) < 0) {
		perror("listen error");
		spdlog::drop_all();
		exit(1);
	}

	/*步骤4:调用accept函数从队列中获得一个客户端的连接请求，
	并返回新的socket描述符*/
	struct sockaddr_in clientaddr;
	socklen_t clientaddr_len = sizeof(clientaddr);

	/*设置线程的分离属性*/
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	Utils::printGreenColor("Trade Server已就绪,等待连接...");

	while (1)
	{
		int fd = accept(sockfd, NULL, NULL);
		if (fd < 0) {
			perror("accept error");
			continue;
		}
		/*步骤5:启动子线程去调用IO函数(read/write)和连接的客户端进行双向通信*/
		pthread_t th;
		int err;
		/*以分离状态启动子线程*/
		if ((err = pthread_create(&th, &attr, th_fn, &fd)) != 0) {
			perror("pthread create error");
		}
		pthread_attr_destroy(&attr);
	}

	return 0;
}
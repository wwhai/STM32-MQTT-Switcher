/*
 * sim900a.h
 *
 *  Created on: 2017年5月7日
 *      Author: xianlee
 */

#ifndef SIM900A_SIM900A_H_
#define SIM900A_SIM900A_H_

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdarg.h"
#include "stdbool.h"
#include "usart.h"

#define BEARER_PROFILE_GPRS "AT+SAPBR=3,1,\"Contype\",\"GPRS\""
#define BEARER_PROFILE_APN "AT+SAPBR=3,1,\"APN\",\"%s\""
#define QUERY_BEARER "AT+SAPBR=2,1"
#define OPEN_GPRS_CONTEXT "AT+SAPBR=1,1"
#define CLOSE_GPRS_CONTEXT "AT+SAPBR=0,1"
#define HTTP_INIT "AT+HTTPINIT"
#define HTTP_CID "AT+HTTPPARA=\"CID\",1"
#define HTTP_PARA "AT+HTTPPARA=\"URL\",\"%s\""
#define HTTP_USER_DATA "AT+HTTPPARA=\"USERDATA\",\"%s\""
#define HTTP_GET "AT+HTTPACTION=0"
#define HTTP_POST "AT+HTTPACTION=1"
#define HTTP_DATA "AT+HTTPDATA=%d,%d"
#define HTTP_READ "AT+HTTPREAD"
#define HTTP_CLOSE "AT+HTTPTERM"
#define HTTP_CONTENT "AT+HTTPPARA=\"CONTENT\",\"application/json\""
#define HTTPS_ENABLE "AT+HTTPSSL=1"
#define HTTPS_DISABLE "AT+HTTPSSL=0"
#define NORMAL_MODE "AT+CFUN=1,1"
#define REGISTRATION_STATUS "AT+CREG?"
#define SIGNAL_QUALITY "AT+CSQ"
#define READ_VOLTAGE "AT+CBC"
#define SLEEP_MODE "AT+CSCLK=1"

#define OK "OK"
#define DOWNLOAD "DOWNLOAD"
#define HTTP_200 ",200,"
#define HTTPS_PREFIX "https://"
#define CONNECTED "+CREG: 0,1"
#define BEARER_OPEN "+SAPBR: 1,1"

#define GSM_DEVICE_SIM800								/* 使用的sim800系列开启此宏 */
#define GSM_DATA_RECORD_MAX_LEN		(4*1024)			/* GSM模块接收缓冲区长度 	*/
#define GSM_IMEI_LEN				(15 + 1)			/* IMEI长度 				*/
#define GSM_CMD_WAIT_NONE			(0)					/* 发送指令不等待结果 		*/
#define GSM_CMD_WAIT_SHORT			(100)				/* 等待100ms 				*/
#define GSM_CMD_WAIT_NORMAL			(500)				/* 等待500ms 				*/
#define GSM_CMD_WAIT_LONG			(2000)				/* 等待2s 					*/
#define GSM_UART					(USART3)			/* GSM模块连接在串口2上     */

#define	sim900a_usart(fmt, ... )	usart_printf(GSM_UART, fmt, ##__VA_ARGS__)
#define sim900a_send_byte(byte)		USART_SendData(GSM_UART, byte)

typedef struct _gsm_data_record {
	char frame_buf[GSM_DATA_RECORD_MAX_LEN];
	int frame_len;
	char finished;
} gsm_data_record;

extern gsm_data_record gsm_global_data;
/* 发送命令并等待返回 */
bool sim900a_cmd_with_reply(const char * cmd, const char * reply1,
		const char * reply2, uint32_t waittime);
/* 进行网络连接 */
int sim900a_tcpudp_connect(uint8_t mode, const char* ipaddr, const char* port);

int sim900a_get_imei(char* imei);

int sim900a_net_init(void);
/* 关闭网络连接 */
int sim900a_close_net(void);
/* IO口初始化 */
void sim900a_gpio_init(void);
/* 检查模块是否已经开启 */
int sim900a_check_status(void);
/* 模块开机 */
void sim900a_poweron(void);
/* 模块关机 */
void sim900a_poweroff(void);
/* 网络发送数据 */
int sim900a_net_send(char* data);
/* HTTP 初始化 */
int sim900a_http_init(void);
/* 执行http get请求 */
int sim900a_http_get(char* url);
/* 读取HTTP返回数据 */
int sim900a_http_read(int start, int size, char** data);

int sim900a_send_raw(uint8_t* data, uint16_t len);

void sim900a_user_config(void);

int connect_update_server(const char* ip, const char* port, int timeout);

int sim900a_read_tcp(uint8_t** data, uint16_t len);

int read_from_sim900a(void);

int sim900a_http_session_set(const char *uri);

int sim900a_http_param_set(const char* params);

int sim900a_http_post(const char *uri, const char *body, int len, char *resp);

#endif /* SIM900A_SIM900A_H_ */

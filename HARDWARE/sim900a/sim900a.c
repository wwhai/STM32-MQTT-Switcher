/*
 * sim900a.c
 *
 *  Created on: 2017年5月7日
 *      Author: xianlee
 */

#include "stm32f10x.h"
#include "stdio.h"
#include "stdlib.h"
#include "sim900a.h"
#include "usart.h"
#include "system.h"

#define READ_BLOCK 10*1024
#define ENABLE_TIMEOUT 0

gsm_data_record gsm_global_data = { "\0", 0, 0};

static const char *modetbl[2] = { "TCP", "UDP" };

void sim900a_gpio_init()
{
	GPIO_InitTypeDef GPIO_InitStructureA;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
	GPIO_InitStructureA.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructureA.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructureA.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructureA);
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}

void sim900a_poweron()
{
    do {
        if(sim900a_check_status()) {
            printf("%s %d:Sim900a is already power on!\n", __func__, __LINE__);
            break;
        } else {
            GPIO_SetBits(GPIOB, GPIO_Pin_0);
            delay_ms(1200);
            GPIO_ResetBits(GPIOB, GPIO_Pin_0);
            delay_ms(1000);
            printf("%s %d:Sim900a power on now!\n", __func__, __LINE__);
        }
    } while(1);
}

int sim900a_check_status()
{
	int try_times = 5;

	/* hardware sync */
	do {
		if(sim900a_cmd_with_reply("AT", "OK", NULL, GSM_CMD_WAIT_SHORT)) {
			return 1;
		}
		delay_ms(100);
	} while(try_times--);

	return 0;
}

bool sim900a_cmd_with_reply (const char *cmd, const char * reply1, const char * reply2, uint32_t waittime)
{
	memset(&gsm_global_data, 0, sizeof(gsm_data_record));
	memset(gsm_global_data.frame_buf, 0, GSM_DATA_RECORD_MAX_LEN);
	gsm_global_data.frame_len = 0;

	if((uint32_t)cmd < 0xFF) {
		sim900a_send_byte((uint32_t)cmd);
	} else {
		sim900a_usart("%s\r\n", cmd );
	}

	printf("--->%s\n", cmd);
	
	if ((reply1 == NULL) && (reply2 == NULL))
		return true;

#if ENABLE_TIMEOUT
	bool ret = 0;
	gsm_global_data.frame_buf[GSM_DATA_RECORD_MAX_LEN - 1] = '\0';
	TIME_LOOP_MS(waittime) {
		if ((reply1 != NULL) && (reply2 != NULL)) {
			ret = (( bool ) strstr(gsm_global_data.frame_buf, reply1)
					|| ( bool ) strstr(gsm_global_data.frame_buf, reply2));
		} else if (reply1 != 0) {
			ret = (( bool ) strstr(gsm_global_data.frame_buf, reply1));
		} else {
			ret = (( bool ) strstr(gsm_global_data.frame_buf, reply2));
		}

		if(ret) {
			break;
		}
	}

	return ret;
#else
	delay_ms(waittime);
	//gsm_global_data.frame_buf[gsm_global_data.frame_len] = '\0';

	if ((reply1 != 0) && (reply2 != 0)) {
		return (( bool ) strstr(gsm_global_data.frame_buf, reply1)
				|| ( bool ) strstr(gsm_global_data.frame_buf, reply2));
	} else if (reply1 != 0) {
		return (( bool ) strstr(gsm_global_data.frame_buf, reply1));

	} else {
		return (( bool ) strstr(gsm_global_data.frame_buf, reply2));

	}
#endif
}

int sim900a_get_imei(char* imei)
{
	memset(&gsm_global_data, 0, sizeof(gsm_data_record));
	gsm_global_data.frame_len = 0;
    sim900a_cmd_with_reply("AT+CGSN", "OK", NULL, GSM_CMD_WAIT_SHORT);

    sscanf(gsm_global_data.frame_buf, "\n%s\n", imei);
    
    return 0;
}

int sim900a_net_init()
{
	if(!sim900a_cmd_with_reply(BEARER_PROFILE_GPRS, "OK", "ALREADY", GSM_CMD_WAIT_LONG)) {
		return -1;
	}

	return 0;
}

int sim900a_close_net()
{
	int ret = 0;
	ret = sim900a_cmd_with_reply("AT+CIPCLOSE=1", "CLOSE OK", NULL, GSM_CMD_WAIT_LONG);
	ret = sim900a_cmd_with_reply("AT+CIPSHUT", "SHUT OK", NULL, GSM_CMD_WAIT_LONG);

	return ret;
}

int sim900a_tcpudp_connect(uint8_t mode, const char* ipaddr, const char* port)
{
	char net_info[256] = {0};

	snprintf(net_info, 256, "AT+CIPSTART=\"%s\",\"%s\",\"%s\"", modetbl[mode],
			 ipaddr, port);

	if(!sim900a_cmd_with_reply(net_info, "ALREADY", NULL, GSM_CMD_WAIT_LONG)) {
		return -1;
	}
    
    if (!sim900a_cmd_with_reply("AT+CIPSTATUS", "CONNECT OK", NULL, GSM_CMD_WAIT_NORMAL)) {
        return -1;
    }        

	return 1;
}

/* assume network status was connected */
int sim900a_net_send(char* data)
{
	if (sim900a_cmd_with_reply("AT+CIPSEND", ">", NULL, GSM_CMD_WAIT_LONG)) {
		delay_us(500);
		sim900a_usart("%s", data);
		delay_us(50);
		if (sim900a_cmd_with_reply((char*) 0X1A, "OK", NULL, 1864)) {
			printf("send success!\n");
		} else {
			printf("send failed!\n");
			return -1;
		}
	} else {
		sim900a_cmd_with_reply((char*) 0X1B, NULL, NULL, GSM_CMD_WAIT_NONE);	//ESC,取消发送
		return -1;
	}

	return 1;
}

int sim900a_send_raw(uint8_t *data, uint16_t len)
{
	if (sim900a_cmd_with_reply("AT+CIPSEND", ">", NULL, GSM_CMD_WAIT_LONG)) {
		delay_us(500);
		usart_dump(GSM_UART, data, len);
		delay_us(50);
		if (sim900a_cmd_with_reply((char*) 0X1A, "OK", NULL, 1864)) {
			printf("send success!\n");
		} else {
			printf("send failed!\n");
			return -1;
		}
	} else {
		sim900a_cmd_with_reply((char*) 0X1B, NULL, NULL, GSM_CMD_WAIT_NONE);	//ESC,取消发送
		return -1;
	}

	return 0;
}

int sim900a_http_init()
{
	int ret = -1;

	sim900a_cmd_with_reply("AT+HTTPTERM", NULL, NULL, GSM_CMD_WAIT_NORMAL);
	
	/* init network before init http */
	ret = sim900a_cmd_with_reply("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK", NULL, GSM_CMD_WAIT_LONG);
	if(ret) {
		printf("Success to set to AT+SAPBR=3,1,GPRS!\n");
	} else {
		// actualy we already init network in tcp step
		printf("Failed to set to AT+SAPBR=3,1,GPRS!\n");
		//return -1;
	}

	ret = sim900a_cmd_with_reply("AT+SAPBR=3,1,\"APN\",\"CMNET\"", "OK", NULL, GSM_CMD_WAIT_LONG);
	if(ret) {
		printf("Success to set to AT+SAPBR=3,1,APN!\n");
	} else {
		printf("Failed to set to AT+SAPBR=3,1,APN!\n");
		//return -1;
	}

	ret = sim900a_cmd_with_reply("AT+SAPBR=1,1", "OK", NULL, GSM_CMD_WAIT_LONG);
	if(ret) {
		printf("Success to set to AT+SAPBR=1,1!\n");
	} else {
		printf("Failed to set to AT+SAPBR=1,1!\n");
		//return -1;
	}

    do {
        ret = sim900a_cmd_with_reply("AT+HTTPINIT", "OK", NULL, GSM_CMD_WAIT_LONG);
        if(ret) {
			printf("Success to init to http!\n");
			break;
        } else {
			printf("Failed to init to http!\n");
        }
    } while(1);
	
	return 0;
}

int sim900a_http_session_set(const char *uri)
{
	int ret = 0;
	char httpPara[1024*8];
	bool https = false;

	snprintf(httpPara, 1024*8, HTTP_PARA, uri);
	
	if (!sim900a_cmd_with_reply(HTTP_CID, OK, NULL, GSM_CMD_WAIT_LONG)) {
		return -1;
	}

	if (!sim900a_cmd_with_reply(httpPara, OK, NULL, GSM_CMD_WAIT_LONG)) {
		return -1;
	}

	https = strncmp(HTTPS_PREFIX, uri, strlen(HTTPS_PREFIX)) == 0;

	if (!sim900a_cmd_with_reply(https ? HTTPS_ENABLE : HTTPS_DISABLE, OK, NULL, GSM_CMD_WAIT_LONG)) {
		return -1;
	}
/*
	if (!sim900a_cmd_with_reply(HTTP_CONTENT, OK, NULL, GSM_CMD_WAIT_LONG)) {
		return -1;
	}
*/
	return ret;
}

int sim900a_http_param_set(const char* params)
{
	char httpPara[128];
	bool https = false;

	snprintf(httpPara, 128, HTTP_USER_DATA, params);

	if (!sim900a_cmd_with_reply(httpPara, OK, NULL, GSM_CMD_WAIT_LONG)) {
		return -1;
	}

	return 0;
}

int sim900a_read_resp(char *response)
{

	return 0;
}


int sim900a_http_get(char* url)
{
	char url_at[128] = {0};
	int ret = -1;
	int i = 0;
	int ret_start = 0;
	
	sim900a_cmd_with_reply("AT+HTTPPARA=\"CONTENT\",\"application/octet-stream\"", "OK", NULL, GSM_CMD_WAIT_SHORT);
	sim900a_cmd_with_reply("AT+HTTPPARA=\"CID\",1", "OK", NULL, GSM_CMD_WAIT_LONG);
	//将请求的url格式化到命令中
	snprintf(url_at, 128, "AT+HTTPPARA=\"URL\",\"http://%s\"", url);
	printf("%s %d:%s\n", __func__, __LINE__, url_at);

    do {
        ret = sim900a_cmd_with_reply(url_at, "OK", NULL, GSM_CMD_WAIT_LONG);
        if(ret) {
			printf("%s %d:Success to connect to server!\n", __func__, __LINE__);
			break;
        } else {
			break;
        }
    } while(!ret);
	
	memset(gsm_global_data.frame_buf, 0, GSM_DATA_RECORD_MAX_LEN);
    sim900a_cmd_with_reply("AT+HTTPACTION=0", "OK", NULL, GSM_CMD_WAIT_LONG);

	// waiting for action cmd return
	printf("\nDowloading:");
	do {
		if((bool) strstr((const char*)gsm_global_data.frame_buf, "+HTTPACTION")) {
			printf("\n%s %d:HTTP GET finished!\n", __func__, __LINE__);
			break;
		} else if((bool) strstr((const char*)gsm_global_data.frame_buf, "+CME ERROR")) {
            return -1;
        } else {
			printf(">");
			delay_ms(1000);
		}
	} while(1);
	
	for(i=0;i<GSM_DATA_RECORD_MAX_LEN - 20;i++) {
		if(gsm_global_data.frame_buf[i] == '+' &&
		   gsm_global_data.frame_buf[i+1] == 'H' &&
		   gsm_global_data.frame_buf[i+2] == 'T' &&
		   gsm_global_data.frame_buf[i+3] == 'T' &&
		   gsm_global_data.frame_buf[i+4] == 'P') {
			ret_start = i;
			printf("%s %d:Found data start pos!\n", __func__, __LINE__);
			break;
		}
	}
	
	//寻找200OK返回，没有的话就返回错误
#ifdef GSM_DEVICE_SIM800
	if(gsm_global_data.frame_buf[ret_start+15] == '2' &&
	   gsm_global_data.frame_buf[ret_start+16] == '0' &&
	   gsm_global_data.frame_buf[ret_start+17] == '0') {
		printf("Http return 200 OK!\n");
		ret = 0;
	} else {
		printf("Http return error code!\n");
		ret = -1;
	}
#else
	if(gsm_global_data.frame_buf[ret_start+14] == '2' &&
	   gsm_global_data.frame_buf[ret_start+15] == '0' &&
	   gsm_global_data.frame_buf[ret_start+16] == '0') {
		printf("Http return 200 OK!\n");
		ret = 0;
	} else {
		printf("Http return error code!\n");
		ret = -1;
	}
#endif
	
	return ret;
}

int sim900a_http_post(const char *uri, const char *body, int len, char *resp)
{
	int ret = -1;
	char httpData[32];
	unsigned int delayToDownload = 10000;
	
	//if (0 != sim900a_http_session_set(uri)) {
	//	return -1;
	//}

	snprintf(httpData, 32, HTTP_DATA, len, 10000);
	if (!sim900a_cmd_with_reply(httpData, DOWNLOAD, NULL, GSM_CMD_WAIT_LONG)) {
		return -1;
	}

	delay_ms(1000);
	
	usart_dump(GSM_UART, (uint8_t*)body, len);

	delay_ms(200);

	if (!sim900a_cmd_with_reply(HTTP_POST, HTTP_200, NULL, GSM_CMD_WAIT_LONG)) {
		return -1;
	}
	
	return -1;
}

int sim900a_http_read(int start, int size, char** data)
{
	int ret = -1;
	int i = 0;
	int read_len = 0;
	int ret_start = 0;
	char read_range[48] = {0};
	char cmd_str[16] = {0};
	
	snprintf(read_range, 48, "AT+HTTPREAD=%d,%d", start, size);
	
    do {
		gsm_global_data.frame_len = 0;
		if(size == 0) {
			ret = sim900a_cmd_with_reply("AT+HTTPREAD", "+HTTPREAD", NULL, GSM_CMD_WAIT_LONG);
		} else {
			ret = sim900a_cmd_with_reply(read_range, "+HTTPREAD", NULL, size ? (size/10):2000);
		}
		
        if(ret) {
			printf("Success to read data from server!\n");
			break;
        } else {
			printf("Failed to read data, retry...!\n");
			delay_ms(1000);
        }
    } while(1);

	for(i=0;i<GSM_DATA_RECORD_MAX_LEN - 20;i++) {
		if(gsm_global_data.frame_buf[i] == '+' &&
		   gsm_global_data.frame_buf[i+1] == 'H' &&
		   gsm_global_data.frame_buf[i+2] == 'T' &&
		   gsm_global_data.frame_buf[i+3] == 'T' &&
		   gsm_global_data.frame_buf[i+4] == 'P') {
			ret_start = i;
			printf("Found data start pos!\r\n");
			break;
		}
	}
	
	gsm_global_data.frame_buf[GSM_DATA_RECORD_MAX_LEN -1] = '\0';

	// 获取读取到的数据长度信息
	sscanf(gsm_global_data.frame_buf+ret_start, "%[^:]:%d", cmd_str, &read_len);
	// 长度信息所占长度
	i = snprintf(cmd_str, 16, "%d", read_len);

	// +HTTPREAD: xxxx
	*data = gsm_global_data.frame_buf+ret_start + 1 + 9 + 1 + i + 1;
#ifdef GSM_DEVICE_SIM800
	*data = gsm_global_data.frame_buf+ret_start + 1 + 9 + 1 + i + 1 + 1;
#endif
	
	printf("Read http data pos:%p,len:%d\n", *data, read_len);
	
	return read_len;
}

int sim900a_read_tcp(uint8_t** data, uint16_t len)
{
	/* read sim900a data in manual */
    sim900a_cmd_with_reply("AT+CIPRXGET=2,1460", "OK", NULL, GSM_CMD_WAIT_SHORT);
	
	return 0;
}

void sim900a_user_config()
{
    sim900a_poweron();

	/* sim900a close eho */
    while(!sim900a_cmd_with_reply("ATE0", "OK", NULL, GSM_CMD_WAIT_SHORT)) {
        delay_ms(100);
    }
	
	/* sim900a check simcard */
    while(!sim900a_cmd_with_reply("AT+IFC=0,0", "OK", NULL, GSM_CMD_WAIT_SHORT)) {
        delay_ms(100);
    }

	/* sim900a reg */
    while(!sim900a_cmd_with_reply("AT+CREG?", "0,1", NULL, GSM_CMD_WAIT_SHORT)) {
        delay_ms(500);
    }

	/* read sim900a data in manual */
    sim900a_cmd_with_reply("AT+CIPRXGET=1", "OK", NULL, GSM_CMD_WAIT_SHORT);
	
    printf("SIM900a core init OK!\n");

	sim900a_net_init();

	/* sim900a reg */
    while(!sim900a_cmd_with_reply("AT+CGATT?", "1", NULL, GSM_CMD_WAIT_SHORT)) {
        delay_ms(500);
		sim900a_cmd_with_reply("AT+CGATT=1", "1", NULL, GSM_CMD_WAIT_SHORT);
    }
}

//连接到TCP服务器
int connect_update_server(const char* ip, const char* port, int timeout)
{
    int times = 0;
    int ret = -1;
    
    /* connect to update server */
    do {
        times++;
        ret = sim900a_tcpudp_connect(0, ip, port);
        if(ret == -1) {
            printf("Failed to connect to server %d times!\n", times);
        } else {
            printf("Success to connect to server!\n");
            break;
        }
        delay_ms(100);
    } while(times < timeout);

    return (ret ? 0:-1);
}

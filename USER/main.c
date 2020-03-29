#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "system.h"
#include "sys.h"
#include "usart.h"
#include "sim900a.h"
#include "relay.h"
#include "mqtt-user.h"
#include "timer.h"
#include "timer_task.h"
#include "frozen.h"
#include "parameter.h"
#include "ctype.h"
#include "MQTTPacket.h"

/* 需要在这里更改服务器地址端口，用户名密码等信息 */
/* 设备默认会将IMEI(SIM800C模块上的标号)设置为clientID */
/*  */
#define MQTT_SERVER_IP  "relay.mqtt.iot.gz.baidubce.com" 
#define MQTT_SERVER_PORT	"1883"

char client_id[128] = {0};
char user_name[128] = {"relay/relay"};
char password[128] = {"Zqu9im83wlVp8++FgyVJb9d+J4EKd4ytXpa9w02N2hE="};
char topic[64] = {"relay"};

int main(void)
{
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init();
    usart1_init(921600);
    usart3_init(115200);
    timer3_init(10000, 7199);//10Khz的计数频率，计数到10000为1s
    sim900a_gpio_init();
    relay_init();
    relay_on();
    
    sim900a_user_config();
    sim900a_get_imei(client_id);   
    
    printf("\n*******MQTT INFO************\n");
    printf("CLIENT ID:%s\n", client_id);
    printf("USER NAME:%s\n", user_name);
    printf("PASSWORD:%s\n", password);
    printf("*******END************\n");

    mqtt_connection_init(MQTT_SERVER_IP, MQTT_SERVER_PORT, client_id, user_name, password);
    mqtt_subscribe(topic, 0);

    while(1) {
        while (!mqtt_state.connected || mqtt_state.error_count > 1) {
            mqtt_connection_init(MQTT_SERVER_IP, MQTT_SERVER_PORT, client_id, user_name, password);
            delay_ms(500);
            sim900a_cmd_with_reply("AT+CIPSTATUS", "OK", NULL, GSM_CMD_WAIT_NORMAL);
            mqtt_subscribe(topic, 0);
            mqtt_send_beat("topic_server", curr_state);
        }
        user_task_loop();
    }
}

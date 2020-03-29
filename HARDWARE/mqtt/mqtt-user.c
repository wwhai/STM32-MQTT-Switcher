#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "mqtt-user.h"
#include "sim900a.h"
#include "parameter.h"
#include "MQTTPacket.h"
#include "system.h"

mqtt_state_t mqtt_state = {0, 0};
uint32_t msg_id = 1;

int mqtt_send_msg(const char *msg, int len)
{
    if (0 > sim900a_send_raw((uint8_t*)msg, len)) {
        mqtt_state.error_count++;
        return -1;
    } else {
        mqtt_state.error_count = 0;
        mqtt_state.connected = 1;
    }

    return 0;
}

int mqtt_connection_init(char* host, char* port, char* id, char* name, char* password)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	char mqtt_msg[MQTT_MSG_MAX_LEN] = {0};
	int len = 0;
    
    if(0 != connect_update_server(host, port, 50)) {
        return -1;
    }

	printf("Sending to hostname %s port %s\n", host, port);

	data.clientID.cstring = id;
	data.keepAliveInterval = 60;
	data.cleansession = 1;
	data.username.cstring = name;
	data.password.cstring = password;
	data.MQTTVersion = 3;
    data.willFlag = 0;

	len = MQTTSerialize_connect((unsigned char *)mqtt_msg, MQTT_MSG_MAX_LEN, &data);
    delay_ms(500);
    return mqtt_send_msg(mqtt_msg, len);
}

int mqtt_subscribe(char* topic, int qos)
{
    MQTTString topicString = MQTTString_initializer;
    char mqtt_msg[MQTT_MSG_MAX_LEN] = {0};
    int len = -1;
    int req_qos = 0;

	/* subscribe */
	topicString.cstring = topic;
	len = MQTTSerialize_subscribe((unsigned char*)mqtt_msg, MQTT_MSG_MAX_LEN, 0, msg_id++, 1, &topicString, &req_qos);
    
    return mqtt_send_msg(mqtt_msg, len);
}

int mqtt_publish(char *topic, const char *payload, uint16_t payload_length, int qos)
{
    MQTTString topicString = MQTTString_initializer;
    char mqtt_msg[MQTT_MSG_MAX_LEN] = {0};
    int len = -1;

	/* subscribe */
	topicString.cstring = topic;
	len = MQTTSerialize_publish((unsigned char*)mqtt_msg, MQTT_MSG_MAX_LEN, 0, 0, 0, 0, topicString, (unsigned char*)payload, payload_length);
    return mqtt_send_msg(mqtt_msg, len);
}

int mqtt_send_beat(char* topic, int online)
{
    char mqtt_msg[MQTT_MSG_MAX_LEN] = {0};
    uint16_t payload_len = 0;
    
    payload_len = snprintf(mqtt_msg, MQTT_MSG_MAX_LEN, "{\"imei\":\"%s\",\"heart\":\"Success\",state:\"%d\"}", client_id, online);

    if (payload_len > 0) {
        printf("send heart beat:%d,%s\n", strlen(mqtt_msg), mqtt_msg);
        return mqtt_publish(topic, mqtt_msg, strlen(mqtt_msg), 0);
    } else {
        printf("send heart beat failed\n");
    }

    return -1;
}

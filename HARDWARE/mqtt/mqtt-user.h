#ifndef __MQTT_USER_H__
#define __MQTT_USER_H__

#include "stdint.h"

extern char client_id[];

typedef struct _mqtt_state {
	int connected;
	int error_count;
} mqtt_state_t;

extern mqtt_state_t mqtt_state;

extern uint32_t msg_id;

int mqtt_send_msg(const char *msg, int len);

int mqtt_connection_init(char* host, char* port, char* id, char* name, char* password);

int mqtt_subscribe(char* topic, int qos);

int mqtt_publish(char *topic, const char *payload, uint16_t payload_length, int qos);

int mqtt_send_beat(char* topic, int online);

#endif

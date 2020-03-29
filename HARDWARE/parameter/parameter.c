#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "parameter.h"
#include "mqtt-user.h"

#define PARAM_ADDR  (0x08000000+0x10000-0x400)

system_state_t sys_state = {
    .state = 2,
    .startup_times = 0,
    .totalB = 0,
    .leftB = 0,
    .totalJ = 0,
};

int gen_beat_msg(char* msg, int len, int online)
{
	int payload_len = 0;

	if (online) {
		payload_len = snprintf(msg, len, MQTT_BEAT_FORM, client_id,
                               sys_state.totalB, sys_state.leftB,
                               sys_state.totalJ, sys_state.state, 1.234, 2.234);
	} else {
		payload_len = snprintf(msg, len, MQTT_OFFLINE_BEAT_FORM, client_id, 1.234, 2.234);
	}

	return payload_len;
}

int gen_ack_msg(char* msg, int len, int id, const char* cmd, int state)
{
	int payload_len = 0;

	if (state) {
		payload_len = snprintf(msg, len, MQTT_ACK_FORM, id, cmd, state, "OK");
	} else {
		payload_len = snprintf(msg, len, MQTT_ACK_FORM, id, cmd, state, "ERROR");
	}

	return payload_len;
}

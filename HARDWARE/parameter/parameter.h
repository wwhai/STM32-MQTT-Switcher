#ifndef __PARAM_H__
#define __PARAM_H__

#include "stdint.h"

#define MQTT_MSG_MAX_LEN	(2*1024)

#define MQTT_BEAT_FORM	"{\
\"action\":\"beat\",\
\"clientId\":\"%s\",\
\"totalB\":%d,\
\"leftB\":%d,\
\"totalJ\":%d,\
\"state\":%d,\
\"latlng\":\"%f,%f\"\
}"

#define MQTT_OFFLINE_BEAT_FORM "{\
\"action\":\"offlineBeat\",\
\"clientId\":\"%s\",\
\"latlng\":\"%f,%f\"\
}"

#define MQTT_ACK_FORM "{\
\"id\":%d,\
\"command\":\"%s\",\
\"state\":%d,\
\"msg\":\"%s\"\
}"

typedef struct _system_state {
    int state;
    uint32_t startup_times;
    uint32_t totalB;
    uint32_t leftB;
    uint32_t totalJ;
} system_state_t;

extern system_state_t sys_state;

int save_system_state(system_state_t* state);

int recovery_system_state(system_state_t* state);

int gen_beat_msg(char* msg, int len, int online);

int gen_buzzer_form(char* msg, int len, int online);

int gen_ack_msg(char* msg, int len, int id, const char* cmd, int state);

#endif

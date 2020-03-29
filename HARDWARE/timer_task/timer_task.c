#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "timer_task.h"
#include "mqtt-user.h"
#include "sim900a.h"
#include "timer.h"
#include "frozen.h"
#include "parameter.h"
#include "system.h"
#include "relay.h"

uint64_t curr_time = 0;
uint64_t curr_cmd_id = 0;
char curr_state = 1;

static int read_tcp_func(void* arg);
static int send_beat_func(void* arg);
static int set_relay_func(void* arg);

timer_task_t read_tcp_task = {
    .last_time = 0,
    .next_time = 1,
    .func = read_tcp_func,
};

timer_task_t send_beat_task = {
    .last_time = 0,
    .next_time = 40,
    .func = send_beat_func,
};

timer_task_t set_relay_task = {
    .last_time = 0,
    .next_time = 0,
    .func = set_relay_func,
};

/*
 *{id:13,command:"relay",data:"5"}
 *{id:14,command:"on",data:"5"}
 *{"imei":"866104823648374","state":"1"}
 */
int read_tcp_func(void* arg)
{
    char* json_start = NULL;
    char* json_end = NULL;
    int json_len = -1;
    int i = 0;

    char* cmd = NULL;
    char* data = NULL;

    sim900a_read_tcp(NULL, 0);

    gsm_global_data.frame_buf[GSM_DATA_RECORD_MAX_LEN - 1] = '\0';

    json_start = gsm_global_data.frame_buf;

    for (i=0;i<GSM_DATA_RECORD_MAX_LEN;i++) {
        if (*json_start == '{') {
            break;
        }
        json_start++;
    }

    if (NULL == json_start) {
        return 0;
    }

    json_end = json_start;

    for (i=0;i<GSM_DATA_RECORD_MAX_LEN;i++) {
        if (*json_end == '}') {
            break;
        } else if (*json_end == '\0') {
            json_end = NULL;
            break;
        }
        json_end++;
    }

    if (NULL == json_end) {
        return 0;
    }

    json_len = json_end -json_start;

    if (json_len < 5) {
        return 0;
    }

    printf("MSG IN--->\n%s\n", json_start);

    json_scanf(json_start, json_len, "{imei:%Q,state:%Q}", &cmd, &data);

    if (atoi(data) > 0) {
        printf("relay cmd on!\n");
        relay_on();
        curr_state = 1;
    } else {
        printf("relay cmd off!\n");
        relay_off();
        curr_state = 0;
    }

    //mqtt_publish("topic_server", json_start, json_len + 1, 0);
    
    free(cmd);
    free(data);

    return 0;
}

int send_beat_func(void* arg)
{
    return mqtt_send_beat("topic_server", curr_state);
}

int set_relay_func(void* arg)
{
    set_relay_task.next_time = 0;

    relay_off();
    
    return 0;
}

/******************************************
 * Checking timeout and run functions
 *******************************************/

int timer_task_executer(timer_task_t* task, void* arg)
{
    int ret = -1;
    uint64_t start_time = curr_time;

    if (curr_time - task->last_time >= task->next_time &&
        task->next_time > 0) {
        ret = task->func(arg);
        if (0 > ret) {
            printf("%s:%d:execute task failed\n", __func__, __LINE__);
        } else {
            task->last_time = start_time;
        }
    }

    return ret;
}

void update_current_time()
{
    curr_time = sys_time_sec;
}

void user_task_loop()
{
    update_current_time();

    read_tcp_func(NULL);
    //timer_task_executer(&read_tcp_task, NULL);
    timer_task_executer(&send_beat_task, NULL);
    timer_task_executer(&set_relay_task, NULL);
}

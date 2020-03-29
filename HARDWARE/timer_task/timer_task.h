#ifndef __TIMER_TASK_H__
#define __TIMER_TASK_H__

#include "stdint.h"

typedef int (*timer_task_func)(void*);

typedef struct _timer_task {
  uint64_t last_time;
  uint64_t next_time;
  timer_task_func func;
} timer_task_t;

extern char curr_state;

int timer_task_executer(timer_task_t* task, void* arg);

void update_current_time(void);

void user_task_loop(void);

#endif

#ifndef __MONITOR_H
#define __MONITOR_H

#include "lvgl.h"

void monitor_show(void);
void base_init(void);
void mem_init(void);
void cpu_init(void);
void disk_init(void);
void cpu_temp_init(void);
void uptime_init(void);
void ethernet_init(void);
void proc_init(void);
void time_init(void);
#endif
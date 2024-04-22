#ifndef __MONITOR_H
#define __MONITOR_H

#include "lvgl.h"

void lv_monitor_monitor_show(void);
void lv_monitor_base_init(void);
void lv_monitor_mem_init(void);
void lv_monitor_cpu_init(void);
void lv_monitor_disk_init(void);
void lv_monitor_thermal_init(void);
void lv_monitor_uptime_init(void);
void lv_monitor_network_init(void);
void lv_monitor_task_counter_init(void);
void lv_monitor_time_init(void);
#endif
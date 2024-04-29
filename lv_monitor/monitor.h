#ifndef __MONITOR_H
#define __MONITOR_H

#include "lvgl.h"

#define OBJ_BACKGROUND_COLOR 0x333333

#define LABEL_TITLE_OFFSET_X    -10
#define LABEL_TITLE_OFFSET_Y    -12
#define LABEL_MSG_OFFSET_X      10
#define LABEL_MSG_OFFSET_Y      -12
#define LABEL_CHART_OFFSET_X    0
#define LABEL_CHART_OFFSET_Y    10

#define CHART_SIZE_X            130*2
#define CHART_SIZE_Y            70*2
#define CPU_LABLE_SIZE_X        140*2
#define MEM_LABLE_SIZE_X        140*2
#define DISK_LABLE_SIZE_X       90*2
#define THERMAL_LABLE_SIZE_X    90*2
#define TASK_LABLE_SIZE_X       95*2
#define TIME_LABLE_SIZE_X       65*2
#define NET_LABLE_SIZE_X        150*2
#define UPTIME_LABLE_SIZE_X     65*2
#define TOP_LABLE_SIZE_Y        95*2
#define MID_LABLE_SIZE_Y        95*2
#define BOT_LABLE_SIZE_Y        20*2
#define LABEL_ARC_SIZE_X        70*2
#define LABEL_ARC_SIZE_Y        70*2
#define LABEL_ARC_WIDTH         6*2

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
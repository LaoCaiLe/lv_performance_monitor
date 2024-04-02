#ifndef __MONITOR_H
#define __MONITOR_H

#include "lvgl.h"


void monitor_show(void);
void base_init(void);
void mem_init(void);
void cpu_init(void);
void disk_init(void);
#endif
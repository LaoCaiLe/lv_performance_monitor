#ifndef _INFO_H
#define _INFO_H
#include "lvgl.h"
#include "stdint.h"

enum Result
{
    ret_ok,
    ret_fail
};

#define PRCO_PATH "/proc"
#define CPU_PROC_PATH "/proc/stat"
#define MEM_PROC_PATH "/proc/meminfo"
#define TIME_PROC_PATH "/proc/uptime"
#define DISK_MSG_CMD "df /"
#define ETHER_DEVICE "eth0"
#define CPU_TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"

int get_cpu_load(uint16_t *cpu_load);
int get_mem_load(uint32_t *use_mem_kb, uint32_t *total_mem_kb, uint16_t *mem_load);
int get_disk_use(uint32_t *disk_all_kb, uint32_t *disk_use_kb, uint32_t *disk_valid_kb, uint16_t *disk_use_rate);
int get_ethernet_speed(uint32_t *upload_speed_bps, uint32_t *download_speed_bps);
int get_cpu_uptime(uint64_t *uptime_sec);
int get_cpu_temperature(uint32_t *temp);
int get_task_num(uint32_t *num_processes, uint32_t *num_threads, uint32_t *num_zombies);
int get_time_string(char *string);

#endif

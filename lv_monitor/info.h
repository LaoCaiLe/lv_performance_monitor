#ifndef _INFO_H
#define _INFO_H

#include "stdint.h"
enum Result
{
    ret_ok,
    ret_fail
};

#define CPU_PROC_PATH "/proc/stat"
#define MEM_PROC_PATH "/proc/meminfo"
#define TIME_PROC_PATH "/proc/uptime"
#define DISK_MSG_CMD "df /"
#define ETHER_DEVICE "eth0"

int get_cpu_load(uint16_t *cpu_load);
int get_mem_load(uint32_t *use_mem_kb, uint32_t *total_mem_kb, uint16_t *mem_load);
int get_disk_use(uint32_t *disk_all_bytes, uint32_t *disk_use_bytes, uint32_t *disk_valid_bytes, uint16_t *disk_use_rate);
int get_ethernet_speed(uint32_t *upload_speed_bps, uint32_t *download_speed_bps);
int get_cpu_uptime(uint64_t *uptime_sec);

#endif

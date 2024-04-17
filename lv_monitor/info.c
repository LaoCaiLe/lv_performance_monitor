#include "info.h"
#include "lvgl/src/misc/lv_log.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int get_cpu_load(uint16_t *cpu_load)
{
    int curr_idle = 0;
    int curr_total = 0;
    static int prev_idle = 0;
    static int prev_total = 0;
    int i = 0;
    char *token = NULL;
    char line[1024];
    FILE *file = fopen(CPU_PROC_PATH, "r");
    if (file == NULL)
    {
        LV_LOG_ERROR("can not open file "CPU_PROC_PATH"!\n");
        return ret_fail;
    }

    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "cpu "))
        {
            token = strtok(line, " ");
            token = strtok(NULL, " ");
            while (token != NULL)
            {
                curr_total += atoi(token);
                if(i == 3 || i == 4)
                    curr_idle += atoi(token);

                token = strtok(NULL, " ");
                i++;
            }
            break;
        }
    }

    fclose(file);

    int total_diff = curr_total - prev_total;
    int idle_diff = curr_idle - prev_idle;

    prev_total = curr_total;
    prev_idle = curr_idle;

    *cpu_load = (total_diff - idle_diff) * 100 / total_diff;
    LV_LOG_INFO("curr_total[%d], curr_idle[%d], prev_total[%d], prev_idle[%d], load:%d\n", curr_total, curr_idle, prev_total, prev_idle, *cpu_load);
    return ret_ok;
}

int get_mem_load(uint32_t *use_mem_kb, uint32_t *total_mem_kb, uint16_t *mem_load)
{
    char line[1024];
    char *token = NULL;
    FILE *file = fopen(MEM_PROC_PATH, "r");
    if (file == NULL)
    {
        LV_LOG_ERROR("can not open file "MEM_PROC_PATH"!\n");
        return ret_fail;
    }

    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "MemTotal: "))
        {
            token = strtok(line, " ");
            token = strtok(NULL, " ");
            *total_mem_kb = atoi(token);
        }

        else if (strstr(line, "MemAvailable: "))
        {
            token = strtok(line, " ");
            token = strtok(NULL, " ");
            *use_mem_kb = *total_mem_kb - atoi(token);
        }
    }

    *mem_load = (int)(*use_mem_kb * 100 / *total_mem_kb);
    fclose(file);

    LV_LOG_INFO("use_mem_kb[%u], total_mem_kb[%u],  load:%d%%\n", *use_mem_kb, *total_mem_kb, *mem_load);
    return ret_ok;
}

int get_disk_use(uint32_t *disk_all_bytes, uint32_t *disk_use_bytes, uint32_t *disk_valid_bytes, uint16_t *disk_use_rate)
{
    FILE *file;
    char line[1024];
    char filesystem[128];

    file = popen(DISK_MSG_CMD, "r");
    if (file == NULL)
    {
        LV_LOG_ERROR("can not open command "DISK_MSG_CMD"!\n");
        return ret_fail;
    }

    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);

    sscanf(line, "%s %u %u %u %u%%", filesystem, disk_all_bytes, disk_use_bytes, disk_valid_bytes, disk_use_rate);
    pclose(file);

    LV_LOG_INFO("filesystem[%s], disk_all_bytes[%u], disk_use_bytes[%u], disk_valid_bytes[%u], disk_use_rate[%u%%]", filesystem, disk_all_bytes, disk_use_bytes, disk_valid_bytes, disk_use_rate);

    return ret_ok;
}

int get_ethernet_speed(uint32_t *upload_speed_bps, uint32_t *download_speed_bps)
{
    char line[1024] = {0};
    char file_path[256] = {0};
    FILE *file = NULL;
    uint32_t curr_upload_speed = 0;
    uint32_t curr_download_speed = 0;
    static uint32_t prev_upload_speed = 0;
    static uint32_t prev_download_speed = 0;

    snprintf(file_path, sizeof(file_path), "/sys/class/net/%s/statistics/rx_bytes", ETHER_DEVICE);
    file = fopen(file_path, "r");
    if (file == NULL)
    {
        LV_LOG_ERROR("can not open file %s!\n", file_path);
        return ret_fail;
    }
    fgets(line, sizeof(line), file);
    curr_download_speed = atoi(line);
    fclose(file);

    snprintf(file_path, sizeof(file_path), "/sys/class/net/%s/statistics/tx_bytes", ETHER_DEVICE);
    file = fopen(file_path, "r");
    if (file == NULL)
    {
        LV_LOG_ERROR("can not open file %s!\n", file_path);
        return ret_fail;
    }
    fgets(line, sizeof(line), file);
    curr_upload_speed = atoi(line);
    fclose(file);

    *upload_speed_bps = (curr_upload_speed - prev_upload_speed) * 8;
    *download_speed_bps = (curr_download_speed - prev_download_speed) * 8;

    LV_LOG_INFO("download_speed_bps[%u], upload_speed_bps[%u]\n", *download_speed_bps, *upload_speed_bps);

    prev_upload_speed = curr_upload_speed;
    prev_download_speed = curr_download_speed;

    return ret_ok;
}

int get_cpu_uptime(uint64_t *uptime_sec)
{
    char line[1024] = {0};
    FILE *file = NULL;
    char *token = NULL;

    file = fopen(TIME_PROC_PATH, "r");
    if (file == NULL)
    {
        LV_LOG_ERROR("can not open file " TIME_PROC_PATH "!\n");
        return ret_fail;
    }
    fgets(line, sizeof(line), file);

    token = strtok(line, " ");
    *uptime_sec = atoi(token);

    LV_LOG_INFO("uptime_sec[%lu]\n", *uptime_sec);

    return ret_ok;
}
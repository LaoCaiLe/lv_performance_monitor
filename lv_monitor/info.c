#include "info.h"
#include "lvgl/src/misc/lv_log.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

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

int get_disk_use(uint32_t *disk_all_kb, uint32_t *disk_use_kb, uint32_t *disk_valid_kb, uint16_t *disk_use_rate)
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

    sscanf(line, "%s %u %u %u %u%%", filesystem, disk_all_kb, disk_use_kb, disk_valid_kb, disk_use_rate);
    pclose(file);

    LV_LOG_INFO("filesystem[%s], disk_all_kb[%u], disk_use_kb[%u], disk_valid_kb[%u], disk_use_rate[%u%%]\n", filesystem, *disk_all_kb, *disk_use_kb, *disk_valid_kb, *disk_use_rate);

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
    static bool is_first = true;

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

    prev_upload_speed = curr_upload_speed;
    prev_download_speed = curr_download_speed;

    if(is_first)
    {
        *upload_speed_bps = 0;
        *download_speed_bps = 0;
        is_first = false;
    }
    LV_LOG_INFO("download_speed_bps[%u], upload_speed_bps[%u]\n", *download_speed_bps, *upload_speed_bps);

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

    fclose(file);
    LV_LOG_INFO("uptime_sec[%lu]\n", *uptime_sec);

    return ret_ok;
}

int get_cpu_temperature(uint32_t *temp)
{
    char line[1024] = {0};
    FILE *file = NULL;

    file = fopen(CPU_TEMP_PATH, "r");
    if (file == NULL)
    {
        LV_LOG_ERROR("can not open file " CPU_TEMP_PATH "!\n");
        return ret_fail;
    }
    fgets(line, sizeof(line), file);
    *temp = atoi(line)/1000;

    fclose(file);
    LV_LOG_INFO("temp[%lu]\n", *temp);

    return ret_ok;
}

int get_task_num(uint32_t *num_processes, uint32_t *num_threads, uint32_t *num_zombies)
{
    DIR *dir = opendir(PRCO_PATH);
    if (dir == NULL) {
        LV_LOG_ERROR("Failed to open /proc directory, errno[%d]", errno);
        return ret_fail;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strtol(entry->d_name, NULL, 10) > 0) {
            (*num_processes)++;

            char task_path[PATH_MAX];
            snprintf(task_path, sizeof(task_path), "%s/%s/task", PRCO_PATH, entry->d_name);

            DIR *task_dir = opendir(task_path);
            if (task_dir == NULL) {
                continue;
            }

            struct dirent *task_entry;
            while ((task_entry = readdir(task_dir)) != NULL) {
                if (task_entry->d_type == DT_DIR && strtol(task_entry->d_name, NULL, 10) > 0) {
                    (*num_threads)++;
                }
            }
            closedir(task_dir);

            char status_path[PATH_MAX];
            snprintf(status_path, sizeof(status_path), "%s/%s/stat", PRCO_PATH, entry->d_name);

            FILE *fd = fopen(status_path, "r");
            if (fd == NULL) {
                continue;
            }
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), fd) != NULL) {
                char *token = strtok(buffer, " ");
                for (int i = 0; i < 2 && token != NULL; i++) {
                    token = strtok(NULL, " ");
                }

                if (token != NULL && strcmp(token, "Z") == 0) {
                    (*num_zombies)++;
                }
            }
            fclose(fd);
        }
    }
    closedir(dir);
    LV_LOG_INFO("num_processes[%u],num_threads[%u],num_zombies[%u]\n", *num_processes, *num_threads, *num_zombies);

    return ret_ok;
}

int get_time_string(uint16_t *hours, uint16_t *minutes, uint16_t *seconds)
{
    time_t current_time;
    struct tm * time_info;

    time(&current_time);
    time_info = localtime(&current_time);
    if (time_info == NULL) {
        LV_LOG_ERROR("Failed to get local time. errno[%d]\n",errno);
        return ret_fail;
    }

    *hours = time_info->tm_hour;
    *minutes = time_info->tm_min;
    *seconds = time_info->tm_sec;

    LV_LOG_INFO("Current time: %d:%d:%d\n", *hours, *minutes, *seconds);
    
    return ret_ok;
}
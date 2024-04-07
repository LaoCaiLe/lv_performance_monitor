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
    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL)
    {
        LV_LOG_ERROR("can not open file /proc/stat!");
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

int get_mem_load(float *use_mem, float *total_mem, uint16_t *mem_load)
{
    char line[1024];
    char *token = NULL;
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL)
    {
        LV_LOG_ERROR("can not open file /proc/meminfo!");
        return ret_fail;
    }

    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "MemTotal: "))
        {
            token = strtok(line, " ");
            token = strtok(NULL, " ");
            *total_mem=atoi(token);
        }

        else if (strstr(line, "MemAvailable: "))
        {
            token = strtok(line, " ");
            token = strtok(NULL, " ");
            *use_mem = *total_mem - atoi(token);
        }
    }

    *mem_load = (int)(*use_mem * 100 / *total_mem);

    LV_LOG_INFO("use_mem[%.1f], total_mem[%.1f],  load:%d\n", *use_mem, *total_mem, *mem_load);
    return ret_ok;
}
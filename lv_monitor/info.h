#ifndef _INFO_H
#define _INFO_H

#include "stdint.h"
enum Result
{
    ret_ok,
    ret_fail
};

int get_cpu_load(uint16_t *cpu_load);
int get_mem_load(float *use_mem, float *total_mem, uint16_t *mem_load);

#endif
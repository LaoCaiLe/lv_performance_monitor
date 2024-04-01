#ifndef __DISK_H
#define __DISK_H

#include "lvgl.h"
#include "base.h"

class Disk{
public:
    Disk(lv_obj_t *base_obj);
    ~Disk() = default;
    static void get_key_value();
    static void show_key_value(void *obj, int32_t v);
    static void show_key_value_enable();
    static void show_key_value_disable();
};

// void cpu_show();

#endif
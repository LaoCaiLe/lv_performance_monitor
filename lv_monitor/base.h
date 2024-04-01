#ifndef __LV_BASE_OBJ_H
#define __LV_BASE_OBJ_H

#include "lvgl.h"
#include "lv_drv_conf.h"


class Base{
public:
    Base() = default;
    Base(lv_obj_t *bg_obj,uint16_t width, uint16_t height);
    virtual ~Base() = default;
    void get_key_value();
    void show_key_value();
    void show_key_value_enable();
    void show_key_value_disable();
    static void timer_task(lv_timer_t *arg);
    

private:
    uint16_t bg_obj_x;
    uint16_t bg_obj_y;
    uint16_t bg_obj_width;
    uint16_t bg_obj_height;
    uint16_t key_value; 
};

#endif
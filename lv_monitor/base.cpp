#include "base.h"
#include <iostream>

using namespace std;

Base::Base(lv_obj_t *bg_obj,uint16_t width, uint16_t height)
{
    bg_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bg_obj, width, height);
    lv_obj_set_style_bg_color(bg_obj, lv_color_hex(0x111111), LV_PART_MAIN);
}

#include "base.h"

lv_obj_t *base_obj;

void base_init()
{
    base_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(base_obj, SDL_HOR_RES, SDL_VER_RES);
    lv_obj_set_style_bg_color(base_obj, lv_color_hex(0x111111), LV_PART_MAIN);

    // lv_obj_set_style_bg_color(base_obj,lv_color_black(),lv_)
}
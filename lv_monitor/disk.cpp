#include "disk.h"
#include "base.h"
lv_anim_t a;
static lv_obj_t *bg_obj;

Disk::Disk(lv_obj_t *base_obj)
{
    bg_obj = lv_obj_create(base_obj);

    lv_obj_set_size(bg_obj, 80, 80);
    lv_obj_align(bg_obj, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_clear_flag(bg_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_opa(bg_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bg_obj, lv_color_hex(0x222222), LV_PART_MAIN);
    /*Create an Arc*/
    lv_obj_t * arc = lv_arc_create(bg_obj);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_angles(arc, 0, 360);
    lv_arc_set_bg_angles(arc, 0, 360);

    static lv_style_t fg_style,bg_style;
    lv_style_init(&fg_style);
    lv_style_init(&bg_style);

    lv_obj_set_size(arc, 60, 60);
    lv_style_set_arc_color(&fg_style, lv_color_white());
    lv_style_set_arc_rounded(&fg_style, false);
    lv_style_set_arc_width(&fg_style, 6);

    lv_style_set_arc_color(&bg_style, lv_color_hex(0xE91E63));
    lv_style_set_arc_rounded(&bg_style, false);
    lv_style_set_arc_width(&bg_style, 6);

    lv_obj_add_style(arc, &fg_style, LV_PART_MAIN);
    lv_obj_add_style(arc, &bg_style, LV_PART_INDICATOR);

    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_center(arc);

    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, show_key_value);
    lv_anim_set_time(&a, 3000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);    /*Just for the demo*/
    lv_anim_set_repeat_delay(&a, 500);
    lv_anim_set_values(&a, 360, 0);
    

}

void Disk::get_key_value()
{
}
void Disk::show_key_value(void *obj, int32_t v)
{
    lv_arc_set_angles(obj, v, 360);
}
void Disk::show_key_value_enable()
{
    lv_anim_start(&a);
}
void Disk::show_key_value_disable()
{
    // lv_timer_del(timer_task);
}
// void Disk::timer_task(lv_timer_t *arg)
// {
//     get_key_value();
//     show_key_value();
// }

#include "cpu.h"

static lv_obj_t *cpu_obj;
extern lv_obj_t *base_obj;
static void set_angle(void * obj, int32_t v)
{
    lv_arc_set_angles(obj, v, 360);
}

/**
 * Create an arc which acts as a loader.
 */
void lv_example_arc_22(void)
{
    /*Create an Arc*/
    lv_obj_t * arc = lv_arc_create(cpu_obj);
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

    lv_style_set_arc_color(&bg_style, lv_color_hex(0xff7777));
    lv_style_set_arc_rounded(&bg_style, false);
    lv_style_set_arc_width(&bg_style, 6);

    lv_obj_add_style(arc, &fg_style, LV_PART_MAIN);
    lv_obj_add_style(arc, &bg_style, LV_PART_INDICATOR);

    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_center(arc);
    // lv_obj_align(arc, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, set_angle);
    lv_anim_set_time(&a, 3000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);    /*Just for the demo*/
    lv_anim_set_repeat_delay(&a, 500);
    lv_anim_set_values(&a, 360, 0);
    lv_anim_start(&a);

}

void obj_init()
{
    cpu_obj = lv_obj_create(base_obj);
    lv_obj_set_size(cpu_obj, 80, 80);
    lv_obj_align(cpu_obj, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_clear_flag(cpu_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_opa(cpu_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cpu_obj, lv_color_hex(0x222222), LV_PART_MAIN);
}
void cpu_show()
{
    obj_init();
    lv_example_arc_22();
    return;
}


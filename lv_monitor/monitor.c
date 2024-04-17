#include "monitor.h"
#include <stdio.h>
#include "info.h"

static lv_obj_t *base_obj;
static lv_obj_t *chart_cpu;
static lv_obj_t *chart_mem;
static lv_chart_series_t *ser_cpu;
static lv_chart_series_t *ser_mem;

static void draw_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);

    /*Add the faded area before the lines are drawn*/
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part == LV_PART_ITEMS)
    {
        if (!dsc->p1 || !dsc->p2)
            return;

        /*Add a line mask that keeps the area below the line*/
        lv_draw_mask_line_param_t line_mask_param;
        lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y,
                                      LV_DRAW_MASK_LINE_SIDE_BOTTOM);
        int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

        /*Add a fade effect: transparent bottom covering top*/
        lv_coord_t h = lv_obj_get_height(obj);
        lv_draw_mask_fade_param_t fade_mask_param;
        lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_TRANSP,
                               obj->coords.y2);
        int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

        /*Draw a rectangle that will be affected by the mask*/
        lv_draw_rect_dsc_t draw_rect_dsc;
        lv_draw_rect_dsc_init(&draw_rect_dsc);
        draw_rect_dsc.bg_opa = LV_OPA_60;
        draw_rect_dsc.bg_color = dsc->line_dsc->color;

        lv_area_t a;
        a.x1 = dsc->p1->x;
        a.x2 = dsc->p2->x - 1;
        a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
        a.y2 = obj->coords.y2;
        lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

        /*Remove the masks*/
        lv_draw_mask_free_param(&line_mask_param);
        lv_draw_mask_free_param(&fade_mask_param);
        lv_draw_mask_remove_id(line_mask_id);
        lv_draw_mask_remove_id(fade_mask_id);
    }
    /*Hook the division lines too*/
    else if (dsc->part == LV_PART_MAIN)
    {
        if (dsc->line_dsc == NULL || dsc->p1 == NULL || dsc->p2 == NULL)
            return;

        /*Vertical line*/
        if (dsc->p1->x == dsc->p2->x)
        {
        }
        /*Horizontal line*/
        else
        {
            dsc->line_dsc->color = lv_palette_lighten(LV_PALETTE_GREY, 1);
            dsc->line_dsc->opa = LV_OPA_20;
        }
    }
}

static void cpu_timer_task(lv_timer_t *arg)
{
    char buf[64] = {0};
    uint16_t cpu_rate;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;

    get_cpu_load(&cpu_rate);

    snprintf(buf, sizeof(buf), "%d%%", cpu_rate);
    lv_label_set_text(text, buf);
    lv_chart_set_next_value(chart_cpu, ser_cpu, cpu_rate);
}

static void mem_timer_task(lv_timer_t *arg)
{
    char buf[64] = {0};
    uint16_t mem_rate;
    uint32_t use_mem_kb, total_mem_kb;
    float use_mem_gb, total_mem_gb;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;

    get_mem_load(&use_mem_kb, &total_mem_kb, &mem_rate);
    // get_ethernet_speed(&a, &b);
    // get_uptime(&a);
    use_mem_gb = use_mem_kb / 1024.0 / 1024.0;
    total_mem_gb = total_mem_kb / 1024.0 / 1024.0;

    snprintf(buf, sizeof(buf), "%.1fGB / %.1fGB", use_mem_gb, total_mem_gb);
    lv_label_set_text(text, buf);
    lv_chart_set_next_value(chart_mem, ser_mem, mem_rate);
}

static void cpu_uptime_task(lv_timer_t *arg)
{
    char buf[64] = {0};
    uint64_t uptime_sec;
    uint32_t uptime_min;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;

    get_cpu_uptime(&uptime_sec);
    uptime_min = uptime_sec / 60;

    if (uptime_min > 1440)
        snprintf(buf, sizeof(buf), "uptime %d days\n", uptime_min / 1440);
    else if (uptime_min > 60)
        snprintf(buf, sizeof(buf), "uptime %d hours\n", uptime_min / 60);
    else
        snprintf(buf, sizeof(buf), "uptime %d min\n", uptime_min);

    lv_label_set_text(text, buf);
}

static void ether_timer_task(lv_timer_t *arg)
{
    char buf[64] = {0};
    uint32_t upload_speed_bps, download_speed_bps;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;

    get_ethernet_speed(&upload_speed_bps, &download_speed_bps);

    snprintf(buf, sizeof(buf), "Up:%.2fMb/s -- Down:%.2fMb/s\n", upload_speed_bps / 1000.0 / 1000.0, download_speed_bps / 1000.0 / 1000.0);

    lv_label_set_text(text, buf);
}

static void disk_timer_task(lv_timer_t *arg)
{
    char buf[64] = {0};
    uint32_t disk_all_bytes, disk_use_bytes, disk_valid_bytes;
    uint16_t disk_use_rate;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *arc = (lv_obj_t *)timer->user_data;
    get_disk_use(&disk_all_bytes, &disk_use_bytes, &disk_valid_bytes, &disk_use_rate);


    // snprintf(buf, sizeof(buf), "Up:%.2fMb/s -- Down:%.2fMb/s\n", upload_speed_bps / 1000.0 / 1000.0, download_speed_bps / 1000.0 / 1000.0);

    lv_arc_set_value(arc, disk_use_rate);
}

static void disk_info_cb(void *obj, int32_t v)
{
    lv_arc_set_value(obj, v);
}

static void cpu_temp_info_cb(void *obj, int32_t v)
{
    lv_arc_set_angles(obj, 0, v );
}

void monitor_show(void)
{
    base_init();
    cpu_init();
    mem_init();
    disk_init();
    cpu_temp_init();
    uptime_init();
    ethernet_init();
}

void base_init(void)
{
    base_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(base_obj, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(base_obj, lv_color_hex(0x111111), LV_PART_MAIN);
}

void cpu_init(void)
{
    static lv_obj_t *cpu_obj;

    cpu_obj = lv_obj_create(base_obj);
    lv_obj_set_size(cpu_obj, 140, 95);
    lv_obj_set_style_bg_color(cpu_obj, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_border_opa(cpu_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_align(cpu_obj, LV_ALIGN_TOP_LEFT);
    lv_obj_clear_flag(cpu_obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(cpu_obj);
    lv_label_set_text(title, "CPU");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, -10, -12);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_10, LV_PART_MAIN);

    static lv_obj_t *text = NULL;

    text = lv_label_create(cpu_obj);
    lv_label_set_text(text, " ");
    lv_obj_align(text, LV_ALIGN_TOP_RIGHT, 10, -12);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_10, LV_PART_MAIN);

    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_color(&font_style, lv_color_white());
    lv_obj_add_style(title, &font_style, LV_PART_MAIN);
    lv_obj_add_style(text, &font_style, LV_PART_MAIN);

    chart_cpu = lv_chart_create(cpu_obj);
    lv_obj_set_size(chart_cpu, 130, 80);
    lv_obj_align(chart_cpu, LV_ALIGN_CENTER, 0, 8);
    lv_obj_set_style_bg_color(chart_cpu, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_border_opa(chart_cpu, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(chart_cpu, LV_OPA_COVER, LV_PART_MAIN);
    lv_chart_set_type(chart_cpu, LV_CHART_TYPE_LINE); /*Show lines and points too*/

    lv_chart_set_div_line_count(chart_cpu, 6, 0);
    lv_chart_set_point_count(chart_cpu, 60);

    lv_obj_add_event_cb(chart_cpu, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_chart_set_update_mode(chart_cpu, LV_CHART_UPDATE_MODE_SHIFT);
    lv_obj_set_style_size(chart_cpu, 0, LV_PART_INDICATOR);

    ser_cpu = lv_chart_add_series(chart_cpu, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
    lv_timer_create(cpu_timer_task, 1000, (void *)text);
}

void mem_init(void)
{
    static lv_obj_t *mem_obj;
    mem_obj = lv_obj_create(base_obj);
    lv_obj_set_size(mem_obj, 140, 95);
    lv_obj_set_style_bg_color(mem_obj, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_border_opa(mem_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_align(mem_obj, LV_ALIGN_TOP_RIGHT);
    lv_obj_clear_flag(mem_obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(mem_obj);
    lv_label_set_text(title, "MEM");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, -10, -12);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_10, LV_PART_MAIN);

    static lv_obj_t *text = NULL;
    text = lv_label_create(mem_obj);
    lv_label_set_text(text, " ");
    lv_obj_align(text, LV_ALIGN_TOP_RIGHT, 10, -12);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_10, LV_PART_MAIN);

    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_color(&font_style, lv_color_white());
    lv_obj_add_style(title, &font_style, LV_PART_MAIN);
    lv_obj_add_style(text, &font_style, LV_PART_MAIN);

    chart_mem = lv_chart_create(mem_obj);
    lv_obj_set_size(chart_mem, 130, 80);
    lv_obj_align(chart_mem, LV_ALIGN_CENTER, 0, 8);
    lv_obj_set_style_bg_color(chart_mem, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_border_opa(chart_mem, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(chart_mem, LV_OPA_COVER, LV_PART_MAIN);
    lv_chart_set_type(chart_mem, LV_CHART_TYPE_LINE); /*Show lines and points too*/

    lv_chart_set_div_line_count(chart_mem, 6, 0);
    lv_chart_set_point_count(chart_mem, 60);

    lv_obj_add_event_cb(chart_mem, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_chart_set_update_mode(chart_mem, LV_CHART_UPDATE_MODE_SHIFT);
    lv_obj_set_style_size(chart_mem, 0, LV_PART_INDICATOR);

    ser_mem = lv_chart_add_series(chart_mem, lv_palette_main(LV_PALETTE_PINK), LV_CHART_AXIS_PRIMARY_Y);
    lv_timer_create(mem_timer_task, 1000, (void*)text);
}

void disk_init(void)
{
    static lv_obj_t *disk_obj;
    disk_obj = lv_obj_create(base_obj);
    lv_obj_set_size(disk_obj, 70, 90);
    lv_obj_align(disk_obj, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_clear_flag(disk_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_opa(disk_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(disk_obj, lv_color_hex(0x222222), LV_PART_MAIN);

    lv_obj_t *arc = lv_arc_create(disk_obj);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_angles(arc, 0, 360);
    lv_arc_set_bg_angles(arc, 0, 360);

    static lv_style_t fg_style, bg_style;
    lv_style_init(&fg_style);
    lv_style_init(&bg_style);

    lv_obj_set_size(arc, 55, 55);
    lv_style_set_arc_color(&fg_style, lv_color_white());
    lv_style_set_arc_rounded(&fg_style, false);
    lv_style_set_arc_width(&fg_style, 5);

    lv_style_set_arc_color(&bg_style, lv_color_hex(0xE91E63));
    lv_style_set_arc_rounded(&bg_style, false);
    lv_style_set_arc_width(&bg_style, 5);

    lv_obj_add_style(arc, &fg_style, LV_PART_MAIN);
    lv_obj_add_style(arc, &bg_style, LV_PART_INDICATOR);

    lv_obj_remove_style(arc, NULL, LV_PART_KNOB); /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, 10);

    lv_timer_create(disk_timer_task, 1000 * 1, (void *)arc);
}

void cpu_temp_init(void)
{
    static lv_obj_t *cpu_temp_obj;
    cpu_temp_obj = lv_obj_create(base_obj);
    lv_obj_set_size(cpu_temp_obj, 70, 90);
    lv_obj_align(cpu_temp_obj, LV_ALIGN_BOTTOM_RIGHT, -75, 0);
    lv_obj_clear_flag(cpu_temp_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_opa(cpu_temp_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cpu_temp_obj, lv_color_hex(0x222222), LV_PART_MAIN);

    lv_obj_t *arc = lv_arc_create(cpu_temp_obj);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_angles(arc, 0, 360);
    lv_arc_set_bg_angles(arc, 0, 360);

    static lv_style_t fg_style, bg_style;
    lv_style_init(&fg_style);
    lv_style_init(&bg_style);

    lv_obj_set_size(arc, 55, 55);
    lv_style_set_arc_color(&fg_style, lv_color_white());
    lv_style_set_arc_rounded(&fg_style, false);
    lv_style_set_arc_width(&fg_style, 5);

    lv_style_set_arc_color(&bg_style, lv_color_hex(0x00fff3));
    lv_style_set_arc_rounded(&bg_style, false);
    lv_style_set_arc_width(&bg_style, 5);

    lv_obj_add_style(arc, &fg_style, LV_PART_MAIN);
    lv_obj_add_style(arc, &bg_style, LV_PART_INDICATOR);

    lv_obj_remove_style(arc, NULL, LV_PART_KNOB); /*Be sure the knob is not displayed*/
    lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, 10);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, cpu_temp_info_cb);
    lv_anim_set_time(&a, 3000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a, 300);
    lv_anim_set_values(&a, 0, 360);
    lv_anim_start(&a);
}

void uptime_init(void)
{
    static lv_obj_t *text = NULL;
    text = lv_label_create(base_obj);
    lv_label_set_text(text, " ");
    lv_obj_align(text, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_10, LV_PART_MAIN);

    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_color(&font_style, lv_color_white());
    lv_obj_add_style(text, &font_style, LV_PART_MAIN);

    lv_timer_create(cpu_uptime_task, 1000 * 1, (void *)text);
}

void ethernet_init(void)
{
    static lv_obj_t *text = NULL;
    text = lv_label_create(base_obj);
    lv_label_set_text(text, " ");
    lv_obj_align(text, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_10, LV_PART_MAIN);

    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_color(&font_style, lv_color_white());
    lv_obj_add_style(text, &font_style, LV_PART_MAIN);

    lv_timer_create(ether_timer_task, 1000 * 1, (void *)text);
}
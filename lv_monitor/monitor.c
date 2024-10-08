#include "monitor.h"
#include <stdio.h>
#include "info.h"

static lv_obj_t *base_obj;
static lv_obj_t *chart_cpu;
static lv_obj_t *chart_mem;
static lv_chart_series_t *ser_cpu;
static lv_chart_series_t *ser_mem;
static lv_style_t default_style;

LV_FONT_DECLARE(JetBrains_Momo_12)
LV_FONT_DECLARE(JetBrains_Momo_14)

typedef struct _disk_info_t{
    lv_obj_t *arc;
    lv_obj_t *title;
    lv_obj_t *userate;
    lv_obj_t *useinfo;
} disk_info_t;

typedef struct _temp_info_t{
    lv_obj_t *arc;
    lv_obj_t *title;
    lv_obj_t *temperature;
} temp_info_t;

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
        draw_rect_dsc.bg_opa = LV_OPA_80;
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

static void task_get_cpu_load(lv_timer_t *arg)
{
    char buffer[64] = {0};
    uint16_t cpu_rate;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;

    get_cpu_load(&cpu_rate);

    snprintf(buffer, sizeof(buffer), "%d%%", cpu_rate);
    lv_label_set_text(text, buffer);
    lv_chart_set_next_value(chart_cpu, ser_cpu, cpu_rate);
}
static void task_get_mem_load(lv_timer_t *arg)
{
    char buffer[64] = {0};
    uint16_t mem_rate;
    uint32_t use_mem_kb, total_mem_kb;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;

    get_mem_load(&use_mem_kb, &total_mem_kb, &mem_rate);

    if(total_mem_kb > (1024 * 1024))
        snprintf(buffer, sizeof(buffer), "%.1fGB/%.1fGB", use_mem_kb / 1024.0 / 1024.0, total_mem_kb / 1024.0 / 1024.0);
    else
        snprintf(buffer, sizeof(buffer), "%.0fMB/%.0fMB", use_mem_kb / 1024.0, total_mem_kb / 1024.0);

    lv_label_set_text(text, buffer);
    lv_chart_set_next_value(chart_mem, ser_mem, mem_rate);
}

static void task_get_uptime(lv_timer_t *arg)
{
    char buffer[64] = {0};
    uint64_t uptime_sec;
    uint32_t uptime_min;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;

    get_cpu_uptime(&uptime_sec);
    uptime_min = uptime_sec / 60;

    if (uptime_min > 1440)
        snprintf(buffer, sizeof(buffer), "%d days", uptime_min / 1440);
    else if (uptime_min > 60)
        snprintf(buffer, sizeof(buffer), "%d hours", uptime_min / 60);
    else
        snprintf(buffer, sizeof(buffer), "%d min", uptime_min);

    lv_label_set_text(text, buffer);
}

static void task_get_net_speed(lv_timer_t *arg)
{
    char buffer_upload[64] = {0};
    char buffer_download[64] = {0};
    uint64_t upload_speed_bps, download_speed_bps;
    float upload_speed_mps, download_speed_mps;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;
    lv_text_decor_t text_decor;

    text_decor = lv_obj_get_style_text_decor(text, LV_PART_MAIN);
    if (ret_ok != get_network_speed(&upload_speed_bps, &download_speed_bps))
    {
        if(!(text_decor & LV_TEXT_DECOR_STRIKETHROUGH))
            lv_obj_set_style_text_decor(text, LV_TEXT_DECOR_STRIKETHROUGH, LV_PART_MAIN);
        upload_speed_mps = 0;
        download_speed_mps = 0;
    } else {
        if(!(text_decor & LV_TEXT_DECOR_NONE))
            lv_obj_set_style_text_decor(text, LV_TEXT_DECOR_NONE, LV_PART_MAIN);
        upload_speed_mps = upload_speed_bps / 1000.0 / 1000.0;
        download_speed_mps = download_speed_bps / 1000.0 / 1000.0;
    }

    if(upload_speed_mps >= 10.0 )
        snprintf(buffer_upload, sizeof(buffer_upload), "#009080 "LV_SYMBOL_UPLOAD "# %-3dMb/s ", (int)upload_speed_mps);
    else
        snprintf(buffer_upload, sizeof(buffer_upload), "#009080 "LV_SYMBOL_UPLOAD "# %.1fMb/s ", upload_speed_mps);

    if(download_speed_mps >= 10.0)
        snprintf(buffer_download, sizeof(buffer_download), "#0060f0 "LV_SYMBOL_DOWNLOAD "# %-3dMb/s", (int)download_speed_mps);
    else
        snprintf(buffer_download, sizeof(buffer_download), "#0060f0 " LV_SYMBOL_DOWNLOAD "# %.1fMb/s", download_speed_mps);

    lv_label_set_text_fmt(text, "%s%s", buffer_upload, buffer_download);
}

static void task_get_disk_load(lv_timer_t *arg)
{
    char buffer[64] = {0};
    uint32_t disk_all_kb, disk_use_kb, disk_valid_kb;
    uint16_t disk_use_rate;
    lv_timer_t *timer = (lv_timer_t *)arg;
    disk_info_t *disk_info = (disk_info_t *)(timer->user_data);

    get_disk_use(&disk_all_kb, &disk_use_kb, &disk_valid_kb, &disk_use_rate);

    lv_arc_set_value(disk_info->arc, disk_use_rate);

    snprintf(buffer, sizeof(buffer), "#E91E63 %.1fGB#\n%.1fGB",
             disk_use_kb / 1024.0 / 1024.0,
             disk_valid_kb / 1024.0 / 1024.0);
    lv_label_set_text(disk_info->useinfo, buffer );
    lv_label_set_text_fmt(disk_info->userate, "%d%%", disk_use_rate);
}

static void task_get_temperature(lv_timer_t *arg)
{
    uint32_t temp = 0;
    lv_timer_t *timer = (lv_timer_t *)arg;
    temp_info_t *temp_info = (temp_info_t *)timer->user_data;
    lv_text_decor_t text_decor;
    text_decor = lv_obj_get_style_text_decor(temp_info->temperature, LV_PART_MAIN);
    if (ret_ok != get_cpu_temperature(&temp))
    {
        if(!(text_decor & LV_TEXT_DECOR_STRIKETHROUGH))
            lv_obj_set_style_text_decor(temp_info->temperature, LV_TEXT_DECOR_STRIKETHROUGH, LV_PART_MAIN);
        temp = lv_rand(30, 50);
    }
    else
    {
        if(!(text_decor & LV_TEXT_DECOR_NONE))
            lv_obj_set_style_text_decor(temp_info->temperature, LV_TEXT_DECOR_NONE, LV_PART_MAIN);
    }
    lv_arc_set_value(temp_info->arc, temp);
    lv_arc_set_value(temp_info->arc, temp);
    lv_label_set_text_fmt(temp_info->temperature, "CPU:#00ffff %d#'C", temp);
}

static void task_get_time(lv_timer_t *arg)
{
    uint16_t hour = 0;
    uint16_t min = 0;
    uint16_t sec = 0;
    static bool colon = false;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;

    char buffer[64];
    get_current_time(&hour, &min, &sec);
    if(colon)
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hour, min, sec);
    else
        snprintf(buffer, sizeof(buffer), "%02d#E91E63 :#%02d#ff4444 :#%02d", hour, min, sec);
    colon=!colon;
    lv_label_set_text(text, buffer);
}

static void task_count_process(lv_timer_t *arg)
{
    char buffer[128];
    uint32_t num_process=0, num_thread=0, num_zombie=0;
    lv_timer_t *timer = (lv_timer_t *)arg;
    lv_obj_t *text = (lv_obj_t *)timer->user_data;

    get_task_num(&num_process, &num_thread, &num_zombie);
    lv_label_set_text_fmt(text, "#0060f0 Process:#%d\n#00f060 Threads:#%d\n#aaaaaa Zombies:#%d", num_process, num_thread, num_zombie);
}

void lv_monitor_show(void)
{
    lv_monitor_base_init();
    lv_monitor_cpu_init();
    lv_monitor_mem_init();
    lv_monitor_disk_init();
    lv_monitor_thermal_init();
    lv_monitor_task_counter_init();
    lv_monitor_time_init();
    lv_monitor_uptime_init();
    lv_monitor_network_init();
}

void lv_monitor_base_init(void)
{
    base_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(base_obj, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(base_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(base_obj, lv_color_hex(0x111111), LV_PART_MAIN);

    lv_style_init(&default_style);
    lv_style_set_text_color(&default_style, lv_color_white());
    lv_style_set_text_font(&default_style, &JetBrains_Momo_12);
}

void lv_monitor_cpu_init(void)
{
    static lv_obj_t *cpu_obj = NULL;
    cpu_obj = lv_obj_create(base_obj);
    lv_obj_set_size(cpu_obj, CPU_LABLE_SIZE_X, TOP_LABLE_SIZE_Y);
    lv_obj_set_style_bg_color(cpu_obj, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_border_opa(cpu_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(cpu_obj, LV_ALIGN_TOP_LEFT, 0, -5);
    lv_obj_clear_flag(cpu_obj, LV_OBJ_FLAG_SCROLLABLE);

    static lv_obj_t *title = NULL;
    title = lv_label_create(cpu_obj);
    lv_label_set_text(title, "CPU");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, LABEL_TITLE_OFFSET_X, LABEL_TITLE_OFFSET_Y);
    lv_obj_add_style(title, &default_style, LV_PART_MAIN);

    static lv_obj_t *text = NULL;
    text = lv_label_create(cpu_obj);
    lv_label_set_text(text, " ");
    lv_obj_align(text, LV_ALIGN_TOP_RIGHT, LABEL_MSG_OFFSET_X, LABEL_MSG_OFFSET_Y);
    lv_obj_add_style(text, &default_style, LV_PART_MAIN);

    chart_cpu = lv_chart_create(cpu_obj);
    lv_obj_set_size(chart_cpu, CHART_SIZE_X, CHART_SIZE_Y);
    lv_obj_align(chart_cpu, LV_ALIGN_BOTTOM_MID, LABEL_CHART_OFFSET_X, LABEL_CHART_OFFSET_Y);
    lv_obj_set_style_bg_color(chart_cpu, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_border_opa(chart_cpu, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(chart_cpu, LV_OPA_COVER, LV_PART_MAIN);
    lv_chart_set_type(chart_cpu, LV_CHART_TYPE_LINE);

    lv_chart_set_div_line_count(chart_cpu, 6, 0);
    lv_chart_set_point_count(chart_cpu, 60);

    lv_obj_add_event_cb(chart_cpu, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_chart_set_update_mode(chart_cpu, LV_CHART_UPDATE_MODE_SHIFT);
    lv_obj_set_style_size(chart_cpu, 0, LV_PART_INDICATOR);

    ser_cpu = lv_chart_add_series(chart_cpu, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    lv_timer_t *timer = lv_timer_create(task_get_cpu_load, 1000, (void *)text);
    task_get_cpu_load(timer);
}

void lv_monitor_mem_init(void)
{
    static lv_obj_t *mem_obj = NULL;
    mem_obj = lv_obj_create(base_obj);
    lv_obj_set_size(mem_obj, MEM_LABLE_SIZE_X, TOP_LABLE_SIZE_Y);
    lv_obj_set_style_bg_color(mem_obj, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_border_opa(mem_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(mem_obj, LV_ALIGN_TOP_RIGHT, 0, -5);
    lv_obj_clear_flag(mem_obj, LV_OBJ_FLAG_SCROLLABLE);

    static lv_obj_t *title = NULL;
    title = lv_label_create(mem_obj);
    lv_label_set_text(title, "MEM");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, LABEL_TITLE_OFFSET_X, LABEL_TITLE_OFFSET_Y);

    static lv_obj_t *text = NULL;
    text = lv_label_create(mem_obj);
    lv_label_set_text(text, " ");
    lv_obj_align(text, LV_ALIGN_TOP_RIGHT, LABEL_MSG_OFFSET_X, LABEL_MSG_OFFSET_Y);

    lv_obj_add_style(title, &default_style, LV_PART_MAIN);
    lv_obj_add_style(text, &default_style, LV_PART_MAIN);

    chart_mem = lv_chart_create(mem_obj);
    lv_obj_set_size(chart_mem, CHART_SIZE_X, CHART_SIZE_Y);
    lv_obj_align(chart_mem, LV_ALIGN_BOTTOM_MID, LABEL_CHART_OFFSET_X, LABEL_CHART_OFFSET_Y);
    lv_obj_set_style_bg_color(chart_mem, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_border_opa(chart_mem, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(chart_mem, LV_OPA_COVER, LV_PART_MAIN);
    lv_chart_set_type(chart_mem, LV_CHART_TYPE_LINE); /*Show lines and points too*/

    lv_chart_set_div_line_count(chart_mem, 6, 0);
    lv_chart_set_point_count(chart_mem, 60);

    lv_obj_add_event_cb(chart_mem, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_chart_set_update_mode(chart_mem, LV_CHART_UPDATE_MODE_SHIFT);
    lv_obj_set_style_size(chart_mem, 0, LV_PART_INDICATOR);

    ser_mem = lv_chart_add_series(chart_mem, lv_palette_main(LV_PALETTE_PINK), LV_CHART_AXIS_PRIMARY_Y);
    lv_timer_t *timer = lv_timer_create(task_get_mem_load, 1000, (void*)text);
    task_get_mem_load(timer);

}

void lv_monitor_disk_init(void)
{
    static lv_obj_t *disk_obj = NULL;
    disk_obj = lv_obj_create(base_obj);
    lv_obj_set_size(disk_obj, DISK_LABLE_SIZE_X, MID_LABLE_SIZE_Y);
    lv_obj_align(disk_obj, LV_ALIGN_LEFT_MID, -2, (MID_LABLE_SIZE_Y - BOT_LABLE_SIZE_Y) / 2);
    lv_obj_clear_flag(disk_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_opa(disk_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(disk_obj, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);

    static disk_info_t disk_info;
    memset(&disk_info, 0x00, sizeof(disk_info_t));
    disk_info.arc = lv_arc_create(disk_obj);
    lv_arc_set_rotation(disk_info.arc, 270);
    lv_arc_set_angles(disk_info.arc, 0, 360);
    lv_arc_set_bg_angles(disk_info.arc, 0, 360);
    lv_obj_set_size(disk_info.arc, LABEL_ARC_SIZE_X, LABEL_ARC_SIZE_Y);

    static lv_style_t fg_style, bg_style;
    lv_style_init(&fg_style);
    lv_style_init(&bg_style);

    lv_style_set_arc_color(&fg_style, lv_color_white());
    lv_style_set_arc_rounded(&fg_style, false);
    lv_style_set_arc_width(&fg_style, LABEL_ARC_WIDTH);

    lv_style_set_arc_color(&bg_style, lv_color_hex(0xE91E63));
    lv_style_set_arc_rounded(&bg_style, false);
    lv_style_set_arc_width(&bg_style, LABEL_ARC_WIDTH);

    lv_obj_add_style(disk_info.arc, &fg_style, LV_PART_MAIN);
    lv_obj_add_style(disk_info.arc, &bg_style, LV_PART_INDICATOR);
    lv_obj_remove_style(disk_info.arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(disk_info.arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(disk_info.arc, LV_ALIGN_BOTTOM_MID, 0, 10);

    disk_info.title = lv_label_create(disk_obj);
    lv_label_set_text(disk_info.title, "Disk");
    lv_obj_align(disk_info.title, LV_ALIGN_TOP_LEFT, LABEL_TITLE_OFFSET_X, LABEL_TITLE_OFFSET_Y);

    disk_info.useinfo = lv_label_create(disk_obj);
    lv_label_set_text(disk_info.useinfo, "");
    lv_obj_align(disk_info.useinfo, LV_ALIGN_CENTER, 0, MID_LABLE_SIZE_Y - DISK_LABLE_SIZE_X);
    lv_obj_set_style_text_align(disk_info.useinfo, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_recolor(disk_info.useinfo, true);

    disk_info.userate = lv_label_create(disk_obj);
    lv_label_set_text(disk_info.userate, "");
    lv_obj_align(disk_info.userate, LV_ALIGN_TOP_RIGHT, LABEL_MSG_OFFSET_X, LABEL_MSG_OFFSET_Y);
    lv_obj_set_style_text_align(disk_info.userate, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_recolor(disk_info.userate, true);

    lv_obj_add_style(disk_info.title, &default_style, LV_PART_MAIN);
    lv_obj_add_style(disk_info.useinfo, &default_style, LV_PART_MAIN);
    lv_obj_add_style(disk_info.userate, &default_style, LV_PART_MAIN);

    lv_timer_t *timer = lv_timer_create(task_get_disk_load, 1000 * 10, (void *)&disk_info);
    task_get_disk_load(timer);
}

void lv_monitor_thermal_init(void)
{
    static lv_obj_t *cpu_temp_obj = NULL;
    cpu_temp_obj = lv_obj_create(base_obj);
    lv_obj_set_size(cpu_temp_obj, THERMAL_LABLE_SIZE_X, MID_LABLE_SIZE_Y);
    lv_obj_align(cpu_temp_obj, LV_ALIGN_CENTER, -2, (MID_LABLE_SIZE_Y - BOT_LABLE_SIZE_Y) / 2);
    lv_obj_clear_flag(cpu_temp_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_opa(cpu_temp_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cpu_temp_obj, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);

    static temp_info_t temp_info;
    memset(&temp_info, 0x00, sizeof(temp_info_t));
    temp_info.arc = lv_arc_create(cpu_temp_obj);
    lv_arc_set_rotation(temp_info.arc, 270);
    lv_arc_set_angles(temp_info.arc, 0, 360);
    lv_arc_set_bg_angles(temp_info.arc, 0, 360);

    static lv_style_t fg_style, bg_style;
    lv_style_init(&fg_style);
    lv_style_init(&bg_style);

    lv_obj_set_size(temp_info.arc, LABEL_ARC_SIZE_X, LABEL_ARC_SIZE_Y);
    lv_style_set_arc_color(&fg_style, lv_color_white());
    lv_style_set_arc_rounded(&fg_style, false);
    lv_style_set_arc_width(&fg_style, LABEL_ARC_WIDTH);

    lv_style_set_arc_color(&bg_style, lv_color_hex(0x00ffff));
    lv_style_set_arc_rounded(&bg_style, false);
    lv_style_set_arc_width(&bg_style, LABEL_ARC_WIDTH);

    lv_obj_add_style(temp_info.arc, &fg_style, LV_PART_MAIN);
    lv_obj_add_style(temp_info.arc, &bg_style, LV_PART_INDICATOR);

    temp_info.title = lv_label_create(cpu_temp_obj);
    lv_label_set_text(temp_info.title, "Thermal");
    lv_obj_align(temp_info.title, LV_ALIGN_TOP_LEFT, LABEL_TITLE_OFFSET_X, LABEL_TITLE_OFFSET_Y);
    lv_obj_add_style(temp_info.title, &default_style, LV_PART_MAIN);

    temp_info.temperature = lv_label_create(cpu_temp_obj);
    lv_label_set_text(temp_info.temperature, " ");
    lv_obj_align(temp_info.temperature, LV_ALIGN_CENTER, 0, MID_LABLE_SIZE_Y - THERMAL_LABLE_SIZE_X);
    lv_label_set_recolor(temp_info.temperature, true);
    lv_obj_add_style(temp_info.temperature, &default_style, LV_PART_MAIN);

    lv_obj_remove_style(temp_info.arc, NULL, LV_PART_KNOB); /*Be sure the knob is not displayed*/
    lv_obj_align(temp_info.arc, LV_ALIGN_BOTTOM_MID, 0, 10);

    lv_timer_t *timer = lv_timer_create(task_get_temperature, 1000 * 3, (void *)&temp_info);
    task_get_temperature(timer);
}

void lv_monitor_task_counter_init(void)
{
    static lv_obj_t *task_obj = NULL;
    task_obj = lv_obj_create(base_obj);
    lv_obj_set_size(task_obj, TASK_LABLE_SIZE_X, MID_LABLE_SIZE_Y);
    lv_obj_align(task_obj, LV_ALIGN_RIGHT_MID, 0, (MID_LABLE_SIZE_Y - BOT_LABLE_SIZE_Y) / 2);
    lv_obj_clear_flag(task_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_opa(task_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(task_obj, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);

    static lv_obj_t *title = NULL;;
    title = lv_label_create(task_obj);
    lv_label_set_text(title, "Task");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, LABEL_TITLE_OFFSET_X, LABEL_TITLE_OFFSET_Y);

    static lv_obj_t *proc_info = NULL;
    proc_info = lv_label_create(task_obj);
    lv_label_set_text(proc_info, " ");
    lv_label_set_recolor(proc_info, true);
    lv_obj_align(proc_info, LV_ALIGN_CENTER, 0, lv_obj_get_height(title));

    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_color(&font_style, lv_color_white());
    lv_style_set_text_line_space(&font_style, MID_LABLE_SIZE_Y/11);
    lv_style_set_text_font(&font_style, &JetBrains_Momo_14);
    lv_obj_add_style(title, &default_style, LV_PART_MAIN);
    lv_obj_add_style(proc_info, &font_style, LV_PART_MAIN);

    lv_timer_t *timer = lv_timer_create(task_count_process, 1000 * 3, (void *)proc_info);
    task_count_process(timer);
}

void lv_monitor_time_init(void)
{
    static lv_obj_t *other_obj = NULL;
    other_obj = lv_obj_create(base_obj);
    lv_obj_set_size(other_obj, TIME_LABLE_SIZE_X, BOT_LABLE_SIZE_Y);
    lv_obj_set_style_bg_color(other_obj, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_border_opa(other_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(other_obj, LV_ALIGN_BOTTOM_LEFT, 0, 4);
    lv_obj_clear_flag(other_obj, LV_OBJ_FLAG_SCROLLABLE);

    static lv_obj_t *text = NULL;
    text = lv_label_create(base_obj);
    lv_label_set_text(text, "");
    lv_label_set_recolor(text, true);
    lv_obj_align(text, LV_ALIGN_BOTTOM_LEFT, 5, 0);
    lv_obj_add_style(text, &default_style, LV_PART_MAIN);

    lv_timer_t *timer = lv_timer_create(task_get_time, 1000 * 1, (void *)text);
    task_get_time(timer);
}

void lv_monitor_network_init(void)
{
    static lv_obj_t *ethernet_obj = NULL;
    ethernet_obj = lv_obj_create(base_obj);
    lv_obj_set_size(ethernet_obj, NET_LABLE_SIZE_X, BOT_LABLE_SIZE_Y);
    lv_obj_set_style_bg_color(ethernet_obj, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_border_opa(ethernet_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(ethernet_obj, LV_ALIGN_BOTTOM_MID, 0, 4);
    lv_obj_clear_flag(ethernet_obj, LV_OBJ_FLAG_SCROLLABLE);

    static lv_obj_t *text = NULL;
    text = lv_label_create(base_obj);
    lv_label_set_text(text, " ");
    lv_label_set_recolor(text, true);
    lv_obj_align(text, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(text, &default_style, LV_PART_MAIN);

    lv_timer_t *timer = lv_timer_create(task_get_net_speed, 1000 * 1, (void *)text);
    task_get_net_speed(timer);
}

void lv_monitor_uptime_init(void)
{
    static lv_obj_t *other_obj = NULL;
    other_obj = lv_obj_create(base_obj);
    lv_obj_set_size(other_obj, UPTIME_LABLE_SIZE_X, BOT_LABLE_SIZE_Y);
    lv_obj_set_style_bg_color(other_obj, lv_color_hex(OBJ_BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_border_opa(other_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(other_obj, LV_ALIGN_BOTTOM_RIGHT, 0, 4);
    lv_obj_clear_flag(other_obj, LV_OBJ_FLAG_SCROLLABLE);

    static lv_obj_t *text = NULL;
    text = lv_label_create(base_obj);
    lv_label_set_text(text, " ");
    lv_obj_set_width(text, UPTIME_LABLE_SIZE_X);
    lv_obj_align(text, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_text_align(text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_style(text, &default_style, LV_PART_MAIN);
    lv_obj_set_style_text_font(text, &JetBrains_Momo_14, LV_PART_MAIN);
    lv_timer_t *timer = lv_timer_create(task_get_uptime, 1000 * 30, (void *)text);
    task_get_uptime(timer);
}
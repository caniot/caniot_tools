
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/periph_ctrl.h"
#include "esp_timer.h"
#include "main_if.h"
#include "lvgl_if.h"

#ifndef _APPL_LVGL_
#define _APPL_LVGL_

extern QueueHandle_t mqtt_hiveMQ_queue;
static lv_obj_t *main_container;

static lv_obj_t *bnt_publisch_ota;

static lv_style_t style_box;

static void bnt_publisch_event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		uint32_t diagnose_event = 1;
		xQueueSend(mqtt_hiveMQ_queue, &diagnose_event, 2);
	}
}
void lvgl_create_publish_container(lv_obj_t *parent)
{
	/* lvgl docu is available on https://docs.lvgl.io/7.11/ */
	lv_style_init(&style_box);
	lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
	lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, -LV_DPX(10));
	lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(30));
	lv_style_set_margin_bottom(&style_box, LV_STATE_DEFAULT, LV_DPX(30));

	
	main_container = lv_cont_create(parent, NULL);

	lv_cont_set_layout(main_container, LV_LAYOUT_GRID);
	lv_obj_add_style(main_container, LV_CONT_PART_MAIN, &style_box);
	lv_obj_set_drag_parent(main_container, true);

	lv_obj_set_style_local_value_str(main_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "OTA Publishing:");

	lv_cont_set_fit2(main_container, LV_FIT_NONE, LV_FIT_TIGHT);
	lv_obj_set_width(main_container, lv_page_get_width_grid(parent, 1, 1));

	bnt_publisch_ota = lv_btn_create(main_container, NULL);
	lv_obj_set_event_cb(bnt_publisch_ota, bnt_publisch_event_handler);
	lv_btn_set_fit2(bnt_publisch_ota, LV_FIT_NONE, LV_FIT_TIGHT);
	lv_obj_set_width(bnt_publisch_ota, lv_obj_get_width_grid(main_container, 1, 1));
	lv_obj_t * label = lv_label_create(bnt_publisch_ota, NULL);
	lv_label_set_text(label, "Publich OTA");

	lv_obj_move_background(main_container);
}
#endif
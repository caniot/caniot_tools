
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

#ifndef _CAN_LVGL_
#define _CAN_LVGL_

static lv_obj_t *main_container;
static lv_obj_t *led_LED0;
static lv_obj_t *led_LED;
static lv_obj_t *label_LED0;
static lv_obj_t *label_LED;

static char *lcd_buff = NULL;
QueueHandle_t status_queue = NULL;
static lv_style_t style_box;

static void can_status_task(lv_task_t *t)
{
	appl_message_t can_event;

	if (xQueueReceive(status_queue, (void *)&can_event, 0))
	{
		switch (can_event.event)
		{
		case 0:
		{

			if (can_event.bufferPtr[0] == 0)
			{
				lv_led_off(led_LED0);
			}
			else
			{
				lv_led_on(led_LED0);
			}
		}
		break;
		case 1:
		{
			if (can_event.bufferPtr[0] == 0)
			{
				lv_led_off(led_LED);
			}
			else
			{
				lv_led_on(led_LED);
			}
		}
		break;

			break;
		default:
			break;
		}
	}
}

void lvgl_create_CAN_container(lv_obj_t *parent)
{
	status_queue = xQueueCreate(100, sizeof(appl_message_t));
	lcd_buff = calloc(1, 240);

	/* lvgl docu is available on https://docs.lvgl.io/7.11/ */
	lv_style_init(&style_box);
	lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
	lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, -LV_DPX(10));
	lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(30));
	lv_style_set_margin_bottom(&style_box, LV_STATE_DEFAULT, LV_DPX(30));

	lv_coord_t grid_w = lv_page_get_width_grid(parent, 2, 1);

	main_container = lv_cont_create(parent, NULL);

	lv_cont_set_layout(main_container, LV_LAYOUT_GRID);
	lv_obj_add_style(main_container, LV_CONT_PART_MAIN, &style_box);
	lv_obj_set_drag_parent(main_container, true);

	lv_obj_set_style_local_value_str(main_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "CAN Status:");

	lv_cont_set_fit2(main_container, LV_FIT_NONE, LV_FIT_TIGHT);
	lv_obj_set_width(main_container, lv_page_get_width_grid(parent, 1, 1));

	label_LED0 = lv_label_create(main_container, NULL);
	lv_label_set_text(label_LED0, "LED0 status:");
	lv_obj_set_width(label_LED0, grid_w);

	led_LED0 = lv_led_create(main_container, NULL);
	lv_coord_t led_size = lv_obj_get_height_fit(label_LED0);
	lv_obj_align(led_LED0, label_LED0, LV_ALIGN_CENTER, 80, 0);
	lv_obj_set_size(led_LED0, led_size, led_size);
	lv_obj_set_style_local_bg_color(led_LED0, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
	lv_obj_set_style_local_shadow_color(led_LED0, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
	lv_led_off(led_LED0);

	label_LED = lv_label_create(main_container, NULL);
	lv_label_set_text(label_LED, "LED1 status:");
	lv_obj_set_width(label_LED, grid_w);

	led_LED = lv_led_create(main_container, NULL);
	led_size = lv_obj_get_height_fit(label_LED);
	lv_obj_align(led_LED, label_LED, LV_ALIGN_CENTER, 80, 0);
	lv_obj_set_size(led_LED, led_size, led_size);
	lv_obj_set_style_local_bg_color(led_LED, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
	lv_obj_set_style_local_shadow_color(led_LED, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
	lv_led_off(led_LED);

	lv_task_create(can_status_task, 30, LV_TASK_PRIO_LOW, NULL);

	lv_obj_move_background(main_container);
}
#endif
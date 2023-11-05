
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
#include "uds_service_fnc.h"

#ifndef _CAN_LVGL_
#define _CAN_LVGL_

static lv_obj_t *main_container = NULL;
static lv_obj_t *bar_flash;

QueueHandle_t FlashLoader_queue = NULL;
static lv_style_t style_box;

static void flashLoader_status_task(lv_task_t *t)
{
	static char tft_bufferTemp[256];
	static int total_byteToFls;
	appl_message_t flashLoader_event;

	if (xQueueReceive(FlashLoader_queue, (void *)&flashLoader_event, 0))
	{
		switch (flashLoader_event.event)
		{
		case 0:
		{

			total_byteToFls = flashLoader_event.length;
			lv_obj_set_style_local_value_str(bar_flash, LV_BAR_PART_BG, LV_STATE_DEFAULT, "wait for start");
			lv_bar_set_range(bar_flash, 0, 100);
			lv_bar_set_value(bar_flash, 0, LV_ANIM_OFF);
		}
		break;
		case 1:
		{

			int b = sprintf(tft_bufferTemp, "%d bytes of %d bytes\nUpdate Progress %d",flashLoader_event.length, total_byteToFls, (flashLoader_event.length * 100 / total_byteToFls));
			tft_bufferTemp[b] = '%';	  /*add %*/
			tft_bufferTemp[b + 1] = '\0'; /*add 0 character*/

			lv_obj_set_style_local_value_str(bar_flash, LV_BAR_PART_BG, LV_STATE_DEFAULT, tft_bufferTemp);
			lv_bar_set_value(bar_flash, (flashLoader_event.length * 100 / total_byteToFls), LV_ANIM_OFF);
		}
		break;

		case 2:
		{
			lv_bar_set_value(bar_flash, 0, LV_ANIM_OFF);
			lv_obj_set_style_local_value_str(bar_flash, LV_BAR_PART_BG, LV_STATE_DEFAULT, "UDS Service is finished");
		}
		break;
		case 3:
		{
			lv_bar_set_value(bar_flash, 0, LV_ANIM_OFF);
			lv_obj_set_style_local_value_str(bar_flash, LV_BAR_PART_BG, LV_STATE_DEFAULT, "UDS Service error");
		}
		break;

		default:
			break;
		}
	}
}
void lvgl_delete_container(lv_obj_t *parent)
{
	if (main_container != NULL)
	{

		lv_obj_del(main_container);
		main_container = NULL;
		xQueueReset(FlashLoader_queue);
	}
}
void lvgl_create_bar_container(lv_obj_t *parent)
{
	if (main_container == NULL)
	{
		FlashLoader_queue = xQueueCreate(100, sizeof(appl_message_t));

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

		lv_obj_set_style_local_value_str(main_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Flash Bar:");

		lv_cont_set_fit2(main_container, LV_FIT_NONE, LV_FIT_TIGHT);
		lv_obj_set_width(main_container, lv_page_get_width_grid(parent, 1, 1));

		bar_flash = lv_bar_create(main_container, NULL);
		lv_obj_set_width(bar_flash, lv_obj_get_width_fit(main_container));
		lv_obj_set_style_local_value_font(bar_flash, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_theme_get_font_small());
		lv_obj_set_style_local_value_align(bar_flash, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_style_local_value_ofs_y(bar_flash, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 20);
		lv_obj_set_style_local_margin_bottom(bar_flash, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 7);
		lv_obj_align(bar_flash, NULL, LV_ALIGN_CENTER, 0, 0);

		lv_task_create(flashLoader_status_task, 30, LV_TASK_PRIO_LOW, NULL);

		lv_obj_move_background(main_container);
	}
}
void lvgl_move_background_container()
{
	lvgl_if_take();
    lv_page_focus(main_table,main_container,LV_ANIM_OFF);
    lvgl_if_give();
	
}
#endif
/* CAN application example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "main_if.h"
#include "lin_if.h"
#include "lvgl_if.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_heap_caps.h"

extern void lvgl_create_bar_container(lv_obj_t *parent);
extern void UdsFlash_mainFunction(const char *filename, const char *uds_attr);
extern QueueHandle_t actions_queue;

static void Flash_Task(void *filename)
{
  const char *filenameArg = (const char *)filename;
  if (main_table != NULL)
  {
    lvgl_if_take();
    lvgl_create_bar_container(main_table);
    lvgl_if_give();
  }
  UdsFlash_mainFunction(filenameArg, "UDS");

  vTaskDelete(NULL);
}
void caniot_actionsTask(void *TaskArg)
{
  int message_actions = -1;
  while (1)
  {
    if (xQueueReceive(actions_queue, &message_actions, portMAX_DELAY))
    {

      switch (message_actions)
      {

      case 1:
      {
        xTaskCreatePinnedToCore(Flash_Task, "Flash_Task1", 4096 * 3, (void *)"/sdcard/uds_ota_flashLoader/S32K1_uds_flash_CAN_blueLed.json", 15, NULL, 0);
      }
      break;
      case 2:
      {
      }
      break;
      case 3:
      {
      }
      break;
      case 4:
      {
      }
      break;
      case 5:
      {
      }
      break;
      case 6:
      {

        break;
      }
      default:
      {
      }
      break;
      }
    }
  }
  vTaskDelete(NULL);
}

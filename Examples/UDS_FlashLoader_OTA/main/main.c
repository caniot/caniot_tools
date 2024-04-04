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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_heap_caps.h"
#include "lvgl_if.h"

extern void caniot_actionsTask(void *TaskArg);
extern void canAppl_init(void);

void send_caniot_data(caniot_message_t *message_arg)
{
  uint8_t caniot_data_length;
  uint8_t *receiveData = prepare_caniot_data(message_arg, &caniot_data_length);
  send_to_caniot_appl((char *)receiveData, caniot_data_length);
  free(receiveData);
}

void app_main(void)
{

  caniotBox_init();

  xTaskCreate(caniot_actionsTask, "caniot_actionsTask", 3020, (void *)0, 15, NULL);

  esp_err_t uart_err = serialOutput_configure(115200);
  assert(uart_err == ESP_OK);

  /*Beginn of application code*/
  uint8_t can_lin_variant;
  nvsIf_get_can_lin_var(&can_lin_variant);
  serialOutput_sendString("%sstart of application %d\n", COLOR_GREEN, can_lin_variant);

  if (can_lin_variant == 0)
  {
  }
  else if (can_lin_variant == 1)
  {
  }
  else
  {
    serialOutput_sendString("unsupported variant %d", can_lin_variant);
  }
}

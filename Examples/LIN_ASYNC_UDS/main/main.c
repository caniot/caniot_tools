/* Template example.
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
#include "nvs_flash.h"
extern void caniot_actionsTask(void *clientSock);
extern void diag_init();
static  const char *nvs_part_name = "LIN_ASYNC_UDS";


void send_caniot_data(caniot_message_t *message_arg)
{
  uint8_t caniot_data_length;
  uint8_t *receiveData = prepare_caniot_data(message_arg, &caniot_data_length);
  send_to_caniot_appl((char *)receiveData, caniot_data_length);
  free(receiveData);
}

void appl_erase_nvs_partition(void)
{
  nvs_flash_erase_partition(nvs_part_name);
}
esp_err_t appl_set_nvs_variant(uint8_t nvs_var)
{
  nvs_handle my_handle;
  esp_err_t err;

  // Open
  err = nvs_open(nvs_part_name, NVS_READWRITE, &my_handle);
  err = nvs_set_u8(my_handle, "LDF_EXIST", nvs_var);

  if (err != ESP_OK)
    return err;

  // Commit
  err = nvs_commit(my_handle);
  if (err != ESP_OK)
    return err;

  // Close
  nvs_close(my_handle);
  return ESP_OK;
}
esp_err_t appl_get_nvs_variant(uint8_t *nvs_var)
{
  nvs_handle my_handle;
  esp_err_t err;

  // Open
  err = nvs_open(nvs_part_name, NVS_READONLY, &my_handle);
  err = nvs_get_u8(my_handle, "LDF_EXIST", nvs_var);

  if (err != ESP_OK)
    return err;

  // Commit
  err = nvs_commit(my_handle);
  if (err != ESP_OK)
    return err;

  // Close
  nvs_close(my_handle);
  return ESP_OK;
}
void app_main(void)
{

  caniotBox_init();

  xTaskCreate(caniot_actionsTask, "caniot_actionsTask", 3020, (void *)0, 15, NULL);

  esp_err_t uart_err = serialOutput_configure(115200);

  /*Beginn of application code*/

  uint8_t nvs_var;
  appl_get_nvs_variant(&nvs_var);
  if(nvs_var != 0x5A)
  {
    appl_set_nvs_variant(0x5A);
    caniotBox_set_mode(1);/* simulation mode standalone */
    nvsIf_loadConfig("/sdcard/SimulationFiles/demo_lin.json");
    esp_restart();
  }


  uint8_t can_lin_variant;
  nvsIf_get_can_lin_var(&can_lin_variant);
  serialOutput_sendString("%sstart of application %d\n", COLOR_GREEN, can_lin_variant);

  if (can_lin_variant == 0)
  {
  }
  else if (can_lin_variant == 1)
  {
    diag_init();
  }
  else
  {
    serialOutput_sendString("unsupported variant %d", can_lin_variant);
  }
}

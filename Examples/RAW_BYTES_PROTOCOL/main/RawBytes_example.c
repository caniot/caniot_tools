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

#define RAW_BYTES_TX 0
#define RAW_BYTES_RX 1

extern void send_caniot_data(caniot_message_t *message_arg);
#define APPL_COLOR_SIZE 6
static char *APPL_COLOR[APPL_COLOR_SIZE] = {COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE, COLOR_YELLOW, COLOR_BLUE, COLOR_GREEN};
static uint8_t appl_color_counter = 0;

static void send_caniot_protocol_data_back(uint8_t *data, uint16_t length)
{
  caniot_message_t mycaniot_message;
  if (length < 64)
  {
    mycaniot_message.TAG_ID = 0xFFFFFFFFU;
    mycaniot_message.length = (uint8_t)length & 0xFFU; /*maximum 64*/
    mycaniot_message.bufferPtr = data;
    send_caniot_data(&mycaniot_message);
  }
  else
  {
    for (uint16_t i = 0; i < length; i += 64)
    {

      mycaniot_message.TAG_ID = 0xFFFFFFFFU;
      mycaniot_message.length = ((i + 64) <= length) ? 64 : length - i; /*maximum 64*/
      mycaniot_message.bufferPtr = &data[i];

      send_caniot_data(&mycaniot_message);
    }
  }
}
void RawBytes_ExampleTask(void *TaskArg)
{
#define INITIAL_BUFFER_LEN 256
  static uint16_t length;
  static uint32_t event;
  static uint16_t actualLength = INITIAL_BUFFER_LEN;
  static uint8_t *data;

  data = malloc(INITIAL_BUFFER_LEN);
  while (1)
  {
    length = 0;
    /* give data pointer as NULL to retrieve only data length*/
    event = 0;
    CPIf_Retrieve_Data_Sync(0, &event, &length, NULL, portMAX_DELAY);
    if (length > 0 || event == RAW_BYTES_RX)
    {
      serialOutput_sendString("%d %d\n", event, length);
      switch (event)
      {
      case RAW_BYTES_TX:
      {
        if (length > 0)
        {
          if (length > INITIAL_BUFFER_LEN || length > actualLength)
          {
            actualLength = length;
            data = realloc(data, length);
          }
          CPIf_Retrieve_Data_Sync(0, &event, &length, data, portMAX_DELAY);
          uint8_t random = (uint8_t)(esp_random() & 0xFF);
          for (size_t i = 0; i < length; i++)
          {
            data[i] = (data[i] ^ random);
          }
          /*debug output*/

          serialOutput_send_buffer_hex("CPIf_Retrieve_Data_Sync0", APPL_COLOR[appl_color_counter], data, length);
          appl_color_counter++;
          if (appl_color_counter >= APPL_COLOR_SIZE)
            appl_color_counter = 0;
        }
      }
      break;
      case RAW_BYTES_RX:
      {
        uint16_t lengthReceiver = 0;
        uint8_t dataReceive[1];
        CPIf_Retrieve_Data_Sync(0, &event, &lengthReceiver, dataReceive, portMAX_DELAY);
        send_caniot_protocol_data_back(data, length);
      }

      break;
      default:
        break;
      }
    }
  }
  vTaskDelete(NULL);
}

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "main_if.h"
#include "lin_if.h"

static portMUX_TYPE lin_spinlock = portMUX_INITIALIZER_UNLOCKED;
#define LIN_ENTER_CRITICAL() portENTER_CRITICAL(&lin_spinlock)
#define LIN_EXIT_CRITICAL() portEXIT_CRITICAL(&lin_spinlock)

#define APPL_NAD 0x1U /*from demo_lin.ldf*/
uint8_t readData[] = {0x22, 0xF0, 0x10};
uint8_t writeData[] = {0x2E, 0xF0, 0x10, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
uint8_t *UDS_diag_data;
uint8_t NAD;
uint16_t length;
extern QueueHandle_t status_queue;

static QueueHandle_t appl_diagnose_queue;

static void Lin_appl_diagnose_Task(void *TaskArg)
{
  static char myBuffer;

  appl_message_t appl_message;
  isoTp_lin_message_status message_status;

  appl_message_t diagnose_event;
  diagnose_event.bufferPtr = &myBuffer;
  diagnose_event.event = 2;
  int32_t isoTP_handler_id;
  while (1)
  {
    if (xQueueReceive(appl_diagnose_queue, &appl_message, portMAX_DELAY))
    {
      switch (appl_message.event)
      {

      case 1:
      {
        if (appl_message.bufferPtr[0] == 1)
        {
          LinIf_TpTransmitSync(&isoTP_handler_id, APPL_NAD, sizeof(writeData), writeData);
          if (isoTP_handler_id == -1)
          {
            diagnose_event.bufferPtr[0] = 1;
            xQueueSend(status_queue, &diagnose_event, pdMS_TO_TICKS(1));
          }
          else
          {
            do
            {
              LinIf_Tp_tx_status(&message_status);
              vTaskDelay(5);
            } while (EN_IN_PROGRESS == message_status);
            LinIf_TpReceiveSync(isoTP_handler_id, &NAD, &length, UDS_diag_data, portMAX_DELAY);

            if (length > 0)
            {
              diagnose_event.bufferPtr[0] = 0;
              xQueueSend(status_queue, &diagnose_event, pdMS_TO_TICKS(1));
            }
            else
            {
              diagnose_event.bufferPtr[0] = 1;
              xQueueSend(status_queue, &diagnose_event, pdMS_TO_TICKS(1));
            }
          }
        }
        else
        {

          LinIf_Tp_set_rx_ready();

          LinIf_TpTransmitSync(&isoTP_handler_id, APPL_NAD, sizeof(readData), readData);
          if (isoTP_handler_id == -1)
          {
            diagnose_event.bufferPtr[0] = 1;
            xQueueSend(status_queue, &diagnose_event, pdMS_TO_TICKS(1));
          }
          else
          {
            do
            {
              LinIf_Tp_tx_status(&message_status);
              vTaskDelay(5);
            } while (EN_IN_PROGRESS == message_status);

            LinIf_TpReceiveSync(isoTP_handler_id, &NAD, &length, UDS_diag_data, portMAX_DELAY);

            if (length > 0)
            {
              diagnose_event.bufferPtr[0] = 0;
              xQueueSend(status_queue, &diagnose_event, pdMS_TO_TICKS(1));
            }
            else
            {
              diagnose_event.bufferPtr[0] = 1;
              xQueueSend(status_queue, &diagnose_event, pdMS_TO_TICKS(1));
            }
          }
        }

        break;
      }

      default:
        break;
      }
    }
  } /*while(1)*/
}

static void LinMessageReceive_0x01(uint8_t dlc, uint8_t *data)
{
  static char myBuffer;
  appl_message_t lin_event;

  if (dlc > 0)
  {
    LIN_ENTER_CRITICAL();
    data[0] = data[0] + 1;
    LIN_EXIT_CRITICAL();
    LinIf_update_frame_data(0x01, data);

    lin_event.event = 0;
    lin_event.bufferPtr = &myBuffer;

    if (data[0] < 128)
    {
      lin_event.bufferPtr[0] = 1;
      xQueueSend(status_queue, &lin_event, pdMS_TO_TICKS(1));
    }
    else
    {
      lin_event.bufferPtr[0] = 0;
      xQueueSend(status_queue, &lin_event, pdMS_TO_TICKS(1));
    }
  }
}
static void LinMessageReceive_0x02(uint8_t dlc, uint8_t *data)
{
  static bool bitResult0;
  static char myBuffer;
  appl_message_t lin_event;

  if (dlc > 0)
  {
    lin_event.bufferPtr = &myBuffer;
    uint32_t *dataPointer = (uint32_t *)data;
    bool bit0 = TestBit(dataPointer, 0);
    if (bit0)
    {

      lin_event.event = 1;

      if (!bitResult0)
      {
        lin_event.bufferPtr[0] = 1;
        xQueueSend(status_queue, &lin_event, pdMS_TO_TICKS(1));
        xQueueSend(appl_diagnose_queue, &lin_event, pdMS_TO_TICKS(1));
      }
    }
    else
    {
      lin_event.event = 1;

      if (bitResult0)
      {
        lin_event.bufferPtr[0] = 0;
        xQueueSend(status_queue, &lin_event, pdMS_TO_TICKS(1));
        xQueueSend(appl_diagnose_queue, &lin_event, pdMS_TO_TICKS(1));
      }
    }
    bitResult0 = bit0;
  }
}

void linAppl_init(void)
{
  appl_diagnose_queue = xQueueCreate(10, sizeof(appl_message_t));

  UDS_diag_data = heap_caps_malloc(4096, MALLOC_CAP_SPIRAM);
  serialOutput_sendString("linAppl: LIN Message started\r\n");

  xTaskCreate(Lin_appl_diagnose_Task, "Lin_appl_diagnose_Task", 3000, (void *)0, 5, NULL);

  /*attach handler to be called when message 0x1 is successfully finished from master*/
  LinIf_onReceive(0x01, &LinMessageReceive_0x01); /*from demo_lin.ldf*/
  /*attach handler to be called when message 0x2 is successfully finished from master*/
  LinIf_onReceive(0x02, &LinMessageReceive_0x02); /*from demo_lin.ldf*/
}

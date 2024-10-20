#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"
#include "driver/uart_select.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "OBD9141.h"
#include "main_if.h"
#include "caniotBox_Pins.h"

#define RX_PIN KLINE_RXD_PIN
#define TX_PIN KLINE_TXD_PIN

/*
  This example shows how to use the library to read the diagnostic trouble codes
  from the ECU and print these in a human readable format to the serial port.

  Huge thanks goes out to to https://github.com/produmann for helping with
  development of this feature and the extensive testing on a real car.
*/

OBD9141 obd;

static const char *TAG = "readDTC";

extern "C" void readDTC_Task(void *pvParameters)
{
  obd.begin(RX_PIN, TX_PIN);
  serialOutput_sendString("%sreadDTC_Task\n", COLOR_GREEN);
  bool init_success = obd.init();
  serialOutput_sendString("init_success %d\n", init_success);

  // init_success = true;
  //  Uncomment this line if you use the simulator to force the init to be
  //  interpreted as successful. With an actual ECU; be sure that the init is
  //  succesful before trying to request PID's.

  // Trouble code consists of a letter and then four digits, we write this
  // human readable ascii string into the dtc_buf which we then write to the
  // serial port.
  uint8_t dtc_buf[5];
  if (init_success)
  {
    uint8_t res;
    while (1)
    {
      // res will hold the number of trouble codes that were received.
      // If no diagnostic trouble codes were retrieved it will be zero.
      // The ECU may return trouble codes which decode to P0000, this is
      // not a real trouble code but instead used to indicate the end of
      // the trouble code list.
      res = obd.readTroubleCodes();
      if (res)
      {

        serialOutput_sendString(" codes:\n");
        for (uint8_t index = 0; index < res; index++)
        {
          // retrieve the trouble code in its raw two byte value.
          uint16_t trouble_code = obd.getTroubleCode(index);

          // If it is equal to zero, it is not a real trouble code
          // but the ECU returned it, print an explanation.
          if (trouble_code == 0)
          {
            serialOutput_sendString("P0000 (reached end of trouble codes)\n");
          }
          else
          {
            // convert the DTC bytes from the buffer into readable string
            obd.decodeDTC(trouble_code, dtc_buf);
            // Print the 5 readable ascii strings to the serial port.
            serialOutput_send_buffer_hex(TAG, COLOR_GREEN, dtc_buf, 5);
          }
        }
      }
      else
      {
        serialOutput_sendString("No trouble codes retrieved.\n");
      }

      vTaskDelay(200);
    }
  }
  vTaskDelay(3000);
  vTaskDelete(NULL);
}

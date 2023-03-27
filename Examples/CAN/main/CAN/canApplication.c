#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "main_if.h"
#include "can_if.h"
#include "mbedtls/aes.h"

extern QueueHandle_t status_queue;

typedef unsigned char ivtype[16];
static ivtype Buffer_AuthenticationKeyIv[2];

void CanTp_processMessage(uint16_t handler_id, uint16_t U16_DLC, uint8_t *Puint8_t_BufferAddress, int is_Tx_Rx_finished)
{
  static uint8_t plaintext[16];
  static uint8_t iv[16];
  static uint8_t key[16];
  static uint8_t Buffer_AuthenticationRequest[16];

  static uint8_t Buffer_AuthenticationResponse[16];
  static int state_handler1;

  if (is_Tx_Rx_finished == 1) /* only when isoTP package has been received*/
  {
    switch (handler_id)
    {
    case 0:
    {
      mbedtls_aes_context ctx;
      mbedtls_aes_init(&ctx);
      mbedtls_aes_setkey_enc(&ctx, (const uint8_t *)&Buffer_AuthenticationKeyIv[0], 128);
      mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, 16, Buffer_AuthenticationKeyIv[1], (uint8_t *)Puint8_t_BufferAddress, (uint8_t *)Buffer_AuthenticationResponse);
      mbedtls_aes_free(&ctx);
      CanIsoTpIf_send(0, sizeof(Buffer_AuthenticationResponse), (uint8_t *)Buffer_AuthenticationResponse);
      break;
    }
    case 1:
    {
      switch (state_handler1)
      {
      case 0:
        if (U16_DLC == 32)
        {
          uint32_t *random_ptr = (uint32_t *)plaintext;
          *(random_ptr++) = esp_random();
          *(random_ptr++) = esp_random();
          *(random_ptr++) = esp_random();
          *(random_ptr) = esp_random();

          memcpy(key, &Puint8_t_BufferAddress[0], 16);

          memcpy(iv, &Puint8_t_BufferAddress[16], 16);

          mbedtls_aes_context ctx;
          mbedtls_aes_init(&ctx);
          mbedtls_aes_setkey_enc(&ctx, (const uint8_t *)key, 128);
          mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, 16, iv, (uint8_t *)plaintext, (uint8_t *)Buffer_AuthenticationRequest);
          mbedtls_aes_free(&ctx);
          CanIsoTpIf_send(1, sizeof(Buffer_AuthenticationRequest), (uint8_t *)Buffer_AuthenticationRequest);
          state_handler1 = 1;
        }

        break;
      case 1:
      {
        if (U16_DLC == 16)
        {
          static char myAesResult;
          appl_message_t can_event;
          can_event.bufferPtr = &myAesResult;

          if (memcmp(Puint8_t_BufferAddress, plaintext, 16) == 0)
          {
            can_event.event = 1;
            can_event.bufferPtr[0] = 1;
            xQueueSend(status_queue, &can_event, pdMS_TO_TICKS(1));
          }
          else
          {
            can_event.event = 1;
            can_event.bufferPtr[0] = 0;
            xQueueSend(status_queue, &can_event, pdMS_TO_TICKS(1));
          }
        }

        state_handler1 = 0;
        break;
      }

      default:
        break;
      }

    default:
    {
      /*do nothing*/
    }
    break;
    }
    }
  }
  else
  {
    // serialOutput_sendString("%sisotp send is finished %d\n",COLOR_CYAN, handler_id);
  }
}
static void canMessageReceive_502(uint8_t dlc, uint8_t *data)
{
  static bool bitResult0;
  static char myBuffer;
  appl_message_t can_event;
  can_event.bufferPtr = &myBuffer;

  uint32_t *dataPointer = (uint32_t *)data;
  bool bit0 = TestBit(dataPointer, 0);
  if (bit0)
  {

    can_event.event = 0;
    can_event.bufferPtr[0] = 1;
    if (!bitResult0)
      xQueueSend(status_queue, &can_event, pdMS_TO_TICKS(1));
  }
  else
  {
    can_event.event = 0;
    can_event.bufferPtr[0] = 0;
    if (bitResult0)
      xQueueSend(status_queue, &can_event, pdMS_TO_TICKS(1));
  }
  bitResult0 = bit0;
}

static void can_preSend_0x503(uint8_t dlc, uint8_t *data)
{
  data[0] = data[0] + 1; /*just increment the first byte*/
  if (data[0] == 1)
  {
    uint32_t *random_ptr = (uint32_t *)&Buffer_AuthenticationKeyIv[0];
    *(random_ptr++) = esp_random(); /*create random key*/
    *(random_ptr++) = esp_random();
    *(random_ptr++) = esp_random();
    *(random_ptr++) = esp_random();

    *(random_ptr++) = esp_random(); /*create random iv*/
    *(random_ptr++) = esp_random();
    *(random_ptr++) = esp_random();
    *(random_ptr) = esp_random();
    CanIsoTpIf_send(0, sizeof(Buffer_AuthenticationKeyIv), (uint8_t *)Buffer_AuthenticationKeyIv);
  }
}
void canAppl_init(void)
{
  serialOutput_sendString("canAppl: can Message started\r\n");

  /*configure ISOTP worker for channel 0*/
  CanIsoTpIf_configure(0, 0, (uint32_t)0x500, (uint32_t)0x501, &CanTp_processMessage, false);
  /*configure ISOTP worker for channel 1*/
  CanIsoTpIf_configure(1, 1, (uint32_t)0x501, (uint32_t)0x500, &CanTp_processMessage, false);
  /*attach handler to be called before sending message 0x503 on channel 1*/
  CanIf_attach_preSendFnc(1, 0x503, &can_preSend_0x503);
  /*attach handler to be called on receiving message 0x502 on channel 1*/
  CanIf_attach_receive(1, 0x502, &canMessageReceive_502);
}
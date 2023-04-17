
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
#include "lin_if.h"
#include "can_if.h"
#include "uds_service_fnc.h"
#include "communication_if.h"

static QueueHandle_t isoTp_dataReceive_queue = NULL;
static uint8_t flash_id = 1;
static uint8_t flash_channel = 1;
static uint8_t can_lin_variant;
// static uint8_t lin_nad_this = 0x11;
extern uint8_t lin_nad;

/************************************************************
**	Description :	using software lookup table to create crc. Input data in @ i_pucDataBuf
**	and data len in @ i_ulDataLen. When create crc successful
**	return CRC.
************************************************************/
static bool send_lin_diagnose(uint8_t *tempBuffer, uint16_t data_length, uint8_t *rxDataArray, uint16_t receive_array_size)
{
    bool service_was_ok = true;
    lin_message_status wait_status;
    uint8_t number_of_retry = 5;
    uint16_t length = 0;
    uint8_t NAD;
    int32_t handle = -1;
    LinIf_Tp_set_rx_ready();

    LinIf_TpTransmitSync(&handle, lin_nad, data_length, tempBuffer);

    do
    {
        LinIf_Tp_tx_status((lin_message_status *)&wait_status);
        if (LD_COMPLETED != wait_status)
            vTaskDelay(5);
    } while (LD_COMPLETED != wait_status);

    do
    {
        if (!service_was_ok)
        {
            vTaskDelay(200);
            if (number_of_retry > 0)
            {
                number_of_retry--;
                LinIf_send_diagnose_header(&handle);
            }
        } 
        LinIf_TpReceiveSync(handle, &NAD, &length, tempBuffer, portMAX_DELAY);
         
        bool dataValide = true;
        if (length > 0)
        {
            for (size_t i = 0; i < receive_array_size; i++)
            {
                if (rxDataArray[i] != tempBuffer[i])
                {
                    dataValide = false;
                    break;
                }
            }
            if (!dataValide)
            {
                service_was_ok = false;
            }
            else
            {
                service_was_ok = true;
            }
        }
        else
        {
            service_was_ok = false;
        }

    } while ((number_of_retry > 0) && (false == service_was_ok));

    return service_was_ok;
}
static bool send_can_diagnose(uint8_t *tempBuffer, uint16_t data_length, uint8_t *rxDataArray, uint16_t receive_array_size)
{
    bool service_was_ok = false;
    uint8_t number_of_retry = 5;
    uint8_t dummyBuffer[2];
    appl_message_t messagefile;
    CanIsoTpIf_send(flash_id, data_length, tempBuffer);

    do
    {

        memset(&messagefile, 0, sizeof(appl_message_t));
        xQueueReceive(isoTp_dataReceive_queue, (void *)&messagefile, portMAX_DELAY);
        bool dataValide = true;
        if (messagefile.length > 0)
        {

            for (size_t i = 0; i < receive_array_size; i++)
            {
                if (rxDataArray[i] != messagefile.bufferPtr[i])
                {
                    dataValide = false;
                    break;
                }
            }
            if (dataValide)
            {
                service_was_ok = true;

                memcpy(tempBuffer, messagefile.bufferPtr, messagefile.length);
            }
            else
            {
                if (number_of_retry > 0)
                {
                    number_of_retry--;
                    dummyBuffer[0] = 0;
                    dummyBuffer[1] = 0;
                    CanIsoTpIf_send(flash_id, 2, &dummyBuffer[0]);
                }
            }
        }
        else
        {
            if (number_of_retry > 0)
            {
                number_of_retry--;
                dummyBuffer[0] = 0;
                dummyBuffer[1] = 0;
                CanIsoTpIf_send(flash_id, 2, &dummyBuffer[0]);
            }
        }

    } while ((number_of_retry > 0) && (false == service_was_ok));
    if (messagefile.bufferPtr)
    {
        free(messagefile.bufferPtr);
    }

    return service_was_ok;
}
static void CanTp_localProcessMessage(uint16_t handler_id, uint16_t U16_DLC, uint8_t *Puint8_t_BufferAddress, int is_Tx_Rx_finished)
{

    if (flash_id == handler_id && is_Tx_Rx_finished == 1)
    {
        /* call application handler for this type of message */
        appl_message_t messagefile;
        messagefile.length = U16_DLC;
        messagefile.bufferPtr = malloc(U16_DLC);
        memcpy(messagefile.bufferPtr, Puint8_t_BufferAddress, messagefile.length);
        if (xQueueSend(isoTp_dataReceive_queue, &messagefile, pdMS_TO_TICKS(1)) != pdTRUE)
        {
        }
        else
        {
        }
    }
}

void communication_if_init(void)
{

    uint8_t can_lin_variant;
    nvsIf_get_can_lin_var(&can_lin_variant);

    if (can_lin_variant == 0)
    {
        isoTp_dataReceive_queue = xQueueCreate(20, sizeof(appl_message_t));
        flash_channel = 1;
        flash_id = 1;
        CanIsoTpIf_configure(flash_channel, flash_id, (uint32_t)0x500, (uint32_t)0x501, &CanTp_localProcessMessage, true);
    }
}
communication_fnc get_communication_if(void)
{
    communication_fnc mycommunication_fnc = NULL;
    uint8_t can_lin_variant;
    nvsIf_get_can_lin_var(&can_lin_variant);
    if (can_lin_variant == 0)
    {
        mycommunication_fnc = &send_can_diagnose;
    }
    else
    {
        mycommunication_fnc = &send_lin_diagnose;
    }
    return mycommunication_fnc;
}

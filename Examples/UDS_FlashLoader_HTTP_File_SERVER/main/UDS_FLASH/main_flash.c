
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
#include "cJSON.h"
#include "driver/uart.h"
#include "main_if.h"
#include <byteswap.h>
#include "cintelhex.h"
#include "mbedtls/aes.h"
#include "communication_if.h"
#include "uds_service_fnc.h"
char *UDS_Appl_filepath = NULL;
extern void lvgl_move_background_container();
extern QueueHandle_t FlashLoader_queue;
appl_message_t appl_messagefile[MAX_FILE_TOFLASH] = {0};
static uint16_t appl_messagefile_size = 0;

static const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}
static void getFileDataFlash(const cJSON *const mainArray)
{
    ihex_recordset_t *appl_hex_record = NULL;
    uint32_t *min = NULL;
    uint32_t *max = NULL;
    appl_message_t messagefile;

    cJSON *array3 = cJSON_GetObjectItem(mainArray, "Infos");

    char *fileName = cJSON_GetObjectItem(array3, "filePath")->valuestring;
    
    if (fileName == NULL || ((fileName) && strlen(fileName) == 0))
    {
        if (UDS_Appl_filepath != NULL)
        {
            fileName = UDS_Appl_filepath;
        }
    }

    if (fileName != NULL && strlen(fileName) > 0)
    {
        const char *filename_ext = get_filename_ext((const char *)fileName);
        if (strstr(filename_ext, "bin") != NULL)
        {
            bool file_opened = appl_read_file_sync(&appl_messagefile[appl_messagefile_size], fileName, "rb");
            if (file_opened)
            {
                appl_messagefile_size++;
            }
        }
        else if (strstr(filename_ext, "hex") != NULL)
        {
            bool file_opened = appl_read_file_sync(&messagefile, fileName, "r");
            if (file_opened)
            {

                appl_hex_record = ihex_rs_from_mem(messagefile.bufferPtr, messagefile.length);
                free(messagefile.bufferPtr);
                if (appl_hex_record != NULL)
                {
                    char *error = ihex_error();

                    if (error == NULL)
                    {

                        int IHEX_ELACounter = 0;
                        for (size_t i = 0; i < appl_hex_record->ihrs_count; i++)
                        {
                            if (IHEX_ELA == appl_hex_record->ihrs_records[i].ihr_type)
                            {
                                IHEX_ELACounter++;
                            }
                            else if (i == 0)
                            {
                                IHEX_ELACounter++;
                            }
                        }

                        min = (uint32_t *)calloc(IHEX_ELACounter, 4);
                        max = (uint32_t *)calloc(IHEX_ELACounter, 4);

                        int getAdress = ihex_rs_get_all_address_range(appl_hex_record, &min[0], &max[0]);
                        uint32_t AllRange = 0;
                        if (getAdress == 0)
                        {
                            for (uint32_t i = 0; i < IHEX_ELACounter; i++)
                            {

                                uint32_t HexdataLength = max[i] - min[i];

                                if (HexdataLength > 0)
                                {
                                    AllRange += HexdataLength;
                                }
                            }
                            if (AllRange > 0)
                            {
                                appl_messagefile[appl_messagefile_size].bufferPtr = malloc(AllRange);
                                appl_messagefile[appl_messagefile_size].length = AllRange;
                                AllRange = 0;
                                for (uint32_t i = 0; i < IHEX_ELACounter; i++)
                                {
                                    uint32_t HexdataLength = max[i] - min[i];

                                    if (HexdataLength > 0)
                                    {
                                        (void)ihex_get_data(appl_hex_record, (uint8_t *)&appl_messagefile[appl_messagefile_size].bufferPtr[AllRange], min[i], max[i]);
                                        AllRange += HexdataLength;
                                    }
                                }
                                appl_messagefile_size++;
                            }
                        }

                        ihex_rs_free(appl_hex_record);
                    }
                }
            }
        }
    }

    if (min != NULL)
    {
        free(min);
    }
    if (max != NULL)
    {
        free(max);
    }
}
static void readApplBinFile()
{
}
void UdsFlash_mainFunction(const char *filename, const char *uds_attr)
{
    appl_message_t flashLoader_message;

    cJSON *root2 = NULL;
    appl_message_t messagefile;
    bool service_was_ok = true;
    size_t j = 0;

    lvgl_move_background_container();
    communication_if_init();
    uds_service_init();
    int SERVICE_LENGTH;
    const flashLoader_obj_t *flashLoader_obj = uds_service_getData(&SERVICE_LENGTH);

    memset(&appl_messagefile, 0, sizeof(appl_message_t) * MAX_FILE_TOFLASH);

    appl_messagefile_size = 0;

    readApplBinFile();

    bool file_opened = appl_read_file_sync(&messagefile, (char *)filename, "r");

    if (file_opened)
    {
        root2 = cJSON_Parse(messagefile.bufferPtr);
        free(messagefile.bufferPtr);
        cJSON *records = cJSON_GetObjectItem(root2, uds_attr);
        if (records != NULL)
        {
            int numberOfServices = cJSON_GetArraySize(records);

            for (size_t i = 0; i < numberOfServices; i++)
            {
                cJSON *array = cJSON_GetArrayItem(records, i);
                char *service_name = cJSON_GetObjectItem(array, "Service_Name")->valuestring;
                if (service_name != NULL)
                {
                    if (strstr(service_name, "RequestDownload") != NULL)
                    {
                        getFileDataFlash(array);
                        if (appl_messagefile_size == 2)
                        {
                            break; /*read only flash_driver.bin, because we read the application bin fron configfile*/
                        }
                    }
                }
            }

            flashLoader_message.length = 0;
            for (size_t i = 0; i < appl_messagefile_size; i++)
            {

                flashLoader_message.event = 0;
                flashLoader_message.length += appl_messagefile[i].length;
            }
            if (flashLoader_message.length > 0)
            {
                xQueueSend(FlashLoader_queue, &flashLoader_message, pdMS_TO_TICKS(1));
            }

            for (size_t i = 0; i < numberOfServices; i++)
            {
                cJSON *array = cJSON_GetArrayItem(records, i);
                char *service_name = cJSON_GetObjectItem(array, "Service_Name")->valuestring;
                if (service_name != NULL)
                {
                    for (j = 0; j < SERVICE_LENGTH;)
                    {
                        service_was_ok = true;
                        if (flashLoader_obj[j].service_name != NULL)
                        {
                            if (strstr(service_name, flashLoader_obj[j].service_name) != NULL)
                            {
                                if (flashLoader_obj[j].service_fnc != NULL)
                                {
                                    service_was_ok = flashLoader_obj[j].service_fnc(array);
                                }
                            }
                        }
                        if (false == service_was_ok)
                        {
                            // ESP_LOGE(TAG, "error= %s", service_name);
                            for (size_t i = 0; i < appl_messagefile_size; i++)
                            {
                                if (appl_messagefile[i].bufferPtr != NULL)
                                {
                                    free(appl_messagefile[i].bufferPtr);
                                }
                            }
                            memset(&appl_messagefile, 0, sizeof(appl_message_t) * MAX_FILE_TOFLASH);

                            flashLoader_message.length = 0;
                            flashLoader_message.event = 3;
                            xQueueSend(FlashLoader_queue, &flashLoader_message, pdMS_TO_TICKS(1));
                            goto END;
                        }
                        else
                        {
                            j++;
                            vTaskDelay(10);
                        }
                    }
                }
            }
        }
        flashLoader_message.length = 0;
        flashLoader_message.event = 2;
        xQueueSend(FlashLoader_queue, &flashLoader_message, pdMS_TO_TICKS(1));
    }
    else
    {
        flashLoader_message.length = 0;
        flashLoader_message.event = 3;
        xQueueSend(FlashLoader_queue, &flashLoader_message, pdMS_TO_TICKS(1));
    }
END:
    for (size_t i = 0; i < appl_messagefile_size; i++)
    {
        if (appl_messagefile[i].bufferPtr != NULL && appl_messagefile[i].length > 0)
        {
            free(appl_messagefile[i].bufferPtr);
        }
    }

    if (root2 != NULL)
        cJSON_Delete(root2);
}
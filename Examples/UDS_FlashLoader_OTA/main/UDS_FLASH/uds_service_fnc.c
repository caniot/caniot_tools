
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
#include "main_if.h"
#include "cintelhex.h"
#include <byteswap.h>
#include "uds_service_fnc.h"
#include "mbedtls/aes.h"
#include "communication_if.h"

extern QueueHandle_t FlashLoader_queue;
extern void CreatSoftwareCrc16(uint8_t *i_pDataBuf, const uint32_t i_dataLen, uint32_t *m_pCurCrc);

static uint32_t appl_crcfile;
static uint16_t appl_messagefile_read_size = 0;
static uint32_t dataReadLength = 0;
static communication_fnc uds_communication_fnc = NULL;

static int getDataFromJson(uint8_t *tempBuffer, cJSON *arrayData, char *idName)
{
    if (tempBuffer == NULL || arrayData == NULL || idName == NULL)
    {
        return -1;
    }
    cJSON *idJSON = cJSON_GetObjectItem(arrayData, idName);
    int records_array_size = cJSON_GetArraySize(idJSON);
    for (size_t i = 0; i < records_array_size; i++)
    {
        char *data = cJSON_GetArrayItem(idJSON, i)->valuestring;
        tempBuffer[i] = (int)strtol(data, NULL, 16);
    }
    return records_array_size;
}

static bool ECUReset11_fnc(const cJSON *const mainArray)
{
    bool service_was_ok = false;
    uint8_t *rxDataArray;
    uint8_t *tempBuffer;
    tempBuffer = malloc(4096);
    char *id = cJSON_GetObjectItem(mainArray, "Service_Id")->valuestring;
    char *Request_Id = cJSON_GetObjectItem(mainArray, "Request_Id")->valuestring;

    cJSON *array3 = cJSON_GetObjectItem(mainArray, "Infos");
    cJSON *rxDataJSON = cJSON_GetObjectItem(array3, "Response_Data");
    int records_array_size = cJSON_GetArraySize(rxDataJSON);

    serialOutput_sendString("records_array_size ECUReset11_fnc =%d", records_array_size);
    rxDataArray = malloc(records_array_size);
    for (size_t i = 0; i < records_array_size; i++)
    {
        char *data = cJSON_GetArrayItem(rxDataJSON, i)->valuestring;
        rxDataArray[i] = (int)strtol(data, NULL, 16);
    }
    tempBuffer[0] = (int)strtol(id, NULL, 16);
    tempBuffer[1] = (int)strtol(Request_Id, NULL, 16);
    if (uds_communication_fnc != NULL)
    {
        service_was_ok = uds_communication_fnc(tempBuffer, 2, rxDataArray, records_array_size);
    }

    if (tempBuffer)
        free(tempBuffer);
    if (rxDataArray)
        free(rxDataArray);

    return service_was_ok;
}
static bool RequestTransferExit37_fnc(const cJSON *const mainArray)
{

    bool service_was_ok = false;
    uint8_t *rxDataArray;
    uint8_t *tempBuffer;
    tempBuffer = malloc(4096);
    char *id = cJSON_GetObjectItem(mainArray, "Service_Id")->valuestring;

    cJSON *array3 = cJSON_GetObjectItem(mainArray, "Infos");
    cJSON *rxDataJSON = cJSON_GetObjectItem(array3, "Response_Data");
    int records_array_size = cJSON_GetArraySize(rxDataJSON);

    serialOutput_sendString("records_array_size RequestTransferExit37_fnc =%d", records_array_size);
    rxDataArray = malloc(records_array_size);
    for (size_t i = 0; i < records_array_size; i++)
    {
        char *data = cJSON_GetArrayItem(rxDataJSON, i)->valuestring;
        rxDataArray[i] = (int)strtol(data, NULL, 16);
    }
    tempBuffer[0] = (int)strtol(id, NULL, 16);
    tempBuffer[1] = 0;
    tempBuffer[2] = 0;
    if (uds_communication_fnc != NULL)
    {
        service_was_ok = uds_communication_fnc(tempBuffer, 3, rxDataArray, records_array_size);
    }

    if (tempBuffer)
        free(tempBuffer);
    if (rxDataArray)
        free(rxDataArray);

    return service_was_ok;
}
static bool TransferData36_fnc(const cJSON *const mainArray)
{

    uint8_t gs_RxBlockNum = 1;

    bool service_was_ok = false;
    bool loop_active = false;
    uint8_t *rxDataArray = NULL;
    uint8_t *tempBuffer = NULL;
    uint8_t *tempBufferRetry = NULL;
    uint16_t data_length = 0;
    uint32_t readIndex = 0;
    static int TotalreadIndex = 0;
    int records_array_size;
    uint32_t lenght = 0;

    if (appl_messagefile_read_size == 0)
    {
        TotalreadIndex = 0;
    }

    tempBuffer = malloc(4096);
    tempBufferRetry = malloc(4096);
    char *id = cJSON_GetObjectItem(mainArray, "Service_Id")->valuestring;

    cJSON *array3 = cJSON_GetObjectItem(mainArray, "Infos");

    /*get RxData*/
    cJSON *idJSON = cJSON_GetObjectItem(array3, "Response_Data");
    records_array_size = cJSON_GetArraySize(idJSON);
    rxDataArray = malloc(records_array_size);
    records_array_size = getDataFromJson(rxDataArray, array3, "Response_Data");
    appl_crcfile = 0;
    do
    {
        tempBuffer[0] = (int)strtol(id, NULL, 16);
        tempBuffer[1] = gs_RxBlockNum;

        if (appl_messagefile[appl_messagefile_read_size].length > 0 && appl_messagefile[appl_messagefile_read_size].bufferPtr != NULL)
        {
            lenght = 0;
            lenght = (readIndex + dataReadLength <= appl_messagefile[appl_messagefile_read_size].length
                          ? dataReadLength
                          : ((appl_messagefile[appl_messagefile_read_size].length - readIndex)));
            memcpy(&tempBuffer[2], &appl_messagefile[appl_messagefile_read_size].bufferPtr[readIndex], lenght);
        }
        else
        {
            serialOutput_sendString("no file io recorded");
            break;
        }
        readIndex += lenght;
        TotalreadIndex += lenght;
        data_length = lenght + 2;
        appl_message_t flashLoader_message;
        flashLoader_message.length = TotalreadIndex;
        flashLoader_message.event = 1;
        xQueueSend(FlashLoader_queue, &flashLoader_message, pdMS_TO_TICKS(1));

        if (uds_communication_fnc != NULL)
        {
            int numberOfUdsRetry = 5;
            memcpy(tempBufferRetry, tempBuffer, data_length);
            do
            {
                service_was_ok = uds_communication_fnc(tempBuffer, data_length, rxDataArray, records_array_size);
                if (false == service_was_ok)
                {
                    memcpy(tempBuffer, tempBufferRetry, data_length);
                }

            } while (false == service_was_ok && --numberOfUdsRetry > 0);
        }

        if (false == service_was_ok)
        {
            break;
        }
        else
        {
            if (gs_RxBlockNum < 255)
            {
                gs_RxBlockNum++;
            }
            else
            {
                gs_RxBlockNum = 0;
            }
        }

        loop_active = readIndex < appl_messagefile[appl_messagefile_read_size].length;

    } while (loop_active);
    appl_crcfile = 0;
    CreatSoftwareCrc16((uint8_t *)appl_messagefile[appl_messagefile_read_size].bufferPtr, appl_messagefile[appl_messagefile_read_size].length, &appl_crcfile);
    appl_messagefile_read_size++;
    if (tempBuffer)
        free(tempBuffer);
    if (rxDataArray)
        free(rxDataArray);
    if (tempBufferRetry)
        free(tempBufferRetry);

    return service_was_ok;
}
static bool RequestDownload34_fnc(const cJSON *const mainArray)
{
    uint8_t l = 0;
    bool service_was_ok = false;
    uint8_t *rxDataArray = NULL;
    uint8_t *tempBuffer;
    uint16_t data_length = 0;
    int records_array_size;

    tempBuffer = malloc(4096);
    char *id = cJSON_GetObjectItem(mainArray, "Service_Id")->valuestring;
    cJSON *compressionMethodcJSON = cJSON_GetObjectItem(mainArray, "compressionMethod");
    cJSON *encryptingMethodcJSON = cJSON_GetObjectItem(mainArray, "encryptingMethod");

    tempBuffer[data_length++] = (int)strtol(id, NULL, 16);
    tempBuffer[data_length] = 0;
    /* The high nibble specifies the “compression Method”*/
    if (compressionMethodcJSON != NULL)
    {
        char *compressionMethod = compressionMethodcJSON->valuestring;
        tempBuffer[data_length] |= (((uint8_t)strtol(compressionMethod, NULL, 16) << 4) & 0xF0);
    }
    else
    {
        tempBuffer[data_length] &= (0xF0U ^ 0xFFU);
    }
    /*the low nibble specifies the “encrypting Method”*/
    if (encryptingMethodcJSON != NULL)
    {
        char *encryptingMethod = encryptingMethodcJSON->valuestring;
        tempBuffer[data_length] |= ((uint8_t)strtol(encryptingMethod, NULL, 16) & 0x0FU);
    }
    else
    {
        tempBuffer[data_length] &= (0x0FU ^ 0xFFU);
    }
    data_length++;

    tempBuffer[data_length++] = 0x44U; /*bit 7 – 4: Length (number of bytes) of the memory size parameter.
                                        bit 3 – 0: Lenght (number of bytes) of the memory address parameter.*/

    cJSON *array3 = cJSON_GetObjectItem(mainArray, "Infos");
    data_length += getDataFromJson(&tempBuffer[data_length], array3, "FlashAddress");

    if (appl_messagefile[appl_messagefile_read_size].length > 0 && appl_messagefile[appl_messagefile_read_size].bufferPtr != NULL)
    {
        uint32_t *pointer = (uint32_t *)&tempBuffer[data_length];
        uint32_t pointerData = __bswap_32(appl_messagefile[appl_messagefile_read_size].length);
        *pointer = pointerData;
        data_length += 4;
    }
    else
    {
        serialOutput_sendString("no file io recorded");
    }

    /*get RxData*/
    cJSON *idJSON = cJSON_GetObjectItem(array3, "Response_Data");
    records_array_size = cJSON_GetArraySize(idJSON);
    rxDataArray = malloc(records_array_size);
    records_array_size = getDataFromJson(rxDataArray, array3, "Response_Data");

    if (uds_communication_fnc != NULL)
    {
        service_was_ok = uds_communication_fnc(tempBuffer, data_length, rxDataArray, records_array_size);
    }

    dataReadLength = 0;
    if (false != service_was_ok)
    {
        if (tempBuffer[1] > 0)
        {
            l = tempBuffer[1] >> 4;

            // for (uint8_t i = 0; i < l; i++)
            // {
            //     dataReadLength |= (tempBuffer[2 + i] << (i * 8));
            // }
            int j = 0;
            for (uint8_t i = l; i > 0; i--)
            {
                dataReadLength |= (tempBuffer[i + 1] << (j * 8));
                j++;
            }
        }
        else
        {
            if (tempBuffer[4] > 2)
            {
                dataReadLength = tempBuffer[4] - 2;
            }
            else
            {
                dataReadLength = 16;
            }
        }
        if (dataReadLength >= 4096)
        {
            dataReadLength = 128;
        }
        serialOutput_sendString("dataReadLength %d %d", l, dataReadLength);
        // esp_restart();
        //  dataReadLength= 128;
    }

    if (tempBuffer)
        free(tempBuffer);
    if (rxDataArray)
        free(rxDataArray);

    return service_was_ok;
}
static bool RoutineControl31_fnc(const cJSON *const mainArray)
{
    bool service_was_ok = false;
    uint8_t *rxDataArray = NULL;
    uint8_t *tempBuffer;
    uint16_t data_length = 0;
    int records_array_size;

    tempBuffer = malloc(4096);
    char *id = cJSON_GetObjectItem(mainArray, "Service_Id")->valuestring;
    tempBuffer[data_length++] = (int)strtol(id, NULL, 16);
    char *Request_Id = cJSON_GetObjectItem(mainArray, "Request_Id")->valuestring;
    tempBuffer[data_length++] = (int)strtol(Request_Id, NULL, 16);
    uint16_t data_lengthTemp = data_length;
    cJSON *array3 = cJSON_GetObjectItem(mainArray, "Infos");
    data_length += getDataFromJson(&tempBuffer[data_length], array3, "Service_Id");
    if (tempBuffer[data_lengthTemp] == 2 && tempBuffer[data_lengthTemp + 1] == 2)
    {
        data_lengthTemp = data_length;
        data_length += getDataFromJson(&tempBuffer[data_length], array3, "data");
        uint32_t *pointer = (uint32_t *)&tempBuffer[data_lengthTemp];
        uint32_t pointerData = __bswap_32(appl_crcfile);
        uint32_t pointerData2 = *pointer;
        *pointer = pointerData;
        if (*pointer != pointerData2)
        {
            serialOutput_sendString("%scalcuded crc don't match the one fron the json file 0x%08X 0x%08X", COLOR_RED, *pointer, pointerData2);
        }
        else
        {
        }
        //  ESP_LOG_BUFFER_HEX_LEVEL("Iso_TpReceiveSync", tempBuffer, data_length, ESP_LOG_WARN);
    }
    else
    {
        data_length += getDataFromJson(&tempBuffer[data_length], array3, "data");
    }

    /*get RxData*/
    cJSON *idJSON = cJSON_GetObjectItem(array3, "Response_Data");
    records_array_size = cJSON_GetArraySize(idJSON);
    rxDataArray = malloc(records_array_size);
    records_array_size = getDataFromJson(rxDataArray, array3, "Response_Data");

    if (uds_communication_fnc != NULL)
    {
        service_was_ok = uds_communication_fnc(tempBuffer, data_length, rxDataArray, records_array_size);
    }

    if (tempBuffer)
        free(tempBuffer);
    if (rxDataArray)
        free(rxDataArray);

    return service_was_ok;
}
static bool WriteDataByIdentifier2E_fnc(const cJSON *const mainArray)
{
    bool service_was_ok = false;
    uint8_t *rxDataArray = NULL;
    uint8_t *tempBuffer;
    uint16_t data_length = 0;
    int records_array_size;

    tempBuffer = malloc(4096);
    char *id = cJSON_GetObjectItem(mainArray, "Service_Id")->valuestring;
    tempBuffer[data_length] = (int)strtol(id, NULL, 16);
    data_length++;
    cJSON *array3 = cJSON_GetObjectItem(mainArray, "Infos");
    data_length += getDataFromJson(&tempBuffer[data_length], array3, "Service_Id");
    data_length += getDataFromJson(&tempBuffer[data_length], array3, "data");

    /*get RxData*/
    cJSON *idJSON = cJSON_GetObjectItem(array3, "Response_Data");
    records_array_size = cJSON_GetArraySize(idJSON);
    rxDataArray = malloc(records_array_size);
    records_array_size = getDataFromJson(rxDataArray, array3, "Response_Data");

    if (uds_communication_fnc != NULL)
    {
        service_was_ok = uds_communication_fnc(tempBuffer, data_length, rxDataArray, records_array_size);
    }

    if (tempBuffer)
        free(tempBuffer);
    if (rxDataArray)
        free(rxDataArray);

    return service_was_ok;
}
static bool CommunicationControl28_fnc(const cJSON *const mainArray)
{
    bool service_was_ok = false;
    uint8_t *rxDataArray;
    uint8_t *tempBuffer;
    tempBuffer = malloc(4096);
    char *id = cJSON_GetObjectItem(mainArray, "Service_Id")->valuestring;
    char *Request_Id = cJSON_GetObjectItem(mainArray, "Request_Id")->valuestring;

    cJSON *array3 = cJSON_GetObjectItem(mainArray, "Infos");
    char *commType = cJSON_GetObjectItem(array3, "commType")->valuestring;
    cJSON *rxDataJSON = cJSON_GetObjectItem(array3, "Response_Data");
    int records_array_size = cJSON_GetArraySize(rxDataJSON);

    // serialOutput_sendString("records_array_size2=%d", records_array_size);
    rxDataArray = malloc(records_array_size);
    for (size_t i = 0; i < records_array_size; i++)
    {
        char *data = cJSON_GetArrayItem(rxDataJSON, i)->valuestring;
        rxDataArray[i] = (int)strtol(data, NULL, 16);
    }
    tempBuffer[0] = (int)strtol(id, NULL, 16);
    tempBuffer[1] = (int)strtol(Request_Id, NULL, 16);
    tempBuffer[2] = (int)strtol(commType, NULL, 16);

    if (uds_communication_fnc != NULL)
    {
        service_was_ok = uds_communication_fnc(tempBuffer, 3, rxDataArray, records_array_size);
    }

    if (tempBuffer)
        free(tempBuffer);
    if (rxDataArray)
        free(rxDataArray);

    return service_was_ok;
}
static bool SecurityAccess27_fnc(const cJSON *const mainArray)
{
    uint8_t key[16];
    uint8_t iv[16];
    static uint8_t plaintext[16];
    cJSON *array3;
    cJSON *keyJSON;
    cJSON *ivJSON;
    char *data;
    int key_size, iv_size;
    bool service_was_ok = false;
    uint8_t *rxDataArray;
    uint8_t *tempBuffer;
    tempBuffer = malloc(4096);
    char *id = cJSON_GetObjectItem(mainArray, "Service_Id")->valuestring;
    char *Request_Id = cJSON_GetObjectItem(mainArray, "Request_Id")->valuestring;

    array3 = cJSON_GetObjectItem(mainArray, "Infos");
    cJSON *rxDataJSON = cJSON_GetObjectItem(array3, "Response_Data");
    int records_array_size = cJSON_GetArraySize(rxDataJSON);

    // serialOutput_sendString("records_array_size2=%d", records_array_size);
    rxDataArray = malloc(records_array_size);
    for (size_t i = 0; i < records_array_size; i++)
    {
        data = cJSON_GetArrayItem(rxDataJSON, i)->valuestring;
        rxDataArray[i] = (int)strtol(data, NULL, 16);
    }
    tempBuffer[0] = (int)strtol(id, NULL, 16);
    tempBuffer[1] = (int)strtol(Request_Id, NULL, 16);

    switch (tempBuffer[1])
    {
    case 1:

        keyJSON = cJSON_GetObjectItem(array3, "key");
        key_size = cJSON_GetArraySize(keyJSON);

        ivJSON = cJSON_GetObjectItem(array3, "iv");
        iv_size = cJSON_GetArraySize(ivJSON);

        for (size_t i = 0; i < key_size; i++)
        {
            data = cJSON_GetArrayItem(keyJSON, i)->valuestring;
            tempBuffer[i + 2] = (int)strtol(data, NULL, 16);
        }
        for (size_t i = 0; i < iv_size; i++)
        {
            data = cJSON_GetArrayItem(ivJSON, i)->valuestring;
            tempBuffer[i + key_size + 2] = (int)strtol(data, NULL, 16);
        }

        if (uds_communication_fnc != NULL)
        {
            service_was_ok = uds_communication_fnc(tempBuffer, sizeof(key) + 2, rxDataArray, records_array_size);
        }

        if (service_was_ok)
        {
            memcpy(plaintext, &tempBuffer[2], 16);
        }

        break;
    case 2:

        keyJSON = cJSON_GetObjectItem(array3, "key");
        key_size = cJSON_GetArraySize(keyJSON);

        ivJSON = cJSON_GetObjectItem(array3, "iv");
        iv_size = cJSON_GetArraySize(ivJSON);

        for (size_t i = 0; i < key_size; i++)
        {
            data = cJSON_GetArrayItem(keyJSON, i)->valuestring;
            key[i] = (int)strtol(data, NULL, 16);
        }
        for (size_t i = 0; i < iv_size; i++)
        {
            data = cJSON_GetArrayItem(ivJSON, i)->valuestring;
            iv[i] = (int)strtol(data, NULL, 16);
        }

        mbedtls_aes_context ctx;
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_enc(&ctx, (const uint8_t *)key, 128);
        mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, 16, iv, (uint8_t *)plaintext, (uint8_t *)&tempBuffer[2]);
        mbedtls_aes_free(&ctx);

        if (uds_communication_fnc != NULL)
        {
            service_was_ok = uds_communication_fnc(tempBuffer, key_size + iv_size + 2, rxDataArray, records_array_size);
        }

        break;
    default:
        break;
    }

    if (tempBuffer)
        free(tempBuffer);
    if (rxDataArray)
        free(rxDataArray);

    return service_was_ok;
}
static bool DiagnosticSessionControl10_fnc(const cJSON *const mainArray)
{
    bool service_was_ok = false;
    uint8_t *rxDataArray;
    uint8_t *tempBuffer;
    tempBuffer = malloc(4096);
    char *id = cJSON_GetObjectItem(mainArray, "Service_Id")->valuestring;
    char *Request_Id = cJSON_GetObjectItem(mainArray, "Request_Id")->valuestring;

    cJSON *array3 = cJSON_GetObjectItem(mainArray, "Infos");
    cJSON *rxDataJSON = cJSON_GetObjectItem(array3, "Response_Data");
    int records_array_size = cJSON_GetArraySize(rxDataJSON);

    // serialOutput_sendString("records_array_size2=%d", records_array_size);
    rxDataArray = malloc(records_array_size);
    for (size_t i = 0; i < records_array_size; i++)
    {
        char *data = cJSON_GetArrayItem(rxDataJSON, i)->valuestring;
        rxDataArray[i] = (int)strtol(data, NULL, 16);
    }
    tempBuffer[0] = (int)strtol(id, NULL, 16);
    tempBuffer[1] = (int)strtol(Request_Id, NULL, 16);

    if (uds_communication_fnc != NULL)
    {
        service_was_ok = uds_communication_fnc(tempBuffer, 2, rxDataArray, records_array_size);
    }

    if (tempBuffer)
        free(tempBuffer);
    if (rxDataArray)
        free(rxDataArray);

    return service_was_ok;
}
static const flashLoader_obj_t flash_obj[] = {
    {"CommunicationControl", &CommunicationControl28_fnc},
    {"SecurityAccess", &SecurityAccess27_fnc},
    {"DiagnosticSessionControl", &DiagnosticSessionControl10_fnc},
    {"WriteDataByIdentifier", &WriteDataByIdentifier2E_fnc},
    {"RoutineControl", &RoutineControl31_fnc},
    {"RequestDownload", &RequestDownload34_fnc},
    {"TransferData", &TransferData36_fnc},
    {"RequestTransferExit", &RequestTransferExit37_fnc},
    {"ECUReset", &ECUReset11_fnc},

};

void uds_service_init()
{
    appl_messagefile_read_size = 0;
    appl_crcfile = 0;
    dataReadLength = 0;

    uds_communication_fnc = get_communication_if();
}
const flashLoader_obj_t *uds_service_getData(int *numberOfServices)
{
#define SERVICE_LENGTH sizeof(flash_obj) / sizeof(flash_obj[0])
    *numberOfServices = SERVICE_LENGTH;
    return flash_obj;
}
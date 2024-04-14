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
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "mqtt_HiveMQ_if.h"

static esp_mqtt_client_handle_t client = NULL;
QueueHandle_t mqtt_hiveMQ_queue;

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_mqtt_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_mqtt_pem_end");

#define CONFIG_BROKER_URL "mqtts://xxx.s2.eu.hivemq.cloud"  // you need to add "mqtts://" to your hivemq default link
#define CONFIG_BROKER_PORT 8883
#define CONFIG_USER "addYouUSER"
#define CONFIG_USER_PASSWRD "addYouPassword"
/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_CONNECTED\n", COLOR_GREEN);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        // serialOutput_sendString("%sCanAPPL : sent publish successful, msg_id=%d\n", COLOR_GREEN, msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_DISCONNECTED\n", COLOR_GREEN);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", COLOR_GREEN, event->msg_id);

        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", COLOR_GREEN, event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_PUBLISHED, msg_id=%d\n", COLOR_GREEN, event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_DATA\n", COLOR_GREEN);
        serialOutput_sendString("%sTOPIC=%.*s\r\n", COLOR_GREEN, event->topic_len, event->topic);
        serialOutput_sendString("%sDATA=%.*s\r\n", COLOR_GREEN, event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_ERROR\n", COLOR_GREEN);
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            serialOutput_sendString("%sreported from esp-tls", COLOR_RED, event->error_handle->esp_tls_last_esp_err);
            serialOutput_sendString("%sreported from tls stack", COLOR_RED, event->error_handle->esp_tls_stack_err);
            serialOutput_sendString("%scaptured as transport's socket errno", COLOR_RED, event->error_handle->esp_transport_sock_errno);
            serialOutput_sendString("%sCanAPPL : Last errno string (%s)\n", COLOR_GREEN, strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        serialOutput_sendString("%sCanAPPL : Other event id:%d\n", COLOR_GREEN, event->event_id);
        break;
    }
}
static void hiveMQ_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
        .port = CONFIG_BROKER_PORT,
        .username = CONFIG_USER,
        .password = CONFIG_USER_PASSWRD,
        .cert_pem = (const char *)server_cert_pem_start,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

static void mqtt_appl_Task(void *TaskArg)
{

    uint32_t diagnose_event;
    char *startOta_Buffer = (char *)malloc(1024);
    while (1)
    {
        if (xQueueReceive(mqtt_hiveMQ_queue, &diagnose_event, portMAX_DELAY))
        {
            switch (diagnose_event)
            {
            case 1:
            {
                appl_message_t messagefile;
                bool file_opened = appl_read_file_sync(&messagefile, (char *)"/sdcard/uds_ota_flashLoader/flash_driver.bin", "rb");

                if (file_opened)
                {
                    /*print START_OTA_TOPIC int send buffer*/
                    int l = sprintf(startOta_Buffer, START_OTA_TOPIC);
                    /*print file length */
                    l += sprintf(&startOta_Buffer[l], "%d", messagefile.length);
                    int msg_id = esp_mqtt_client_publish(client, START_OTA_TOPIC, startOta_Buffer, l, 0, 0);
                    serialOutput_sendString("%sCanAPPL : sent publish successful, msg_id=%d\n", COLOR_GREEN, msg_id);
                    vTaskDelay(5000);
                    for (size_t i = 0; i < messagefile.length; i += 150000)
                    {
                        int requireLen = ((i + 150000) <= messagefile.length) ? 150000 : messagefile.length - i;
                        memcpy(startOta_Buffer, &messagefile.bufferPtr[i], requireLen);
                        msg_id = esp_mqtt_client_publish(client, OTA_DATA_TOPIC, startOta_Buffer, requireLen, 0, 0);
                        serialOutput_sendString("%sCanAPPL : sent publish successful, messagefile.length=%d\n", COLOR_GREEN, messagefile.length);
                    }

                    msg_id = esp_mqtt_client_publish(client, END_DATA_TOPIC, END_DATA_TOPIC, 0, 0, 0);
                    serialOutput_sendString("%sCanAPPL : sent publish successful, messagefile.length=%d\n", COLOR_GREEN, messagefile.length);
                }
            }

            break;

            default:
                break;
            }
        }
    }
}

void mqtt_hiveMQ_publisher_init(void)
{
    serialOutput_sendString("mqtt_hiveMQ_publisher_init started\r\n");
    mqtt_hiveMQ_queue = xQueueCreate(100, 4);

    hiveMQ_app_start();

    xTaskCreate(mqtt_appl_Task, "mqtt_appl_Task", 3000, (void *)0, 5, NULL);
}
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

extern QueueHandle_t status_queue;
static esp_mqtt_client_handle_t client = NULL;
static QueueHandle_t appl_diagnose_queue;

#define MESSAGE_0x502_LENGTH 4

#define CONFIG_BROKER_URL "192.168.0.81"
#define CONFIG_BROKER_PORT 1883
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
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_CONNECTED\n", COLOR_GREEN);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        // serialOutput_sendString("%sCanAPPL : sent publish successful, msg_id=%d\n", COLOR_GREEN, msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "Room0", 0);
        // serialOutput_sendString("%sCanAPPL : sent subscribe successful, msg_id=%d\n", COLOR_GREEN, msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // serialOutput_sendString("%sCanAPPL : sent subscribe successful, msg_id=%d\n", COLOR_GREEN, msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // serialOutput_sendString("%sCanAPPL : sent unsubscribe successful, msg_id=%d\n", COLOR_GREEN, msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_DISCONNECTED\n", COLOR_GREEN);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        serialOutput_sendString("%sCanAPPL : MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", COLOR_GREEN, event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        serialOutput_sendString("%sCanAPPL : sent publish successful, msg_id=%d\n", COLOR_GREEN, msg_id);
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
static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .host = CONFIG_BROKER_URL,
        .port = CONFIG_BROKER_PORT,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
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

    xQueueSend(appl_diagnose_queue, data, pdMS_TO_TICKS(1));
}
static void mqtt_appl_Task(void *TaskArg)
{

    uint8_t *diagnose_event = (uint8_t *)malloc(100 * 16);
    int counter = 0;
    int64_t startTime = 0;
    while (1)
    {
        if (xQueueReceive(appl_diagnose_queue, &diagnose_event[counter], portMAX_DELAY))
        {
            counter += MESSAGE_0x502_LENGTH;
            if (counter == 100 * 16 || ((esp_timer_get_time() - startTime) >= (1000 * 1000)))
            {
                startTime = esp_timer_get_time();
                int msg_id = esp_mqtt_client_publish(client, "Room0", (const char *)diagnose_event, counter, 1, 0);
                counter = 0;
            }
        }
    }
}

void canAppl_init(void)
{
    serialOutput_sendString("canAppl: can Message started\r\n");
    appl_diagnose_queue = xQueueCreate(100, MESSAGE_0x502_LENGTH);
    /*attach handler to be called on receiving message 0x502 on channel 1*/
    CanIf_attach_receive(1, 0x502U, &canMessageReceive_502);

    uint8_t connection;
    nvsIf_get_connection_var(&connection);
    if (connection == 1) /*only wifi mode*/
    {
        mqtt_app_start();
    }

 
    xTaskCreate(mqtt_appl_Task, "mqtt_appl_Task", 3000, (void *)0, 5, NULL);
}
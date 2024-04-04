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

#ifndef ASYNC_DIAG_C
#define ASYNC_DIAG_C

#define MAX_LENGTH_SERVICE 4096

#define NAD_TASK1  0x01
#define NAD_TASK2  0x01
#define NAD_TASK3  0x01

typedef struct
{
    uint8_t NAD; /*  node adress value from the target node */
    uint16_t rx_data_length;
    uint8_t *rx_data; 
} async_uds_diagnose_rx_t;

static void local_send_diagnose(async_uds_diagnose_rx_t *async_uds_diagnose_rx, uint8_t NAD, uint16_t data_length, const uint8_t *data)
{
    int32_t handle;
    LinIf_TpTransmitSync(&handle, NAD, data_length, data);
    if (handle != -1)
    {
        /* retrieve if data is available by passing NULL pointer for data argument*/
        LinIf_TpReceiveSync(handle, &async_uds_diagnose_rx->NAD, &async_uds_diagnose_rx->rx_data_length, NULL, portMAX_DELAY);
        if (async_uds_diagnose_rx->rx_data_length > 0)
        {
            assert(NAD == async_uds_diagnose_rx->NAD);
            async_uds_diagnose_rx->rx_data = malloc(async_uds_diagnose_rx->rx_data_length);
            LinIf_TpReceiveSync(handle, &async_uds_diagnose_rx->NAD, &async_uds_diagnose_rx->rx_data_length, async_uds_diagnose_rx->rx_data, portMAX_DELAY);
        }
    }
}
static async_uds_diagnose_rx_t *diag_assign_frame_id_range(
    
    uint8_t NAD,
    /* specifies which is the first frame to assign a PID */
    uint8_t start_index,
    /* list of protected identifier */
    const uint8_t *PIDs)
{
    static async_uds_diagnose_rx_t my_async_uds_diagnose_rx;

    memset(&my_async_uds_diagnose_rx, 0, sizeof(my_async_uds_diagnose_rx));

    /* pack data into a single frame */
    uint8_t internal_temp_buffer[6];

    internal_temp_buffer[0] = 0xB7;
    internal_temp_buffer[1] = start_index;
    internal_temp_buffer[2] = PIDs[0];
    internal_temp_buffer[3] = PIDs[1];
    internal_temp_buffer[4] = PIDs[2];
    internal_temp_buffer[5] = PIDs[3];
    /* put data into TX_QUEUE */
    local_send_diagnose(&my_async_uds_diagnose_rx, NAD, 6, internal_temp_buffer);

    return &my_async_uds_diagnose_rx;
}

static async_uds_diagnose_rx_t *diag_read_data_by_identifier(const uint8_t NAD, const uint8_t *data)
{
    static async_uds_diagnose_rx_t my_async_uds_diagnose_rx = {0};

    memset(&my_async_uds_diagnose_rx, 0, sizeof(my_async_uds_diagnose_rx));

    /* pack data */
    uint8_t internal_temp_buffer[3];

    internal_temp_buffer[0] = 0x22;
    internal_temp_buffer[1] = data[0];
    internal_temp_buffer[2] = data[1];

    /* send message to transport layer */
    local_send_diagnose(&my_async_uds_diagnose_rx, NAD, 3, internal_temp_buffer);

    return &my_async_uds_diagnose_rx;
}

static async_uds_diagnose_rx_t *diag_write_data_by_identifier(const uint8_t NAD, uint16_t data_length, const uint8_t *data)
{
    uint16_t i;
    static async_uds_diagnose_rx_t my_async_uds_diagnose_rx = {0};
    memset(&my_async_uds_diagnose_rx, 0, sizeof(my_async_uds_diagnose_rx));

    /* Check if length of data is greater than length maximum */
    if (data_length < MAX_LENGTH_SERVICE)
    {

        uint8_t *internal_temp_buffer = malloc(data_length + 1);
        /* pack data */
        internal_temp_buffer[0] = 0x2E;

        for (i = 0; i < data_length; i++)
        {
            internal_temp_buffer[i + 1] = data[i];
        }
        local_send_diagnose(&my_async_uds_diagnose_rx, NAD, data_length + 1, internal_temp_buffer);
        free(internal_temp_buffer);
    }
    return &my_async_uds_diagnose_rx;
}

static async_uds_diagnose_rx_t *diag_send_service(const uint8_t NAD, const uint8_t SID, uint16_t data_length, const uint8_t *data)
{
    uint16_t i;
    static async_uds_diagnose_rx_t my_async_uds_diagnose_rx = {0};
    memset(&my_async_uds_diagnose_rx, 0, sizeof(my_async_uds_diagnose_rx));

    /* Check if length of data is greater than length maximum */
    if (data_length < MAX_LENGTH_SERVICE)
    {
        uint8_t *internal_temp_buffer = malloc(data_length + 1);
        /* pack data */
        internal_temp_buffer[0] = SID;

        for (i = 0; i < data_length; i++)
        {
            internal_temp_buffer[i + 1] = data[i];
        }
        local_send_diagnose(&my_async_uds_diagnose_rx, NAD, data_length + 1, internal_temp_buffer);
        free(internal_temp_buffer);
    }
    return &my_async_uds_diagnose_rx;
}

/* ----------------- Session control -------------------- */
static async_uds_diagnose_rx_t *diag_session_control(const uint8_t NAD, uint16_t data_length, const uint8_t *data)
{
    uint16_t i;
    static async_uds_diagnose_rx_t my_async_uds_diagnose_rx = {0};
    memset(&my_async_uds_diagnose_rx, 0, sizeof(my_async_uds_diagnose_rx));

    /* Check if length of data is greater than length maximum */
    if (data_length < MAX_LENGTH_SERVICE)
    {
        uint8_t *internal_temp_buffer = malloc(data_length + 1);
        /* pack data */
        internal_temp_buffer[0] = 0x10;

        for (i = 0; i < data_length; i++)
        {
            internal_temp_buffer[i + 1] = data[i];
        }
        local_send_diagnose(&my_async_uds_diagnose_rx, NAD, data_length + 1, internal_temp_buffer);
        free(internal_temp_buffer);
    }
    return &my_async_uds_diagnose_rx;
}
static async_uds_diagnose_rx_t *diag_read_status(const uint8_t NAD, uint16_t data_length, const uint8_t *data)
{

    uint16_t i;
    static async_uds_diagnose_rx_t my_async_uds_diagnose_rx = {0};
    memset(&my_async_uds_diagnose_rx, 0, sizeof(my_async_uds_diagnose_rx));

    /* Check if length of data is greater than length maximum */
    if (data_length < MAX_LENGTH_SERVICE)
    {
        uint8_t *internal_temp_buffer = malloc(data_length + 1);
        /* pack data */
        internal_temp_buffer[0] = 0x31;

        for (i = 0; i < data_length; i++)
        {
            internal_temp_buffer[i + 1] = data[i];
        }
        local_send_diagnose(&my_async_uds_diagnose_rx, NAD, data_length + 1, internal_temp_buffer);
        free(internal_temp_buffer);
    }
    return &my_async_uds_diagnose_rx;
}
/* ----------------- Fault memory -------------------- */
static async_uds_diagnose_rx_t *diag_fault_memory_read(const uint8_t NAD, uint16_t data_length, const uint8_t *data)
{
    uint16_t i;
    static async_uds_diagnose_rx_t my_async_uds_diagnose_rx = {0};
    memset(&my_async_uds_diagnose_rx, 0, sizeof(my_async_uds_diagnose_rx));

    /* Check if length of data is greater than length maximum */
    if (data_length < MAX_LENGTH_SERVICE)
    {
        uint8_t *internal_temp_buffer = malloc(data_length + 1);
        /* pack data */
        internal_temp_buffer[0] = 0x19;

        for (i = 0; i < data_length; i++)
        {
            internal_temp_buffer[i + 1] = data[i];
        }
        local_send_diagnose(&my_async_uds_diagnose_rx, NAD, data_length + 1, internal_temp_buffer);
        free(internal_temp_buffer);
    }
    return &my_async_uds_diagnose_rx;
}
static async_uds_diagnose_rx_t *diag_fault_memory_clear(const uint8_t NAD, uint16_t data_length, const uint8_t *data)
{
    uint16_t i;
    static async_uds_diagnose_rx_t my_async_uds_diagnose_rx = {0};
    memset(&my_async_uds_diagnose_rx, 0, sizeof(my_async_uds_diagnose_rx));

    /* Check if length of data is greater than length maximum */
    if (data_length < MAX_LENGTH_SERVICE)
    {
        uint8_t *internal_temp_buffer = malloc(data_length + 1);
        /* pack data */
        internal_temp_buffer[0] = 0x14;

        for (i = 0; i < data_length; i++)
        {
            internal_temp_buffer[i + 1] = data[i];
        }
        local_send_diagnose(&my_async_uds_diagnose_rx, NAD, data_length + 1, internal_temp_buffer);
        free(internal_temp_buffer);
    }
    return &my_async_uds_diagnose_rx;
}
/* ----------------- I/O control -------------------- */
static async_uds_diagnose_rx_t *diag_IO_control(const uint8_t NAD, uint16_t data_length, const uint8_t *data)
{
    uint16_t i;
    static async_uds_diagnose_rx_t my_async_uds_diagnose_rx = {0};

    memset(&my_async_uds_diagnose_rx, 0, sizeof(my_async_uds_diagnose_rx));

    /* Check if length of data is greater than length maximum */
    if (data_length < MAX_LENGTH_SERVICE)
    {
        uint8_t *internal_temp_buffer = malloc(data_length + 1);
        /* pack data */
        internal_temp_buffer[0] = 0x2F;

        for (i = 0; i < data_length; i++)
        {
            internal_temp_buffer[i + 1] = data[i];
        }
        local_send_diagnose(&my_async_uds_diagnose_rx, NAD, data_length + 1, internal_temp_buffer);
        free(internal_temp_buffer);
    }
    return &my_async_uds_diagnose_rx;
}
void asyncDiagTask1(void *TaskArg)
{
    const uint8_t PIDs[3] = {0xAA, 0x55, 0xF0};
    const uint8_t buffer_read_data_by_identifier[2] = {0xF0, 0x10};
    const uint8_t buffer_write_data_by_identifier[4] = {0xF0, 0x10, 0xAA, 0x55};
    while (1)
    {
        async_uds_diagnose_rx_t *async_uds_diagnose_rx1 = diag_assign_frame_id_range(NAD_TASK1,  0, PIDs);
        if (async_uds_diagnose_rx1->rx_data_length > 0)
        {
            serialOutput_send_buffer_hex("diag_assign_frame_id_range",COLOR_GREEN, async_uds_diagnose_rx1->rx_data, async_uds_diagnose_rx1->rx_data_length);
            free(async_uds_diagnose_rx1->rx_data);
        }
        else
        {
            break;
        }
        async_uds_diagnose_rx1 = diag_read_data_by_identifier(NAD_TASK1,  buffer_read_data_by_identifier);
        if (async_uds_diagnose_rx1->rx_data_length > 0)
        {
            serialOutput_send_buffer_hex("diag_read_data_by_identifier",COLOR_GREEN, async_uds_diagnose_rx1->rx_data, async_uds_diagnose_rx1->rx_data_length);
            free(async_uds_diagnose_rx1->rx_data);
        }
        else
        {
            break;
        }
        async_uds_diagnose_rx1 = diag_write_data_by_identifier(NAD_TASK1,  sizeof(buffer_write_data_by_identifier), buffer_write_data_by_identifier);
        if (async_uds_diagnose_rx1->rx_data_length > 0)
        {
            serialOutput_send_buffer_hex("diag_write_data_by_identifier",COLOR_GREEN, async_uds_diagnose_rx1->rx_data, async_uds_diagnose_rx1->rx_data_length);
            free(async_uds_diagnose_rx1->rx_data);
        }
        else
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    vTaskDelete(NULL);
}
void asyncDiagTask2(void *TaskArg)
{
    const uint8_t uds_service_data[4] = {0xFF, 0xFE, 0xAA, 0x55};
    const uint8_t diag_session_control_data[1] = {0x02};
    const uint8_t diag_read_status_data[4] = {0xF0, 0xF1, 0xA5, 0x5A};
    while (1)
    {

        async_uds_diagnose_rx_t *async_uds_diagnose_rx1 = diag_send_service(NAD_TASK2,  0x31, sizeof(uds_service_data), uds_service_data);
        if (async_uds_diagnose_rx1->rx_data_length > 0)
        {
            serialOutput_send_buffer_hex("diag_send_service",COLOR_YELLOW, async_uds_diagnose_rx1->rx_data, async_uds_diagnose_rx1->rx_data_length);
            free(async_uds_diagnose_rx1->rx_data);
        }
        else
        {
            break;
        }
        async_uds_diagnose_rx1 = diag_session_control(NAD_TASK2,  sizeof(diag_session_control_data), diag_session_control_data);
        if (async_uds_diagnose_rx1->rx_data_length > 0)
        {
            serialOutput_send_buffer_hex("diag_session_control",COLOR_YELLOW, async_uds_diagnose_rx1->rx_data, async_uds_diagnose_rx1->rx_data_length);
            free(async_uds_diagnose_rx1->rx_data);
        }
        else
        {
            break;
        }
        async_uds_diagnose_rx1 = diag_read_status(NAD_TASK2,  sizeof(diag_read_status_data), diag_read_status_data);
        if (async_uds_diagnose_rx1->rx_data_length > 0)
        {
            serialOutput_send_buffer_hex("diag_read_status",COLOR_YELLOW, async_uds_diagnose_rx1->rx_data, async_uds_diagnose_rx1->rx_data_length);
            free(async_uds_diagnose_rx1->rx_data);
        }
        else
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    vTaskDelete(NULL);
}
void asyncDiagTask3(void *TaskArg)
{
    const uint8_t diag_fault_memory_read_data[3] = {0xAA, 0x55, 0xF0};
    const uint8_t diag_fault_memory_clear_data[2] = {0xF0, 0x10};
    const uint8_t diag_IO_control_data[4] = {0xF0, 0x10, 0xAA, 0x55};
    while (1)
    {

        async_uds_diagnose_rx_t *async_uds_diagnose_rx1 = diag_fault_memory_read(NAD_TASK3,  sizeof(diag_fault_memory_read_data), diag_fault_memory_read_data);
        if (async_uds_diagnose_rx1->rx_data_length > 0)
        {
            serialOutput_send_buffer_hex("diag_fault_memory_read",COLOR_CYAN, async_uds_diagnose_rx1->rx_data, async_uds_diagnose_rx1->rx_data_length);
            free(async_uds_diagnose_rx1->rx_data);
        }
        else
        {
            break;
        }
        async_uds_diagnose_rx1 = diag_fault_memory_clear(NAD_TASK3,  sizeof(diag_fault_memory_clear_data), diag_fault_memory_clear_data);
        if (async_uds_diagnose_rx1->rx_data_length > 0)
        {
            serialOutput_send_buffer_hex("diag_fault_memory_clear",COLOR_CYAN, async_uds_diagnose_rx1->rx_data, async_uds_diagnose_rx1->rx_data_length);
            free(async_uds_diagnose_rx1->rx_data);
        }
        else
        {
            break;
        }
        async_uds_diagnose_rx1 = diag_IO_control(NAD_TASK3,  sizeof(diag_IO_control_data), diag_IO_control_data);
        if (async_uds_diagnose_rx1->rx_data_length > 0)
        {
            serialOutput_send_buffer_hex("diag_IO_control",COLOR_CYAN, async_uds_diagnose_rx1->rx_data, async_uds_diagnose_rx1->rx_data_length);
            free(async_uds_diagnose_rx1->rx_data);
        }
        else
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    vTaskDelete(NULL);
}
void diag_init()
{
    xTaskCreatePinnedToCore(asyncDiagTask1, "asyncDiagTask1", 3020, (void *)0, 15, NULL, 0);
    xTaskCreatePinnedToCore(asyncDiagTask2, "asyncDiagTask2", 3020, (void *)0, 5, NULL, 1);
    xTaskCreatePinnedToCore(asyncDiagTask3, "asyncDiagTask3", 3020, (void *)0, 10, NULL, 0);
}
#endif
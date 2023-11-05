/* OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "errno.h"
#include "main_if.h"
#include "uds_https_ota.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <errno.h>
#include <sys/param.h>
#include "esp_crt_bundle.h"
#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

#define BUFFSIZE 1024
#define HASH_LEN 32 /* SHA-256 digest length */
#define OTA_URL_SIZE 256
// #define CONFIG_EXAMPLE_ENABLE_PARTIAL_HTTP_DOWNLOAD
#define CONFIG_EXAMPLE_HTTP_REQUEST_SIZE BUFFSIZE

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

#define OTA_URL_SIZE 256

#define IMAGE_HEADER_SIZE sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t) + 1
#define DEFAULT_OTA_BUF_SIZE IMAGE_HEADER_SIZE
#define DEFAULT_REQUEST_SIZE (64 * 1024)

typedef enum
{
    uds_https_ota_INIT,
    uds_https_ota_BEGIN,
    uds_https_ota_IN_PROGRESS,
    uds_https_ota_SUCCESS,
} uds_https_ota_state;

struct uds_https_ota_handle
{
    esp_http_client_handle_t http_client;
    char *ota_upgrade_buf;
    char *ota_total_buf;
    size_t ota_upgrade_buf_size;
    int binary_file_len;
    int image_length;
    int max_http_request_size;
    uds_https_ota_state state;
    bool bulk_flash_erase;
    bool partial_http_download;
};

typedef struct uds_https_ota_handle uds_https_ota_t;

static bool redirection_required(int status_code)
{
    switch (status_code)
    {
    case HttpStatus_MovedPermanently:
    case HttpStatus_Found:
    case HttpStatus_SeeOther:
    case HttpStatus_TemporaryRedirect:
    case HttpStatus_PermanentRedirect:
        return true;
    default:
        return false;
    }
    return false;
}

static bool process_again(int status_code)
{
    switch (status_code)
    {
    case HttpStatus_MovedPermanently:
    case HttpStatus_Found:
    case HttpStatus_SeeOther:
    case HttpStatus_TemporaryRedirect:
    case HttpStatus_PermanentRedirect:
    case HttpStatus_Unauthorized:
        return true;
    default:
        return false;
    }
    return false;
}

static esp_err_t _http_handle_response_code(esp_http_client_handle_t http_client, int status_code)
{
    esp_err_t err;
    if (redirection_required(status_code))
    {
        err = esp_http_client_set_redirection(http_client);
        if (err != ESP_OK)
        {
            serialOutput_sendString("URL redirection Failed");
            return err;
        }
    }
    else if (status_code == HttpStatus_Unauthorized)
    {
        esp_http_client_add_auth(http_client);
    }
    else if (status_code == HttpStatus_NotFound || status_code == HttpStatus_Forbidden)
    {
        serialOutput_sendString("File not found(%d)", status_code);
        return ESP_FAIL;
    }
    else if (status_code >= HttpStatus_BadRequest && status_code < HttpStatus_InternalError)
    {
        serialOutput_sendString("Client error (%d)", status_code);
        return ESP_FAIL;
    }
    else if (status_code >= HttpStatus_InternalError)
    {
        serialOutput_sendString("Server error (%d)", status_code);
        return ESP_FAIL;
    }

    char upgrade_data_buf[DEFAULT_OTA_BUF_SIZE];
    // process_again() returns true only in case of redirection.
    if (process_again(status_code))
    {
        while (1)
        {
            /*
             *  In case of redirection, esp_http_client_read() is called
             *  to clear the response buffer of http_client.
             */
            int data_read = esp_http_client_read(http_client, upgrade_data_buf, DEFAULT_OTA_BUF_SIZE);
            if (data_read <= 0)
            {
                return ESP_OK;
            }
        }
    }
    return ESP_OK;
}

static esp_err_t _http_connect(esp_http_client_handle_t http_client)
{
    esp_err_t err = ESP_FAIL;
    int status_code, header_ret;
    do
    {
        char *post_data = NULL;
        /* Send POST request if body is set.
         * Note: Sending POST request is not supported if partial_http_download
         * is enabled
         */
        int post_len = esp_http_client_get_post_field(http_client, &post_data);
        err = esp_http_client_open(http_client, post_len);
        if (err != ESP_OK)
        {
            serialOutput_sendString("Failed to open HTTP connection: %s", esp_err_to_name(err));
            return err;
        }
        if (post_len)
        {
            int write_len = 0;
            while (post_len > 0)
            {
                write_len = esp_http_client_write(http_client, post_data, post_len);
                if (write_len < 0)
                {
                    serialOutput_sendString("Write failed");
                    return ESP_FAIL;
                }
                post_len -= write_len;
                post_data += write_len;
            }
        }
        header_ret = esp_http_client_fetch_headers(http_client);
        if (header_ret < 0)
        {
            return header_ret;
        }
        status_code = esp_http_client_get_status_code(http_client);
        err = _http_handle_response_code(http_client, status_code);
        if (err != ESP_OK)
        {
            return err;
        }
    } while (process_again(status_code));
    return err;
}

static void _http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

static esp_err_t _ota_write(uds_https_ota_t *https_ota_handle, const void *buffer, size_t buf_len)
{
    if (buffer == NULL || https_ota_handle == NULL)
    {
        return ESP_FAIL;
    }
    memcpy(&https_ota_handle->ota_total_buf[https_ota_handle->binary_file_len], buffer, buf_len);
    https_ota_handle->binary_file_len += buf_len;

    return ESP_ERR_HTTPS_OTA_IN_PROGRESS;
}

static bool is_server_verification_enabled(uds_https_ota_config_t *ota_config)
{
    return (ota_config->http_config->cert_pem || ota_config->http_config->use_global_ca_store || ota_config->http_config->crt_bundle_attach != NULL);
}

esp_err_t uds_https_ota_begin(uds_https_ota_config_t *ota_config, uds_https_ota_handle_t *handle)
{
    esp_err_t err;

    if (handle == NULL || ota_config == NULL || ota_config->http_config == NULL)
    {
        serialOutput_sendString("uds_https_ota_begin: Invalid argument");
        if (handle)
        {
            *handle = NULL;
        }
        return ESP_ERR_INVALID_ARG;
    }

    if (!is_server_verification_enabled(ota_config))
    {
#if CONFIG_OTA_ALLOW_HTTP
        ESP_LOGW(TAG, "Continuing with insecure option because CONFIG_OTA_ALLOW_HTTP is set.");
#else
        serialOutput_sendString("No option for server verification is enabled in esp_http_client config.");
        *handle = NULL;
        return ESP_ERR_INVALID_ARG;
#endif
    }

    uds_https_ota_t *https_ota_handle = calloc(1, sizeof(uds_https_ota_t));
    if (!https_ota_handle)
    {
        serialOutput_sendString("Couldn't allocate memory to upgrade data buffer");
        *handle = NULL;
        return ESP_ERR_NO_MEM;
    }

    https_ota_handle->partial_http_download = ota_config->partial_http_download;
    https_ota_handle->max_http_request_size = DEFAULT_REQUEST_SIZE;

    /* Initiate HTTP Connection */
    https_ota_handle->http_client = esp_http_client_init(ota_config->http_config);
    if (https_ota_handle->http_client == NULL)
    {
        serialOutput_sendString("Failed to initialise HTTP connection");
        err = ESP_FAIL;
        goto failure;
    }

    if (ota_config->http_client_init_cb)
    {
        err = ota_config->http_client_init_cb(https_ota_handle->http_client);
        if (err != ESP_OK)
        {
            serialOutput_sendString("http_client_init_cb returned 0x%x", err);
            goto http_cleanup;
        }
    }

    if (https_ota_handle->partial_http_download)
    {
        esp_http_client_set_method(https_ota_handle->http_client, HTTP_METHOD_HEAD);
        err = esp_http_client_perform(https_ota_handle->http_client);
        if (err == ESP_OK)
        {
            int status = esp_http_client_get_status_code(https_ota_handle->http_client);
            if (status != HttpStatus_Ok)
            {
                serialOutput_sendString("Received incorrect http status %d", status);
                err = ESP_FAIL;
                goto http_cleanup;
            }
        }
        else
        {
            serialOutput_sendString("ESP HTTP client perform failed: %d", err);
            goto http_cleanup;
        }

        https_ota_handle->image_length = esp_http_client_get_content_length(https_ota_handle->http_client);
        serialOutput_sendString("https_ota_handle->image_length: %d", https_ota_handle->image_length);
        esp_http_client_close(https_ota_handle->http_client);

        if (https_ota_handle->image_length > https_ota_handle->max_http_request_size)
        {
            char *header_val = NULL;
            asprintf(&header_val, "bytes=0-%d", https_ota_handle->max_http_request_size - 1);

            if (header_val == NULL)
            {
                serialOutput_sendString("Failed to allocate memory for HTTP header");
                err = ESP_ERR_NO_MEM;
                goto http_cleanup;
            }
            esp_http_client_set_header(https_ota_handle->http_client, "Range", header_val);
            free(header_val);
        }
        esp_http_client_set_method(https_ota_handle->http_client, HTTP_METHOD_GET);
    }

    err = _http_connect(https_ota_handle->http_client);
    if (err != ESP_OK)
    {
        serialOutput_sendString("Failed to establish HTTP connection");
        goto http_cleanup;
    }

    if (!https_ota_handle->partial_http_download)
    {
        https_ota_handle->image_length = esp_http_client_get_content_length(https_ota_handle->http_client);
        serialOutput_sendString("https_ota_handle->image_length: %d", https_ota_handle->image_length);
    }
    https_ota_handle->ota_upgrade_buf = (char *)malloc(https_ota_handle->max_http_request_size);
    https_ota_handle->ota_total_buf = (char *)malloc(https_ota_handle->image_length);
    if (!https_ota_handle->ota_upgrade_buf || !https_ota_handle->ota_total_buf)
    {
        serialOutput_sendString("Couldn't allocate memory to upgrade data buffer");
        err = ESP_ERR_NO_MEM;
        goto http_cleanup;
    }
    https_ota_handle->ota_upgrade_buf_size = https_ota_handle->max_http_request_size;
    https_ota_handle->bulk_flash_erase = ota_config->bulk_flash_erase;
    https_ota_handle->binary_file_len = 0;
    *handle = (uds_https_ota_handle_t)https_ota_handle;
    https_ota_handle->state = uds_https_ota_BEGIN;
    return ESP_OK;

http_cleanup:
    _http_cleanup(https_ota_handle->http_client);
failure:
    free(https_ota_handle);
    *handle = NULL;
    return err;
}
esp_err_t uds_https_ota_perform(uds_https_ota_handle_t https_ota_handle)
{
    static int read = 0;
    uds_https_ota_t *handle = (uds_https_ota_t *)https_ota_handle;
    if (handle == NULL)
    {
        serialOutput_sendString("uds_https_ota_perform: Invalid argument");
        return ESP_ERR_INVALID_ARG;
    }
    if (handle->state < uds_https_ota_BEGIN)
    {
        serialOutput_sendString("uds_https_ota_perform: Invalid state");
        return ESP_FAIL;
    }

    esp_err_t err;
    int data_read;
    int len2 = 0;
    switch (handle->state)
    {
    case uds_https_ota_BEGIN:

        handle->state = uds_https_ota_IN_PROGRESS;
        /* In case `uds_https_ota_read_img_desc` was invoked first,
           then the image data read there should be written to OTA partition
           */
        int binary_file_len = 0;
        if (handle->binary_file_len)
        {
            /*
             * Header length gets added to handle->binary_file_len in _ota_write
             * Clear handle->binary_file_len to avoid additional 289 bytes in binary_file_len
             */
            binary_file_len = handle->binary_file_len;
            handle->binary_file_len = 0;
        }
        else
        {
            binary_file_len = IMAGE_HEADER_SIZE;
        }

        return ESP_ERR_HTTPS_OTA_IN_PROGRESS;
        break;
    case uds_https_ota_IN_PROGRESS:

        len2 = ((handle->ota_upgrade_buf_size + handle->binary_file_len) <= handle->image_length)
                   ? handle->ota_upgrade_buf_size
                   : handle->image_length - handle->binary_file_len;
        data_read = esp_http_client_read(handle->http_client,
                                         handle->ota_upgrade_buf, len2);
        serialOutput_sendString("Image bytes read%d: %d %d %d", ++read, data_read, len2, handle->ota_upgrade_buf_size);
        if (data_read == 0)
        {
            /*
             *  esp_http_client_is_complete_data_received is added to check whether
             *  complete image is received.
             */
            bool is_recv_complete = esp_http_client_is_complete_data_received(handle->http_client);
            /*
             * As esp_http_client_read doesn't return negative error code if select fails, we rely on
             * `errno` to check for underlying transport connectivity closure if any.
             * Incase the complete data has not been received but the server has sent
             * an ENOTCONN or ECONNRESET, failure is returned. We close with success
             * if complete data has been received.
             */
            if ((errno == ENOTCONN || errno == ECONNRESET || errno == ECONNABORTED) && !is_recv_complete)
            {
                serialOutput_sendString("Connection closed, errno = %d", errno);
                return ESP_FAIL;
            }
            else if (!is_recv_complete)
            {
                return ESP_ERR_HTTPS_OTA_IN_PROGRESS;
            }
            serialOutput_sendString("Connection closed");
            handle->state = uds_https_ota_SUCCESS;
        }
        else if (data_read > 0)
        {
            return _ota_write(handle, (const void *)handle->ota_upgrade_buf, data_read);
        }
        else
        {
            serialOutput_sendString("data read %d, errno %d", data_read, errno);
            return ESP_FAIL;
        }
        if (!handle->partial_http_download || (handle->partial_http_download && handle->image_length == handle->binary_file_len))
        {
            serialOutput_sendString("uds_https_ota_SUCCESS");
            handle->state = uds_https_ota_SUCCESS;
        }
        break;
    default:
        serialOutput_sendString("Invalid ESP HTTPS OTA State");
        return ESP_FAIL;
        break;
    }
    if (handle->partial_http_download)
    {
        if (handle->state == uds_https_ota_IN_PROGRESS && handle->image_length > handle->binary_file_len)
        {
            esp_http_client_close(handle->http_client);
            char *header_val = NULL;
            if ((handle->image_length - handle->binary_file_len) > handle->max_http_request_size)
            {
                asprintf(&header_val, "bytes=%d-%d", handle->binary_file_len, (handle->binary_file_len + handle->max_http_request_size - 1));
            }
            else
            {
                asprintf(&header_val, "bytes=%d-", handle->binary_file_len);
            }
            if (header_val == NULL)
            {
                serialOutput_sendString("Failed to allocate memory for HTTP header");
                return ESP_ERR_NO_MEM;
            }
            esp_http_client_set_header(handle->http_client, "Range", header_val);
            free(header_val);
            err = _http_connect(handle->http_client);
            if (err != ESP_OK)
            {
                serialOutput_sendString("Failed to establish HTTP connection");
                return ESP_FAIL;
            }
            serialOutput_sendString("Connection start");
            return ESP_ERR_HTTPS_OTA_IN_PROGRESS;
        }
    }
    return ESP_OK;
}

bool uds_https_ota_is_complete_data_received(uds_https_ota_handle_t https_ota_handle)
{
    bool ret = false;
    uds_https_ota_t *handle = (uds_https_ota_t *)https_ota_handle;
    if (handle->partial_http_download)
    {
        ret = (handle->image_length == handle->binary_file_len);
    }
    else
    {
        ret = esp_http_client_is_complete_data_received(handle->http_client);
    }
    return ret;
}

esp_err_t uds_https_ota_finish(uds_https_ota_handle_t https_ota_handle)
{
    uds_https_ota_t *handle = (uds_https_ota_t *)https_ota_handle;
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (handle->state < uds_https_ota_BEGIN)
    {
        return ESP_FAIL;
    }

    esp_err_t err = ESP_OK;
    switch (handle->state)
    {
    case uds_https_ota_SUCCESS:
    case uds_https_ota_IN_PROGRESS:
        /* falls through */
    case uds_https_ota_BEGIN:
        if (handle->ota_upgrade_buf)
        {
            free(handle->ota_upgrade_buf);
        }
        if (handle->http_client)
        {
            _http_cleanup(handle->http_client);
        }
        break;
    default:
        serialOutput_sendString("Invalid ESP HTTPS OTA State");
        break;
    }

    free(handle);
    return err;
}

esp_err_t uds_https_ota_abort(uds_https_ota_handle_t https_ota_handle)
{
    uds_https_ota_t *handle = (uds_https_ota_t *)https_ota_handle;
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (handle->state < uds_https_ota_BEGIN)
    {
        return ESP_FAIL;
    }

    esp_err_t err = ESP_OK;
    switch (handle->state)
    {
    case uds_https_ota_SUCCESS:
    case uds_https_ota_IN_PROGRESS:
        /* falls through */
    case uds_https_ota_BEGIN:
        if (handle->ota_upgrade_buf)
        {
            free(handle->ota_upgrade_buf);
        }
        if (handle->http_client)
        {
            _http_cleanup(handle->http_client);
        }
        break;
    default:
        err = ESP_ERR_INVALID_STATE;
        serialOutput_sendString("Invalid ESP HTTPS OTA State");
        break;
    }
    free(handle);
    return err;
}

int uds_https_ota_get_image_len_read(uds_https_ota_handle_t https_ota_handle)
{
    uds_https_ota_t *handle = (uds_https_ota_t *)https_ota_handle;
    if (handle == NULL)
    {
        return -1;
    }
    if (handle->state < uds_https_ota_IN_PROGRESS)
    {
        return -1;
    }
    return handle->binary_file_len;
}

int uds_https_ota_get_image_size(uds_https_ota_handle_t https_ota_handle)
{
    uds_https_ota_t *handle = (uds_https_ota_t *)https_ota_handle;
    if (handle == NULL)
    {
        return -1;
    }
    if (handle->state < uds_https_ota_BEGIN)
    {
        return -1;
    }
    return handle->image_length;
}

static esp_err_t _http_client_init_cb(esp_http_client_handle_t http_client)
{
    esp_err_t err = ESP_OK;
    /* Uncomment to add custom headers to HTTP request */
    // err = esp_http_client_set_header(http_client, "Custom-Header", "Value");
    return err;
}

char *advanced_ota_example(const char *fileName, uint32_t *fileLength)
{
    // serialOutput_sendString("Starting Advanced OTA example %.*s", (char *)server_cert_pem_end - (char *)server_cert_pem_start, (char *)server_cert_pem_start);
    serialOutput_sendString("download %s", fileName);

    esp_err_t ota_finish_err = ESP_OK;
    esp_http_client_config_t config = {
        .url = fileName,
        .cert_pem = (char *)server_cert_pem_start,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 50000,
        .keep_alive_enable = true,
        .buffer_size = 4096,
        .buffer_size_tx = 4096,
    };

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    uds_https_ota_config_t ota_config = {
        .http_config = &config,
        .http_client_init_cb = _http_client_init_cb, // Register a callback to be invoked after esp_http_client is initialized
#ifdef CONFIG_EXAMPLE_ENABLE_PARTIAL_HTTP_DOWNLOAD
        .partial_http_download = true,
#endif
        .max_http_request_size = CONFIG_EXAMPLE_HTTP_REQUEST_SIZE,

    };

    uds_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = uds_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK)
    {
        serialOutput_sendString("ESP HTTPS OTA Begin failed");
        return NULL;
    }

    while (1)
    {
        err = uds_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
        {
            serialOutput_sendString("err %d", err);
            break;
        }
        // uds_https_ota_perform returns after every read operation which gives user the ability to
        // monitor the status of OTA upgrade by calling uds_https_ota_get_image_len_read, which gives length of image
        // data read so far.
    }

    if (uds_https_ota_is_complete_data_received(https_ota_handle) != true)
    {
        // the OTA image was not completely received and user can customise the response to this situation.
        serialOutput_sendString("Complete data was not received.");
    }
    else
    {
        ota_finish_err = uds_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK))
        {
            serialOutput_sendString("uds_https_ota upgrade successful");
            uds_https_ota_t *handle = (uds_https_ota_t *)https_ota_handle;
            *fileLength = handle->binary_file_len;

            return handle->ota_total_buf;
        }
        else
        {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED)
            {
                serialOutput_sendString("Image validation failed, image is corrupted");
            }
            serialOutput_sendString("uds_https_ota upgrade failed 0x%x", ota_finish_err);
            return NULL;
        }
    }

    uds_https_ota_abort(https_ota_handle);
    serialOutput_sendString("uds_https_ota upgrade failed");
    return NULL;
}

char *uds_http_ota_task(const char *filePath, uint32_t *fileLength)
{
    char *buffer = NULL;
    buffer = advanced_ota_example(filePath, fileLength);
    return buffer;
}

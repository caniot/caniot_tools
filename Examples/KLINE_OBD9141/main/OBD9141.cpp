/*
 *  Copyright (c) 2015, Ivor Wanders
 *  MIT License, see the LICENSE.md file in the root folder.
 */
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


static uart_port_t myuart_port;
#define OBD_UART myuart_port

OBD9141::OBD9141(){};
void OBD9141::begin(uint8_t rx_pin, uint8_t tx_pin)
{
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
    uart_config_t uart_config = {
        .baud_rate = OBD9141_KLINE_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk=UART_SCLK_APB,
    };
    myuart_port = Kline_uart_init(&uart_config);
#if CONFIG_IDF_TARGET_ESP32S3
    gpio_set_direction(KLINE_NOT_SLEEP_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(KLINE_NOT_SLEEP_PIN, 1u);
#endif

    use_kwp_ = false;
}

void OBD9141::kline(bool enabled)
{
#if CONFIG_IDF_TARGET_ESP32
    {
        if (enabled)
        {
            uart_set_line_inverse(OBD_UART,UART_SIGNAL_RXD_INV | UART_SIGNAL_TXD_INV);
        }
        else
        {
            uart_set_line_inverse(OBD_UART, UART_SIGNAL_INV_DISABLE);
        }
    }
#elif CONFIG_IDF_TARGET_ESP32S3
    {
        if (!enabled)
        {
            uart_set_line_inverse(OBD_UART, UART_SIGNAL_TXD_INV);
        }
        else
        {
            uart_set_line_inverse(OBD_UART, UART_SIGNAL_INV_DISABLE);
        }
    }
#endif
}

uint8_t OBD9141::checksum(void *b, uint8_t len)
{
    uint8_t ret = 0;
    for (uint8_t i = 0; i < len; i++)
    {
        ret += reinterpret_cast<uint8_t *>(b)[i];
    }
    return ret;
}

void OBD9141::write(uint8_t b)
{
    uint8_t tmp[1]; // temporary variable to read into.
    uart_write_bytes(OBD_UART, (const char *)&b, 1);

    (void)uart_read_bytes(OBD_UART, tmp, 1, OBD9141_REQUEST_ECHO_MS_PER_BYTE * 1 + OBD9141_WAIT_FOR_ECHO_TIMEOUT);
}

void OBD9141::write(void *b, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        uart_write_bytes(OBD_UART, reinterpret_cast<const char *>(b + i), 1);
        vTaskDelay(OBD9141_INTERSYMBOL_WAIT);
    }
    uint8_t tmp[len]; // temporary variable to read into.
    (void)uart_read_bytes(OBD_UART, tmp, len, OBD9141_REQUEST_ECHO_MS_PER_BYTE * len + OBD9141_WAIT_FOR_ECHO_TIMEOUT);
}

bool OBD9141::request(void *request, uint8_t request_len, uint8_t ret_len)
{
    if (use_kwp_)
    {
        // have to modify the first bytes.
        uint8_t rbuf[request_len];
        memcpy(rbuf, request, request_len);
        // now we modify the header, the payload is the request_len - 3 header bytes
        rbuf[0] = (0b11 << 6) | (request_len - 3);
        rbuf[1] = 0x33; // second byte should be 0x33
        return (this->requestKWP(&rbuf, request_len) == ret_len);
    }
    return request9141(request, request_len, ret_len);
}

bool OBD9141::request9141(void *request, uint8_t request_len, uint8_t ret_len)
{
    uint8_t buf[request_len + 1];
    memcpy(buf, request, request_len); // copy request

    buf[request_len] = this->checksum(&buf, request_len); // add the checksum

    this->write(&buf, request_len + 1);

    // wait after the request, officially 30 ms, but we might as well wait
    // for the data in the readBytes function.

    // set proper timeout
    memset(this->buffer, 0, ret_len + 1);

    // OBD9141print("Trying to get x bytes: "); OBD9141println(ret_len+1);
    if (uart_read_bytes(OBD_UART, this->buffer, ret_len + 1, OBD9141_REQUEST_ANSWER_MS_PER_BYTE * ret_len + OBD9141_WAIT_FOR_REQUEST_ANSWER_TIMEOUT))
    {
        return (this->checksum(&(this->buffer[0]), ret_len) == this->buffer[ret_len]); // have data; return whether it is valid.
    }
    else
    {
        OBD9141println("Timeout on reading bytes.");
        return false; // failed getting data.
    }
}

uint8_t OBD9141::request(void *request, uint8_t request_len)
{
    if (use_kwp_)
    {
        // have to modify the first bytes.
        uint8_t rbuf[request_len];
        memcpy(rbuf, request, request_len);
        // now we modify the header, the payload is the request_len - 3 header bytes
        rbuf[0] = (0b11 << 6) | (request_len - 3);
        rbuf[1] = 0x33; // second byte should be 0x33
        return requestKWP(rbuf, request_len);
    }
    bool success = true;
    // wipe the entire buffer to ensure we are in a clean slate.
    memset(this->buffer, 0, OBD9141_BUFFER_SIZE);

    // create the request with checksum.
    uint8_t buf[request_len + 1];
    memcpy(buf, request, request_len);                    // copy request
    buf[request_len] = this->checksum(&buf, request_len); // add the checksum

    // manually write the bytes onto the serial port
    // this does NOT read the echoes.
    OBD9141print("W: ");
#ifdef OBD9141_DEBUG
    for (uint8_t i = 0; i < (request_len + 1); i++)
    {
        OBD9141print(buf[i]);
        OBD9141print(" ");
    };
    OBD9141println();
#endif
    for (uint8_t i = 0; i < request_len + 1; i++)
    {
        uart_write_bytes(OBD_UART, reinterpret_cast<const char *>(buf + i), 1);
        vTaskDelay(OBD9141_INTERSYMBOL_WAIT);
    }

    // next step, is to read the echo from the serial port.
    uint8_t tmp[request_len + 1]; // temporary variable to read into.
    uart_read_bytes(OBD_UART, tmp, request_len + 1, OBD9141_REQUEST_ECHO_MS_PER_BYTE * 1 + OBD9141_WAIT_FOR_ECHO_TIMEOUT);

    OBD9141print("E: ");
    for (uint8_t i = 0; i < request_len + 1; i++)
    {
#ifdef OBD9141_DEBUG
        OBD9141print(tmp[i]);
        OBD9141print(" ");
#endif
        // check if echo is what we wanted to send
        success &= (buf[i] == tmp[i]);
    }

    // so echo is dealt with now... next is listening to the reply, which is a variable number.
    // set the timeout for the first read to include the wait for answer timeout

    uint8_t answer_length = 0;
    // while readBytes returns a byte, keep reading.
    while (uart_read_bytes(OBD_UART, &(this->buffer[answer_length]), 1, OBD9141_REQUEST_ANSWER_MS_PER_BYTE * 1 + OBD9141_WAIT_FOR_REQUEST_ANSWER_TIMEOUT) && (answer_length < OBD9141_BUFFER_SIZE))
    {
        answer_length++;
        vTaskDelay(OBD9141_REQUEST_ANSWER_MS_PER_BYTE * 1);
    }

    OBD9141println();
    OBD9141print("A (");
    OBD9141print(answer_length);
    OBD9141print("): ");
#ifdef OBD9141_DEBUG
    for (uint8_t i = 0; i < min(answer_length, OBD9141_BUFFER_SIZE); i++)
    {
        OBD9141print(this->buffer[i]);
        OBD9141print(" ");
    };
    OBD9141println();
#endif

    // next, calculate the checksum
    bool checksum = (this->checksum(&(this->buffer[0]), answer_length - 1) == this->buffer[answer_length - 1]);
    OBD9141print("C: ");
    OBD9141println(checksum);
    OBD9141print("S: ");
    OBD9141println(success);
    OBD9141print("R: ");
    OBD9141println(answer_length - 1);
    if (checksum && success)
    {
        return answer_length - 1;
    }
    return 0;
}

uint8_t OBD9141::requestKWP(void *request, uint8_t request_len)
{
    uint8_t buf[request_len + 1];
    memcpy(buf, request, request_len); // copy request

    buf[request_len] = this->checksum(&buf, request_len); // add the checksum

    this->write(&buf, request_len + 1);

    // wait after the request, officially 30 ms, but we might as well wait
    // for the data in the readBytes function.

    // set proper timeout
    memset(this->buffer, 0, OBD9141_BUFFER_SIZE);

    // Example response: 131 241 17 193 239 143 196 0
    // Try to read the fmt byte.
    if (!(uart_read_bytes(OBD_UART, this->buffer, 1, OBD9141_REQUEST_ANSWER_MS_PER_BYTE * 1 + OBD9141_WAIT_FOR_REQUEST_ANSWER_TIMEOUT)))
    {
        return 0; // failed reading the response byte.
    }

    const uint8_t msg_len = (this->buffer[0]) & 0b111111;
    // If length is zero, there is a length byte at the end of the header.
    // This is likely very rare, we do not handle this for now.

    // This means that we should now read the 2 addressing bytes, the payload
    // and the checksum byte.
    const uint8_t remainder = msg_len + 2 + 1;
    OBD9141print("Rem: ");
    OBD9141println(remainder);
    const uint8_t ret_len = remainder + 1;
    OBD9141print("ret_len: ");
    OBD9141println(ret_len);

    // OBD9141print("Trying to get x bytes: "); OBD9141println(ret_len+1);
    if (uart_read_bytes(OBD_UART, &(this->buffer[1]), remainder, OBD9141_REQUEST_ANSWER_MS_PER_BYTE * (remainder + 1)))
    {
        OBD9141print("R: ");
        for (uint8_t i = 0; i < (ret_len + 1); i++)
        {
            OBD9141print(this->buffer[i]);
            OBD9141print(" ");
        };
        OBD9141println();

        const uint8_t calc_checksum = this->checksum(&(this->buffer[0]), ret_len - 1);
        OBD9141print("calc cs: ");
        OBD9141println(calc_checksum);
        OBD9141print("buf cs: ");
        OBD9141println(this->buffer[ret_len - 1]);

        if (calc_checksum == this->buffer[ret_len - 1])
        {
            return ret_len - 1; // have data; return whether it is valid.
        }
    }
    else
    {
        OBD9141println("Timeout on reading bytes.");
        return 0; // failed getting data.
    }
    return 0;
}
/*
    No header description to be found on the internet?

    for 9141-2:
      raw request: {0x68, 0x6A, 0xF1, 0x01, 0x0D}
          returns:  0x48  0x6B  0x11  0x41  0x0D  0x00  0x12
          returns 1 byte
          total of 7 bytes.

      raw request: {0x68, 0x6A, 0xF1, 0x01, 0x00}
          returns   0x48  0x6B  0x11  0x41  0x00  0xBE  0x3E  0xB8  0x11  0xCA
          returns 3 bytes
          total of 10 bytes

      Mode seems to be 0x40 + mode, unable to confirm this though.

    for ISO 14230 KWP:
      First byte lower 6 bits are length, first two bits always 0b11?

      raw request: {0xc2, 0x33, 0xf1, 0x01, 0x0d, 0xf4}
      returns       0x83  0xf1  0x11  0x41  0xd  0x0  0xd3

      raw request: {0xc2, 0x33, 0xf1, 0x01, 0x0c, 0xf3}
      returns       0x84, 0xf1, 0x11, 0x41, 0x0c, 0x0c, 0x4c, 0x2b, 0xf3
*/

bool OBD9141::getPID(uint8_t pid, uint8_t mode, uint8_t return_length)
{
    uint8_t message[5] = {0x68, 0x6A, 0xF1, mode, pid};
    // header of request is 5 long, first three are always constant.

    bool res = this->request(&message, 5, return_length + 5);
    // checksum is already checked, verify the PID.

    if (this->buffer[4] != pid)
    {
        return false;
    }
    return res;
}

bool OBD9141::getCurrentPID(uint8_t pid, uint8_t return_length)
{
    return this->getPID(pid, 0x01, return_length);
}

bool OBD9141::clearTroubleCodes()
{
    uint8_t message[4] = {0x68, 0x6A, 0xF1, 0x04};
    // 0x04 without PID value should clear the trouble codes or
    // malfunction indicator lamp.

    // No data is returned to this request, we expect the request itself
    // to be returned.
    bool res = this->request(&message, 4, 4);
    return res;
}

uint8_t OBD9141::readTroubleCodes()
{
    uint8_t message[4] = {0x68, 0x6A, 0xF1, 0x03};
    uint8_t response = this->request(&message, 4);
    if (response >= 4)
    {
        // OBD9141print("T: ");OBD9141println((response - 4) / 2);
        return (response - 4) / 2; // every DTC is 2 bytes, header was 4 bytes.
    }
    return 0;
}

uint8_t OBD9141::readPendingTroubleCodes()
{
    uint8_t message[4] = {0x68, 0x6A, 0xF1, 0x07};
    uint8_t response = this->request(&message, 4);
    if (response >= 4)
    {
        // OBD9141print("T: ");OBD9141println((response - 4) / 2);
        return (response - 4) / 2; // every DTC is 2 bytes, header was 4 bytes.
    }
    return 0;
}

uint8_t OBD9141::readUint8()
{
    return this->buffer[5];
}

uint16_t OBD9141::readUint16()
{
    return this->buffer[5] * 256 + this->buffer[6]; // need to reverse endianness
}

uint8_t OBD9141::readUint8(uint8_t index)
{
    return this->buffer[5 + index];
}

uint32_t OBD9141::readUint32()
{
    return (uint32_t(this->buffer[5]) << 24) | (uint32_t(this->buffer[6]) << 16) | (uint32_t(this->buffer[7]) << 8) | this->buffer[8]; // need to reverse endianness
}

uint8_t OBD9141::readBuffer(uint8_t index)
{
    return this->buffer[index];
}

uint16_t OBD9141::getTroubleCode(uint8_t index)
{
    return *reinterpret_cast<uint16_t *>(&(this->buffer[index * 2 + 4]));
}

bool OBD9141::init()
{

    use_kwp_ = false;
    // this function performs the ISO9141 5-baud 'slow' init.
    // this->set_port(false); // disable the port.

    this->kline(true);
    vTaskDelay(OBD9141_INIT_IDLE_BUS_BEFORE); // no traffic on bus for 3 seconds.

    OBD9141println("Before magic 5 baud.");
    // next, send the startup 5 baud init..
    this->kline(false);
    vTaskDelay(200); // start
    this->kline(true);
    vTaskDelay(400); // first two bits
    this->kline(false);
    vTaskDelay(400); // second pair
    this->kline(true);
    vTaskDelay(400); // third pair
    this->kline(false);
    vTaskDelay(400); // last pair
    this->kline(true);
    vTaskDelay(200); // stop bit
    // this last 200 ms vTaskDelay could also be put in the setTimeout below.
    // But the spec says we have a stop bit.

    // done, from now on it the bus can be treated ad a 10400 baud serial port.

    OBD9141println("Before setting port.");
    // this->set_port(true);
    OBD9141println("After setting port.");
    uint8_t buffer[1];
    uart_flush(OBD_UART);

    // wait should be between 20 and 300 ms long
    // read first value into buffer, should be 0x55
    int lengthReceived = uart_read_bytes(OBD_UART, buffer, 1, 300 + 200);
    if (lengthReceived > 0)
    {
        OBD9141print("First read is: ");
        OBD9141println(buffer[0]);
        if (buffer[0] != 0x55)
        {
            return false;
        }
    }
    else
    {
        OBD9141println("Timeout on read 0x55.");
        return false;
    }

    uint8_t v1 = 0, v2 = 0; // sent by car:  (either 0x08 or 0x94)

    // read v1
    lengthReceived = uart_read_bytes(OBD_UART, buffer, 1, 20);
    if (0 == lengthReceived)
    {
        OBD9141println("Timeout on read v1.");
        return false;
    }
    else
    {
        v1 = buffer[0];
        OBD9141print("read v1: ");
        OBD9141println(v1);
    }

    // read v2
    lengthReceived = uart_read_bytes(OBD_UART, buffer, 1, 200);
    if (0 == lengthReceived)
    {
        OBD9141println("Timeout on read v2.");
        return false;
    }
    else
    {
        v2 = buffer[0];
        OBD9141print("read v2: ");
        OBD9141println(v2);
    }

    OBD9141print("v1: ");
    OBD9141println(v1);
    OBD9141print("v2: ");
    OBD9141println(v2);

    // these two should be identical according to the spec.
    if (v1 != v2)
    {
        return false;
    }

    // we obtained w1 and w2, now invert and send it back.
    // tester waits w4 between 25 and 50 ms:
    vTaskDelay(30);
    this->write(~v2);

    // finally, attempt to read 0xCC from the ECU, indicating succesful init.
    if (!uart_read_bytes(OBD_UART, buffer, 1, 50))
    {
        OBD9141println("Timeout on 0xCC read.");
        return false;
    }
    else
    {
        // OBD9141print("read 0xCC?: "); OBD9141println(buffer[0], HEX);
        if ((buffer[0] == 0xCC))
        { // done if this is inverse of 0x33
            vTaskDelay(OBD9141_INIT_POST_INIT_DELAY);
            // this vTaskDelay is not in the spec, but prevents requests immediately
            // after the finishing of the init sequency.

            return true; // yay! we are initialised.
        }
        else
        {
            return false;
        }
    }
}

bool OBD9141::initKWP()
{
    use_kwp_ = true;
    // this function performs the KWP2000 fast init.
    // this->set_port(false); // disable the port.
    this->kline(true);                        // set to up
    vTaskDelay(OBD9141_INIT_IDLE_BUS_BEFORE); // no traffic on bus for 3 seconds.
    OBD9141println("Before 25 ms / 25 ms startup.");
    this->kline(false);
    vTaskDelay(25); // start with 25 ms low
    this->kline(true);
    vTaskDelay(25); // 25 ms high.
    uart_flush(OBD_UART);

    // immediately follow this by a startCommunicationRequest
    OBD9141println("Enable port.");
    // this->set_port(true);

    // startCommunicationRequest message:
    uint8_t message[4] = {0xC1, 0x33, 0xF1, 0x81};
    // checksum (0x66) is calculated by request method.

    // Send this request and read the response
    if (this->requestKWP(&message, 4) == 6)
    {
        // check positive response service ID, should be 0xC1.
        if (this->buffer[3] == 0xC1)
        {
            // Not necessary to do anything with this data?
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

void OBD9141::decodeDTC(uint16_t input_bytes, uint8_t *output_string)
{
    const uint8_t A = reinterpret_cast<uint8_t *>(&input_bytes)[0];
    const uint8_t B = reinterpret_cast<uint8_t *>(&input_bytes)[1];
    const static char type_lookup[4] = {'P', 'C', 'B', 'U'};
    const static char digit_lookup[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    // A7-A6 is first dtc character, error type:
    output_string[0] = type_lookup[A >> 6];
    // A5-A4 is second dtc character
    output_string[1] = digit_lookup[(A >> 4) & 0b11];
    // A3-A0 is third dtc character.
    output_string[2] = digit_lookup[A & 0b1111];
    // B7-B4 is fourth dtc character
    output_string[3] = digit_lookup[B >> 4];
    // B3-B0 is fifth dtc character
    output_string[4] = digit_lookup[B & 0b1111];
}

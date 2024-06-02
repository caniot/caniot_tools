# CAN communication example



This example demonstrates how to use an can bus  simulation with caniot tools. Example does the following steps:

1. Use an  `caniotBox_init` function to initialize caniotBoxLib peripheral.
2. Use an  `serialOutput_configure` function to initialize console serial with the given baudrate(only in wifi mode for HW15).
3. write to console using `serialOutput_sendString` or `serialOutput_sendRaw` .
4. call `canAppl_init` to start your CAN application. this application code demonstrate how to use ISO TP handler :

     * **Step 1**. a random generated AES key is send from channel 0 to channel 1 handler(code Line 142),
     * **Step 2**. the channel 1 will receive that key to encrypt random generated plaintext and send it to channel 0(code Line 53).
     * **Step 3**. channel 0 will receive it and decrypt it using the key from step 1 and send the result to channel1 (code Line 38).
     * **Step 4**. channel 1 will compare  plaintext from step 2 and step 3 and turn the LED1 green in case of matches.

## Hardware

This example runs on CANIOTBOX boards without any extra modifications required,

1. connect  CaniotBox to a  USB Port with your computer.
2. connect  `CAN1 Low(Dsub2)` with `CAN0 Low(Dsub4)`.
3. connect  `CAN1 High(Dsub7)` with `CAN0 High(Dsub8)`.
4. the SD card needs to be inserted into the slot.
4. connect CaniotBox with caniot application as described here [USB connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#usb-connection).


### IDF Tools installations:

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please follow this Video for Quick Start and refer to [ESP-IDF(release/v4.4.7) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32/get-started/index.html#installation-step-by-step) to download it. 

https://github.com/caniot/caniot_tools/assets/91652497/2f6cad55-b033-4529-8a4f-9472c1907f05


### Build and Flash

Follow this Video to Build the project using idf.py build and flash it to the board using caniot application

https://github.com/caniot/caniot_tools/assets/91652497/9fbb6125-5705-40ad-a361-f04848f71ef4


### Troubleshooting

After flashing the BIN image, the SW should run correctly, but if the SW keeps doing reset loops forever, this means that the SW is running in some exception. In this Case we should bring the SW to factory image. TODO that: connect IO44(RXD0) to Ground for 10 Seconds and the SW will start from factory image. alternatively set `factory_reset_request` to 1 from the sdcard File  `Config/factory_reset.json`.

Please check what is wrong in your new Image by doing debugging!!!, that helps you to identify and fix errors or bugs in your new code.

### Debugging

Follow this Video for starting OpenOCD  and debugging using GDB

https://github.com/caniot/caniot_tools/assets/91652497/169ffa78-65f8-488a-a980-3cf4d0a883c0



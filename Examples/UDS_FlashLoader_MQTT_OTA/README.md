# UDS OTA Firmware Upgrade via MQTT example

![MQTT_UPDATE](https://github.com/caniot/caniot_tools/assets/91652497/22651323-0d7a-4109-bbe8-ff1e837fd88d)

> **Note**
> S32K1xx NXP EVB is required in this example. Please refer to [S32K116-Q048 Evaluation Board for Automotive General Purpose](https://www.nxp.com/design/development-boards/automotive-development-platforms/s32k-mcu-platforms/s32k116-q048-evaluation-board-for-automotive-general-purpose:S32K116EVB).

> **Note**
> you need a MQTT Cloud to run this Example , you can visit https://www.hivemq.com/company/get-hivemq/ and create the FREE Cloud MQTT Broker that enables you to connect up to 100 devices.after that you have to  Change this Defines  `CONFIG_BROKER_URL` `CONFIG_USER` `CONFIG_USER_PASSWRD` according to the ones from HIVEMQ from the file `mqtt_HiveMQ_subscriber.c` `mqtt_HiveMQ_publisher.c` .


This example demonstrate a FOTA (Firmware Over-The-Air) firmware updates via MQTT Protocol (HIVEMQ)  we will use 2 CaniotBoxs , CaniotBox 1 as MQTT Publischer to publisch the Firmware `S32K1_uds_flash_CAN_BlueLed.bin` over MQTT and CaniotBox 2 as MQTT subscriber to receive this Firmware and send this  to the NXP S32K controller over CAN FD communication using UDS Protocol. The bin  file is stored into CaniotBox 1 SDCARD. Example does the following steps:

1. Use an  `caniotBox_init` function to initialize caniotBoxLib peripheral.
2. Use an  `serialOutput_configure` function to initialize console serial with the given baudrate(only in wifi mode for HW15).
3. write to console using `serialOutput_sendString` or `serialOutput_sendRaw` .
4. call `mqtt_hiveMQ_publisher_init` to start  mqtt publishing  and `mqtt_hiveMQ_subscriber_init` to start  mqtt subscribing:

     * **Step 1**. flash `S32K116_CAN_bootloader.bin` BIN file from [S32K116_bootloader](./S32K116_bootloader) to your S32K116 EVB.

## Hardware

This example needs 2 CANIOTBOX boards without any extra modifications required, 

1. connect CaniotBox 1 and 2 with internet in Station client mode as described here [wifi connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#wifi-connection) .
2. copy `uds_ota_flashLoader` Directory from [SD_CARD](./SD_CARD) to your micro SD CARD and  insert it into the slot of CaniotBox 1.
3. connect  CaniotBox 2 `CAN1 Low(Dsub2)` with `CAN_L` on S32K116 EVB.
4. connect  CaniotBox 2 `CAN1 High(Dsub7)` with `CAN_H` on S32K116 EVB.
5. connect  CaniotBox 2 `GND(Dsub3)` with  with `GND` on S32K116 EVB.
4. power supply CaniotBox 1 and 2

### IDF Tools installations:

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please follow this Video for Quick Start and refer to [ESP-IDF(release/v4.4.8) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.8/esp32/get-started/index.html#installation-step-by-step) to download it. 

https://github.com/caniot/caniot_tools/assets/91652497/2f6cad55-b033-4529-8a4f-9472c1907f05


### Build and Flash

Build the project using idf.py build and flash it to the board using caniot application

1. set MQTT_VARIANT_PUBLISCHER  to "1" in [CMakeLists.txt](./CMakeLists.txt) to build the Publischer Variant for CaniotBox 1.

Follow this Video to Build the project using idf.py build and flash it to the board using caniot application

https://github.com/caniot/caniot_tools/assets/91652497/9fbb6125-5705-40ad-a361-f04848f71ef4

2. set MQTT_VARIANT_PUBLISCHER  to "0" in [CMakeLists.txt](./CMakeLists.txt) to build the subscriber Variant for CaniotBox 2, build again and flash it to  CaniotBox 2 .

### Troubleshooting

After flashing the BIN image, the SW should run correctly, but if the SW keeps doing reset loops forever, this means that the SW is running in some exception. In this Case we should bring the SW to factory image. TODO that: connect IO44(RXD0) to Ground for 10 Seconds and the SW will start from factory image. alternatively set `factory_reset_request` to 1 from the sdcard File  `Config/factory_reset.json`.

Please check what is wrong in your new Image by doing debugging!!!, that helps you to identify and fix errors or bugs in your new code.

### Debugging

Follow this Video for starting OpenOCD  and debugging using GDB

https://github.com/caniot/caniot_tools/assets/91652497/169ffa78-65f8-488a-a980-3cf4d0a883c0




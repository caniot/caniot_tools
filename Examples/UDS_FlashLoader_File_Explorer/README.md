# MSC (File Explorer) UDS FlashLoader example


> **Note**
> S32K1xx NXP EVB is required in this example. Please refer to [S32K116-Q048 Evaluation Board for Automotive General Purpose](https://www.nxp.com/design/development-boards/automotive-development-platforms/s32k-mcu-platforms/s32k116-q048-evaluation-board-for-automotive-general-purpose:S32K116EVB).


This example demonstrate a  firmware updates for a NXP S32K controller over CAN FD communication using UDS Protocol with CANIOT Tools.This example contains code to make CaniotBox recognizable by USB-hosts(PC) as a USB FAT12 Mass Storage Device names `CANIOTBOX` . the user can drag and drop The bin file `BlueLed.bin` into it and start the firmware updates by confirming  the popup message on CaniotBox Main screen. 

> **Note**
> this FAT12 version supports only short file names with only 11 characters (inclusive file extension), longer names are cut out from the software.

Example does the following steps:

1. Use an  `caniotBox_init` function to initialize caniotBoxLib peripheral.
2. Use an  `serialOutput_configure` function to initialize console serial with the given baudrate(only in wifi mode for HW15).
3. write to console using `serialOutput_sendString` or `serialOutput_sendRaw` .
4. start `fat12Task` task to start your CAN application. this application code demonstrate how to flash S32K controller using UDS Protocol,   :

     * **Step 1**. flash `S32K116_CAN_bootloader.bin` BIN file from [S32K116_bootloader](./S32K116_bootloader) to your S32K116 EVB.
     * **Step 2**. connect  CaniotBox to a  USB Port with your computer , that recongnize it as removable device and can access the internal FAT12 partition over USB MSC .  drag and drop The bin file `BlueLed.bin` into it and start the firmware updates by confirming  the popup message on CaniotBox Main screen.

## Hardware

This example runs on CANIOTBOX V3.0 or V3.1  boards without any extra modifications required, 

1. connect  CaniotBox to a  USB Port with your computer.
2. connect  `CAN1 Low(Dsub2)` with `CAN_L` on S32K116 EVB.
3. connect  `CAN1 High(Dsub7)` with `CAN_H` on S32K116 EVB.
3. connect  `GND(Dsub3)` with  with `GND` on S32K116 EVB.
4. copy `uds_msc_flashLoader` Directory from [SD_CARD](./SD_CARD) to your SD CARD and  insert it into the slot.
4. connect CaniotBox in wifi mode as described here [wifi connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#wifi-connection) .


### IDF Tools installations:

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please follow this Video for Quick Start and refer to [ESP-IDF(release/v4.4.6) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.6/esp32/get-started/index.html#installation-step-by-step) to download it. 

https://github.com/caniot/caniot_tools/assets/91652497/7316d7dd-b231-48c8-bc9a-04b1b8e1cc21


### Build and Flash

Follow this Video to Build the project using idf.py build and flash it to the board using caniot application

https://github.com/caniot/caniot_tools/assets/91652497/9fbb6125-5705-40ad-a361-f04848f71ef4

### Troubleshooting

After flashing the BIN image, the SW should run correctly, but if the SW keeps doing reset loops forever, this means that the SW is running in some exception. In this Case we should bring the SW to factory image. TODO that: connect IO44(RXD0) to Ground for 10 Seconds and the SW will start from factory image. alternatively set `factory_reset_request` to 1 from the sdcard File  `Config/factory_reset.json`.

Please check what is wrong in your new Image by doing debugging!!!, that helps you to identify and fix errors or bugs in your new code.

### Debugging

Follow this Video for starting OpenOCD  and debugging using GDB

https://github.com/caniot/caniot_tools/assets/91652497/169ffa78-65f8-488a-a980-3cf4d0a883c0

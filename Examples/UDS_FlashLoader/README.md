# CAN communication example

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please refer to [ESP-IDF(release/v4.4.4) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.4/esp32/get-started/index.html#installation-step-by-step).


> **Note**
> S32K116 NXP EVB is required in this example. Please refer to [S32K116-Q048 Evaluation Board for Automotive General Purpose](https://www.nxp.com/design/development-boards/automotive-development-platforms/s32k-mcu-platforms/s32k116-q048-evaluation-board-for-automotive-general-purpose:S32K116EVB).


This example Shows Four Demonstrations how to flash S32K controller over CAN FD communication using UDS Protocol with CANIOT Tools. Example does the following steps:

1. Use an  `caniotBox_init` function to initialize caniotBoxLib peripheral.
2. Use an  `serialOutput_configure` function to initialize console serial with the given baudrate(only in wifi mode).
3. write to console using `serialOutput_sendString` or `serialOutput_sendRaw` .
4. call `canAppl_init` to start your CAN application. this application code demonstrate how to flash S32K controller using UDS Protocol :

     * **Step 1**. flash `S32K116_CAN_bootloader.bin` BIN file from [S32K116_bootloader](./Examples/UDS_FlashLoader/S32K116_bootloader) to your S32K116 EVB.
     * **Step 2**. start action 1 from caniot application or from `Action Tasks`  on CaniotBox Main screen, this will flash Green LED application from `S32K1_uds_flash_CAN_GreenLed.json`  configuration files.
     * **Step 3**. start action 2 from caniot application or from `Action Tasks`  on CaniotBox Main screen, this will flash Red LED application from `S32K1_uds_flash_CAN_RedLed.json`  configuration files.
     * **Step 4**. start action 3 from caniot application or from `Action Tasks`  on CaniotBox Main screen, this will flash Blue LED application from `S32K1_uds_flash_CAN_BlueLed.json`  configuration files.
     * **Step 5**. start action 3 from caniot application or from `Action Tasks`  on CaniotBox Main screen, this will flash white LED application from `S32K1_uds_flash_CAN_WhiteLed.json`  configuration files.

## Hardware

This example runs on CANIOTBOX boards without any extra modifications required, 

1. connect  CaniotBox to a  USB Port with your computer.
2. connect  `CAN1 Low(Dsub2)` with `CAN_L` on S32K116 EVB.
3. connect  `CAN1 High(Dsub7)` with `CAN_H` on S32K116 EVB.
3. connect  `GND(Dsub3)` with  with `GND` on S32K116 EVB.
4. copy `uds_flashLoader` Directory from [SD_CARD](./Examples/UDS_FlashLoader/SD_CARD) to your SD CARD and  insert it into the slot.
4. connect CaniotBox with caniot application as described here [USB connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#usb-connection).

### Build and Flash

Build the project using idf.py build and flash it to the board using caniot application
1. Go to the project directory `Examples\UDS_FlashLoader`  and run: `idf.py build` on `ESP-IDF 4.4.4 CMD`.
2. connect CaniotBox with caniot application as described here [USB or wifi connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#usb-connection).
3. select  from `menubar` `Update caniotBox`.
4. browse to  `/build` folder and select `UDS_CANFD.bin`.
5. wait until Flash process is finished and CaniotBox is restarted again.



https://user-images.githubusercontent.com/91652497/226187525-6365eb8f-911f-48c5-b33c-2ce0dd8cc20e.mp4



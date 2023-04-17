# CAN communication example

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please refer to [ESP-IDF(release/v4.4.4) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.4/esp32/get-started/index.html#installation-step-by-step).

This example demonstrates how to use an can bus  simulation with caniot tools. Example does the following steps:

1. Use an  `caniotBox_init` function to initialize caniotBoxLib peripheral.
2. Use an  `serialOutput_configure` function to initialize console serial with the given baudrate(only in wifi mode).
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

### Build and Flash

Build the project using idf.py build and flash it to the board using caniot application
1. open  `ESP-IDF 4.4.4 CMD`, Go to the project directory with `cd {YOUR PATH}\caniot_tools\Examples\CAN_APPL` and   on .
2. run: `del sdkconfig && idf.py -B build_s1 -D SDKCONFIG_DEFAULTS=sdkconfig_s1 -D IDF_TARGET=esp32 build` for HW V1.5 or  run `del sdkconfig && idf.py -B build_s3 -D SDKCONFIG_DEFAULTS=sdkconfig_s3 -D IDF_TARGET=esp32s3 build` for HW V3.0 
3. connect CaniotBox with caniot application as described here [USB or wifi connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#usb-connection).
4. select  from `menubar` `Update caniotBox`.
5. browse to  `/build_s1` or `/build_s3` for HW V3.0 folder and select `CAN_APPL.bin`.
6. alternatively you can copy  `CAN_APPL.bin` to `caniotbox_images`  directory in the sd card and flash it from the `utility->CaniotBox Update:`  menu in CaniotBox.
7. wait until Flash process is finished and CaniotBox is restarted again.
8. run: `idf.py -B build_s1 monitor -p COMx` for HW V1.5 or  run `idf.py -B build_s3 monitor -p COMx` for HW V3.0 to start the serial monitor.



https://user-images.githubusercontent.com/91652497/226187525-6365eb8f-911f-48c5-b33c-2ce0dd8cc20e.mp4



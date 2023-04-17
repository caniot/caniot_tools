# CAN communication example

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please refer to [ESP-IDF(release/v4.4.4) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.4/esp32/get-started/index.html#installation-step-by-step).


This example demonstrates how to publish a CAN bus message to a MQTT Broker with caniot tools.
It uses ESP-MQTT library which implements mqtt client to connect to mqtt broker.

Example does the following steps:

1. Use an  `caniotBox_init` function to initialize caniotBoxLib peripheral.
2. Use an  `serialOutput_configure` function to initialize console serial with the given baudrate(only in wifi mode).
3. write to console using `serialOutput_sendString` or `serialOutput_sendRaw` .
4. call `canAppl_init` to start your CAN application. this function does following steps :

     * **Step 1**. create handler to be called when message ID 0x502 has been received on CAN channel 1.     
     * **Step 2**. Use `mqtt_app_start` to connect to the local broker using  `CONFIG_BROKER_URL` and `CONFIG_BROKER_PORT`.
     * **Step 3**. create mqtt_appl_Task to send all 0x502 messages to the connected MQTT broker on topic `Room0`.


## Hardware

This example runs on CANIOTBOX boards without any extra modifications required, the only required interface is WiFi and connection to internet.(the SD card needs to be inserted into the slot.)

1. connect  CaniotBox to a USB Port with your computer.
2. connect  `CAN1 Low(Dsub2)` with `CAN0 Low(Dsub4)`.
3. connect  `CAN1 High(Dsub7)` with `CAN0 High(Dsub8)`.
4. connect CaniotBox with internet to your own station using caniot application as described here [1.2 Station client mode](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#wifi-connection).


### Build and Flash

Build the project using idf.py build and flash it to the board using caniot application
1. open  `ESP-IDF 4.4.4 CMD`, Go to the project directory with `cd {YOUR PATH}\caniot_tools\Examples\CAN_MQTT` and   on .
2. run: `del sdkconfig && idf.py -B build_s1 -D SDKCONFIG_DEFAULTS=sdkconfig_s1 -D IDF_TARGET=esp32 build` for HW V1.5 or  run `del sdkconfig && idf.py -B build_s3 -D SDKCONFIG_DEFAULTS=sdkconfig_s3 -D IDF_TARGET=esp32s3 build` for HW V3.0 
3. connect CaniotBox with caniot application as described here [USB or wifi connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#usb-connection).
4. select  from `menubar` `Update caniotBox`.
5. browse to  `/build_s1` or `/build_s3` for HW V3.0 folder and select `CAN_MQTT.bin`.
6. alternatively you can copy  `CAN_MQTT.bin` to `caniotbox_images`  directory in the sd card and flash it from the `utility->CaniotBox Update:`  menu in CaniotBox.
7. wait until Flash process is finished and CaniotBox is restarted again.
8. run: `idf.py -B build_s1 monitor -p COMx` for HW V1.5 or  run `idf.py -B build_s3 monitor -p COMx` for HW V3.0 to start the serial monitor.




https://user-images.githubusercontent.com/91652497/226187612-f4caf1be-de90-448a-ab5f-d77905327315.mp4



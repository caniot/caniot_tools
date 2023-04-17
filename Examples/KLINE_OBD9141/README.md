# LIN communication example

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please refer to [ESP-IDF(release/v4.4.4) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.4/esp32/get-started/index.html#installation-step-by-step).

This example demonstrates how  to read data from the OBD-II port through the K-line pin using either ISO 9141-2 or ISO 14230-2 (KWP) with caniot tools.the OBD9141 Library is derived from the original library from `https://github.com/iwanders/OBD9141` and adapted to be use with CaniotBox
Example does the following steps:

1. Use an  `caniotBox_init` function to initialize caniotBoxLib peripheral.
2. Use an  `serialOutput_configure` function to initialize console serial with the given baudrate(only in wifi mode).
3. write to console using `serialOutput_sendString` or `serialOutput_sendRaw` .
4. create `readDTC_Task`  task . this shows how to use the OBD9141 library to read the diagnostic trouble codes
   from the ECU and print these in a human readable format to the serial port.

## Hardware

This example runs on CANIOTBOX boards without any extra modifications required,

1. connect  CaniotBox to a  USB Port with your computer.
2. connect  `12V+(Dsub1)` with external 12V power supply and `Ground(Dsub3)` with external Ground.
3. connect  `K-Line Tx(Dsub5)` to the Kline Pin of the  OBD Connector (example in your Car).for HW 1.5 `K-Line Tx(Dsub5)` and `K-Line Rx(Dsub6)` need to be shorted in order to receive back the same data which were sent out.
4. the SD card needs to be inserted into the slot.
5. connect CaniotBox with caniot application as described here [USB connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#usb-connection).


### Build and Flash

Build the project using idf.py build and flash it to the board using caniot application
1.open  `ESP-IDF 4.4.4 CMD`, Go to the project directory with `cd {YOUR PATH}\caniot_tools\Examples\KLINE_OBD9141` and   on .
2. run: `del sdkconfig && idf.py -B build_s1 -D SDKCONFIG_DEFAULTS=sdkconfig_s1 -D IDF_TARGET=esp32 build` for HW V1.5 or  run `del sdkconfig && idf.py -B build_s3 -D SDKCONFIG_DEFAULTS=sdkconfig_s3 -D IDF_TARGET=esp32s3 build` for HW V3.0 
3. connect CaniotBox with caniot application as described here [USB or wifi connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#usb-connection).
4. select  from `menubar` `Update caniotBox`.
5. browse to  `/build_s1` or `/build_s3` for HW V3.0 folder and select `KLINE_OBD9141.bin`.
6. alternatively you can copy  `KLINE_OBD9141.bin` to the `caniotbox_images`  directory in the sd card and flash it from the `utility->CaniotBox Update:`  menu in CaniotBox.
7. wait until Flash process is finished and CaniotBox is restarted again.
8. run: `idf.py -B build_s1 monitor -p COMx` for HW V1.5 or  run `idf.py -B build_s3 monitor -p COMx` for HW V3.0 to start the serial monitor.



https://user-images.githubusercontent.com/91652497/226187670-49911961-6214-4a17-b4a1-366dab3c3664.mp4


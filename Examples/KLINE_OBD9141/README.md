# KLINE_OBD9141 example

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please follow this Video for Quick Start and refer to [ESP-IDF(release/v4.4.6) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.6/esp32/get-started/index.html#installation-step-by-step) to download it. 

https://github.com/caniot/caniot_tools/assets/91652497/7316d7dd-b231-48c8-bc9a-04b1b8e1cc21

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

Follow this Video to Build the project using idf.py build and flash it to the board using caniot application

https://github.com/caniot/caniot_tools/assets/91652497/9fbb6125-5705-40ad-a361-f04848f71ef4


### Debugging

Follow this Video for starting OpenOCD  and debugging using GDB

https://github.com/caniot/caniot_tools/assets/91652497/169ffa78-65f8-488a-a980-3cf4d0a883c0
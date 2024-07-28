# KLINE_OBD9141 example


This example demonstrates how  to read data from the OBD-II port through the K-line pin using either ISO 9141-2 or ISO 14230-2 (KWP) with caniot tools.the OBD9141 Library is derived from the original library from `https://github.com/iwanders/OBD9141` and adapted to be use with CaniotBox
Example does the following steps:

1. Use an  `caniotBox_init` function to initialize caniotBoxLib peripheral.
2. Use an  `serialOutput_configure` function to initialize console serial with the given baudrate(only in wifi mode for HW15).
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

### IDF Tools installations:

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please follow this Video for Quick Start and refer to [ESP-IDF(release/v4.4.8) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.8/esp32/get-started/index.html#installation-step-by-step) to download it. 

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
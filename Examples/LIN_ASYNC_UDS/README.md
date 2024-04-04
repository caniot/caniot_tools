# LIN asynchronous UDS Diagnostic requests/response example

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please follow this Video for Quick Start and refer to [ESP-IDF(release/v4.4.6) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.6/esp32/get-started/index.html#installation-step-by-step) to download it. 

https://github.com/caniot/caniot_tools/assets/91652497/7316d7dd-b231-48c8-bc9a-04b1b8e1cc21

This example demonstrates how to three asynchronous tasks running on core 0 and core 1 can send a request/response UDS diagnostic sessions  on LIN bus with caniot tools. Example does the following steps:

1. Use an  `caniotBox_init` function to initialize caniotBoxLib peripheral.
2. Use an  `serialOutput_configure` function to initialize console serial with the given baudrate(only in wifi mode).
3. write to console using `serialOutput_sendString` or `serialOutput_sendRaw` .
4. call `diag_init` to start your LIN application. this application code demonstrate how to use ISO TP handler to send and receive UDS Diagnostic frames:

The example starts three FreeRTOS tasks on core 1 and 0:
1. The first task `asyncDiagTask1` periodically transmits three UDS Diagnostic requests to the LIN node adress `NAD_TASK1`  and listens, receives and prints response data to the serial port.
2. The second task `asyncDiagTask2` periodically transmits three UDS Diagnostic requests  to the LIN node adress `NAD_TASK2`  and listens, receives and prints response data to the serial port.
3. The last task `asyncDiagTask3` periodically transmits three UDS Diagnostic requests to the LIN node adress `NAD_TASK3`  and listens, receives and prints response data to the serial port.

## Hardware

This example runs on CANIOTBOX boards without any extra modifications required,

1. connect  CaniotBox to a  USB Port with your computer.
2. connect  `12V+(Dsub1)` with external 12V power supply and `Ground(Dsub3)` with external Ground.
3. connect  `LIN Data(Dsub9)` with LIN bus connected to another slaves(example NXP S32K116-Q048 Evaluation Board).
4. the SD card needs to be inserted into the slot.
4. connect CaniotBox with caniot application as described here [USB connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#usb-connection).


### Build and Flash

Follow this Video to Build the project using idf.py build and flash it to the board using caniot application

https://github.com/caniot/caniot_tools/assets/91652497/9fbb6125-5705-40ad-a361-f04848f71ef4


### Debugging

Follow this Video for starting OpenOCD  and debugging using GDB

https://github.com/caniot/caniot_tools/assets/91652497/169ffa78-65f8-488a-a980-3cf4d0a883c0
## Example Output

You will receive the following repeating output from the monitoring console:

![Capture](https://user-images.githubusercontent.com/91652497/232492293-0c099d75-76a9-4e1b-96f8-f5ded446726d.PNG)

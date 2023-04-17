# LIN communication example

> **Note**
> ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please refer to [ESP-IDF(release/v4.4.4) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.4/esp32/get-started/index.html#installation-step-by-step).

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

Build the project using idf.py build and flash it to the board using caniot application
1.open  `ESP-IDF 4.4.4 CMD`, Go to the project directory with `cd {YOUR PATH}\caniot_tools\Examples\LIN_ASYNC_UDS` and   on .
2. run: `del sdkconfig && idf.py -B build_s1 -D SDKCONFIG_DEFAULTS=sdkconfig_s1 -D IDF_TARGET=esp32 build` for HW V1.5 or  run `del sdkconfig && idf.py -B build_s3 -D SDKCONFIG_DEFAULTS=sdkconfig_s3 -D IDF_TARGET=esp32s3 build` for HW V3.0 
3. connect CaniotBox with caniot application as described here [USB or wifi connection](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html#usb-connection).
4. select  from `menubar` `Update caniotBox`.
5. browse to  `/build_s1` or `/build_s3` for HW V3.0 folder and select `LIN_ASYNC_UDS.bin`.
6. alternatively you can copy  `LIN_ASYNC_UDS.bin` to `caniotbox_images`  directory in the sd card and flash it from the `utility->CaniotBox Update:`  menu in CaniotBox.
7. wait until Flash process is finished and CaniotBox is restarted again.
8. run: `idf.py -B build_s1 monitor -p COMx` for HW V1.5 or  run `idf.py -B build_s3 monitor -p COMx` for HW V3.0 to start the serial monitor.

https://user-images.githubusercontent.com/91652497/226187670-49911961-6214-4a17-b4a1-366dab3c3664.mp4


## Example Output

You will receive the following repeating output from the monitoring console:

![Capture](https://user-images.githubusercontent.com/91652497/232492293-0c099d75-76a9-4e1b-96f8-f5ded446726d.PNG)

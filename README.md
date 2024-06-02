## Caniot Tools

[![1684683847504](https://github.com/caniot/caniot_tools/assets/91652497/e4af399a-6381-4585-ab5b-3603b0a178d9)](https://github.com/caniot/caniot_tools/assets/91652497/8dab8eee-b722-4974-b26d-4e5a41577ff8)

Caniot tool is a Multi-platform simulation, analysis and test environment for development and test of CAN/LIN/KLine Automotive bus systems.It consists of caniot application and caniotBox. Simplicity is the highest principle when setting up, operating and expanding the system.


## Features

* Caniot Tool can be used on Windows 10,11, Linux(Ubuntu 22.04.1 LTS) ,macOS Monterey and Android.
* stream real-time CAN/LIN data to your own MQTT Server.
* intuitive Caniot application for analysis: user panels, signal plots, dashboards, trace window.
* DBC/LDF decode data to physical values and plot.
* Support ISO-TP 15765-2 and UDS ISO 14229.
* CaniotBox has simple logger which writes all messages from CAN/LIN bus to csv file on a SD card class 10 (8-32GB).
* Flexible and reusable LVGL GUI framework.
* Pmod™ compatible headers support expand peripheral modules(V3.0).


### Quick Start

* **Step 1**. Run command `git clone --recursive https://github.com/caniot/caniot_tools.git` to download the source of this project.
* **Step 2**. install Caniot Application for your OS [caniot app](./caniot_app).
* **Step 3**. ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please follow this Video for Quick Start and refer to [ESP-IDF(release/v4.4.7) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32/get-started/index.html#installation-step-by-step) to download it. 

https://github.com/caniot/caniot_tools/assets/91652497/2f6cad55-b033-4529-8a4f-9472c1907f05

* **Step 4**. now you can build a UDS FlashLoader OTA example[build a UDS OTA example](./Examples/UDS_FlashLoader_OTA).

https://github.com/caniot/caniot_tools/assets/91652497/9fbb6125-5705-40ad-a361-f04848f71ef4


## User Guide

* For more details of hardware information, please refer to the [Hardware Overview](https://caniot-docu.readthedocs.io/en/latest/Hardware.html).
* For getting start of caniot application, please refer to the [Getting Started](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html).
* For experiencing CAN BUS Simulation on caniot application, please refer to the [CAN](https://caniot-docu.readthedocs.io/en/latest/CAN.html).
* For experiencing LIN BUS Simulation on caniot application, please refer to the [LIN](https://caniot-docu.readthedocs.io/en/latest/LIN.html).
* For experiencing withn  LVGL Library, please refer to the  [LVGL GUI Quick overview](https://docs.lvgl.io/7.11/widgets/index.html).



## Contact Us

* If you have any technical issues or need support, please send us a email to support@caniot.eu or submit issue tickets at [GitHub Issues](https://github.com/caniot/caniot_tools/issues) for help.


## Buy it on Tindie

<a href="https://www.tindie.com/stores/caniot_team/?ref=offsite_badges&utm_source=sellers_CANIOT_team&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>

## Caniot Tools

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

## User Guide

* For more details of hardware information, please refer to the [Hardware Overview](https://caniot-docu.readthedocs.io/en/latest/Hardware.html).
* For getting start of caniot application, please refer to the [Getting Started](https://caniot-docu.readthedocs.io/en/latest/getting-started-caniot.html).
* For experiencing CAN BUS Simulation on caniot application, please refer to the [CAN](https://caniot-docu.readthedocs.io/en/latest/CAN.html).
* For experiencing LIN BUS Simulation on caniot application, please refer to the [LIN](https://caniot-docu.readthedocs.io/en/latest/LIN.html).
* For experiencing withn  LVGL Library, please refer to the  [LVGL GUI Quick overview](https://docs.lvgl.io/7.11/widgets/index.html).


### Quick Start

* **Step 1**. Run command `git clone --recursive https://github.com/caniot/caniot_tools.git` to download the source of this project.
* **Step 2**. install Caniot Application for your OS [caniot app](./caniot_app).
* **Step 3**. ESP-IDF (Espressif IoT Development Framework) is required to build caniotBox application. If this is your first time using the ESP-IDF,Please refer to [ESP-IDF(release/v4.4.4) Installation Step by Step](https://docs.espressif.com/projects/esp-idf/en/v4.4.4/esp32/get-started/index.html#installation-step-by-step).
* **Step 4**. now you can build a CAN example[build a CAN example](./Examples/CAN).


## Contact Us

* If you have any technical issues or need support, please submit issue tickets at [GitHub Issues](https://github.com/caniot/caniot_tools/issues) for help.


## Buy it on Tindie

<a href="https://www.tindie.com/stores/caniot_team/?ref=offsite_badges&utm_source=sellers_CANIOT_team&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>

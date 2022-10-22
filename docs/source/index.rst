.. caniot_docs documentation master file, created by
   sphinx-quickstart on Sat Sep 24 10:20:33 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to Caniot Tool!
=======================


.. figure:: /images/logo.png
   :align: center
   :height: 400
   :width: 400
   :alt: caniot logo
   
   *caniot logo*
   
   




Usage Warning
--------------

The use of the CANIOT Tool must be done with caution and an understanding of the risks involved. The operation of the device may be dangerous as you may affect the operation and behaviour of a your communication bus system. Improper installation or usage of the device can lead to serious malfunction, loss of data, equipment damage and physical injury. This is particularly relevant when the device is physically connected to a CAN/LIN or K-LINE based application that may be controlled via the bus. In this setup you can potentially cause an operational change in the system, turn on/off certain modules and functions or change to an unintended mode. While the device supports a high degree of security in regards to wireless data transfer and over-the-air up-dates, it is recommended that these features are used with caution. Incorrect usage of this functionality can result in a device being unable to connect to your server. Further, changing transmit messages overthe-air should be done with extreme caution. The device should only be used by persons who are qualified/trained, understand the risks and understand how the device interacts with the system in which it is integrated.


Description
--------------
Caniot tool is a Multi-platform simulation, analysis and test environment for development and test of CAN/LIN/K-Line bus systems.It consists of caniot application and caniotBox.  Simplicity is the highest principle when setting up, operating and expanding the system.

**The Main Advantages:**
   
    * Caniot Tool can be used on Windows 10,11, Linux(Ubuntu 22.04.1 LTS) ,macOS Monterey and Android.
    * remaining bus simulation: Simulates messages and signals of missing control units from databases (CAN dbc file, LIN ldf File).
    * intuitive Caniot application for analysis: user panels, signal plots, dashboards, trace window.
    * DBC/LDF decode data to physical values and plot
    * Support ISO-TP 15765-2 and UDS ISO 14229.
    * CaniotBox has simple logger which writes everything from CAN/LIN bus to csv file on a SD card (8-32GB).
    * CaniotBox can be used on standalone mode to simulate your bus without the need  to use the Caniot Appl with your computer.
    * stream real-time data via USB or TCP Connection to your PC.
    * stream real-time data to your own MQTT Server.
    * It supports CAN 2.0A and 2.0B  and CANFD protocol.
    * Caniot application updates.
    * The last saved user preferences are loaded automatically at the start of application.

.. important:: Caniot tool is an enginnering sample and is not fully tested.This part should only be used for functional evaluation and proof of concept, not for production or end of use.


.. toctree::
   :maxdepth: 20
   :caption: Contents:

   Hardware
   getting-started-caniot
   CAN





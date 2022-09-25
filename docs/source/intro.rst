Introduction
================

.. figure:: /images/logo.png
   :align: center
   :height: 400
   :width: 400
   :alt: caniot logo
   
   *caniot logo*
   
   




Usage Warning
--------------

The use of the CANIOT Tool must be done with caution and an understand-ing of the risks involved. The operation of the device may be dangerous as you may affect the operation and behaviour of a CAN bus system. Improper installation or usage of the device can lead to serious malfunction, loss of data, equipment damage and physical injury. This is particularly relevant when the device is physically connected to a CAN based application that may be controlled via the CAN bus. In this setup you can potentially cause an operational change in the system, turn on/off certain modules and func-tions or change to an unintended mode. While the device supports a high degree of security in regards to wireless data transfer and over-the-air up-dates, it is recommended that these features are used with caution. Incorrect usage of this functionality can result in a device being unable to connect to your server. Further, changing transmit messages overthe-air should be done with extreme caution. The device should only be used by persons who are qualified/trained, understand the risks and understand how the device interacts with the system in which it is integrated.


Description
--------------
Caniot tool is a Multi-platform simulation, analysis and test environment for development and test of CAN/LIN bus systems. Simplicity is the highest principle when setting up, operating and expanding the system.

**The Main Advantages:**
   
    * remaining bus simulation: Simulates messages and signals of missing con-trol units from databases (CAN dbc file, LIN ldf File).
    * intuitive Caniot application for analysis: user panels, signal plots, dashboards, trace window.
    * DBC/LDF decode data to physical values and plot
    * Support ISO-TP 15765-2 and UDS ISO 14229.
    * CaniotBox has simple logger which writes everything from CAN/LIN bus to csv file on a SD card (8-32GB).
    * stream real-time data via USB or TCP Connection on your PC.
    * stream real-time data to your own MQTT Server.
    * CaniotBox update notification.

    1.1	CaniotBox Specifications:

        * Controller: ESP32 wrover-ib with 240 MHz 
        * Flash: 4MB Serial Flash Memory 
        * RAM: 4MB Serial RAM Memory
        * 1 CAN0 channels up to 1 Mbit/s
        * 1 CAN1 FD channel up to 5 Mbit/s
        * Supports both 11-bit (CAN 2.0A) and 29-bit (CAN 2.0B active) identifi-ers High-speed CAN connection (compliant with ISO 11898-2).
        * 1 LIN channel up to 330 Kbit/s.
        * 2.4 GHz proprietary protocol.
        * external antenna with max gain of 3dBi.
        * Power supplied through 5V USB or 1200MAH Li-ion


    1.2	CaniotBox  Connectors:

        * D-SUB9: The CaniotBox has a D-SUB9 connector with one LIN and 2 CAN channels.
        * USB Connector: Connect your PC and the CaniotBox via USB to charge the internal batteries and communicate with caniot application.

        1.2.1	Pin Assignment


            =====  ======================== 
            Pins:  Assignment    
            =====  ========================
            1      LIN/ K-Line Vbat  (12V+)            
            2      CAN-FD 1 L              
            3      GND  
            4      CAN 0 L
            5      K-Line Tx
            6      K-Line Rx
            7      CAN-FD 1 H
            8      CAN 0 H
            9      LIN Data   
            =====  ========================  

            
This is a typical paragraph.  An indented literal block follows.

::

    for a in [5,4,3,2,1]:   # this is program code, shown as-is
        print a
    print "it's..."
    # a literal block continues until the indentation ends

This text has returned to the indentation of the first paragraph,
is outside of the literal block, and is therefore treated as an
ordinary paragraph.
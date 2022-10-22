.. _hardware:

CaniotBox Hardware
====================================

The CaniotBox is a small, inexpensive ESP32 based USB/Wifi to CAN/LIN/K-LINE adapter. The CaniotBox logs in as a virtual serial port on your computer and acts as an interface between serial line and your bus. With the Caniot Application, the CaniotBox can be operated as a native Bus interface under Windows/Linux and macOS. it supports both standard CAN and CAN-FD for the CAN protocol.

Specifications
---------------

1.1	CaniotBox Specifications:

        .. figure:: /images/HW1_5.png
            :align: center
            :height: 500
            :width: 900 
            :alt: caniot logo

            *HW 1.5* 

        1.1.1	HW V1.5

            ================    ==========================================
            Controller          ESP32 wrover S1             
            Flash               8MB              
            PSRAM               4MB
            CAN0 Channel        up to 1 Mbit/s
            CAN1 FD Channel     up to 5 Mbit/s Data Rate
            LIN Channel         up to 20 Kbit/s.
            K-LINE Channel      up to 150 Kbit/s.
            Wifi Antenna        max gain of 2dBi.
            Display             2.4 Inch 320x240 TFT, SPI
            Power0              5V USB,Min 3.55V@300mA
            Power1              3.7V ,1200MAH Li-ion
            Switch              Switch between Power0 and Power1 
            Button              Reset button
            Size                77x42mm  
            ================    ========================================== 

        1.1.2	HW V3.0

            ================    ==========================================
            Controller          ESP32 wroom S3
            Flash               8MB              
            PSRAM               8MB
            CAN0 Channel        up to 1 Mbit/s
            CAN1 FD Channel     up to 5 Mbit/s Data Rate
            LIN Channel         up to 20 Kbit/s.
            K-LINE Channel      up to 150 Kbit/s.
            Wifi Antenna        max gain of 2dBi.
            Display             2.4 Inch 320x240 TFT, SPI
            Power0              5V USB,Min 3.55V@300mA
            Power1              external 12V+ on dsub9 Pin1(max 40V input)
            Switch              Switch between Power0 and Power1
            Button              Reset button
            Size                77x42mm  
            ================    ========================================== 


            
1.2	CaniotBox  Connectors:

        * D-SUB9: The CaniotBox has a male D-SUB9 connector with one LIN and 2 CAN channels.
        * USB Connector: Connect your PC and the CaniotBox via USB to charge the internal batteries(HW1.5) and communicate with caniot application.

        1.2.1	D-SUB9 Pin Assignment HW V1.5


            =====  ======================== 
            Pins:  Assignment    
            =====  ========================
            1      Vbat (12V+)            
            2      CAN1-FD Low              
            3      GND  
            4      CAN0 Low
            5      K-Line Tx
            6      K-Line Rx
            7      CAN1-FD High
            8      CAN0 High
            9      LIN Data    
            =====  ========================


        1.2.2	D-SUB9 Pin Assignment HW V3.0


            =====  ======================== 
            Pins:  Assignment    
            =====  ========================
            1      Vbat (12V+)            
            2      CAN1-FD Low              
            3      GND  
            4      CAN0 Low
            5      K-Line Tx/Rx
            6      
            7      CAN1-FD High
            8      CAN0 High
            9      LIN Data   
            =====  ========================      

.. .. tabs::

..     .. tab:: socket_task.c

..         .. include:: ../../examples/can_example/socket_task.c
..             :code: cpp

This is a typical paragraph.  An indented literal block follows.

::

    for a in [5,4,3,2,1]:   # this is program code, shown as-is
        print a
    print "it's..."
    # a literal block continues until the indentation ends

This text has returned to the indentation of the first paragraph,
is outside of the literal block, and is therefore treated as an
ordinary paragraph.
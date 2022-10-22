.. _CAN_tool:


CAN 
===

Introduction
------------

Caniot  is a user friendly and cost effective testing and development tool for CAN bus systems that runs on Windows,Linux,macOS and android.You can monitor, analyze and simulate nodes behavior when DBC file is loaded.

  Main features:

    * Received CAN messages are displayed with ID, Data Length, and data bytes in a Trace list.

    * Trace list can be saved and loaded to and from files.

    * Filters.

    * Different time stamping

    * Signal Watch in user friendly Card view. 

    * Data messages can be added to a **Transmit Message Card**. These messages can either be transmitted manually, automatically in fixed time intervals.

    * Error Frames on the CAN bus are shown.

    * Received Remote Request frames are shown.
    
    * Stream real-time Received data to your own MQTT Server.

CAN Bus Simulation
------------------


   1.1.1	Database Configuration

   .. note:: currently only dbc file format is supported, if you have another format like arxml format we recommend to use third party tools like python tool `can matrix`_ to convert arxml to dbc file.

|
  
      The use of CAN Databases is recommended for the developement ,analysis and testing, because it contains the communication data(messages and signals) to simulate nodes behavior . User can import any number of dbc database and make them as active databases.

         * firstly you need to connect to caniotBox on ``USB mode`` or ``Wifi mode`` .
         * choose from ``menubar`` the **Import DBC File**.
         * a pop-up Window will appear,asking you to select thr last one. 
         * if you selected ``no`` a files Explorer Window will appear,select your dbc file  and click open.
         * a new pop-up Window will appear,select Hardware CAN channel to be associated.
         * all nodes are listed , checkin the ones to be simulated.
         * click OK and wait until caniot is restarted again.

.. only:: html
    
   .. video:: _static/import_dbc.mp4
      :width: 675
      :height: 400


.. only:: latex

   .. figure:: /images/wifi_client0.png
      :align: center
      :height: 500
      :width: 900 

      *select connection mode from menubar*


   .. figure:: /images/wifi_client1.png
      :align: center
      :height: 500
      :width: 900     

      *select connection wifi mode*   

|
|
|

   1.1.2   Transmitting CAN Messages


   CAN messages can be transmitted to other CAN nodes by creating a new transmit message in **Transmit Message Card**.
   
   * firstly you need to connect to caniotBox on ``USB mode`` or ``Wifi mode`` .
   * write the Message ID in hex format.
   * write the Message Data in hex format (bytes can be separated with ``-``).
   * A message can be transmitted periodically if the cycle time is configured to a value higher than 0 ms.
   * Click **send** Icon above to send the configured Message.
   * To Stop the periodic sending of this message set the cycle time to 0  and send again.




.. only:: html
    
   .. video:: _static/send_message.mp4
      :width: 675
      :height: 400


.. only:: latex

   .. figure:: /images/wifi_client0.png
      :align: center
      :height: 500
      :width: 900 

      *select connection mode from menubar*


   .. figure:: /images/wifi_client1.png
      :align: center
      :height: 500
      :width: 900     

      *select connection wifi mode*   

|
|
|



.. _can matrix: https://canmatrix.readthedocs.io/en/latest/

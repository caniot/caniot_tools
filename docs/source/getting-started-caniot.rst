.. _caniot_appl:


Getting Started
=======================================

Setup
-----

.. note:: for the first use or after Factory Reset,Caniot Application need access to the Internet to  ``Register`` your software.If you do not have access to the Internet, the application will be closed after 10 seconds.


    

1. To begin, download and install Caniot Application for your OS from the `Caniot download page`_

2. With the CaniotBox plugged into your computer, start the Caniot Application. If this is the first time you are using your caniot Tool or after a Factory Reset,the Caniot Application will ask you to enter your ``application Key`` . we recommend you to read the license first.

3. after you have entered the application key and the registration was successful, you have to select a work Directory, in this Directory named ``Caniot_data``  you will found later all Settings and Configurations stored during run time from Caniot Application.

.. only:: html
    
    .. video:: _static/intro.mp4
       :width: 675
       :height: 400



.. only:: latex

    .. figure:: /images/Caniot_key_window.png
       :align: center
       :height: 500
       :width: 900 
       :alt: caniot logo
   
       *enter your provided Key* 
.. only:: latex

    .. figure:: /images/Caniot_dir_window.png
       :align: center
       :height: 500
       :width: 900 
       :alt: caniot logo
   
       *enter Work directory* 

|
|
|

.. _usb howTo:

USB Connection
---------------

.. note::

    **Linux and Mac**
    Drivers are not required on Linux and Mac. The CaniotBox will appear as a USB CDC device: /dev/ttyACMX or /dev/ttyUSBX on Linux or /dev/cu.usbmodemXXXX on on Mac.
    **Windows**
    Modern versions of Windows do not require a driver for USB-CDC devices, and the CaniotBox will enumerate properly after plugging in.    
   
   
If one or more CaniotBoxes are connected to your computer, a pop-up window will appear listing their virtual serial ports, please select one and click **start** button.

|
|

.. only:: html
    
   .. video:: _static/usb_start.mp4
       :width: 675
       :height: 400
    
.. only:: latex

   .. figure:: /images/usb_start.png
      :align: center
      :height: 500
      :width: 900 
      :alt: caniot logo
   
      *press start button to connect*



|
|
|

Wifi Connection
----------------

If you want to use the caniot tool without messing with cables. Then, a Wi-Fi connection is the most convenient way 

1.1 AP mode:


* firstly you need to connect to caniotBox as described here :ref:`usb howTo`.
* choose from ``menubar``   |menubarIcon|   the **connection mode**.
* a pop-up window will appear, select connection wifi mode and **Click** ``ok``  .
* wait until caniot appl and CaniotBox is restarted again ``connect to caniotBox`` window will appear.
* the CaniotBox will start as wifi AP with a network name **CANIOTBOX**.
* for windows 10 please refer to `windows Wifi`_ to see how to connect to a wifi network.
* select **CANIOTBOX** network and type **CaniotBox2020** for the password.
* when the connection between your computer and the CaniotBox is established, ``Device 1 is connected`` is shown on CaniotBox Main Screen.
* Type the IP address of your CaniotBox on ``connect to caniotBox`` window and Click **ok** .

|
|

.. only:: html
    
   .. video:: _static/wifi_start.mp4
      :width: 675
      :height: 400


.. only:: latex

   .. figure:: /images/wifi_start2.png
      :align: center
      :height: 500
      :width: 900 

      *select connection mode from menubar*

   .. figure:: /images/wifi_start0.png
      :align: center
      :height: 500
      :width: 900     

      *select connection wifi mode*   

   .. figure:: /images/wifi_start.png
      :align: center
      :height: 500
      :width: 900   

      *type you caniotBox adress and press ok button to connect*

|
|
|

1.2 Station client mode:

the caniotBox can connect to your own Wifi network as client instead of Access Point(AP)

* firstly you need to connect to caniotBox on ``USB mode`` or ``Wifi AP mode`` .
* choose from ``menubar`` the **Wifi configuration**.
* a pop-up window will appear, enter your network name(SSID) and your network password.
* select from ``Wifi mode`` **Station client** and click  **ok** and wait until CaniotBox is restarted again.
* when the connection between your network and the CaniotBox is established,the new ``IP:`` is shown on CaniotBox Main Screen.
* Type the new IP address of your CaniotBox on ``connect to caniotBox`` window and Click **ok** .


.. only:: html
    
   .. video:: _static/wifiClient.mp4
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

.. _windows Wifi: https://support.microsoft.com/en-us/windows/connect-to-a-wi-fi-network-in-windows-1f881677-b569-0cd5-010d-e3cd3579d263

.. _Caniot download page: https://github.com/caniot/caniot-tool
.. |menubarIcon| image:: images/menubar.png
                 :scale: 30 %
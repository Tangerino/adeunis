# Adeunis AMR
Adeunis AMR PULSE Transceiver W-Mbus

This is a sample code for Linux console application that interfaces the Adeunis W-MBUS usb dongle and displays some information.

Output screen:

<pre>
New frame
0C 44 46 06 65 01 01 10 03 07 7A 78 00 00 00 2F 
2F 04 12 32 05 00 00 CD 

---- Sun Apr 30 12:26:48 2017
Meter: 1
Serial number: 0646100101650307
Water meter: 133L (0.133m3)
Status OK (0)
RSSI: -22 dBm
</pre>
More details, please go to http://www.adeunis-rf.com

If you got an error opening the serial port, please try to issue the following command in BASH

<pre>
    $sudo chmod 666 /dev/ttyACM0
</pre>

Please adapt the device name to match the existing in your system. Look at the /dev folder.

The project is better viewd in Netbeas IDE.

To build the project just run

<pre>
$make all
</pre>

To run the program type:

<pre>
    $adeunis /dev/ttyACM0
</pre>
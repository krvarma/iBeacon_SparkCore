Detect iBeacon using Spark Core
-------------------------------

Here is a sample application I worked after [BLE Mini HCI experiment][1]. This sample uses [BLE Mini][2] with [HCI Firmware][3] to scan iBeacons. I tested with [RFDuino][4] with iBeacon example and [Apple AirLocate example][5]. The AirLocate application turns your iOS 7.0 or later device into an iBeacon transmitter. 

To detect a BLE Device/iBeacon, we have to:

1. Initialize HCI
2. Start device discovery
3. Wait for GAP_DeviceInformation packet (event code 0x060D). We will receive this for each device scanned.
4. Parse the GAP_DeviceInformation packet which contains RSSI, iBeacon UUID, Major, Minor and Measured Power
5. Wait for the scan to complete and repeat step 2 onwards

Here I am using BLE Mini as the BLE Central device to scan surrounding BLE device and communicate with Spark Core using Serial. The application initialize and start device discovery, waits for scanned devices and then publish the result using `Spark.publish`. After scanning is started, we will get GAP_DeviceInformation for every device present in the surrounding area. The device information contains RSSI, iBeacon UUID, Major, Minor and Measured Power. 

Using the RSSI value we get from the Device Information packet, we can find the distance of the device using the formula [described here][6]. This is only an approximate distance, we can use this one to determine whether the device is too close or too far, etc... The Apple iBeacon framework uses some other formula that is not public. [Radius Networks][7] derived the above formula and is used in the iBeacon library.

The application looks for a beacon, when found one and if the distance is less than 0.5m then turn on an LED. We can test this by placing an iBeacon close to the Spark Core. If it is very close the LED turns on and when we move away from Spark Core the LED turns off.

Here is a demo video of the sample application in action.

https://www.youtube.com/watch?v=Jsnnv8sTSeM

You can also use the companion Web Application to see the nearby iBeacons. To use the Web Page, you should replace the tags *deviceid* and *accesstoken*.

**Wiring**

1. BLE Mini Vin to 5V
2. BLE GND to GND
3. BLE Mini Tx to Rx
4. BLE Mini Rx to Tx
5. LED to Spark Core D2

**Screenshots**

![enter image description here][8]

![enter image description here][9]

![enter image description here][10]

![enter image description here][11]

![enter image description here][12]


  [1]: https://community.spark.io/t/ble-mini-and-spark-core/5377
  [2]: http://redbearlab.com/blemini/
  [3]: https://github.com/RedBearLab/BLE_HCI/tree/master/cc2540_hci_fw
  [4]: http://www.rfduino.com/
  [5]: https://developer.apple.com/library/ios/samplecode/AirLocate/Introduction/Intro.html
  [6]: http://stackoverflow.com/questions/20416218/understanding-ibeacon-distancing
  [7]: http://developer.radiusnetworks.com/ibeacon/
  [8]: https://raw.githubusercontent.com/krvarma/iBeacon_SparkCore/master/screenshots/IMG_0105.PNG
  [9]: https://raw.githubusercontent.com/krvarma/iBeacon_SparkCore/master/screenshots/IMG_0107.JPG
  [10]: https://raw.githubusercontent.com/krvarma/iBeacon_SparkCore/master/screenshots/IMG_0108.JPG
  [11]: https://raw.githubusercontent.com/krvarma/iBeacon_SparkCore/master/screenshots/IMG_3637.JPG
  [12]: https://raw.githubusercontent.com/krvarma/iBeacon_SparkCore/master/screenshots/web%20page.png
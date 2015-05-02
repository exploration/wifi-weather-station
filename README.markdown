# WiFi Weather Station

Inspired by [this tutorial](https://learn.adafruit.com/wifi-weather-station-arduino-cc3000), modified to use some different sensors, and to post data to <https://data.sparkfun.com>.


# Instructions
The code should work using the provided wiring diagrams. Beyond that, follow these steps:

## General Arduino IDE Setup
- You'll need to install the [Arduino IDE](http://www.arduino.cc/en/Main/Software).
- Once you've got the IDE installed, you need to install some libraries:
  - See [this guide on installing libraries](http://www.arduino.cc/en/Guide/Libraries)
- [Adafruit Sensor Library](https://github.com/adafruit/Adafruit_Sensor)
- [Adafruit DHT Unified Library](https://github.com/adafruit/Adafruit_DHT_Unified)
- [Adafruit DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)

## Code Modifications
- Replace the `YOUR_NETWORK_ID` and `YOUR_NETWORK_PASSWORD` in the `WiFi Constants` section to be the actual network SSID and password for your local WiFi.
- Replace `YOUR_STATION_NAME` in the `WiFi Constants` section with a unique name for your particular weather station. You can name it anything, so go crazy!
- Replace the `YOUR_PUBLIC_KEY` and `YOUR_PRIVATE_KEY` in the `SparkFun Data` section with the public key and private key from your [SparkFun Data Stream](https://data.sparkfun.com/streams/make).


# TO DO
- We still need to add the code for the barometric pressure sensor, and add that information to the Sparkfun data stream export.

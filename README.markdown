# WiFi Weather Station

Inspired by [this tutorial](https://learn.adafruit.com/wifi-weather-station-arduino-cc3000), modified to use some different sensors, and to post data to <https://data.sparkfun.com>.

# Instructions
The code should work using the provided wiring diagrams. Beyond that, follow these steps:

- Replace the `YOUR-NETWORK-ID` and `YOUR-NETWORK-PASSWORD` in the `WiFi Constants` section to be the actual network SSID and password for your local WiFi.
- Replace `YOUR-STATION-NAME` in the `WiFi Constants` section with a unique name for your particular weather station. You can name it anything, so go crazy!
- Replace the `YOUR-PUBLIC-KEY` and `YOUR-PRIVATE-KEY` in the `SparkFun Data` section with the public key and private key from your [SparkFun Data Stream](https://data.sparkfun.com/streams/make).

# TO DO
- We still need to add the code for the barometric pressure sensor, and add that information to the SparkFun data stream export.

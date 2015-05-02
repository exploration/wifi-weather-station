/*
WiFi Weather Station!

Adapted from https://learn.adafruit.com/wifi-weather-station-arduino-cc3000
Modified to post to https://data.sparkfun.com

Authors: Marco Schwartz for Adafruit ( https://learn.adafruit.com/users/marco26fr )
         Donald Merand for Explo ( https://github.com/dmerand )
         Andy Smith for Explo ( https://github.com/asmithexplo )

Requires some Arduino libraries. 
- Adafruit Sensor Library: https://github.com/adafruit/Adafruit_Sensor
- Adafruit DHT Unified Library: https://github.com/adafruit/Adafruit_DHT_Unified
- Adafruit DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library

See this guide on installing libraries: http://www.arduino.cc/en/Guide/Libraries
*/

// This is for the CC3000 WiFi Module
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
// This is the DHT temperature/humidity sensor library
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

////////////////////////////////////
// CC3000 Shield Pins & Variables //
////////////////////////////////////
// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed
Adafruit_CC3000_Client www;

////////////////////
// WiFi Constants //
////////////////////
#define STATION_NAME    "YOUR_STATION_NAME"    // ID of device when posting data
#define WLAN_SSID       "YOUR_WIFI_NETWORK"    // cannot be longer than 32 characters!
#define WLAN_PASS       "YOUR_WIFI_PASSWORD"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS 3000      // Amount of time to wait (in milliseconds) with no data 
                                  // received before closing the connection.  If you know the server
                                  // you're accessing is quick to respond, you can reduce this value.
uint32_t ip;                      // IP address of the site to which we will be connecting

// What page to grab!
#define SERVER          "data.sparkfun.com"

////////////////////////////
// DHT11 sensor           //
////////////////////////////
#define DHTPIN 7 
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);
float temperature;
float humidity;
sensor_t sensor;
sensors_event_t event;

////////////////
// Input Pins //
////////////////
// press the button at this pin to send data
#define TRIGGERPIN   2        


/////////////////////////////////
// Phant (Sparkfun Data) Stuff //
// http://data.sparkfun.com    //
/////////////////////////////////
#define PUBLIC_KEY   "YOUR_PUBLIC_KEY"
#define PRIVATE_KEY  "YOUR_PRIVATE_KEY"

// these are your "fields" of data that you will be sending to SparkFun.
const byte NUM_FIELDS = 3;
// make sure these are spelled *exactly* as they are in your stream definition
// there should only be as many array entries here as there are "NUM_FIELDS" above
const String fieldNames[NUM_FIELDS] = {"humidity", "temp", "station"};
String fieldData[NUM_FIELDS];






// NOW THE ARDUINO BEGINS

// Arduino default setup procedure
void setup() {
  Serial.begin(115200);
  Serial.println("Hello, Explo Coders!");
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  // Setup Input Pins:
  digitalWrite(TRIGGERPIN, HIGH); //pullup pin powers posting
  
  // Since the Wifi setup is extensive, it is moved into its own function at the bottom.
  setupWiFi();

  // Initialize DHT11 temp/humidity sensor. The function for this is defined below as well.
  setupDHT11();
}

void loop() {
  // If the trigger pin (2) goes low (button press), send the data.
  if (digitalRead(TRIGGERPIN) == LOW) {
    Serial.println("Posting Data!");
    Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
    // attempt to store temperature into a variable
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println("Error reading temperature!");
    }
    else {
      Serial.print("Temperature:");
      Serial.println(event.temperature);
      temperature = event.temperature;
    }
    // attempt to store humidity into a variable
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println("Error reading humidity!");
    }
    else {
      Serial.print("Humidity:");
      Serial.println(event.relative_humidity);
      humidity = event.relative_humidity;
    }
    
    // Gather variables for posting to the web:
    fieldData[0] = (String)humidity; // humidity
    fieldData[1] = (String)temperature; // temperature
    fieldData[2] = STATION_NAME; // station name

    postData();  // the postData() function does all the work, 
                 // check it out below.
    delay(1000); // wait 1 second (1000 milliseconds)
  }
 
}


/**************************************************************************/
/*!
    @brief  Post weather station data to the web.
*/
/**************************************************************************/
void postData() {
  /* Try connecting to the website.
     Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  */
  // instantiate the web client
  www = cc3000.connectTCP(ip, 80);

  // request the site, passing along variables in the GET request
  if (www.connected()) {
    www.fastrprint(F("GET /input/"));
    www.fastrprint(PUBLIC_KEY);
    www.fastrprint(F("?private_key="));
    www.fastrprint(PRIVATE_KEY);
    for (int i=0; i<NUM_FIELDS; i++) {
      www.fastrprint(F("&"));
      www.print(fieldNames[i]);
      www.fastrprint(F("="));
      www.print(fieldData[i]);
    }
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(SERVER); www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.println();
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }
  Serial.println(F("-------------------------------------"));
  
  /* Read data until either the connection is closed, or the idle timeout is reached. */ 
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      lastRead = millis();
    }
  }
  //www.close();
  Serial.println(F("-------------------------------------"));

}


/**************************************************************************/
/*!
    @brief  Configure the DHT11 + print the results to serial output
*/
/**************************************************************************/
void setupDHT11() {
  dht.begin();
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
  // Set delay between sensor readings based on sensor details.
  
  Serial.println(F("\n=========== Ready to Stream ==========="));
  Serial.println(F("Press the button (D2) to send an update"));
}


/**************************************************************************/
/*!
    @brief  Configure the CC3000 + print the results to serial output
*/
/**************************************************************************/
void setupWiFi() {
// Initialise the Wifi module
  Serial.println(F("\nInitializing CC3000 Wifi Module..."));
  if (!cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1); // don't move forward until the CC3000 initializes properly
  }
  
  // Now connect to the network defined above...
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1); // don't move forward until the CC3000 connects to WiFi
  }
  Serial.println(F("Connected!"));
  
  // Wait for the local network to assign an IP address using DHCP
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  
  
  // Display the IP address DNS, Gateway, etc.
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  
  ip = 0;
  // Try looking up the website's IP address
  Serial.print(SERVER); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(SERVER, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }

  // now print out some IP information about the site to which we are connecting
  cc3000.printIPdotsRev(ip);
  Serial.println();
}


/**************************************************************************/
/*!
    @brief  Tries to read the CC3000 IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

/* 
WiFi Weather Station!

Adapted from https://learn.adafruit.com/wifi-weather-station-arduino-cc3000
Modified to post to https://data.sparkfun.com

Authors: Marco Schwartz for Adafruit ( https://learn.adafruit.com/users/marco26fr )
         Donald Merand for Explo ( https://github.com/dmerand )
         Andy Smith for Explo ( https://github.com/asmithexplo )
*/

// SPI and the pair of SFE_CC3000 include statements are required for using the
// CC300 shield as a client device.
#include <SPI.h>
#include <SFE_CC3000.h>
#include <SFE_CC3000_Client.h>
// Progmem allows us to store big strings in flash using F().  We'll sacrifice
// some flash for extra DRAM space.
#include <Progmem.h>
#include <string.h>
// This is the DHT temperature/humidity sensor library
#include "DHT.h"



////////////////////////////////////
// CC3000 Shield Pins & Variables //
////////////////////////////////////
#define CC3000_INT      3   // Needs to be an interrupt pin (D2/D3)
#define CC3000_EN       5   // Can be any digital pin
#define CC3000_CS       10  // Preferred is pin 10 on Uno
#define IP_ADDR_LEN     4   // Length of IP address in bytes

////////////////////////////
// DHT11 sensor constants //
////////////////////////////
#define DHTPIN 7 
#define DHTTYPE DHT11

////////////////
// Input Pins //
////////////////
#define triggerPin      2   // post activation button

////////////////////
// WiFi Constants //
////////////////////
#define STATION_NAME "YOUR-STATION-NAME"             // ID of device when posting data
#define ap_security WLAN_SEC_WPA2
// ap_security can be any of: WLAN_SEC_UNSEC, WLAN_SEC_WEP, 
//  WLAN_SEC_WPA, or WLAN_SEC_WPA2
char ap_ssid[] = "YOUR-NETWORK-ID";           // SSID of network
char ap_password[] = "YOUR-NETWORK-PASSWORD"; // Password of network
// Security of network
unsigned int timeout = 5000;             // Milliseconds
char server[] = "data.sparkfun.com";     // Remote host site

/////////////////////////////////
// Phant (Sparkfun Data) Stuff //
/////////////////////////////////
const String publicKey = "YOUR-PUBLIC-KEY";
const String privateKey = "YOUR-PRIVATE-KEY";
const byte NUM_FIELDS = 3;
// make sure these are spelled *exactly* as they are in your stream definition
const String fieldNames[NUM_FIELDS] = {"humidity", "temp", "station"};
String fieldData[NUM_FIELDS];



// Initialize the CC3000 objects (wifi breakout board and client)://
SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
SFE_CC3000_Client client = SFE_CC3000_Client(wifi);

// Initialize the DHT temp/humidity sensor board
DHT dht(DHTPIN, DHTTYPE);



void setup() {
  Serial.begin(115200);
  
  // Setup Input Pins:
  digitalWrite(triggerPin, HIGH); //pullup pin

  // Initialize DHT sensor
  dht.begin();
  
  // Set Up WiFi:
  setupWiFi();
  
  Serial.println(F("\n=========== Ready to Stream ==========="));
  Serial.println(F("Press the button (D2) to send an update"));
}

void loop() {
  // If the trigger pin (2) goes low (button press), send the data.
  if (digitalRead(triggerPin) == LOW) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // Gather data:
    fieldData[0] = (String)h; // humidity
    fieldData[1] = (String)t; // temperature
    fieldData[2] = STATION_NAME; // station name
    
    // Post data:
    Serial.println("Posting Data!");

    postData();  // the postData() function does all the work, 
                 // check it out below.
    delay(1000); // wait 1 second (1000 milliseconds)
  }
 
}

void postData() {
  
  // Make a TCP connection to remote host
  if ( !client.connect(server, 80) ) {
    // Error: 4 - Could not make a TCP connection
    Serial.println(F("Error: 4"));
  }
  
  // Post the data! Request should look a little something like:
  // GET /input/publicKey?private_key=privateKey&light=1024&switch=0&time=5201 HTTP/1.1\n
  // Host: data.sparkfun.com\n
  // Connection: close\n
  // \n
  client.print("GET /input/");
  client.print(publicKey);
  client.print("?private_key=");
  client.print(privateKey);
  for (int i=0; i<NUM_FIELDS; i++) {
    client.print("&");
    client.print(fieldNames[i]);
    client.print("=");
    client.print(fieldData[i]);
  }
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server);
  client.println("Connection: close");
  client.println();
  
  // TODO: we need to print what gets sent back from the server without
  // blocking subsequent posts. currently we're not printing the server result
  // at all
  /*
  while (client.connected()) {
    if ( client.available() ) {
      char c = client.read();
      Serial.print(c);
    } else {
      if ( !client.close() ) {
        Serial.println("Error: Could not close socket");
      }
    }
  }
  Serial.println();
  */
}

void setupWiFi() {
  ConnectionInfo connection_info;
  int i;
  
  // Initialize CC3000 (configure SPI communications)
  if ( wifi.init() ) {
    Serial.println(F("CC3000 Ready!"));
  } else {
    // Error: 0 - Something went wrong during CC3000 init!
    Serial.println(F("Error: 0"));
  }
  
  // Connect using DHCP
  Serial.print(F("Connecting to: "));
  Serial.println(ap_ssid);
  if(!wifi.connect(ap_ssid, ap_security, ap_password, timeout)) {
    // Error: 1 - Could not connect to AP
    Serial.println(F("Error: 1"));
  }
  
  // Gather connection details and print IP address
  if ( !wifi.getConnectionInfo(connection_info) ) {
    // Error: 2 - Could not obtain connection details
    Serial.println(F("Error: 2"));
  } else {
    Serial.print(F("My IP: "));
    for (i = 0; i < IP_ADDR_LEN; i++) {
      Serial.print(connection_info.ip_address[i]);
      if ( i < IP_ADDR_LEN - 1 ) {
        Serial.print(".");
      }
    }
    Serial.println();
  }
}

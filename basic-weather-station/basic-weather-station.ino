
/*
Weather Station!
Author: Donald Merand for Explo ( https://github.com/dmerand )

Requires some Arduino libraries.
- Adafruit CC3000 Library: https://github.com/adafruit/Adafruit_CC3000_Library
- Adafruit Sensor Library: https://github.com/adafruit/Adafruit_Sensor
- Adafruit DHT Unified Library: https://github.com/adafruit/Adafruit_DHT_Unified
- Adafruit DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
- Adafruit BMP Unified Library: https://github.com/adafruit/Adafruit_BMP085_Unified

See this guide on installing libraries: http://www.arduino.cc/en/Guide/Libraries
*/

// This is the DHT temperature/humidity sensor library
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
// This the BMP085 sensor
#include <Adafruit_BMP085_U.h>
#include <Wire.h>



// Generic sensor definitions
sensor_t sensor;
sensors_event_t event;

////////////////////////////
// DHT11 sensor           //
////////////////////////////
// DHT11 Pinouts:
// pin 1: 5V line
// pin 2: signal line
// pin 4: ground line
#define DHTPIN 7 
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);

////////////////////////////
// BMP180 Sensor          //
////////////////////////////
/*
 Connections
   ===========
   Connect SCL to ANALOG 5 (not digital pin 5)
   Connect SDA to ANALOG 4
   Connect VDD/VCC/VIN to 3.3V DC
   Connect GROUND to common ground
*/
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10042);



////////////////
// Input Pins //
////////////////
// press the button at this pin to send data
#define TRIGGERPIN   2        

//////////////////////
// Sensor Variables //
//////////////////////
float temperature;
float humidity;
float pressure;
float altitude;
float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;




// NOW THE ARDUINO BEGINS

// Arduino default setup procedure
void setup() {
  Serial.begin(115200);
  Serial.println("Hello, Explo Coders!");

  // Setup Input Pins:
  pinMode(TRIGGERPIN, INPUT);
  digitalWrite(TRIGGERPIN, HIGH); //pullup pin powers posting
  
  // Initialize DHT11 temp/humidity sensor. The function for this is defined below.
  setupDHT11();
  
  // Initialize BMP085 barometric pressure sensor
  setupBMP085();
  
  
  Serial.println(F("\n=========== Ready! ==========="));
  Serial.println(F("Press the button (D2) to see the weather"));
}

void loop() {
  // If the trigger pin (2) goes low (button press), send the data.
  if (digitalRead(TRIGGERPIN) == LOW) {
    Serial.println("\nLet's Read Some Data!");
  
    // attempt to store temperature into a variable
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println("Error reading temperature!");
    } else {
      Serial.print("Temperature:");
      Serial.println(event.temperature);
      temperature = event.temperature;
    }
    // attempt to store humidity into a variable
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println("Error reading humidity!");
    } else {
      Serial.print("Humidity:");
      Serial.println(event.relative_humidity);
      humidity = event.relative_humidity;
    }
    
    /// now for barometric pressure + altitude
    bmp.getEvent(&event);
    /* Display the results (barometric pressure is measure in hPa) */
    if (event.pressure) {
      /* Display atmospheric pressue in hPa */
      pressure = event.pressure;
      
      /* Calculating altitude with reasonable accuracy requires             *
       * sea level pressure for your position at the moment the data is     *
       * converted, as well as the ambient temperature in degress           *
       * celcius.  If you don't have these values, a 'generic' value of     *
       * 1013.25 hPa can be used (defined as SENSORS_PRESSURE_SEALEVELHPA   *
       * in sensors.h), but this isn't ideal and will give variable         *
       * results from one day to the next.                                  *
       *                                                                    *
       * You can usually find the current SLP value by looking at weather   *
       * websites or from environmental information centers near any major  *
       * airport.                                                           *
       *                                                                    *
       * For example, for Paris, France you can check the current mean      *
       * pressure and sea level at: http://bit.ly/16Au8ol                   */
  
      /* Convert the atmospheric pressure, and SLP to altitude              */
      /* Update this next line with the current SLP for better results      */
      float NORTON_SLP = 1020;
      //altitude = bmp.pressureToAltitude(seaLevelPressure, event.pressure); 
      altitude = bmp.pressureToAltitude(NORTON_SLP, event.pressure); 
      Serial.print("Altitude: ");
      Serial.println(altitude);
      Serial.print("Pressure: ");
      Serial.println(pressure);

    }
    else {
      Serial.println("BMP085 Sensor error");
    }
    
    delay(1000); // wait 1 second (1000 milliseconds)
  }
 
}




/**************************************************************************/
/*!
    @brief  Configure the DHT11 + print the results to serial output
*/
/**************************************************************************/
void setupDHT11() {
  dht.begin();
  dht.temperature().getSensor(&sensor);  
  dht.humidity().getSensor(&sensor);
  // Set delay between sensor readings based on sensor details.
  delay(500);
}

/**************************************************************************/
/*!
    @brief  Configure the BMP085 + print the results to serial output
*/
/**************************************************************************/
void setupBMP085() {
  /* Initialise the sensor */
  if(!bmp.begin()) {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  bmp.getSensor(&sensor);
  delay(500);
}

/*
  File:           Greenhouse_DEV_FW.ino
  Date Last Mod:  2AUG2021
  Description:    Proof-of-concept for a greenhouse monitoring device to be installed at Snyder Farm in Everett, PA

  Top Level Requirements:

  Sensor Inputs

    - [x]  Temperature
    - [x]  Humidity
    - [ ]  Duration of sunlight
        - [x]  Measure illuminance (FC)
    - [ ]  Soil pH  (TBD)
    - [ ]  Hydroponics water quality  (TBD)
    - [ ]  propane heater particulate matter (CO2 or TVOC) (???)
    - [ ]  Monitor position of LOUVERS

  Actuator Outputs

    - [ ]  Open/close LOUVERS
    - [ ]  Relay to turn FAN on/off
    - [ ]  Relay to turn HEATER on/off
    - [x]  Onboard LED

  Communications

    - [x]  Send telemetry to cloud
    - [x]  Send manual control command to device
    - [x]  WiFi

  UI / UX

    - [ ]  Mobile App
    - [ ]  Web Browser
  
*/

#include "arduino_secrets.h"
#include "thingProperties.h"
#include "system_settings.h"
#include <Arduino_MKRENV.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>

#define DEBUG
#define FW_VERSION_ID "0.0.1-alpha"

#ifdef DEBUG
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#endif


int SENSOR_READING_COUNTER = 0;
bool ENOUGH_SENSOR_READINGS_FOR_AVG = false;

float avgTemperature = 0.0;
float avgHumidity = 0.0;
float avgPressure = 0.0;
float avgIlluminance = 0.0;


void setup() {
  /* Initialize serial and wait up to 5 seconds for port to open */
  Serial.begin(9600);
  for (unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime > 8000);) {}

  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1)
      ;
  }

  /* Configure LED pin as an output */
  WiFiDrv::pinMode(25, OUTPUT);  //Green LED
  WiFiDrv::pinMode(26, OUTPUT);  //Red LED
  WiFiDrv::pinMode(27, OUTPUT);  //Blue LED
  WiFiDrv::analogWrite(25, 0);   //GREEN
  WiFiDrv::analogWrite(26, 0);   //RED
  WiFiDrv::analogWrite(27, 0);   //BLUE

  Serial.print(F("Snyder Greenhouse System (version "));
  Serial.print(FW_VERSION_ID);
  Serial.println(F(") initializing..."));

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(DBG_INFO);
  ArduinoCloud.printDebugInfo();
  delay(1000);
  Serial.println(F("Initialization complete.  System running...\n\n"));
}




void loop() {
  ArduinoCloud.update();
  readSensors();
  recordSensorData();
  calculateAverageSensorReadings();

#ifdef DEBUG
  printSensorSerial();
  if (ENOUGH_SENSOR_READINGS_FOR_AVG == true) {
    printAvgSensorSerial();
  }
#endif


  delay(1000);
}


/*
 * 'onLedChange' is called when the "led" property of your Thing changes
 */
void onManualControlChange() {
  Serial.print(F("MANUAL CONTROL set to "));
  if (MANUAL_CONTROL_ON == false) {
    WiFiDrv::analogWrite(25, 0);  //GREEN
    WiFiDrv::analogWrite(26, 0);  //RED
    WiFiDrv::analogWrite(27, 0);  //BLUE
    Serial.println(F("OFF."));
  } else {
    WiFiDrv::analogWrite(25, 0);    //GREEN
    WiFiDrv::analogWrite(26, 255);  //RED
    WiFiDrv::analogWrite(27, 0);    //BLUE
    Serial.println(F("ON."));
  }
}




void printSensorSerial() {
  Serial.print("Temperature (F): ");
  Serial.println(temperature);
  Serial.print("Humidity(%): ");
  Serial.println(humidity);
  Serial.print("Pressure (PSI): ");
  Serial.println(pressure);
  Serial.print("Illuminance (FC): ");
  Serial.println(illuminance);
  Serial.print("UVA:  ");
  Serial.println(uva);
  Serial.print("UVB: ");
  Serial.println(uvb);
  Serial.print("UV Index: ");
  Serial.println(uvIndex);
  Serial.print("Manual Control Status: ");
  Serial.println(MANUAL_CONTROL_ON);
  Serial.println();
}





void readSensors() {
  temperature = ENV.readTemperature(FAHRENHEIT);
  humidity = ENV.readHumidity();
  pressure = ENV.readPressure(PSI);
  illuminance = ENV.readIlluminance(FOOTCANDLE);
  uva = ENV.readUVA();
  uvb = ENV.readUVB();
  uvIndex = ENV.readUVIndex();
}



void recordSensorData() {

  if (SENSOR_READING_COUNTER > 9) {
    SENSOR_READING_COUNTER = 0;
    ENOUGH_SENSOR_READINGS_FOR_AVG = true;
  }

  TEMP_LAST_X_READINGS[SENSOR_READING_COUNTER] = temperature;
  HUMIDITY_LAST_X_READINGS[SENSOR_READING_COUNTER] = humidity;
  PRESSURE_LAST_X_READINGS[SENSOR_READING_COUNTER] = pressure;
  ILLUMINANCE_LAST_X_READINGS[SENSOR_READING_COUNTER] = illuminance;
  SENSOR_READING_COUNTER++;
}


void calculateAverageSensorReadings() {
  if (ENOUGH_SENSOR_READINGS_FOR_AVG == true) {
    for (int ctr = 0; ctr < NUM_SENSOR_READINGS_TO_STORE; ctr++) {
      avgTemperature += TEMP_LAST_X_READINGS[ctr];
      avgHumidity += HUMIDITY_LAST_X_READINGS[ctr];
      avgPressure += PRESSURE_LAST_X_READINGS[ctr];
      avgIlluminance += ILLUMINANCE_LAST_X_READINGS[ctr];
    }

    avgTemperature /= NUM_SENSOR_READINGS_TO_STORE;
    avgHumidity /= NUM_SENSOR_READINGS_TO_STORE;
    avgPressure /= NUM_SENSOR_READINGS_TO_STORE;
    avgIlluminance /= NUM_SENSOR_READINGS_TO_STORE;
  }
}


void printAvgSensorSerial() {
  Serial.print("Avg. Temperature (F): ");
  Serial.println(avgTemperature);
  Serial.print("Avg. Humidity(%): ");
  Serial.println(avgHumidity);
  Serial.print("Avg. Pressure (PSI): ");
  Serial.println(avgPressure);
  Serial.print("Avg. Illuminance (FC): ");
  Serial.println(avgIlluminance);
  Serial.println();
}

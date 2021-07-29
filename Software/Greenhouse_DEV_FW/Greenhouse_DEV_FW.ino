/*
  This sketch demonstrates how to exchange data between your board and the Arduino IoT Cloud.

  * Connect a potentiometer (or other analog sensor) to A0.
  * When the potentiometer (or sensor) value changes the data is sent to the Cloud.
  * When you flip the switch in the Cloud dashboard the onboard LED lights gets turned ON or OFF.

  IMPORTANT:
  This sketch works with WiFi, GSM, NB and Lora enabled boards supported by Arduino IoT Cloud.
  On a LoRa board, if it is configuered as a class A device (default and preferred option), values from Cloud dashboard are received
  only after a value is sent to Cloud.

  This sketch is compatible with:
   - MKR 1000
   - MKR WIFI 1010
   - MKR GSM 1400
   - MKR NB 1500
   - MKR WAN 1300/1310
   - Nano 33 IoT
   - ESP 8266
*/

#include "arduino_secrets.h"
#include "thingProperties.h"
#include <Arduino_MKRENV.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>

#define DEBUG

#ifdef DEBUG
 #define DEBUG_PRINTLN(x)  Serial.println (x)
 #define DEBUG_PRINT(x)  Serial.print(x)
#else
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINT(x)
#endif


void setup() {
  /* Initialize serial and wait up to 5 seconds for port to open */
  Serial.begin(9600);
  for (unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime > 5000);) {}

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

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(DBG_INFO);
  ArduinoCloud.printDebugInfo();
}




void loop() {
  ArduinoCloud.update();
  readSensors();

#ifdef DEBUG
printSensorSerial();
#endif

delay(1000);
}





/*
 * 'onLedChange' is called when the "led" property of your Thing changes
 */
void onLedChange() {
  Serial.print("LED set to ");
  Serial.println(led);
  if (led == 0) {
    WiFiDrv::analogWrite(25, 0);  //GREEN
    WiFiDrv::analogWrite(26, 0);  //RED
    WiFiDrv::analogWrite(27, 0);  //BLUE
  } else {
    WiFiDrv::analogWrite(25, 0);    //GREEN
    WiFiDrv::analogWrite(26, 255);  //RED
    WiFiDrv::analogWrite(27, 0);    //BLUE
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
  Serial.print("LED Status: ");
  Serial.println(led);
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

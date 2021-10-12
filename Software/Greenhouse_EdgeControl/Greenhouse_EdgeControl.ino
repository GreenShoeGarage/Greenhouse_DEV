#include <Arduino_EdgeControl.h>
#include <openmvrpc.h>

openmv::rpc_scratch_buffer<256> scratch_buffer; // All RPC objects share this buffer.
openmv::rpc_i2c_master interface(0x12, 10000);

constexpr uint32_t printInterval { 5000 };
uint32_t printNow { 0 };

void send_battery_voltage()
{
  auto vbat = Power.getVBat();
  String str = "@BATTERY_VOLTAGE,";
  str += vbat;
  str += "#";
  char buffer[str.length() + 1] {};
  str.toCharArray(buffer, sizeof(buffer));
  interface.call("serial_print", buffer, sizeof(buffer));
}



void setup()
{
  interface.begin();
  Serial.begin(115200);
  Serial.println("Beginning Edge Control initialization...");
  EdgeControl.begin();
  Power.on(PWR_3V3);
  Power.on(PWR_VBAT);
  Power.on(PWR_MKR2);
  delay(5000); // Wait for MKR2 to power-on
  
  Wire.begin();
  delay(500);

  Serial.print("I/O Expander initializazion ");
  if (!Expander.begin()) {
    Serial.println("failed.");
    Serial.println("Please, be sure to enable gated 3V3 and 5V power rails");
    Serial.println("via Power.on(PWR_3V3) and Power.on(PWR_VBAT).");
  }
  Serial.println("succeeded.");

  Expander.pinMode(EXP_FAULT_SOLAR_PANEL, INPUT);
  Expander.pinMode(EXP_FAULT_5V, INPUT);

  printNow = millis();
}




void loop()
{
  if (millis() > printNow) {
    send_battery_voltage();
    printNow = millis() + printInterval;
  }
}

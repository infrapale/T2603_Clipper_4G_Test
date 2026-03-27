#include <Arduino.h>
#include "LTE_SMS.h"

HardwareSerial LTE(1);
LTE_SMS sms(LTE);

void setup() {
  Serial.begin(115200);
  sms.begin();

  sms.sendSMS("+358401234567", "Sensor value: 23.7C");
}

void loop() {
  // Print all messages every 20 seconds
  delay(20000);
  Serial.println(sms.readAll());
}

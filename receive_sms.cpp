#include <Arduino.h>

HardwareSerial LTE(1);

void sendAT(const char *cmd, uint32_t wait = 500) {
  LTE.println(cmd);
  delay(wait);
  while (LTE.available()) {
    Serial.write(LTE.read());
  }
}

void setup() {
  Serial.begin(115200);
  LTE.begin(115200);

  delay(2000);
  Serial.println("Initialising modem…");

  sendAT("AT");
  sendAT("AT+CMGF=1");   // text mode

  // List unread messages
  sendAT("AT+CMGL=\"REC UNREAD\"", 1000);
}

void loop() {
  // Poll for new SMS every 10 seconds
  delay(10000);
  sendAT("AT+CMGL=\"REC UNREAD\"", 1000);
}

#include <Arduino.h>

HardwareSerial LTE(1);   // UART1 on Pico 2
// Pico 2 UART1 default pins: TX=GP4, RX=GP5

void sendAT(const char *cmd, uint32_t wait = 500) {
  LTE.println(cmd);
  delay(wait);
  while (LTE.available()) {
    Serial.write(LTE.read());
  }
}

void setup() {
  Serial.begin(115200);
  LTE.begin(115200);   // SIMCom default baud

  delay(2000);
  Serial.println("Initialising modem…");

  sendAT("AT");
  sendAT("AT+CMGF=1");   // SMS text mode

  // Replace with your number
  sendAT("AT+CMGS=\"+358401234567\"");

  // Message body
  LTE.print("Hello from Pico 2 + Clipper LTE!");

  // End with Ctrl+Z
  LTE.write(26);

  Serial.println("SMS sent.");
}

void loop() {
}

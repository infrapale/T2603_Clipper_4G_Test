#include <Arduino.h>
#define LteSerial Serial2

#define PIN_TX0         (0u)
#define PIN_RX0         (1u)
#define PIN_TX1         (4u)
#define PIN_RX1         (5u)

//HardwareSerial LTE(1);   // UART1 on Pico 2
// Pico 2 UART1 default pins: TX=GP4, RX=GP5

void sendAT(const char *cmd, uint32_t wait = 500) {
  LteSerial.println(cmd);
  delay(wait);
  while (LteSerial.available()) {
    Serial.write(LteSerial.read());
  }
}

void setup() {
  Serial1.setTX(PIN_TX0);   
  Serial1.setRX(PIN_RX0);
  Serial2.setTX(PIN_TX1);   
  Serial2.setRX(PIN_RX1);

  Serial.begin(115200);
  LteSerial.begin(115200);   // SIMCom default baud

  delay(2000);
  Serial.println("Initialising modem…");

  sendAT("AT");
  sendAT("AT+CPIN=\"1234\"");
  sendAT("AT+CMGF=1");   // SMS text mode

  // Replace with your number
  sendAT("AT+CMGS=\"+358405056630\"");

  // Message body
  LteSerial.print("Hello from Pico 2 + Clipper LTE!");

  // End with Ctrl+Z
  LteSerial.write(26);

  Serial.println("SMS sent.");
}

void loop() {
}

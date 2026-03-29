// Pimoroni Pico Plus 2 and Clipper LTE
// https://github.com/infrapale/T2603_Clipper_4G_Test.git



#include <Arduino.h>
#define LteSerial Serial1

#define PIN_TX0         (32u)
#define PIN_RX0         (33u)
#define PIN_PWRKEY      (36u)
#define PIN_RESET       (35u)


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

  Serial.begin(115200);
  LteSerial.begin(115200);   // SIMCom default baud

  delay(2000);
  Serial.println("Initialising modem…");
  pinMode(PIN_PWRKEY, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);

  digitalWrite(PIN_RESET,HIGH);
  digitalWrite(PIN_PWRKEY,HIGH);
  delay(1000);
  digitalWrite(PIN_RESET,LOW);
  delay(500);
  digitalWrite(PIN_RESET,HIGH);
  delay(1000);
  digitalWrite(PIN_PWRKEY,LOW);
  delay(500);
  digitalWrite(PIN_PWRKEY,HIGH);
  delay(1000);


  sendAT("AT");
  sendAT("AT+CPIN=\"1234\"");
  sendAT("AT+CMGF=1");   // SMS text mode

  // Replace with your number
  // sendAT("AT+CMGS=\"+358405056630\"");
  sendAT("AT+CMGS=\"+358400737682\"");     // Sauli
  // sendAT("AT+CMGS=\"+358400454270\"");     // Hessu

  // Message body
  LteSerial.print("Hello from Pico Plus 2 + Clipper LTE!");

  // End with Ctrl+Z
  LteSerial.write(26);

  Serial.println("SMS sent.");
}

void loop() {
}

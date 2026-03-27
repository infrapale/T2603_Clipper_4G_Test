#pragma once
#include <Arduino.h>

class LTE_SMS {
public:
  LTE_SMS(HardwareSerial &serial) : modem(serial) {}

  void begin(uint32_t baud = 115200) {
    modem.begin(baud);
    delay(2000);
    sendAT("AT");
    sendAT("AT+CMGF=1");  // text mode
  }

  void sendSMS(const String &number, const String &text) {
    sendAT("AT+CMGF=1");
    modem.print("AT+CMGS=\"");
    modem.print(number);
    modem.println("\"");
    delay(300);

    modem.print(text);
    modem.write(26);  // Ctrl+Z
    delay(2000);
  }

  String readAll() {
    sendAT("AT+CMGL=\"ALL\"", 1000);
    return buffer;
  }

private:
  HardwareSerial &modem;
  String buffer;

  void sendAT(const char *cmd, uint32_t wait = 500) {
    modem.println(cmd);
    delay(wait);
    buffer = "";
    while (modem.available()) {
      buffer += (char)modem.read();
    }
    Serial.println(buffer);
  }
};

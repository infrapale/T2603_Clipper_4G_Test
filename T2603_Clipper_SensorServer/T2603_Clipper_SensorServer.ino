// !!!!!!!!!  NOT YET TESTED
#include <Arduino.h>

#define LteSerial Serial1


void sendAT(const char *cmd, uint32_t wait = 500) {
  Serial.print(">> ");
  Serial.println(cmd);
  LteSerial.println(cmd);
  delay(wait);

  while (LteSerial.available()) {
    Serial.write(LteSerial.read());
  }
  Serial.println();
}

String readLineFromLTE() {
  String s = "";
  while (LteSerial.available()) {
    char c = LteSerial.read();
    s += c;
    if (c == '\n') break;
  }
  return s;
}

void readSMS(int index);

// ------------------------------------------------------------
// Check for incoming SMS
// ------------------------------------------------------------
void checkIncomingSMS() {
  if (!LteSerial.available()) return;

  String line = readLineFromLTE();
  line.trim();

  if (line.length() == 0) return;

  Serial.print("[LTE] ");
  Serial.println(line);

  if (line.startsWith("+CMTI")) {
    int comma = line.indexOf(',');
    if (comma > 0) {
      int index = line.substring(comma + 1).toInt();
      Serial.print("[INFO] New SMS at index ");
      Serial.println(index);
      readSMS(index);
    }
  }
}

// ------------------------------------------------------------
// Read SMS and auto‑reply
// ------------------------------------------------------------
void readSMS(int index) {
  Serial.print("[ACTION] Reading SMS ");
  Serial.println(index);

  LteSerial.print("AT+CMGR=");
  LteSerial.println(index);
  delay(500);

  String sender = "";
  String message = "";

  while (LteSerial.available()) {
    String line = readLineFromLTE();
    Serial.print("[SMS] ");
    Serial.println(line);

    if (line.startsWith("+CMGR")) {
      int firstQuote = line.indexOf('"', 0);
      int secondQuote = line.indexOf('"', firstQuote + 1);
      int thirdQuote = line.indexOf('"', secondQuote + 1);
      int fourthQuote = line.indexOf('"', thirdQuote + 1);

      if (thirdQuote > 0 && fourthQuote > 0) {
        sender = line.substring(thirdQuote + 1, fourthQuote);
        Serial.print("[INFO] Sender: ");
        Serial.println(sender);
      }
    } else if (line.length() > 0 && !line.startsWith("+") && !line.startsWith("OK")) {
      message += line;
    }
  }

  Serial.print("[INFO] Message: ");
  Serial.println(message);

  // Auto‑reply
  if (sender.length() > 0) {
    Serial.println("[ACTION] Sending auto‑reply…");

    LteSerial.print("AT+CMGS=\"");
    LteSerial.print(sender);
    LteSerial.println("\"");
    delay(200);

    LteSerial.print("Auto‑reply: Message received!");
    LteSerial.write(26); // Ctrl+Z
    delay(1000);
  }

  // Delete SMS
  Serial.println("[ACTION] Deleting SMS");
  LteSerial.print("AT+CMGD=");
  LteSerial.println(index);
  delay(300);
}

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Pico Plus 2 LTE Modem Initialisation ===");

  Serial1.setTX(PIN_TX0);
  Serial1.setRX(PIN_RX0);
  LteSerial.begin(115200);

  pinMode(PIN_PWRKEY, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);

  digitalWrite(PIN_RESET, HIGH);
  digitalWrite(PIN_PWRKEY, HIGH);
  delay(1000);

  // Reset pulse
  Serial.println("[ACTION] Resetting modem…");
  digitalWrite(PIN_RESET, LOW);
  delay(500);
  digitalWrite(PIN_RESET, HIGH);
  delay(1500);

  // Power‑on pulse
  Serial.println("[ACTION] Powering modem ON…");
  digitalWrite(PIN_PWRKEY, LOW);
  delay(800);
  digitalWrite(PIN_PWRKEY, HIGH);
  delay(3000);

  // Basic AT init
  sendAT("AT");
  sendAT("ATE0");               // Echo off
  sendAT("AT+CPIN?");           // SIM status
  sendAT("AT+CMGF=1");          // SMS text mode
  sendAT("AT+CNMI=2,1,0,0,0");  // Enable new SMS notifications

  // Send initial SMS
  Serial.println("[ACTION] Sending test SMS…");
  LteSerial.println("AT+CMGS=\"+3585056630\"");
  delay(300);
  LteSerial.print("Hello from Pico Plus 2 + LTE!");
  LteSerial.write(26);
  delay(1000);

  Serial.println("[INFO] Setup complete. Waiting for SMS…");
}

// ------------------------------------------------------------
// Main loop
// ------------------------------------------------------------
void loop() {
  checkIncomingSMS();
}
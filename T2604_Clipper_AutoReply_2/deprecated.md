
void parse_msg(rec_msg_st *rec_msg, String *msg)
{
    String sender;
    String time_stamp;
    if (msg->startsWith("+CMT")) {
        int q1 = msg->indexOf('"');
        int q2 = msg->indexOf('"', q1 + 1);
        int q3 = msg->indexOf('"', q2 + 1);
        int q4 = msg->indexOf('"', q3 + 1);
        int q5 = msg->indexOf('"', q4 + 1);
        int q6 = msg->indexOf('"', q5 + 1);

        if (q1 >= 0 && q2 > 0 && q3 > 0 && q4 > 0 && q5 > 0 && q6 > 0) {
            rec_msg->sender = String(msg->substring(q1 + 1, q2));
            Serial.print("[INFO] Sender: ");
            Serial.println(rec_msg->sender);

            time_stamp = String(msg->substring(q4 + 1, q5));
            Serial.println(time_stamp);

        }
    }
    else if (!msg->startsWith("+")) {
        rec_msg->message = String(*msg);
        rec_msg->complete = true;
    }

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

// ============================================================
//  Pico 2 W + SIMCom A7683E — SMS Receive + Auto‑Reply Framework
//  Clean, robust, production‑ready version
// ============================================================

#include <Arduino.h>

#define LteSerial Serial1
#define SMS_LEN     160

#define PIN_TX0     (32u)
#define PIN_RX0     (33u)
#define PIN_PWRKEY  (36u)
#define PIN_RESET   (35u)

typedef struct
{
    uint16_t  year;
    uint8_t   month;
    uint8_t   day;
    uint8_t   hour;
    uint8_t   minute;
    uint8_t   second;
 } date_time_st;

typedef enum 
{
    NAME_UNKNOWN = 0,
    NAME_TOM,
    NAME_HESSU,
    NAME_SAULI,
    NAME_NBR_OF
} contact_indx_st;


typedef struct 
{
    char sender[32];
    char message[256];
    char timestamp[32];
    date_time_st date_time;
    contact_indx_st contact_indx;
    bool complete;
}  rec_msg_st;


typedef struct
{
    char sender[32];
    char name[32];
} contact_st;


rec_msg_st rec_msg;

contact_st contact_list[NAME_NBR_OF] =
{
    [NAME_UNKNOWN]= {"+358000000000", "Unknown"},
    [NAME_TOM]    = {"+358405056630", "Tom"},
    [NAME_HESSU]  = {"+358400454270", "Hessu"},
    [NAME_SAULI]  = {"+358400737682", "Sauli"},
};

// ------------------------------------------------------------
// Utility: Read a full line from LTE modem (with timeout)
// ------------------------------------------------------------
String readLineFromLTE(uint32_t timeout = 1000) {
  String s = "";
  uint32_t start = millis();

  while (millis() - start < timeout) {
    if (LteSerial.available()) {
      char c = LteSerial.read();
      s += c;
      if (c == '\n') break;
    } else {
      delay(2);
    }
  }
  return s;
}

// ------------------------------------------------------------
// Utility: Flush leftover UART data
// ------------------------------------------------------------
void flushLTE(uint32_t timeout = 300) {
  uint32_t start = millis();
  while (millis() - start < timeout) {
    while (LteSerial.available()) {
      LteSerial.read();
      start = millis();
    }
    delay(2);
  }
}

// ------------------------------------------------------------
// Send AT command and print response
// ------------------------------------------------------------
void sendAT(const char *cmd, uint32_t wait = 500) {
  flushLTE();
  Serial.print(">> ");
  Serial.println(cmd);
  LteSerial.println(cmd);

  uint32_t start = millis();
  while (millis() - start < wait) {
    if (LteSerial.available()) {
      Serial.write(LteSerial.read());
    }
  }
  Serial.println();
}

// ------------------------------------------------------------
// SMS Reader
// ------------------------------------------------------------
void readSMS(int index) {
  Serial.print("[ACTION] Reading SMS ");
  Serial.println(index);

  flushLTE();
  LteSerial.print("AT+CMGR=");
  LteSerial.println(index);

  String sender = "";
  String message = "";

  uint32_t start = millis();
  while (millis() - start < 2000) {
    if (!LteSerial.available()) {
      delay(5);
      continue;
    }

    String line = readLineFromLTE();
    String t = line;
    t.trim();

    if (t.length() == 0) continue;

    Serial.print("[SMS] ");
    Serial.println(t);

    // Header line
    if (t.startsWith("+CMGR")) {
      int q1 = t.indexOf('"');
      int q2 = t.indexOf('"', q1 + 1);
      int q3 = t.indexOf('"', q2 + 1);
      int q4 = t.indexOf('"', q3 + 1);

      if (q3 > 0 && q4 > 0) {
        sender = t.substring(q3 + 1, q4);
        Serial.print("[INFO] Sender: ");
        Serial.println(sender);
      }
    }
    // Body line
    else if (!t.startsWith("+") && t != "OK") {
      message += t + "\n";
    }
  }

  Serial.print("[INFO] Message: ");
  Serial.println(message);

  // ----------------------------------------------------------
  // Auto‑reply
  // ----------------------------------------------------------
  if (sender.length() > 0) {
    Serial.println("[ACTION] Sending auto‑reply…");

    flushLTE();
    LteSerial.print("AT+CMGS=\"");
    LteSerial.print(sender);
    LteSerial.println("\"");
    delay(200);

    LteSerial.print("Auto‑reply: Message received!");
    LteSerial.write(26); // Ctrl+Z
    delay(1500);
  }

  // ----------------------------------------------------------
  // Delete SMS
  // ----------------------------------------------------------
  Serial.println("[ACTION] Deleting SMS");
  flushLTE();
  LteSerial.print("AT+CMGD=");
  LteSerial.println(index);
  delay(300);
}

void clear_rec_msg(rec_msg_st *rec_msg)
{
    rec_msg->complete = false;
    memset(rec_msg->sender,0x00,sizeof(rec_msg->sender));
    memset(rec_msg->timestamp,0x00,sizeof(rec_msg->timestamp));
    memset(rec_msg->message,0x00,sizeof(rec_msg->message));
    rec_msg->date_time.year = 0;
    rec_msg->date_time.month = 0;
    rec_msg->date_time.day = 0;
    rec_msg->date_time.hour = 0;
    rec_msg->date_time.minute = 0;
    rec_msg->date_time.second = 0;

}

void print_rec_msg(rec_msg_st *rec_msg)
{
    if(rec_msg->complete) Serial.print("Complete: ");
    else Serial.print("Draft: ");
    Serial.printf("From: %s %s\n", 
        rec_msg->sender, 
        contact_list[rec_msg->contact_indx].name);
    Serial.printf(" %d-%d-%d %d:%d:%d\n",
        rec_msg->date_time.year,
        rec_msg->date_time.month,
        rec_msg->date_time.day,
        rec_msg->date_time.hour,
        rec_msg->date_time.minute,
        rec_msg->date_time.second);
    Serial.print("Message: ");
    Serial.println(rec_msg->message);

}

uint8_t get_contact_index(char *nbr)
{
    uint8_t contact = NAME_UNKNOWN;
    uint8_t cindx = NAME_UNKNOWN + 1;

    while((contact == NAME_UNKNOWN) && (cindx < NAME_NBR_OF))
    {
        Serial.printf("Compare -%s-%s-\n",nbr,contact_list[cindx].sender);
        if (strcmp(nbr, contact_list[cindx].sender) == 0) contact = cindx;
        else cindx++;
    }
    return contact;
}
void parse_msg(rec_msg_st *rec_msg, const char *msg)
{
    // +CMT: "+358401234567","","24/03/27,12:45:10+08"
    if (strncmp(msg, "+CMT", 4) == 0) {

        const char *p = msg;
        const char *q;
        char *end_ptr;
        char ch_nbr[8];
        uint16_t  u16;

        // char sender[32] = {0};
        // char timestamp[32] = {0};

        // Find first quoted field (sender)
        p = strchr(p, '"');
        if (!p) return;
        q = strchr(p + 1, '"');
        if (!q) return;
        size_t len = q - (p + 1);
        if (len < sizeof(rec_msg->sender))
            memcpy(rec_msg->sender, p + 1, len);

        Serial.print("[INFO] Sender: ");
        Serial.println(rec_msg->sender);

        // Timestamp: 26/04/22,11:08:12+12
        // Find timestamp (4th quoted field)
        p = strchr(q + 1, '"'); if (!p) return;
        p = strchr(p + 1, '"'); if (!p) return;
        p = strchr(p + 1, '"'); if (!p) return;
        q = strchr(p + 1, '"'); if (!q) return;

        len = q - (p + 1);
        if (len < sizeof(rec_msg->timestamp))
        {
            memcpy(rec_msg->timestamp, p+1, len);
            rec_msg->timestamp[len] = '\0';
            p = rec_msg->timestamp;
            q = strchr(p + 1, '/');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') rec_msg->date_time.year = u16;

            p = q+1;
            q = strchr(p + 1, '/');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') rec_msg->date_time.month = (uint8_t)u16;

            p = q+1;
            q = strchr(p + 1, ',');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') rec_msg->date_time.day = (uint8_t)u16;

            p = q+1;
            q = strchr(p + 1, ':');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') rec_msg->date_time.hour = (uint8_t)u16;
            p = q+1;
            q = strchr(p + 1, ':');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') rec_msg->date_time.minute = (uint8_t)u16;
            p = q+1;
            q = strchr(p + 1, '+');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') rec_msg->date_time.second = (uint8_t)u16;

        }

        Serial.print("[INFO] Timestamp: ");
        Serial.println(rec_msg->timestamp);

        // strncpy(rec_msg->sender, sender, sizeof(rec_msg->sender));
        // strncpy(rec_msg->timestamp, timestamp, sizeof(rec_msg->timestamp));
    }
    else if (msg[0] != '+') {
        // Body line
        strncpy(rec_msg->message, msg, sizeof(rec_msg->message));
        rec_msg->complete = true;
    }
}


void reply_msg(rec_msg_st *rec_msg)
{ 
    char reply[SMS_LEN] = {0};
    char at_cmd[40];

    uint16_t ch_avail = SMS_LEN;
    char *cp;
    uint16_t  len;

    strncpy(reply,"Terve ", ch_avail);
    ch_avail = SMS_LEN - strlen(reply);

    cp = contact_list[rec_msg->contact_indx].name;
    len = strlen(cp);
    if (ch_avail > len) {
        strncat(reply, cp, ch_avail);
        ch_avail = SMS_LEN - strlen(reply);
    }

    cp = "  Tupa Temp = 22.3C";
    len = strlen(cp);
    if (ch_avail > len) {
        strncat(reply, cp, ch_avail);
        ch_avail = SMS_LEN - strlen(reply);
    }

    sprintf(at_cmd, "AT+CMGS=\"%s\"", rec_msg->sender);
    sendAT(at_cmd);
    LteSerial.print(reply);
    LteSerial.write(26);
    Serial.printf("Reply: %s\n", reply);

}

// ------------------------------------------------------------
// Check for incoming SMS (CMTI)
// ------------------------------------------------------------
void checkIncomingSMS() {
  if (!LteSerial.available()) return;

  String line = readLineFromLTE();
  char chline[200];

  // String t = line;
  // t.trim();
  line.trim();

  if (line.length() == 0) return;
  line.toCharArray(chline,200);
  Serial.print("[LTE read] ");
  Serial.println(chline);

  parse_msg(&rec_msg,chline);

  // if (t.startsWith("+CMTI")) {
  //   int comma = line.indexOf(',');
  //   if (comma > 0) {
  //     int index = line.substring(comma + 1).toInt();
  //     delay(50); // allow modem to finish writing SMS
  //     readSMS(index);
  //   }
  // }
  // else if (t.startsWith("+CMT")) {
  //     Serial.println("Header Line");
  //     parse_msg(&rec_msg,&t);
  // }  
  // else if (!t.startsWith("+")) {
  //   Serial.println("Message");

  // }

}

// ------------------------------------------------------------
// Modem Boot Sequence
// ------------------------------------------------------------
void modemBoot() {
  Serial.println("[ACTION] Resetting modem…");
  digitalWrite(PIN_RESET, LOW);
  delay(500);
  digitalWrite(PIN_RESET, HIGH);
  delay(1500);

  Serial.println("[ACTION] Powering modem ON…");
  digitalWrite(PIN_PWRKEY, LOW);
  delay(800);
  digitalWrite(PIN_PWRKEY, HIGH);
  delay(3000);
}

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Pico 2 W + A7683E SMS Framework ===");

  Serial1.setTX(PIN_TX0);
  Serial1.setRX(PIN_RX0);
  LteSerial.begin(115200);

  pinMode(PIN_PWRKEY, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_PWRKEY, HIGH);
  digitalWrite(PIN_RESET, HIGH);

  modemBoot();
  clear_rec_msg(&rec_msg);

  sendAT("AT");
  sendAT("ATE0");
  sendAT("AT+CPIN=\"1234\"");
  sendAT("AT+CMGF=1");          // SMS text mode
  sendAT("AT+CNMI=2,2,0,0,0");  // Push SMS immediately

  Serial.println("[INFO] Setup complete. Waiting for SMS…");
}

// ------------------------------------------------------------
// Main loop
// ------------------------------------------------------------
void loop() {
    checkIncomingSMS();
    if(rec_msg.complete){
        rec_msg.contact_indx = (contact_indx_st)get_contact_index(rec_msg.sender);
        print_rec_msg(&rec_msg);
        reply_msg(&rec_msg);
        clear_rec_msg(&rec_msg);
    } 
}

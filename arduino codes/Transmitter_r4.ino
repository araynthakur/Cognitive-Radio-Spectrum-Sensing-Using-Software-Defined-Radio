#include <RH_ASK.h>
#include <SPI.h>

RH_ASK driver(1000, 99, 4, 99);

const int TX_LED_GREEN  = 8;
const int TX_LED_RED    = 9;
const int TX_LED_YELLOW = 10;

bool channelBusy = false;
unsigned long lastTransmitTime = 0;
const unsigned long TRANSMIT_EVERY = 2000;
int packetCount = 0;
  
// Channel shifting variables
int currentChannel = 1;
const int TOTAL_CHANNELS = 3;
String channelFreqs[] = {"433.4 MHz", "434.0 MHz", "434.6 MHz"};

void setup() {
  Serial.begin(9600);
  if (!driver.init()) {
    Serial.println("RF init failed!");
  } else {
    Serial.println("RF init OK");
  }
  pinMode(TX_LED_GREEN,  OUTPUT);
  pinMode(TX_LED_RED,    OUTPUT);
  pinMode(TX_LED_YELLOW, OUTPUT);

  digitalWrite(TX_LED_YELLOW, HIGH);
  digitalWrite(TX_LED_GREEN,  LOW);
  digitalWrite(TX_LED_RED,    LOW);

  Serial.println("=== COGNITIVE RADIO TX READY ===");
  Serial.println("Current Channel: " + String(currentChannel) +
                 " (" + channelFreqs[currentChannel-1] + ")");
}

void loop() {

  // Read BUSY or FREE from GNU Radio
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "BUSY") {
      channelBusy = true;

      digitalWrite(TX_LED_RED,    HIGH);
      digitalWrite(TX_LED_YELLOW, LOW);
      digitalWrite(TX_LED_GREEN,  LOW);

      // ── CHANNEL SHIFTING HAPPENS HERE ──
      int previousChannel = currentChannel;
      currentChannel = (currentChannel % TOTAL_CHANNELS) + 1;

      Serial.println(">>> PRIMARY USER DETECTED on Channel " +
                     String(previousChannel) +
                     " (" + channelFreqs[previousChannel-1] + ")");
      Serial.println(">>> BACKING OFF...");
      Serial.println(">>> SHIFTING to Channel " +
                     String(currentChannel) +
                     " (" + channelFreqs[currentChannel-1] + ")");
    }
    else if (command == "FREE") {
      channelBusy = false;

      digitalWrite(TX_LED_RED,    LOW);
      digitalWrite(TX_LED_YELLOW, HIGH);
      digitalWrite(TX_LED_GREEN,  LOW);

      Serial.println(">>> CHANNEL FREE — Resuming on Channel " +
                     String(currentChannel) +
                     " (" + channelFreqs[currentChannel-1] + ")");
    }
  }

  // Transmit only when FREE
  if (!channelBusy) {
    unsigned long now = millis();
    if (now - lastTransmitTime >= TRANSMIT_EVERY) {

      packetCount++;

      // Packet includes current channel number
      String msgStr = "CH" + String(currentChannel) +
                      " PKT:" + String(packetCount);
      const char* msg = msgStr.c_str();

      digitalWrite(TX_LED_GREEN,  HIGH);
      digitalWrite(TX_LED_YELLOW, LOW);

      driver.send((uint8_t*)msg, strlen(msg));
      driver.waitPacketSent();

      Serial.println("SENT on CH" + String(currentChannel) +
                     " (" + channelFreqs[currentChannel-1] +
                     "): PKT:" + String(packetCount));

      digitalWrite(TX_LED_GREEN,  LOW);
      digitalWrite(TX_LED_YELLOW, HIGH);

      lastTransmitTime = now;
    }
  }
}
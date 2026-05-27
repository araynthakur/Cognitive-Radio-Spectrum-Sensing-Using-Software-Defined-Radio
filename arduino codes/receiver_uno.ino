// =====================================================
// COGNITIVE RADIO — RECEIVER ARDUINO (Arduino R4)
// Basic 433 MHz OOK Receiver on Pin 7
// Pulse counting method
// =====================================================

const int RX_LED_BLUE   = 9;
const int RX_LED_RED    = 10;
const int RX_LED_GREEN  = 11;
const int RECEIVER_DATA = 7;

unsigned long pulseCount = 0;
unsigned long lastCheck  = 0;
const unsigned long CHECK_INTERVAL   = 200;
const int           SIGNAL_THRESHOLD = 10;

bool signalDetected = false;
int  lastDataState  = LOW;

void setup() {
  Serial.begin(9600);

  pinMode(RX_LED_BLUE,   OUTPUT);
  pinMode(RX_LED_RED,    OUTPUT);
  pinMode(RX_LED_GREEN,  OUTPUT);
  pinMode(RECEIVER_DATA, INPUT);

  digitalWrite(RX_LED_BLUE,  HIGH);
  digitalWrite(RX_LED_RED,   LOW);
  digitalWrite(RX_LED_GREEN, LOW);

  Serial.println("RX Ready.");
}

void loop() {

  // Count pulses from 433 MHz receiver DATA pin
  int currentState = digitalRead(RECEIVER_DATA);
  if (currentState != lastDataState) {
    pulseCount++;
    lastDataState = currentState;
  }

  // Every 200ms — decide if signal is present
  if (millis() - lastCheck >= CHECK_INTERVAL) {

    if (pulseCount >= SIGNAL_THRESHOLD) {
      if (!signalDetected) {
        signalDetected = true;
        Serial.println("SIGNAL DETECTED — TX transmitting!");
      }
      digitalWrite(RX_LED_GREEN, HIGH);

    } else {
      if (signalDetected) {
        signalDetected = false;
        Serial.println("Signal gone — channel quiet");
      }
      digitalWrite(RX_LED_GREEN, LOW);
    }

    pulseCount = 0;
    lastCheck  = millis();
  }

  // GNU Radio sends BUSY or FREE via serial
  if (Serial.available() > 0) {
    String status = Serial.readStringUntil('\n');
    status.trim();

    if (status == "BUSY") {
      digitalWrite(RX_LED_RED,  HIGH);
      digitalWrite(RX_LED_BLUE, LOW);
      Serial.println("BUSY — Red LED ON");
    }
    else if (status == "FREE") {
      digitalWrite(RX_LED_RED,  LOW);
      digitalWrite(RX_LED_BLUE, HIGH);
      Serial.println("FREE — Blue LED ON");
    }
  }
}
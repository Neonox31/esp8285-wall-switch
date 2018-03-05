#include <Homie.h>

#define FW_NAME "wall-switch-firmware"
#define FW_VERSION "1.0.0"

const int RELAY_PIN = 12;
const int LED_PIN= 13;
const int BUTTON_PIN = 0;
const int BUTTON_SHORT_PRESS = 300;
const int BUTTON_LONG_PRESS = 5000;

const int OFF = 0;
const int ON = 1;

int lightState = OFF;
unsigned long millisSinceChange = 0;
unsigned long millisSincePress = 0;
bool buttonStateHasJustChanged = false;
bool flaggedForReset = false;

HomieNode lightNode("light", "switch");

void on() {
  digitalWrite(RELAY_PIN, LOW);
  lightState = ON;
  if (Homie.isConnected()) {
    lightNode.setProperty("state").send("on");
  }
  Homie.getLogger() << "switch on light..." << endl;
}

void off() {
  digitalWrite(RELAY_PIN, HIGH);
  lightState = OFF;
  if (Homie.isConnected()) {
      lightNode.setProperty("state").send("off");
  }
  Homie.getLogger() << "switch off light..." << endl;

}

bool lightStateHandler(const HomieRange& range, const String& value) {
  if (value == "on") {
    on();
  } else if (value == "off") {
    off();
  } else {
    return false;
  }
  return true;
}

void handleButton() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    buttonStateHasJustChanged = false;
    millisSincePress = millis();
  }

  if (digitalRead(BUTTON_PIN) == LOW && (millis() - millisSinceChange) > BUTTON_SHORT_PRESS && !buttonStateHasJustChanged) {
    millisSinceChange = millis();
    lightState == ON ? off() : on();
    buttonStateHasJustChanged = true;
  } else if (digitalRead(BUTTON_PIN) == LOW && (millis() - millisSincePress) > BUTTON_LONG_PRESS && !flaggedForReset) {
    Homie.getLogger() << "reset by button long press..." << endl;
    flaggedForReset = true;
    Homie.reset();
  }
}

void onHomieEvent(const HomieEvent& event) {
  switch(event.type) {
    case HomieEventType::CONFIGURATION_MODE:
      digitalWrite(LED_PIN, LOW);
      break;
    case HomieEventType::MQTT_READY:
      digitalWrite(LED_PIN, HIGH);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  pinMode(RELAY_PIN, OUTPUT);

  off();

  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.disableResetTrigger();
  Homie.setLedPin(LED_PIN, LOW);
  Homie.onEvent(onHomieEvent);
  lightNode.advertise("state").settable(lightStateHandler);
  Homie.setup();
}

void loop() {
  handleButton();
  Homie.loop();
  yield();
}

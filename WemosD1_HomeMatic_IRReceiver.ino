#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define TasterPin     D7 //Taster gegen GND, um den Konfigurationsmodus zu aktivieren

#define SerialDEBUG  true

char SendIntervalMilliSeconds[8]  = "1000";
char ccuip[16];
String Variable;

//WifiManager - don't touch
byte ConfigPortalTimeout = 180;
bool shouldSaveConfig        = false;
String configJsonFile        = "config.json";
bool wifiManagerDebugOutput = true;
char ip[16]      = "0.0.0.0";
char netmask[16] = "0.0.0.0";
char gw[16]      = "0.0.0.0";
boolean startWifiManager = false;

unsigned long lastSendMillis = 0;
String lastCode = "";

uint16_t RECV_PIN = D5;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  pinMode(D8, OUTPUT);
  digitalWrite(D8, LOW);
  Serial.begin(115200);

  pinMode(TasterPin,    INPUT_PULLUP);

  if (digitalRead(TasterPin) == LOW) {
    startWifiManager = true;
    bool state = LOW;
    for (int i = 0; i < 7; i++) {
      state = !state;
      digitalWrite(LED_BUILTIN, state);
      delay(100);
    }
  }

  loadSystemConfig();

  if (doWifiConnect()) {
    printSerial("WLAN erfolgreich verbunden!");
    irrecv.enableIRIn();
  } else ESP.restart();
}

void loop() {
  if (lastSendMillis > millis())
    lastSendMillis = 0;

  if (irrecv.decode(&results)) {
    if (results.bits > 0) {
      String Code = recStr(results.value);
      Serial.println(Code + " (" + String(results.bits) + " bits)");
      if (lastSendMillis == 0 || millis() - lastSendMillis > atoi(SendIntervalMilliSeconds) || Code != lastCode) {
        lastSendMillis = millis();
        lastCode = Code;
        Serial.println("Sende an CCU.");
        setStateCCU(Code);
      }
    }
    irrecv.resume();
  }
}

String recStr(uint64_t input) {
  char buf[8 * sizeof(input) + 1];  // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];

  *str = '\0';

  do {
    char c = input % 16;
    input /= 16;

    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while (input);
  return str;
}

void printSerial(String text) {
  if (SerialDEBUG) {
    Serial.println(text);
  }
}

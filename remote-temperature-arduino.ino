#include <SPI.h>
#include <Ethernet.h>
#include <WebSocketClient.h> // https://github.com/flamecze/Arduino-Websocket
#include <Streaming.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "config.h"

EthernetClient client;
WebSocketClient webSocketClient;

byte mac[] = { 0xDE, 0xAB, 0xBE, 0xEF, 0xFE, 0xEF };
const char hostname[] = HOSTNAME;
const int port = PORT;

#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float tempC = 0;
char tempStr[6];

bool connected = false;

#define delayMillis 1000UL

unsigned long thisMillis = 0;
unsigned long lastMillis = 0;

void setup() {
  // disable SD SPI
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  Serial.begin(9600);

  Serial.print("HOSTNAME=");
  Serial.println(HOSTNAME);

  Serial.print("PORT=");
  Serial.println(PORT);
  
  Serial.println(F("Starting..."));
  Ethernet.begin(mac); // initialize ethernet
  Serial.println(Ethernet.localIP());
  delay(300);

  sensors.begin();
  connect();
}

void loop() {
  thisMillis = millis();

  if (thisMillis - lastMillis > delayMillis) {
    lastMillis = thisMillis;

    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);

    dtostrf(tempC, 4, 2, tempStr);

    if (client.connected()) {
      webSocketClient.sendData(tempStr);
    } else {
      Serial.println("Client disconnected.");
      connected = false;
      connect();
    }
  }
}

void connect() {
  while(!connected) {
    Serial.println("Connecting");
    // Connect to the websocket server
    if (client.connect(HOSTNAME, PORT)) {
      Serial.println("Connected");
      connected = true;
    } else {
      Serial.println("Connection failed.");
    }
    delay(1000);
  }
  
  char fullServerName[40];
  
  sprintf(fullServerName,"%s:%i", HOSTNAME, PORT);

  // Handshake with the server
  webSocketClient.path = "/";
  webSocketClient.host = fullServerName;

  while(!webSocketClient.handshake(client)) {
    Serial.println("Handshake failed.");
    delay(1000);
  }
  Serial.println("Handshake successful");
}


// Adafruit.io docs: https://learn.adafruit.com/mqtt-adafruit-io-and-you/intro-to-adafruit-mqtt

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <EEPROM.h>

#define WLAN_SSID "YOUR SSID" //insert SSID
#define WLAN_PASS "YOUR PASS"  //insert password

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "YOUR ADAFRUIT USER"
#define AIO_KEY         "YOUR ADAFRUIT KEY"

#define MQTT_CONN_KEEPALIVE 180

#define Relay1            D1
#define Relay2            D2

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe light = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/bedroom-light"); // set adafruit FeedName
Adafruit_MQTT_Subscribe fan = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/bedroom-fan"); // set adafruit FeedName

void setup() {

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  
  EEPROM.begin(512);
  lastState(); //recover last state of relays

  Serial.begin(115200);
  delay(10);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&light);
  mqtt.subscribe(&fan);
}

void loop() {

  MQTT_connect();
  
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &light) {
      Serial.print(F("Got: "));
      Serial.println((char *)light.lastread);
      
      if (EEPROM.read(0) == 1){
        digitalWrite(Relay1, HIGH);
        EEPROM.write(0, 0);
      }
      else {
        digitalWrite(Relay1, LOW);
        EEPROM.write(0, 1);
      }      
    }
    if (subscription == &fan) {
      Serial.print(F("Got: "));
      Serial.println((char *)fan.lastread);
      
      if (EEPROM.read(1) == 1){
        digitalWrite(Relay2, HIGH);
        EEPROM.write(1, 0);
      }
      else {
        digitalWrite(Relay2, LOW);
        EEPROM.write(1, 1);
      }      
    }
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  
}

//Functions

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void lastState() {
  if (EEPROM.read(0) == 0){
    EEPROM.write(0, 1);
  }
  else {
    EEPROM.write(0, 0);
  }
  if (EEPROM.read(1) == 0){
    EEPROM.write(1, 1);
  }
  else {
    EEPROM.write(1, 0);
  }
  
}

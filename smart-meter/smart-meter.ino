/*
 * esp8266 based Smart Meter
 *
 * Based on https://learn.openenergymonitor.org/electricity-monitoring/pulse-counting/interrupt-based-pulse-counter
 *
 * Designed for integration with HomeAssistant
 *
 * v3
 * 
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

// Power measurement
volatile int jouleCount = 0;

// Used to track MQTT message publication
unsigned long currentTime, lastMessage, timeElapsed;

// power
double power;

void setup()
{
  Serial.begin(115200);
  while (!Serial) {}

  Serial.setDebugOutput(true);

  Serial.print("Welcome ");
  Serial.println(device);
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  
  // switch on WiFi
  WiFi.hostname(wifi_hostname);
  WiFi.begin(wifi_ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // configure MQTT server
  client.setServer(mqtt_server, 1883);

  // connect to MQTT server
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    } //end if client.connect
  } // end if client.connected
  
  // KWH interrupt attached to IRQ 1 = D1 = GPIO5
  attachInterrupt(digitalPinToInterrupt(optical), onPulse, FALLING);

  lastMessage = millis();
}

void loop()
{
  currentTime = millis();
  timeElapsed = currentTime - lastMessage;

  if (timeElapsed >= sample_ms) { // every 5 seconds   

    // reset timer
    lastMessage = currentTime;

    // detach interrupt while working with the shared
    // jouleCount variable
    detachInterrupt(digitalPinToInterrupt(optical));
    
    // calculate power
    power = float(jouleCount) / (float(timeElapsed)/1000.0);
    
    // reset counter
    jouleCount = 0;

    // re-attach interrupt
    attachInterrupt(digitalPinToInterrupt(optical), onPulse, FALLING);

    // connect to MQTT server
    if (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      if (client.connect("ESP8266Client")) {
        Serial.println("connected");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      } //end if client.connect
    } // end if client.connected
    Serial.println("Publishing to MQTT...");    
     // publish power (kWh)
    client.publish(power_topic, String(power/1000.0).c_str(), true);
  } else {
    delay(100);
  } // end if it's time to publish
}

// The interrupt routine
// Needs ICACHE_RAM_ATTR for esp8266 architecture
ICACHE_RAM_ATTR void onPulse()
{
  //pulseCounter
  jouleCount += jp_impulse;
}

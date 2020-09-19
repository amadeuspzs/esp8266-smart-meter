/*
 * esp8266 based Smart Meter
 *
 * Based on https://learn.openenergymonitor.org/electricity-monitoring/pulse-counting/interrupt-based-pulse-counter
 *
 * Designed for integration with HomeAssistant
 *
 */

#define debug true

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

// Used to measure power.
unsigned long pulseTime, lastTime;

// Used to track MQTT message publication
unsigned long currentTime, lastMessage, timeElapsed;

// power
double power, T;
    
void setup()
{
  if (debug) {
    Serial.begin(115200);
    while (!Serial) {}
  
    Serial.setDebugOutput(true);
  
    Serial.print("Welcome ");
    Serial.println(device);
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);
  }
    
  // switch on WiFi
  WiFi.hostname(wifi_hostname);
  WiFi.begin(wifi_ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    if (debug) Serial.print(".");
  }

  if (debug) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }  
  // configure MQTT server
  client.setServer(mqtt_server, 1883);

  // KWH interrupt attached to IRQ 1 = D1 = GPIO5
  attachInterrupt(digitalPinToInterrupt(optical), onPulse, FALLING);

  // avoid first 0 submission
  lastMessage = millis();
}

void loop()
{
  currentTime = millis();
  timeElapsed = currentTime - lastMessage;

  if (timeElapsed >= 5000) { // every 5 seconds
    lastMessage = millis();
  
    // Calculate optical power (W)
    T = (pulseTime - lastTime)/1000000.0; // in s
    power = (3600 * ppwh) / T;

    if (debug) {
      Serial.print(power,3);
      Serial.println(" W");
    }
        
    // connect to MQTT server
    if (!client.connected()) {
      if (debug) Serial.print("Attempting MQTT connection...");
      if (client.connect("ESP8266Client")) {
        if (debug) Serial.println("connected");
      } else {
        if (debug) {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
        }
        // Wait 5 seconds before retrying
        delay(5000);
      } //end if client.connect
    } // end if client.connected
    if (debug) Serial.println("Publishing to MQTT...");    
    // publish new reading
    client.publish(power_topic, String(power).c_str(), true);
  } else {
    delay(100);
  } // end if it's time to publish 

}

// The interrupt routine
// Needs ICACHE_RAM_ATTR for esp8266 architecture
ICACHE_RAM_ATTR void onPulse()
{
  //used to measure time between pulses.
  lastTime = pulseTime;
  pulseTime = micros();
}

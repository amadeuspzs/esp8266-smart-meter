/*
 * esp8266 based Smart Meter
 *
 * Based on https://learn.openenergymonitor.org/electricity-monitoring/pulse-counting/interrupt-based-pulse-counter
 *
 * Designed for integration with HomeAssistant
 *
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

// Number of pulses, used to measure energy.
long pulseCount = 0;

// Used to measure power.
unsigned long pulseTime, lastTime;

// Used to track MQTT message publication
unsigned long currentTime, lastMessage, timeElapsed;

// power and energy
volatile double power, elapsedkWh;
double T;

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

  // avoid first 0 submission
  lastMessage = millis();
}

void loop()
{
  currentTime = millis();
  timeElapsed = currentTime - lastMessage;

  if (timeElapsed >= 5000) { // every 5 seconds
    lastMessage = millis();
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
     // publish new reading
    client.publish(power_topic, String(power/1000.0).c_str(), true);
    client.publish(energy_topic, String(elapsedkWh).c_str(), true);
  } else {
    delay(100);
    // Serial.println("Not time to publish yet, waiting");
  } // end if it's time to publish
}

// The interrupt routine
// Needs ICACHE_RAM_ATTR for esp8266 architecture
ICACHE_RAM_ATTR void onPulse()
{
  //used to measure time between pulses.
  lastTime = pulseTime;
  pulseTime = micros();
  T = (pulseTime - lastTime)/1000000.0; // in s
  
  //pulseCounter
  pulseCount++;
  
  //Calculate power (W)
  power = (3600 * ppwh) / T;
  
  //Find kwh elapsed (kWh)
  elapsedkWh = (1.0*pulseCount/(ppwh*1000)); //multiply by 1000 to convert pulses per wh to kwh
}

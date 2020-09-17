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

//Number of pulses, used to measure energy.
long pulseCount = 0;

//Used to measure power.
unsigned long pulseTime,lastTime;

//power and energy
double power, elapsedkWh, T;

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

    // KWH interrupt attached to IRQ 1 = D1 = GPIO5
    attachInterrupt(digitalPinToInterrupt(optical), onPulse, FALLING);
}

void loop()
{

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

    //Print the values.
    Serial.print(power/1000.0,4);
    Serial.print(" kW ");
    Serial.print(elapsedkWh,3);
    Serial.println(" kWh");

    // connect to MQTT server
    if (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      if (client.connect("ESP8266Client")) {
        Serial.println("connected");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 1 second");
        // Wait 1 second before retrying
        delay(1000);
      } //end if client.connect
    } // end if client.connected

    Serial.println("Publishing to MQTT...");
  
    // publish new reading
    client.publish(power_topic, String(power/1000.0).c_str(), true);
    client.publish(energy_topic, String(elapsedkWh).c_str(), true);
}

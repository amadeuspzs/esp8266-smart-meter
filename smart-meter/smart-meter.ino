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

    // calculate CT current

    // 50Hz = 0.02s per peak = 20ms per cycle
    // In total capture 5 cycles not to slow down the whole system too much (100ms)
    // emonlib.h use 1480 samples with no delay
    // Delay = 100 ms / sample size
    // For 1000 sample size, Delay = 100 uS
    // For 2000 sample size, Delay = 50 uS
    // On measurement the analogRead function takes a chunk of time so will try sampling with no delay
    // Gives ~80-200 ms sample window with 2000 samples which is enough!
    
    int samples = 2000;
    int min = 1023;
    int max = 0;  
    for (int i = 0; i<samples; i++) {
      // offset reading
      int sensor = analogRead(ct) - (1024/2);
      min = (sensor < min) ? sensor : min;
      max = (sensor > max) ? sensor : max;    
      //delayMicroseconds(50);
    }
  
    // max reading is 1023 @ 3.3V
    double voltage = 3.3 * ( (float(max-min) / 1024.0) );
    
    // 100A:50mA with 22 ohm burden resistor
    double current = (100.0/3.11) * voltage;
    double power_ct = 240.0 * current; // assume 240V - RMS?
  
    Serial.print("Peak-to-peak: ");
    Serial.println(max - min);
    
    Serial.print(voltage*1000.0);
    Serial.println(" mV"); 
  
    Serial.print(current,3);
    Serial.println(" A");
  
    Serial.print(power_ct,3);
    Serial.println(" W"); 
    
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

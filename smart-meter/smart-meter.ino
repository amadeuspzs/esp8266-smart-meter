/*
 * esp8266 based Smart Meter
 *
 * Based on https://learn.openenergymonitor.org/electricity-monitoring/pulse-counting/interrupt-based-pulse-counter
 *
 * Designed for integration with HomeAssistant
 *
 */

// where is the sensor connected?
int optical = D1;

//Number of pulses, used to measure energy.
long pulseCount = 0;

//Used to measure power.
unsigned long pulseTime,lastTime;

//power and energy
double power, elapsedkWh, T;

//Number of pulses per Wh - found or set on the meter.
double ppwh = 1/3.2; //3200 pulses/kWh = 1/3.2 pulse per Wh

void setup()
{
    Serial.begin(115200);
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
}

# esp8266-smart-meter

Monitoring electricity usage of a previously "smart" meter via esp8266, optical measurement and current measurement.

Hardware/software based on https://openenergymonitor.org/

This version uses the [optical readout of a smart meter](https://learn.openenergymonitor.org/electricity-monitoring/pulse-counting/introduction-to-pulse-counting); the [current transformer approach](https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/introduction) was too low fidelity and inaccurate by comparison.

## Requirements

1. esp8266 module (e.g. NodeMCU)
2. [Optical monitor](https://shop.openenergymonitor.com/optical-utility-meter-led-pulse-sensor/)

## Flow

![Flowchart](esp8266-smart-meter.png)

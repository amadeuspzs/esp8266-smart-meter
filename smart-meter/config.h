#define device "device"

// WiFi credentials.
#define wifi_hostname device
#define wifi_ssid "SSID"
#define wifi_password "PASSWORD"

// Sensor configuration
#define optical D1
#define ppwh 1/3.2 //3200 pulses/kWh = 1/3.2 pulse per Wh
#define maxpower 24000 // max power (W) you should see, for low pass filtering

// Reporting
#define mqtt_server "0.0.0.0"
#define power_topic device "/power"

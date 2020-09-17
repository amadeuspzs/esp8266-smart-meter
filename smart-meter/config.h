#define device "smart-meter"

// WiFi credentials.
#define wifi_hostname device
#define wifi_ssid "SSID"
#define wifi_password "PASSPHRASE"
IPAddress ip(0,0,0,0);
IPAddress gateway(0,0,0,0);
IPAddress subnet(255,255,255,0);

// Sensor configuration
#define optical D1
#define ppwh 1/3.2 //3200 pulses/kWh = 1/3.2 pulse per Wh

// Reporting
#define mqtt_server "0.0.0.0"
#define power_topic device "/power"
#define energy_topic device "/energy"
